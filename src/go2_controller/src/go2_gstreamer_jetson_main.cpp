#include <rclcpp/rclcpp.hpp>
#include "go2_controller/go2_gstreamer_jetson.hpp"

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<go2_controller::Go2GstreamerJetsonNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
