import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, OpaqueFunction, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


class Go2LaunchConfig:
    def __init__(self):
        dirs = {
            'controller': get_package_share_directory('go2_controller'),
            'navigation': get_package_share_directory('go2_navigation'),
            'bringup': get_package_share_directory('go2_bringup'),
        }
        self.configs = {
            'cyclonedds_pc': os.path.join(dirs['controller'], 'config', 'cyclonedds_pc.xml'),
            'cyclonedds_jetson': os.path.join(dirs['controller'], 'config', 'cyclonedds_jetson.xml'),
            'slam_params': os.path.join(dirs['navigation'], 'config', 'slam_params.yaml'),
            'rviz_slam': os.path.join(dirs['controller'], 'rviz', 'mapping.rviz'),
            'bringup_launch': os.path.join(dirs['bringup'], 'launch', 'go2_bringup.launch.py'),
        }


class Go2NodeFactory:
    def __init__(self, config: Go2LaunchConfig):
        self.config = config

    def create_launch_args(self):
        return [
            DeclareLaunchArgument('camera', default_value='true'),
            DeclareLaunchArgument('platform', default_value='jetson'),
            DeclareLaunchArgument('mqtt', default_value='false'),
        ]

    def create_env_setup(self):
        def setup_env(context):
            platform = context.launch_configurations.get('platform', 'jetson')
            xml_config = self.config.configs[f'cyclonedds_{platform}']
            return [
                SetEnvironmentVariable('RMW_IMPLEMENTATION', 'rmw_cyclonedds_cpp'),
                SetEnvironmentVariable('CYCLONEDDS_URI', xml_config),
            ]
        return [OpaqueFunction(function=setup_env)]

    def create_bringup(self):
        return [
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(self.config.configs['bringup_launch']),
                launch_arguments={
                    'camera': LaunchConfiguration('camera'),
                    'platform': LaunchConfiguration('platform'),
                    'mqtt': LaunchConfiguration('mqtt'),
                }.items(),
            ),
        ]

    def create_slam_launches(self):
        return [
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
                ),
                launch_arguments={
                    'slam_params_file': self.config.configs['slam_params'],
                }.items(),
            ),
        ]

    def create_visualization_nodes(self):
        def setup_rviz(context):
            platform = context.launch_configurations.get('platform', 'jetson')
            if platform == 'jetson':
                return []
            return [
                Node(
                    package='rviz2',
                    executable='rviz2',
                    arguments=['-d', self.config.configs['rviz_slam']],
                ),
            ]
        return [OpaqueFunction(function=setup_rviz)]


def generate_launch_description():
    config = Go2LaunchConfig()
    factory = Go2NodeFactory(config)

    return LaunchDescription([
        *factory.create_launch_args(),
        *factory.create_env_setup(),
        *factory.create_bringup(),
        *factory.create_slam_launches(),
        *factory.create_visualization_nodes(),
    ])
