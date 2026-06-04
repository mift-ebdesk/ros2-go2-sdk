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
        self.robot_ip = os.getenv('ROBOT_IP', '192.168.123.161')


class Go2NodeFactory:
    def __init__(self, config: Go2LaunchConfig):
        self.config = config

    def create_env_setup(self):
        return [SetEnvironmentVariable('RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp')]

    def create_launch_args(self):
        default_map = os.path.join(self.config.navigation_dir, 'maps', 'lobby.yaml')
        return [
            DeclareLaunchArgument('map', default_value=default_map),
        ]

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

    def create_nav2_launches(self):
        nav2_params = os.path.join(self.config.navigation_dir, 'config', 'nav2_params.yaml')
        return [
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(get_package_share_directory('nav2_bringup'), 'launch', 'navigation_launch.py')
                ),
                launch_arguments={
                    'use_sim_time': 'False',
                    'params_file': nav2_params,
                }.items(),
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(get_package_share_directory('nav2_bringup'), 'launch', 'localization_launch.py')
                ),
                launch_arguments={
                    'use_sim_time': 'False',
                    'map': LaunchConfiguration('map'),
                    'params_file': nav2_params,
                }.items(),
            ),
        ]

    def create_visualization_nodes(self):
        rviz_config = os.path.join(self.config.controller_dir, 'rviz', 'navigation.rviz')
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
        *factory.create_launch_args(),
        *factory.create_aggregator_nodes(),
        *factory.create_laserscan_nodes(),
        *factory.create_teleop_nodes(),
        *factory.create_nav2_launches(),
        *factory.create_visualization_nodes(),
    ])
