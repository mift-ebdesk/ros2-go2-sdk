import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


class Go2LaunchConfig:
    def __init__(self):
        self.controller_dir = get_package_share_directory('go2_controller')
        self.description_dir = get_package_share_directory('go2_description')


class Go2NodeFactory:
    def __init__(self, config: Go2LaunchConfig):
        self.config = config

    def create_env_setup(self):
        return [
            SetEnvironmentVariable('RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp'),
        ]

    def create_launch_args(self):
        return [
            DeclareLaunchArgument('use_rviz', default_value='false'),
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

    def create_state_nodes(self):
        urdf_path = os.path.join(self.config.description_dir, 'urdf', 'go2.urdf')
        return [
            Node(
                package='robot_state_publisher',
                executable='robot_state_publisher',
                parameters=[{
                    'robot_description': open(urdf_path).read(),
                }],
            ),
        ]


def generate_launch_description():
    config = Go2LaunchConfig()
    factory = Go2NodeFactory(config)

    return LaunchDescription([
        *factory.create_env_setup(),
        *factory.create_launch_args(),
        *factory.create_driver_nodes(),
        *factory.create_state_nodes(),
    ])
