import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


class DisplayLaunchConfig:
    def __init__(self):
        self.pkg_dir = FindPackageShare('go2_description').find('go2_description')

    def get_urdf_path(self):
        return os.path.join(self.pkg_dir, 'urdf', 'go2.urdf.xacro')

    def get_rviz_path(self):
        return os.path.join(self.pkg_dir, 'rviz', 'urdf.rviz')


def generate_launch_description():
    config = DisplayLaunchConfig()

    return LaunchDescription([
        DeclareLaunchArgument('gui', default_value='true'),
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': Command(['xacro ', config.get_urdf_path()])
            }]
        ),
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            condition=lambda: LaunchConfiguration('gui') == 'true'
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', config.get_rviz_path()]
        ),
    ])
