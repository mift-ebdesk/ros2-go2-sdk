#pragma once
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>

namespace go2_controller
{

class Go2TfBroadcaster
{
public:
    explicit Go2TfBroadcaster(rclcpp::Node * node);
    void OdomCallback(const nav_msgs::msg::Odometry::SharedPtr msg);

private:
    rclcpp::Node * node_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
};

}
