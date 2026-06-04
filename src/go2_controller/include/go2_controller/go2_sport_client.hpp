#pragma once
#include <rclcpp/rclcpp.hpp>
#include <unitree_api/msg/request.hpp>
#include <unitree_api/msg/response.hpp>

namespace go2_controller
{

class Go2SportClient
{
public:
    explicit Go2SportClient(rclcpp::Node * node);
    void Move(unitree_api::msg::Request & req, float vx, float vy, float vyaw);
    void StopMove(unitree_api::msg::Request & req);
    void StandUp(unitree_api::msg::Request & req);
    void StandDown(unitree_api::msg::Request & req);
    void BalanceStand(unitree_api::msg::Request & req);
    void Damp(unitree_api::msg::Request & req);
    void RecoveryStand(unitree_api::msg::Request & req);
    void Euler(unitree_api::msg::Request & req, float roll, float pitch, float yaw);
    void FreeWalk(unitree_api::msg::Request & req);
    void SpeedLevel(unitree_api::msg::Request & req, int level);
    void SwitchJoystick(unitree_api::msg::Request & req, bool flag);
    void Heart(unitree_api::msg::Request & req);

private:
    rclcpp::Node * node_;
    rclcpp::Publisher<unitree_api::msg::Request>::SharedPtr req_pub_;
};

}
