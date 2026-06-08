#pragma once
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

namespace go2_controller
{

class Go2GstreamerNode : public rclcpp::Node
{
public:
    Go2GstreamerNode();
    ~Go2GstreamerNode();

private:
    void DeclareParameters();
    std::string BuildPcPipeline();
    std::string BuildJetsonPipeline();
    void ReaderLoop();
    void TimerCallback();

    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;

    std::mutex frame_mutex_;
    cv::Mat latest_frame_;
    std::atomic<bool> running_{false};
    std::thread reader_thread_;

    std::string platform_;
    std::string frame_id_;
    int jpeg_quality_;
    int output_width_;
    int output_height_;
    int output_fps_;
    int64_t min_publish_period_ns_;
    int64_t last_publish_ns_;
};

}
