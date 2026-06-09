#include "go2_controller/go2_sport_client.hpp"
#include "go2_controller/go2_types.hpp"
#include <nlohmann/json.hpp>

namespace go2_controller
{

Go2SportClient::Go2SportClient(rclcpp::Node * node)
: node_(node)
{
    req_pub_ = node_->create_publisher<unitree_api::msg::Request>(
        "/api/sport/request", 10);
}

void Go2SportClient::Move(unitree_api::msg::Request & req, float vx, float vy, float vyaw)
{
    nlohmann::json js;
    js["x"] = vx;
    js["y"] = vy;
    js["z"] = vyaw;
    req.parameter = js.dump();
    req.header.identity.api_id = SPORT_API_MOVE;
    req_pub_->publish(req);
}

void Go2SportClient::StopMove(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_STOPMOVE;
    req_pub_->publish(req);
}

void Go2SportClient::StandUp(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_STANDUP;
    req_pub_->publish(req);
}

void Go2SportClient::StandDown(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_STANDDOWN;
    req_pub_->publish(req);
}

void Go2SportClient::BalanceStand(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_BALANCESTAND;
    req_pub_->publish(req);
}

void Go2SportClient::Damp(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_DAMP;
    req_pub_->publish(req);
}

void Go2SportClient::RecoveryStand(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_RECOVERYSTAND;
    req_pub_->publish(req);
}

void Go2SportClient::Euler(unitree_api::msg::Request & req, float roll, float pitch, float yaw)
{
    nlohmann::json js;
    js["x"] = roll;
    js["y"] = pitch;
    js["z"] = yaw;
    req.parameter = js.dump();
    req.header.identity.api_id = SPORT_API_EULER;
    req_pub_->publish(req);
}

void Go2SportClient::FreeWalk(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_FREEWALK;
    req_pub_->publish(req);
}

void Go2SportClient::SpeedLevel(unitree_api::msg::Request & req, int level)
{
    nlohmann::json js;
    js["data"] = level;
    req.parameter = js.dump();
    req.header.identity.api_id = SPORT_API_SPEEDLEVEL;
    req_pub_->publish(req);
}

void Go2SportClient::SwitchJoystick(unitree_api::msg::Request & req, bool flag)
{
    nlohmann::json js;
    js["data"] = flag;
    req.parameter = js.dump();
    req.header.identity.api_id = SPORT_API_SWITCHJOYSTICK;
    req_pub_->publish(req);
}

void Go2SportClient::Hello(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_HELLO;
    req_pub_->publish(req);
}

void Go2SportClient::Content(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_CONTENT;
    req_pub_->publish(req);
}

void Go2SportClient::Heart(unitree_api::msg::Request & req)
{
    req.header.identity.api_id = SPORT_API_HEART;
    req_pub_->publish(req);
}

}
