#pragma once
#include <rclcpp/rclcpp.hpp>
#include <unitree_go/msg/sport_mode_state.hpp>
#include <unitree_go/msg/low_state.hpp>
#include "go2_controller/go2_odometry.hpp"
#include "go2_controller/go2_joint_state.hpp"
#include "go2_controller/go2_cmd_vel_bridge.hpp"

namespace go2_controller
{

class Go2DriverNode : public rclcpp::Node
{
public:
    Go2DriverNode();

private:
    void DeclareParameters();
    void SetupSubscribers();
    void SportModeCallback(const unitree_go::msg::SportModeState::SharedPtr msg);
    void LowStateCallback(const unitree_go::msg::LowState::SharedPtr msg);

    std::unique_ptr<Go2Odometry> odometry_;
    std::unique_ptr<Go2JointState> joint_state_;
    std::unique_ptr<Go2CmdVelBridge> cmd_vel_bridge_;

    rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr sport_sub_;
    rclcpp::Subscription<unitree_go::msg::LowState>::SharedPtr low_state_sub_;
};

}
