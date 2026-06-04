# Unitree Go2 Pro — ROS2 SDK Workspace

ROS2 Humble workspace for the Unitree Go2 Pro. Native-based architecture that consumes the robot's standard ROS2 topics directly via CycloneDDS over Ethernet. Provides SLAM, Nav2 navigation, teleoperation, and front camera streaming.

## Architecture

```
sdk_ws/src/
├── go2_sdk/                 # Official unitree_ros2 message definitions (unitree_go, unitree_api)
├── go2_description/         # URDF, meshes, sensor frames
├── go2_controller/          # C++ bridge nodes, sport client, gstreamer, configs
├── go2_navigation/          # SLAM & Nav2 params, maps
├── go2_bringup/             # Launch files (OOP pattern)
└── pointcloud2_aggregator/  # External submodule (lidar point cloud aggregation)
```

### Data Flow

```
Go2 Robot (192.168.123.161) ──── Ethernet ──── PC (192.168.123.99)
                                         CycloneDDS

 ┌─────────────── Native (standard ROS2, zero conversion) ───────────────┐
 │                                                                       │
 │  /utlidar/robot_odom (nav_msgs/Odometry, 150Hz)                      │
 │      └─► Go2TfBroadcaster ──► TF odom→base_link                      │
 │      └─► Nav2 velocity_smoother, bt_navigator (direct subscription)  │
 │                                                                       │
 │  /utlidar/imu (sensor_msgs/Imu, 250Hz)                               │
 │      └─► Available to Nav2, SLAM, and other nodes directly            │
 │                                                                       │
 │  /utlidar/cloud_deskewed (sensor_msgs/PointCloud2, 15Hz)             │
 │      └─► pointcloud2_aggregator ──► pointcloud_to_laserscan ──► /scan│
 │                                                                       │
 │  /utlidar/robot_pose (geometry_msgs/PoseStamped, 150Hz)              │
 │      └─► Available for localization overlays                          │
 │                                                                       │
 └───────────────────────────────────────────────────────────────────────┘

 ┌─────────────── Bridge (robot doesn't provide natively) ──────────────┐
 │                                                                       │
 │  /lf/lowstate (unitree_go/LowState)                                  │
 │      └─► Go2JointState ──► /joint_states (sensor_msgs/JointState)   │
 │                                                                       │
 │  /cmd_vel_out (geometry_msgs/Twist)                                  │
 │      └─► Go2CmdVelBridge ──► /api/sport/request (Move API)          │
 │                                                                       │
 │  /joy (sensor_msgs/Joy)                                              │
 │      └─► Go2CmdVelBridge ──► /api/sport/request (StandUp/Down/Stop) │
 │                                                                       │
 └───────────────────────────────────────────────────────────────────────┘

 ┌─────────────── Camera (GStreamer H.264 RTP multicast) ──────────────┐
 │                                                                       │
 │  UDP 230.1.1.1:1720 (H.264 stream)                                  │
 │      ├─► go2_gstreamer (PC, avdec_h264 software decode)             │
 │      └─► go2_gstreamer_jetson (Jetson, nvv4l2decoder HW decode)    │
 │      └─► /frontcamera/compressed (sensor_msgs/CompressedImage)      │
 │                                                                       │
 └───────────────────────────────────────────────────────────────────────┘
```

The robot natively publishes standard ROS2 messages (`nav_msgs/Odometry`, `sensor_msgs/Imu`, `sensor_msgs/PointCloud2`) at high frequency. Bridge nodes only fill 3 gaps the robot doesn't provide: TF broadcasting, velocity→sport API conversion, and joint state conversion.

### TF Tree

```
map ──► odom ──► base_link ──► [12 leg joints + imu + utlidar_imu + utlidar_lidar + camera]
                ▲                         ▲
                │                         │
     AMCL/SLAM publishes      Go2TfBroadcaster publishes
     (from /scan + /map)      (from /utlidar/robot_odom)
                               + robot_state_publisher
                                 (from URDF fixed joints)
```

## Prerequisites

- Ubuntu 22.04
- ROS2 Humble
- CycloneDDS (`ros-humble-rmw-cyclonedds-cpp`)
- OpenCV with GStreamer support (`libopencv-dev`, GStreamer plugins)
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
  libopencv-dev \
  libyaml-cpp-dev \
  nlohmann-json3-dev \
  python3-gi \
  gir1.2-gst-1.0 \
  gstreamer1.0-plugins-base-apps \
  gstreamer1.0-libav
```

### Jetson Additional Dependencies

For `go2_gstreamer_jetson`:
- Jetson-specific GStreamer plugins (`nvv4l2decoder`, `nvvidconv`)
- OpenCV built with GStreamer support for Jetson

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
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# Base bringup (driver + URDF + TF)
ros2 launch go2_bringup go2_bringup.launch.py

# SLAM mapping (bringup + aggregator + laserscan + SLAM Toolbox + teleop + RViz)
ros2 launch go2_bringup go2_mapping.launch.py

# Navigation (bringup + aggregator + laserscan + Nav2 + AMCL + teleop + RViz)
ros2 launch go2_bringup go2_navigation.launch.py map:=/path/to/map.yaml

# Remote teleop only (driver + joystick)
ros2 launch go2_bringup go2_remote.launch.py

# Camera (run separately)
ros2 run go2_controller go2_gstreamer --ros-args --params-file src/go2_controller/config/gstreamer.yaml
ros2 run go2_controller go2_gstreamer_jetson --ros-args --params-file src/go2_controller/config/gstreamer.yaml
```

