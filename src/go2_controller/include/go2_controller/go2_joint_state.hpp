#pragma once
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <unitree_go/msg/low_state.hpp>
#include <vector>
#include <string>

namespace go2_controller
{

class Go2JointState
{
public:
    explicit Go2JointState(rclcpp::Node * node);
    void Update(const unitree_go::msg::LowState::SharedPtr msg);

private:
    rclcpp::Node * node_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    std::vector<std::string> joint_names_;
};

}
