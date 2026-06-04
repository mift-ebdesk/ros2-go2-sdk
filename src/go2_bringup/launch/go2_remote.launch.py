import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


class Go2LaunchConfig:
    def __init__(self):
        self.controller_dir = get_package_share_directory('go2_controller')
        self.robot_ip = os.getenv('ROBOT_IP', '192.168.123.161')


class Go2NodeFactory:
    def __init__(self, config: Go2LaunchConfig):
        self.config = config

    def create_env_setup(self):
        return [SetEnvironmentVariable('RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp')]

    def create_launch_args(self):
        return [
            DeclareLaunchArgument('use_camera', default_value='false'),
        ]

    def create_driver_nodes(self):
        return [
            Node(
                package='go2_controller',
                executable='go2_driver_node',
                name='go2_driver_node',
                output='screen',
            ),
        ]

    def create_teleop_nodes(self):
        controller_dir = self.config.controller_dir
        return [
            Node(
                package='joy',
                executable='joy_node',
                parameters=[os.path.join(controller_dir, 'config', 'joystick.yaml')],
            ),
            Node(
                package='teleop_twist_joy',
                executable='teleop_node',
                name='go2_teleop_node',
                parameters=[os.path.join(controller_dir, 'config', 'twist_mux.yaml')],
            ),
            Node(
                package='twist_mux',
                executable='twist_mux',
                parameters=[os.path.join(controller_dir, 'config', 'twist_mux.yaml')],
            ),
        ]


def generate_launch_description():
    config = Go2LaunchConfig()
    factory = Go2NodeFactory(config)

    return LaunchDescription([
        *factory.create_env_setup(),
        *factory.create_launch_args(),
        *factory.create_driver_nodes(),
        *factory.create_teleop_nodes(),
    ])
