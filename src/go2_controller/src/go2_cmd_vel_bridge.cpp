#include "go2_controller/go2_cmd_vel_bridge.hpp"

namespace go2_controller
{

Go2CmdVelBridge::Go2CmdVelBridge(rclcpp::Node * node)
: node_(node),
  sport_client_(node)
{
    cmd_vel_sub_ = node_->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel_out", 10,
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
            CmdVelCallback(msg);
        });

    joy_sub_ = node_->create_subscription<sensor_msgs::msg::Joy>(
        "/joy", 10,
        [this](const sensor_msgs::msg::Joy::SharedPtr msg) {
            JoyCallback(msg);
        });
}

void Go2CmdVelBridge::CmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    sport_client_.Move(req_, msg->linear.x, msg->linear.y, msg->angular.z);
}

void Go2CmdVelBridge::JoyCallback(const sensor_msgs::msg::Joy::SharedPtr msg)
{
    if (msg->buttons.size() > 0 && msg->buttons[0] == 1) {
        sport_client_.StandUp(req_);
    }
    if (msg->buttons.size() > 1 && msg->buttons[1] == 1) {
        sport_client_.StandDown(req_);
    }
    if (msg->buttons.size() > 2 && msg->buttons[2] == 1) {
        sport_client_.StopMove(req_);
    }
}

}
