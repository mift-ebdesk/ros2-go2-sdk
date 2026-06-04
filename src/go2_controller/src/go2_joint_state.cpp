#include "go2_controller/go2_joint_state.hpp"

namespace go2_controller
{

Go2JointState::Go2JointState(rclcpp::Node * node)
: node_(node),
  joint_names_{
    "FL_hip_joint", "FL_thigh_joint", "FL_calf_joint",
    "FR_hip_joint", "FR_thigh_joint", "FR_calf_joint",
    "RL_hip_joint", "RL_thigh_joint", "RL_calf_joint",
    "RR_hip_joint", "RR_thigh_joint", "RR_calf_joint"
  }
{
    joint_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
    imu_pub_ = node_->create_publisher<sensor_msgs::msg::Imu>("/imu/data", 10);
}

void Go2JointState::Update(const unitree_go::msg::LowState::SharedPtr msg)
{
    sensor_msgs::msg::JointState js;
    js.header.stamp = node_->now();
    js.name.resize(12);
    js.position.resize(12);
    js.velocity.resize(12);
    js.effort.resize(12);

    for (size_t i = 0; i < 12; ++i) {
        js.name[i] = joint_names_[i];
        js.position[i] = msg->motor_state[i].q;
        js.velocity[i] = msg->motor_state[i].dq;
        js.effort[i] = msg->motor_state[i].tau_est;
    }
    joint_pub_->publish(js);

    sensor_msgs::msg::Imu imu;
    imu.header.stamp = node_->now();
    imu.header.frame_id = "imu";
    imu.orientation.w = msg->imu_state.quaternion[0];
    imu.orientation.x = msg->imu_state.quaternion[1];
    imu.orientation.y = msg->imu_state.quaternion[2];
    imu.orientation.z = msg->imu_state.quaternion[3];
    imu.angular_velocity.x = msg->imu_state.gyroscope[0];
    imu.angular_velocity.y = msg->imu_state.gyroscope[1];
    imu.angular_velocity.z = msg->imu_state.gyroscope[2];
    imu.linear_acceleration.x = msg->imu_state.accelerometer[0];
    imu.linear_acceleration.y = msg->imu_state.accelerometer[1];
    imu.linear_acceleration.z = msg->imu_state.accelerometer[2];
    imu_pub_->publish(imu);
}

}
