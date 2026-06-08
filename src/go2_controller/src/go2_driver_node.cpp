#include "go2_controller/go2_driver_node.hpp"

namespace go2_controller
{

Go2DriverNode::Go2DriverNode()
: Node("go2_driver_node")
{
    RCLCPP_INFO(this->get_logger(), "Initializing Go2 Driver Node (native-based)...");

    tf_broadcaster_ = std::make_unique<Go2TfBroadcaster>(this);
    joint_state_ = std::make_unique<Go2JointState>(this);
    cmd_vel_bridge_ = std::make_unique<Go2CmdVelBridge>(this);

    RCLCPP_INFO(this->get_logger(), "Go2 Driver Node initialized");
    RCLCPP_INFO(this->get_logger(), "  TF:       /utlidar/robot_odom (native) -> odom->base_link");
    RCLCPP_INFO(this->get_logger(), "  IMU:      /utlidar/imu (native, available directly)");
    RCLCPP_INFO(this->get_logger(), "  Odom:     /utlidar/robot_odom (native, available directly)");
    RCLCPP_INFO(this->get_logger(), "  Joints:   /lf/lowstate -> /joint_states");
    RCLCPP_INFO(this->get_logger(), "  CmdVel:   /cmd_vel_out -> /api/sport/request");
}

}
