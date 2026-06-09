import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, OpaqueFunction, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


FILTER_DEFS = {
    'keepout': {
        'mask_yaml': 'keepout/keepout_mask.yaml',
        'mask_topic': '/keepout_filter_mask',
        'mask_node': 'keepout_filter_mask_server',
        'info_topic': '/keepout_costmap_filter_info',
        'info_node': 'keepout_costmap_filter_info_server',
        'type': 0,
        'base': 0.0,
        'multiplier': 1.0,
    },
    'passable': {
        'mask_yaml': 'passable/passable_mask.yaml',
        'mask_topic': '/passable_filter_mask',
        'mask_node': 'passable_filter_mask_server',
        'info_topic': '/passable_costmap_filter_info',
        'info_node': 'passable_costmap_filter_info_server',
        'type': 0,
        'base': 0.0,
        'multiplier': 1.0,
    },
    'guidance': {
        'mask_yaml': 'guidance/guidance_mask.yaml',
        'mask_topic': '/guidance_filter_mask',
        'mask_node': 'guidance_filter_mask_server',
        'info_topic': '/guidance_costmap_filter_info',
        'info_node': 'guidance_costmap_filter_info_server',
        'type': 0,
        'base': 0.0,
        'multiplier': 1.0,
    },
    'speed': {
        'mask_yaml': 'speed/speed_mask.yaml',
        'mask_topic': '/speed_filter_mask',
        'mask_node': 'speed_filter_mask_server',
        'info_topic': '/speed_costmap_filter_info',
        'info_node': 'speed_costmap_filter_info_server',
        'type': 1,
        'base': 100.0,
        'multiplier': -1.0,
    },
}


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
            'nav2_params': os.path.join(dirs['navigation'], 'config', 'nav2_params.yaml'),
            'rviz_nav': os.path.join(dirs['controller'], 'rviz', 'navigation.rviz'),
            'bringup_launch': os.path.join(dirs['bringup'], 'launch', 'go2_bringup.launch.py'),
        }
        self.masks = {
            name: os.path.join(dirs['navigation'], 'mask', fdef['mask_yaml'])
            for name, fdef in FILTER_DEFS.items()
        }
        self.dirs = dirs


class Go2NodeFactory:
    def __init__(self, config: Go2LaunchConfig):
        self.config = config

    def create_launch_args(self):
        default_map = os.path.join(self.config.dirs['navigation'], 'maps', 'Studio.yaml')
        return [
            DeclareLaunchArgument('map', default_value=default_map),
            DeclareLaunchArgument('camera', default_value='true'),
            DeclareLaunchArgument('platform', default_value='jetson'),
            DeclareLaunchArgument('mqtt', default_value='false'),
            DeclareLaunchArgument('enable_keepout', default_value='true'),
            DeclareLaunchArgument('enable_passable', default_value='true'),
            DeclareLaunchArgument('enable_guidance', default_value='true'),
            DeclareLaunchArgument('enable_speed', default_value='true'),
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

    def create_filter_servers(self):
        def spawn_filters(context):
            nodes = []
            filter_node_names = []

            for name, fdef in FILTER_DEFS.items():
                enabled = context.launch_configurations.get(f'enable_{name}', 'true')
                if enabled.lower() != 'true':
                    continue

                mask_yaml = self.config.masks[name]

                nodes.append(Node(
                    package='nav2_map_server',
                    executable='map_server',
                    name=fdef['mask_node'],
                    parameters=[{
                        'yaml_filename': mask_yaml,
                        'topic_name': fdef['mask_topic'],
                    }],
                    output='screen',
                ))

                nodes.append(Node(
                    package='nav2_map_server',
                    executable='costmap_filter_info_server',
                    name=fdef['info_node'],
                    parameters=[{
                        'type': fdef['type'],
                        'filter_info_topic': fdef['info_topic'],
                        'mask_topic': fdef['mask_topic'],
                        'base': fdef['base'],
                        'multiplier': fdef['multiplier'],
                    }],
                    output='screen',
                ))

                filter_node_names.append(fdef['mask_node'])
                filter_node_names.append(fdef['info_node'])

            if filter_node_names:
                nodes.append(Node(
                    package='nav2_lifecycle_manager',
                    executable='lifecycle_manager',
                    name='lifecycle_manager_filters',
                    parameters=[{
                        'use_sim_time': False,
                        'autostart': True,
                        'node_names': filter_node_names,
                        'bond_timeout': 4.0,
                        'attempt_respawn_reconnection': True,
                        'bond_respawn_max_duration': 10.0,
                        'bond_heartbeat_period': 0.25,
                        'introspection_mode': 'disabled',
                    }],
                    output='screen',
                ))

            return nodes

        return [OpaqueFunction(function=spawn_filters)]

    def create_nav2_launches(self):
        nav2_params = self.config.configs['nav2_params']
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
        def setup_rviz(context):
            platform = context.launch_configurations.get('platform', 'jetson')
            if platform == 'jetson':
                return []
            return [
                Node(
                    package='rviz2',
                    executable='rviz2',
                    arguments=['-d', self.config.configs['rviz_nav']],
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
        *factory.create_filter_servers(),
        *factory.create_nav2_launches(),
        *factory.create_visualization_nodes(),
    ])
