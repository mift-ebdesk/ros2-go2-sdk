import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


class Go2LaunchConfig:
    def __init__(self):
        self.controller_dir = get_package_share_directory('go2_controller')
        self.navigation_dir = get_package_share_directory('go2_navigation')
        self.description_dir = get_package_share_directory('go2_description')
        self.robot_ip = os.getenv('ROBOT_IP', '192.168.123.161')


class Go2NodeFactory:
    def __init__(self, config: Go2LaunchConfig):
        self.config = config

    def create_env_setup(self):
        return [SetEnvironmentVariable('RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp')]

    def create_aggregator_nodes(self):
        aggregator_dir = get_package_share_directory('pointcloud2_aggregator')
        return [
            Node(
                package='pointcloud2_aggregator',
                executable='aggregator',
                parameters=[os.path.join(aggregator_dir, 'config', 'aggregator.yaml')],
            ),
        ]

    def create_laserscan_nodes(self):
        return [
            Node(
                package='pointcloud_to_laserscan',
                executable='pointcloud_to_laserscan_node',
                remappings=[
                    ('cloud_in', '/utlidar/cloud_deskewed_aggregated'),
                ],
                parameters=[os.path.join(self.config.navigation_dir, 'config', 'pointcloud_to_laserscan.yaml')],
            ),
        ]

    def create_slam_launches(self):
        slam_params = os.path.join(self.config.navigation_dir, 'config', 'slam_params.yaml')
        return [
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
                ),
                launch_arguments={'slam_params_file': slam_params}.items(),
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

    def create_visualization_nodes(self):
        rviz_config = os.path.join(self.config.controller_dir, 'rviz', 'mapping.rviz')
        return [
            Node(
                package='rviz2',
                executable='rviz2',
                arguments=['-d', rviz_config],
            ),
        ]


def generate_launch_description():
    config = Go2LaunchConfig()
    factory = Go2NodeFactory(config)

    return LaunchDescription([
        *factory.create_env_setup(),
        *factory.create_aggregator_nodes(),
        *factory.create_laserscan_nodes(),
        *factory.create_teleop_nodes(),
        *factory.create_slam_launches(),
        *factory.create_visualization_nodes(),
    ])
