#pragma once
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <unitree_api/msg/request.hpp>
#include "go2_controller/go2_sport_client.hpp"

namespace go2_controller
{

class Go2CmdVelBridge
{
public:
    explicit Go2CmdVelBridge(rclcpp::Node * node);

private:
    void CmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void JoyCallback(const sensor_msgs::msg::Joy::SharedPtr msg);

    rclcpp::Node * node_;
    Go2SportClient sport_client_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
    unitree_api::msg::Request req_;
};

}
