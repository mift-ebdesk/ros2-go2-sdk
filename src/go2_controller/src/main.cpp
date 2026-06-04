#include <rclcpp/rclcpp.hpp>
#include "go2_controller/go2_driver_node.hpp"

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<go2_controller::Go2DriverNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