### Environment Variables

| Variable | Default | Description |
|---|---|---|
| `ROBOT_IP` | `192.168.123.161` | Robot IP address |
| `RMW_IMPLEMENTATION` | `rmw_cyclonedds_cpp` | DDS middleware |
| `CYCLONEDDS_URI` | (auto) | CycloneDDS network interface config |

## Packages

### go2_sdk
Clipped copy of the official [unitree_ros2](https://github.com/unitreerobotics/unitree_ros2) repository. Contains only `unitree_go` and `unitree_api` message definitions (all docs, examples, and `unitree_hg` removed).

### go2_description
Robot URDF (`go2.urdf`), meshes, and joint definitions. Defines all sensor frames (`utlidar_lidar`, `utlidar_imu`, `imu`, `front_camera`) as fixed joints from `base_link`. Published via `robot_state_publisher`.

### go2_controller
C++ bridge nodes and utilities. OOP class-based design:
- **`go2_driver_node`** — Composition root containing:
  - `Go2TfBroadcaster` — Subscribes to `/utlidar/robot_odom`, broadcasts `odom→base_link` TF
  - `Go2JointState` — Subscribes to `/lf/lowstate`, publishes `/joint_states`
  - `Go2CmdVelBridge` — Subscribes to `/cmd_vel_out` and `/joy`, calls sport API via `Go2SportClient`
- **`go2_gstreamer`** — PC front camera (H.264 RTP → `avdec_h264` → JPEG → `CompressedImage`)
- **`go2_gstreamer_jetson`** — Jetson front camera (H.264 RTP → `nvv4l2decoder` HW decode + reader thread → JPEG → `CompressedImage`)
- **`Go2SportClient`** — Sport API request builder (Move, StandUp, StandDown, StopMove, etc.)

### go2_navigation
SLAM Toolbox and Nav2 configuration files. All topic references point to native robot topics (`/utlidar/robot_odom`, `/scan`). Includes 12 saved maps.

### go2_bringup
Launch files using OOP `Go2LaunchConfig` + `Go2NodeFactory` pattern:
- `go2_bringup.launch.py` — Driver + URDF
- `go2_mapping.launch.py` — Bringup + aggregator + laserscan + SLAM + teleop + RViz
- `go2_navigation.launch.py` — Bringup + aggregator + laserscan + Nav2 + AMCL + teleop + RViz
- `go2_remote.launch.py` — Driver + teleop only

### pointcloud2_aggregator
Git submodule from [Robotics-IoT-Pixel-Digital](https://github.com/Robotics-IoT-Pixel-Digital/pointcloud2_aggregator). Aggregates partial point cloud scans from `/utlidar/cloud_deskewed` into a full 360° scan at `/utlidar/cloud_deskewed_aggregated`.

## Topic Reference

### Robot → PC (Native, Subscribe Directly)

| Topic | Type | Hz | Used By |
|---|---|---|---|
| `/utlidar/robot_odom` | `nav_msgs/Odometry` | 150 | TF broadcaster, Nav2 |
| `/utlidar/imu` | `sensor_msgs/Imu` | 250 | Nav2, SLAM |
| `/utlidar/cloud_deskewed` | `sensor_msgs/PointCloud2` | 15 | aggregator → laserscan |
| `/utlidar/robot_pose` | `geometry_msgs/PoseStamped` | 150 | optional |
| `/lf/lowstate` | `unitree_go/LowState` | 20 | Go2JointState |
| `/lf/sportmodestate` | `unitree_go/SportModeState` | 50 | read-only monitoring |
| `/wirelesscontroller` | `unitree_go/WirelessController` | — | read-only |

### PC → Robot (Bridge Publishes)

| Topic | Type | Hz | Published By |
|---|---|---|---|
| `/api/sport/request` | `unitree_api/Request` | on-demand | Go2CmdVelBridge |
| `/joint_states` | `sensor_msgs/JointState` | 20 | Go2JointState |
| `/frontcamera/compressed` | `sensor_msgs/CompressedImage` | 20 | go2_gstreamer / go2_gstreamer_jetson |

### Internal Pipeline Topics

| Topic | Type | Description |
|---|---|---|
| `/cmd_vel_joy` | `geometry_msgs/Twist` | teleop_twist_joy output |
| `/cmd_vel` | `geometry_msgs/Twist` | Nav2 velocity_smoother output |
| `/cmd_vel_out` | `geometry_msgs/Twist` | twist_mux output → Go2CmdVelBridge input |
| `/scan` | `sensor_msgs/LaserScan` | pointcloud_to_laserscan output |
| `/utlidar/cloud_deskewed_aggregated` | `sensor_msgs/PointCloud2` | aggregator output |

## Motor & Sensor Reference

- Motor index: 0-2=FL(hip,thigh,calf), 3-5=FR, 6-8=RL, 9-11=RR
- Quaternion order in `SportModeState.imu_state.quaternion`: [w, x, y, z]
- Native odom `frame_id`: `odom`, `child_frame_id`: `base_link`
- URDF sensor offsets: `utlidar_lidar` at `(0.289, 0, -0.047)` rotated pitch=2.8782 rad, `utlidar_imu` at `(-0.026, 0, 0.042)`

## Complete DDS Reference

See [GO2_DDS.md](./GO2_DDS.md) for the full list of ~90 robot DDS topics with types, frequencies, and field descriptions.
