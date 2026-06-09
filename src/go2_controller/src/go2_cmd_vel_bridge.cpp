#include "go2_controller/go2_cmd_vel_bridge.hpp"

namespace go2_controller
{

static constexpr double kModeChangeCooldownSec = 2.0;
static constexpr int kButtonStart = 7;
static constexpr int kButtonSelect = 6;
static constexpr int kButtonStandUp = 3;
static constexpr int kButtonStandDown = 0;
static constexpr int kButtonHello = 2;
static constexpr int kButtonContent = 1;

Go2CmdVelBridge::Go2CmdVelBridge(rclcpp::Node * node)
: node_(node),
  sport_client_(node),
  last_mode_change_time_(node->now()),
  joy_enabled_(false)
{
    cmd_vel_sub_ = node_->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel_out", 10,
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
            CmdVelCallback(msg);
        });

    joy_cmd_sub_ = node_->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel_joy", 10,
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
            JoyCmdCallback(msg);
        });

    joy_sub_ = node_->create_subscription<sensor_msgs::msg::Joy>(
        "/joy", 10,
        [this](const sensor_msgs::msg::Joy::SharedPtr msg) {
            JoyCallback(msg);
        });

    RCLCPP_INFO(node_->get_logger(), "Joystick mode: DISABLED (navigation active)");
}

void Go2CmdVelBridge::SendZeroVelocity()
{
    unitree_api::msg::Request req;
    sport_client_.Move(req, 0.0f, 0.0f, 0.0f);
}

void Go2CmdVelBridge::CmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    if (!joy_enabled_) {
        unitree_api::msg::Request req;
        sport_client_.Move(req, msg->linear.x, msg->linear.y, msg->angular.z);
    }
}

void Go2CmdVelBridge::JoyCmdCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    if (joy_enabled_) {
        unitree_api::msg::Request req;
        sport_client_.Move(req, msg->linear.x, msg->linear.y, msg->angular.z);
    }
}

void Go2CmdVelBridge::JoyCallback(const sensor_msgs::msg::Joy::SharedPtr msg)
{
    if (prev_buttons_.size() != msg->buttons.size()) {
        prev_buttons_.assign(msg->buttons.size(), 0);
    }

    if (msg->buttons.size() > kButtonStart && msg->buttons[kButtonStart] == 1 && prev_buttons_[kButtonStart] == 0) {
        if (!joy_enabled_) {
            joy_enabled_ = true;
            SendZeroVelocity();
            RCLCPP_INFO(node_->get_logger(), "Joystick mode: ENABLED (navigation paused)");
        }
    }
    if (msg->buttons.size() > kButtonSelect && msg->buttons[kButtonSelect] == 1 && prev_buttons_[kButtonSelect] == 0) {
        if (joy_enabled_) {
            joy_enabled_ = false;
            SendZeroVelocity();
            RCLCPP_INFO(node_->get_logger(), "Joystick mode: DISABLED (navigation active)");
        }
    }

    auto now = node_->now();
    bool cooldown_ok = (now - last_mode_change_time_).seconds() >= kModeChangeCooldownSec;

    if (msg->buttons.size() > kButtonStandUp && msg->buttons[kButtonStandUp] == 1 && prev_buttons_[kButtonStandUp] == 0 && cooldown_ok) {
        unitree_api::msg::Request req;
        sport_client_.StandUp(req);
        last_mode_change_time_ = now;
        balance_stand_timer_ = node_->create_wall_timer(
            std::chrono::milliseconds(2000),
            [this]() {
                unitree_api::msg::Request req;
                sport_client_.BalanceStand(req);
                RCLCPP_INFO(node_->get_logger(), "BalanceStand command sent (auto after StandUp)");
                balance_stand_timer_->cancel();
            });
        RCLCPP_INFO(node_->get_logger(), "StandUp command sent");
    }
    if (msg->buttons.size() > kButtonStandDown && msg->buttons[kButtonStandDown] == 1 && prev_buttons_[kButtonStandDown] == 0 && cooldown_ok) {
        unitree_api::msg::Request req;
        sport_client_.StandDown(req);
        last_mode_change_time_ = now;
        RCLCPP_INFO(node_->get_logger(), "StandDown command sent");
    }
    if (msg->buttons.size() > kButtonHello && msg->buttons[kButtonHello] == 1 && prev_buttons_[kButtonHello] == 0 && cooldown_ok) {
        unitree_api::msg::Request req;
        sport_client_.Hello(req);
        last_mode_change_time_ = now;
        RCLCPP_INFO(node_->get_logger(), "Hello command sent");
    }
    if (msg->buttons.size() > kButtonContent && msg->buttons[kButtonContent] == 1 && prev_buttons_[kButtonContent] == 0 && cooldown_ok) {
        unitree_api::msg::Request req;
        sport_client_.Content(req);
        last_mode_change_time_ = now;
        RCLCPP_INFO(node_->get_logger(), "Content command sent");
    }

    prev_buttons_.assign(msg->buttons.begin(), msg->buttons.end());
}

}
