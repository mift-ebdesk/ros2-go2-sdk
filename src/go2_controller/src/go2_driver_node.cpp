#include "go2_controller/go2_driver_node.hpp"

namespace go2_controller
{

Go2DriverNode::Go2DriverNode()
: Node("go2_driver_node")
{
    RCLCPP_INFO(this->get_logger(), "Initializing Go2 Driver Node...");
    DeclareParameters();
    SetupSubscribers();

    odometry_ = std::make_unique<Go2Odometry>(this);
    joint_state_ = std::make_unique<Go2JointState>(this);
    cmd_vel_bridge_ = std::make_unique<Go2CmdVelBridge>(this);

    RCLCPP_INFO(this->get_logger(), "Go2 Driver Node initialized");
}

void Go2DriverNode::DeclareParameters()
{
}

void Go2DriverNode::SetupSubscribers()
{
    sport_sub_ = this->create_subscription<unitree_go::msg::SportModeState>(
        "/lf/sportmodestate", 10,
        [this](const unitree_go::msg::SportModeState::SharedPtr msg) {
            SportModeCallback(msg);
        });

    low_state_sub_ = this->create_subscription<unitree_go::msg::LowState>(
        "/lf/lowstate", 10,
        [this](const unitree_go::msg::LowState::SharedPtr msg) {
            LowStateCallback(msg);
        });
}

void Go2DriverNode::SportModeCallback(
    const unitree_go::msg::SportModeState::SharedPtr msg)
{
    odometry_->Update(msg);
}

void Go2DriverNode::LowStateCallback(
    const unitree_go::msg::LowState::SharedPtr msg)
{
    joint_state_->Update(msg);
}

}
