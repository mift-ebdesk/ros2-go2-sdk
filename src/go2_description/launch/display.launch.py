import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_dir = get_package_share_directory('go2_description')
    urdf_path = os.path.join(pkg_dir, 'urdf', 'go2.urdf')
    rviz_path = os.path.join(pkg_dir, 'rviz', 'urdf.rviz')

    return LaunchDescription([
        DeclareLaunchArgument('gui', default_value='true'),
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': open(urdf_path).read(),
            }],
        ),
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            condition=IfCondition(LaunchConfiguration('gui')),
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_path],
        ),
    ])
