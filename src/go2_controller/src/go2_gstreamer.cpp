#include "go2_controller/go2_gstreamer.hpp"
#include <chrono>

namespace go2_controller
{

Go2GstreamerNode::Go2GstreamerNode()
: Node("go2_gstreamer"),
  last_publish_ns_(0)
{
    DeclareParameters();

    std::string image_topic = this->get_parameter("image_topic").as_string();
    frame_id_ = this->get_parameter("frame_id").as_string();
    jpeg_quality_ = this->get_parameter("jpeg_quality").as_int();
    output_width_ = this->get_parameter("output_width").as_int();
    output_height_ = this->get_parameter("output_height").as_int();
    output_fps_ = this->get_parameter("output_fps").as_int();
    min_publish_period_ns_ = output_fps_ > 0 ? static_cast<int64_t>(1e9 / output_fps_) : 0;

    double timer_period = this->get_parameter("timer_period").as_double();

    pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(image_topic, 10);

    std::string pipeline_param = this->get_parameter("pipeline").as_string();
    std::string pipeline = pipeline_param.empty() ? BuildPipeline() : pipeline_param;

    RCLCPP_INFO(this->get_logger(), "GStreamer pipeline: %s", pipeline.c_str());

    cap_.open(pipeline, cv::CAP_GSTREAMER);
    if (!cap_.isOpened()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open GStreamer pipeline");
        throw std::runtime_error("GStreamer pipeline could not be opened");
    }

    timer_ = this->create_wall_timer(
        std::chrono::duration<double>(timer_period),
        std::bind(&Go2GstreamerNode::TimerCallback, this));

    RCLCPP_INFO(this->get_logger(), "GStreamer camera node started, publishing to %s", image_topic.c_str());
}

void Go2GstreamerNode::DeclareParameters()
{
    this->declare_parameter("image_topic", std::string("/frontcamera/compressed"));
    this->declare_parameter("frame_id", std::string("go2_front_camera"));
    this->declare_parameter("multicast_address", std::string("230.1.1.1"));
    this->declare_parameter("port", 1720);
    this->declare_parameter("multicast_iface", std::string("enp2s0"));
    this->declare_parameter("buffer_size", 524288);
    this->declare_parameter("latency_ms", 40);
    this->declare_parameter("timer_period", 0.05);
    this->declare_parameter("jpeg_quality", 90);
    this->declare_parameter("output_width", 640);
    this->declare_parameter("output_height", 360);
    this->declare_parameter("output_fps", 20);
    this->declare_parameter("pipeline", std::string(""));
}

std::string Go2GstreamerNode::BuildPipeline()
{
    std::string multicast_address = this->get_parameter("multicast_address").as_string();
    int port = this->get_parameter("port").as_int();
    std::string multicast_iface = this->get_parameter("multicast_iface").as_string();
    int buffer_size = this->get_parameter("buffer_size").as_int();
    int latency_ms = this->get_parameter("latency_ms").as_int();

    return "udpsrc address=" + multicast_address + " port=" + std::to_string(port) +
        " multicast-iface=" + multicast_iface + " buffer-size=" + std::to_string(buffer_size) +
        " ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, payload=96"
        " ! rtpjitterbuffer latency=" + std::to_string(latency_ms) + " drop-on-latency=true"
        " ! rtph264depay"
        " ! h264parse"
        " ! avdec_h264"
        " ! videoconvert"
        " ! video/x-raw,format=BGR"
        " ! queue max-size-buffers=1 leaky=downstream"
        " ! appsink drop=true max-buffers=1 sync=false";
}

void Go2GstreamerNode::TimerCallback()
{
    cv::Mat frame;
    if (!cap_.read(frame) || frame.empty()) {
        return;
    }

    auto now_ns = this->now().nanoseconds();
    if (min_publish_period_ns_ > 0 && last_publish_ns_ > 0) {
        if (now_ns - last_publish_ns_ < min_publish_period_ns_) {
            return;
        }
    }

    if (output_width_ > 0 && output_height_ > 0) {
        cv::resize(frame, frame, cv::Size(output_width_, output_height_), 0, 0, cv::INTER_AREA);
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
