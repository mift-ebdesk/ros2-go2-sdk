#include "go2_controller/go2_gstreamer.hpp"
#include <chrono>

namespace go2_controller
{

Go2GstreamerNode::Go2GstreamerNode()
: Node("go2_gstreamer"),
  last_publish_ns_(0)
{
    DeclareParameters();

    platform_ = this->get_parameter("platform").as_string();
    std::string image_topic = this->get_parameter("image_topic").as_string();
    frame_id_ = this->get_parameter("frame_id").as_string();
    jpeg_quality_ = this->get_parameter("jpeg_quality").as_int();
    output_width_ = this->get_parameter("output_width").as_int();
    output_height_ = this->get_parameter("output_height").as_int();
    output_fps_ = this->get_parameter("output_fps").as_int();
    min_publish_period_ns_ = output_fps_ > 0 ? static_cast<int64_t>(1e9 / output_fps_) : 0;

    double timer_period = this->get_parameter("timer_period").as_double();

    pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(image_topic, platform_ == "jetson" ? 3 : 10);

    std::string pipeline_param = this->get_parameter("pipeline").as_string();
    std::string pipeline = pipeline_param.empty()
        ? (platform_ == "jetson" ? BuildJetsonPipeline() : BuildPcPipeline())
        : pipeline_param;

    RCLCPP_INFO(this->get_logger(), "GStreamer %s pipeline: %s", platform_.c_str(), pipeline.c_str());

    cap_.open(pipeline, cv::CAP_GSTREAMER);
    if (!cap_.isOpened()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open GStreamer pipeline");
        throw std::runtime_error("GStreamer pipeline could not be opened");
    }

    if (platform_ == "jetson") {
        running_ = true;
        reader_thread_ = std::thread(&Go2GstreamerNode::ReaderLoop, this);
    }

    timer_ = this->create_wall_timer(
        std::chrono::duration<double>(timer_period),
        std::bind(&Go2GstreamerNode::TimerCallback, this));

    RCLCPP_INFO(this->get_logger(), "GStreamer %s camera node started, publishing to %s", platform_.c_str(), image_topic.c_str());
}

Go2GstreamerNode::~Go2GstreamerNode()
{
    running_ = false;
    if (reader_thread_.joinable()) {
        reader_thread_.join();
    }
    if (cap_.isOpened()) {
        cap_.release();
    }
}

void Go2GstreamerNode::DeclareParameters()
{
    this->declare_parameter("platform", std::string("jetson"));
    this->declare_parameter("image_topic", std::string("/frontcamera/compressed"));
    this->declare_parameter("frame_id", std::string("go2_front_camera"));
    this->declare_parameter("multicast_address", std::string("230.1.1.1"));
    this->declare_parameter("port", 1720);
    this->declare_parameter("multicast_iface", std::string("enP8p1s0"));
    this->declare_parameter("buffer_size", 524288);
    this->declare_parameter("latency_ms", 10);
    this->declare_parameter("timer_period", 0.015);
    this->declare_parameter("jpeg_quality", 60);
    this->declare_parameter("output_width", 480);
    this->declare_parameter("output_height", 270);
    this->declare_parameter("output_fps", 20);
    this->declare_parameter("pipeline", std::string(""));
}

std::string Go2GstreamerNode::BuildPcPipeline()
{
    std::string addr = this->get_parameter("multicast_address").as_string();
    int port = this->get_parameter("port").as_int();
    std::string iface = this->get_parameter("multicast_iface").as_string();
    int buf = this->get_parameter("buffer_size").as_int();
    int latency = this->get_parameter("latency_ms").as_int();

    return "udpsrc address=" + addr + " port=" + std::to_string(port) +
        " multicast-iface=" + iface + " buffer-size=" + std::to_string(buf) +
        " ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, payload=96"
        " ! rtpjitterbuffer latency=" + std::to_string(latency) + " drop-on-latency=true"
        " ! rtph264depay"
        " ! h264parse"
        " ! avdec_h264"
        " ! videoconvert"
        " ! video/x-raw,format=BGR"
        " ! queue max-size-buffers=1 leaky=downstream"
        " ! appsink drop=true max-buffers=1 sync=false";
}

std::string Go2GstreamerNode::BuildJetsonPipeline()
{
    std::string addr = this->get_parameter("multicast_address").as_string();
    int port = this->get_parameter("port").as_int();
    std::string iface = this->get_parameter("multicast_iface").as_string();
    int buf = this->get_parameter("buffer_size").as_int();
    int latency = this->get_parameter("latency_ms").as_int();

    return "udpsrc address=" + addr + " port=" + std::to_string(port) +
        " multicast-iface=" + iface + " buffer-size=" + std::to_string(buf) +
        " ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, payload=96"
        " ! rtpjitterbuffer latency=" + std::to_string(latency) + " drop-on-latency=true"
        " ! rtph264depay"
        " ! h264parse config-interval=-1"
        " ! nvv4l2decoder enable-max-performance=1"
        " ! nvvidconv"
        " ! video/x-raw, format=BGRx"
        " ! videoconvert"
        " ! video/x-raw, format=BGR"
        " ! appsink drop=true max-buffers=1 sync=false emit-signals=false";
}

void Go2GstreamerNode::ReaderLoop()
{
    while (running_) {
        cv::Mat frame;
        if (cap_.read(frame) && !frame.empty()) {
            std::lock_guard<std::mutex> lock(frame_mutex_);
            latest_frame_ = frame;
        }
    }
}

void Go2GstreamerNode::TimerCallback()
{
    cv::Mat frame;

    if (platform_ == "jetson") {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        frame = latest_frame_;
        latest_frame_ = cv::Mat();
    } else {
        if (!cap_.read(frame) || frame.empty()) {
            return;
        }
    }

    if (frame.empty()) {
        return;
    }

    auto now_ns = this->now().nanoseconds();
    if (min_publish_period_ns_ > 0 && last_publish_ns_ > 0) {
        if (now_ns - last_publish_ns_ < min_publish_period_ns_) {
            return;
        }
    }

    if (output_width_ > 0 && output_height_ > 0) {
        int interp = platform_ == "jetson" ? cv::INTER_LINEAR : cv::INTER_AREA;
        cv::resize(frame, frame, cv::Size(output_width_, output_height_), 0, 0, interp);
    }

    std::vector<uint8_t> jpeg_buf;
    std::vector<int> params{cv::IMWRITE_JPEG_QUALITY, jpeg_quality_};
    if (!cv::imencode(".jpg", frame, jpeg_buf, params)) {
        RCLCPP_WARN(this->get_logger(), "Failed to encode frame to JPEG");
        return;
    }

    auto msg = sensor_msgs::msg::CompressedImage();
    msg.header.stamp = this->now();
    msg.header.frame_id = frame_id_;
    msg.format = "jpeg";
    msg.data = jpeg_buf;

    pub_->publish(msg);
    last_publish_ns_ = now_ns;
}

}

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<go2_controller::Go2GstreamerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
