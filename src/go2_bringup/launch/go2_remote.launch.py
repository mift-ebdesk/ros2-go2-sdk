import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction, SetEnvironmentVariable
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


class Go2LaunchConfig:
    def __init__(self):
        dirs = {
            'controller': get_package_share_directory('go2_controller'),
        }
        self.configs = {
            'gstreamer_jetson': os.path.join(dirs['controller'], 'config', 'gstreamer_jetson.yaml'),
            'gstreamer_pc': os.path.join(dirs['controller'], 'config', 'gstreamer_pc.yaml'),
            'joystick': os.path.join(dirs['controller'], 'config', 'joystick.yaml'),
            'twist_mux': os.path.join(dirs['controller'], 'config', 'twist_mux.yaml'),
            'cyclonedds_pc': os.path.join(dirs['controller'], 'config', 'cyclonedds_pc.xml'),
            'cyclonedds_jetson': os.path.join(dirs['controller'], 'config', 'cyclonedds_jetson.xml'),
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

    def create_driver_nodes(self):
        return [
            Node(
                package='go2_controller',
                executable='go2_driver_node',
                name='go2_driver_node',
                output='screen',
            ),
        ]

    def create_camera_nodes(self):
        def setup_camera(context):
            platform = context.launch_configurations.get('platform', 'jetson')
            gstreamer_config = self.config.configs[f'gstreamer_{platform}']
            return [
                Node(
                    package='go2_controller',
                    executable='go2_gstreamer',
                    name='go2_gstreamer',
                    parameters=[gstreamer_config],
                    condition=IfCondition(LaunchConfiguration('camera')),
                ),
            ]
        return [OpaqueFunction(function=setup_camera)]

    def create_teleop_nodes(self):
        return [
            Node(
                package='joy',
                executable='joy_node',
                parameters=[self.config.configs['joystick']],
                condition=UnlessCondition(LaunchConfiguration('mqtt')),
            ),
            Node(
                package='teleop_twist_joy',
                executable='teleop_node',
                name='go2_teleop_node',
                parameters=[self.config.configs['twist_mux']],
                condition=UnlessCondition(LaunchConfiguration('mqtt')),
            ),
            Node(
                package='twist_mux',
                executable='twist_mux',
                parameters=[self.config.configs['twist_mux']],
            ),
        ]


def generate_launch_description():
    config = Go2LaunchConfig()
    factory = Go2NodeFactory(config)

    return LaunchDescription([
        *factory.create_launch_args(),
        *factory.create_env_setup(),
        *factory.create_driver_nodes(),
        *factory.create_camera_nodes(),
        *factory.create_teleop_nodes(),
    ])
