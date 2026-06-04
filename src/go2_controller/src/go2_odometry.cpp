#include "go2_controller/go2_odometry.hpp"

namespace go2_controller
{

Go2Odometry::Go2Odometry(rclcpp::Node * node)
: node_(node),
  odom_frame_("odom"),
  base_frame_("base_link")
{
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*node_);
}

void Go2Odometry::Update(const unitree_go::msg::SportModeState::SharedPtr msg)
{
    geometry_msgs::msg::TransformStamped t;
    t.header.stamp = node_->now();
    t.header.frame_id = odom_frame_;
    t.child_frame_id = base_frame_;

    t.transform.translation.x = msg->position[0];
    t.transform.translation.y = msg->position[1];
    t.transform.translation.z = msg->position[2];

    t.transform.rotation.w = msg->imu_state.quaternion[0];
    t.transform.rotation.x = msg->imu_state.quaternion[1];
    t.transform.rotation.y = msg->imu_state.quaternion[2];
    t.transform.rotation.z = msg->imu_state.quaternion[3];

    tf_broadcaster_->sendTransform(t);
}

}
