#pragma once
#include <rclcpp/rclcpp.hpp>
#include "go2_controller/go2_tf_broadcaster.hpp"
#include "go2_controller/go2_joint_state.hpp"
#include "go2_controller/go2_cmd_vel_bridge.hpp"

namespace go2_controller
{

class Go2DriverNode : public rclcpp::Node
{
public:
    Go2DriverNode();

private:
    std::unique_ptr<Go2TfBroadcaster> tf_broadcaster_;
    std::unique_ptr<Go2JointState> joint_state_;
    std::unique_ptr<Go2CmdVelBridge> cmd_vel_bridge_;
};

}
