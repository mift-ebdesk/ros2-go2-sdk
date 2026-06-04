# Unitree Go2 Pro - ROS2 Workspace

ROS2 Humble workspace for the Unitree Go2 Pro robot. Provides SLAM, Nav2 navigation, and teleoperation via CycloneDDS over Ethernet.

## Architecture

```
sdk_ws/src/
├── go2_sdk/                 # Official unitree_ros2 (message definitions)
├── go2_description/         # URDF, meshes, robot properties
├── go2_controller/          # Bridge nodes, sport client, config, scripts
├── go2_navigation/          # SLAM & Nav2 configs, maps
├── go2_bringup/             # Launch files
└── pointcloud2_aggregator/  # External submodule (lidar aggregation)
```

### Data Flow

```
Go2 Robot (192.168.123.161, CycloneDDS)
  │
  ├─ /utlidar/robot_odom (nav_msgs/Odometry, 150Hz)
  │     └─► go2_driver_node (TF broadcaster) ──► TF odom→base_link
  │
  ├─ /utlidar/cloud_deskewed (PointCloud2, 15Hz)
  │     └─► pointcloud2_aggregator ──► pointcloud_to_laserscan ──► /scan
  │
  ├─ /lf/lowstate (unitree_go/LowState, 20Hz)
  │     └─► go2_driver_node (joint state + IMU converter)
  │           └─► /joint_states, /imu/data
  │
  ├─ Nav2 /cmd_vel (geometry_msgs/Twist)
  │     └─► go2_driver_node (cmd_vel bridge)
  │           └─► /api/sport/request (unitree_api/Request, Move API)
  │
  └─ UDP:230.1.1.1:1720 (H.264)
        └─► go2_gstreamer_node ──► /frontcamera/compressed
```

The robot natively publishes standard ROS2 messages (`nav_msgs/Odometry`, `sensor_msgs/PointCloud2`, `sensor_msgs/Imu`) via CycloneDDS. The bridge nodes fill the gaps: TF broadcasting, velocity command conversion to sport API, and standard message conversion.

## Prerequisites

- Ubuntu 22.04
- ROS2 Humble
- CycloneDDS (`ros-humble-rmw-cyclonedds-cpp`)
- Ethernet connection to robot (PC: `192.168.123.99`, Robot: `192.168.123.161`)

### System Dependencies

```bash
sudo apt install \
  ros-humble-rmw-cyclonedds-cpp \
  ros-humble-rosidl-generator-dds-idl \
  ros-humble-navigation2 \
  ros-humble-nav2-bringup \
  ros-humble-slam-toolbox \
  ros-humble-pointcloud-to-laserscan \
  ros-humble-twist-mux \
  ros-humble-teleop-twist-joy \
  ros-humble-joy \
  ros-humble-robot-state-publisher \
  ros-humble-xacro \
  ros-humble-tf2-ros \
  ros-humble-image-transport \
  ros-humble-compressed-image-transport \
  libyaml-cpp-dev \
  nlohmann-json3-dev \
  python3-gi \
  gir1.2-gst-1.0 \
  gstreamer1.0-plugins-base-apps \
  gstreamer1.0-libav
```

## Build

```bash
source /opt/ros/humble/setup.bash
cd /path/to/sdk_ws

# Initialize submodules
git submodule update --init --recursive

# Build all packages
colcon build --symlink-install

# Source workspace
source install/setup.bash
```

## Usage

### Environment Setup

```bash
# Set CycloneDDS as RMW
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# Configure network interface (replace enp3s0 with your interface)
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces>
  <NetworkInterface name="enp3s0" priority="default" multicast="default" />
</Interfaces></General></Domain></CycloneDDS>'
```

Or use the setup script:
```bash
source src/go2_controller/scripts/setup_cyclonedds.sh
```

### Launch Modes

```bash
source install/setup.bash

# Base bringup (driver + TF)
ros2 launch go2_bringup go2_bringup.launch.py

# Mapping (SLAM Toolbox)
ros2 launch go2_bringup go2_mapping.launch.py

# Navigation (Nav2 + AMCL)
ros2 launch go2_bringup go2_navigation.launch.py map:=$(ros2 pkg prefix go2_navigation)/share/go2_navigation/maps/your_map.yaml

# Remote teleop
ros2 launch go2_bringup go2_remote.launch.py
```

### Environment Variables

| Variable | Default | Description |
|---|---|---|
| `ROBOT_IP` | `192.168.123.161` | Robot IP address |
| `RMW_IMPLEMENTATION` | `rmw_cyclonedds_cpp` | DDS middleware |
| `CYCLONEDDS_URI` | (auto) | CycloneDDS network config |

## Packages

### go2_sdk
Official [unitree_ros2](https://github.com/unitreerobotics/unitree_ros2) clone. Provides `unitree_go` and `unitree_api` message definitions.

### go2_description
Robot URDF (xacro), meshes, and joint definitions for the Go2.

### go2_controller
Bridge nodes that connect the robot's native CycloneDDS topics to standard ROS2:
- **go2_driver_node** (C++): TF broadcaster, cmd_vel bridge, joint state converter
- **go2_gstreamer_node** (Python): Front camera via GStreamer

### go2_navigation
SLAM and Nav2 configuration files and saved maps.

### go2_bringup
Launch files for bringup, mapping, navigation, and remote modes.

## Robot DDS Topics Reference

See [GO2_DDS.md](./GO2_DDS.md) for the complete list of topics published by the robot via CycloneDDS.
