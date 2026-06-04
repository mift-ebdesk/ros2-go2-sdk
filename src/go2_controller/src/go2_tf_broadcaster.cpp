#include "go2_controller/go2_tf_broadcaster.hpp"

namespace go2_controller
{

Go2TfBroadcaster::Go2TfBroadcaster(rclcpp::Node * node)
: node_(node)
{
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*node_);

    odom_sub_ = node_->create_subscription<nav_msgs::msg::Odometry>(
        "/utlidar/robot_odom", 10,
        [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
            OdomCallback(msg);
        });
}

void Go2TfBroadcaster::OdomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
    geometry_msgs::msg::TransformStamped t;
    t.header.stamp = msg->header.stamp;
    t.header.frame_id = msg->header.frame_id;
    t.child_frame_id = msg->child_frame_id;
    t.transform.translation.x = msg->pose.pose.position.x;
    t.transform.translation.y = msg->pose.pose.position.y;
    t.transform.translation.z = msg->pose.pose.position.z;
    t.transform.rotation = msg->pose.pose.orientation;
    tf_broadcaster_->sendTransform(t);
}

}
