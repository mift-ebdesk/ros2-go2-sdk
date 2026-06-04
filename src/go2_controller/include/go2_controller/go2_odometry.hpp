#pragma once
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <unitree_go/msg/sport_mode_state.hpp>

namespace go2_controller
{

class Go2Odometry
{
public:
    explicit Go2Odometry(rclcpp::Node * node);
    void Update(const unitree_go::msg::SportModeState::SharedPtr msg);

private:
    rclcpp::Node * node_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    std::string odom_frame_;
    std::string base_frame_;
};

}
