# Go2 SDK Workspace

ROS2 Humble workspace for the Unitree Go2 Pro. Connects to the robot over Ethernet via CycloneDDS, consumes native ROS2 topics directly, and provides SLAM, Nav2 navigation, teleoperation, and camera streaming.

Designed for **Jetson** (default) and **PC** deployment.

---

## Architecture

```
src/
├── go2_sdk/                  # Unitree message definitions
│   ├── unitree_go/           #   SportModeState, LowState, WirelessController, ...
│   ├── unitree_api/          #   Request, Response (sport API)
│   ├── setup.sh              #   Quick environment setup
│   └── LICENSE
├── go2_description/          # URDF, meshes, display launch
├── go2_controller/           # C++ bridge nodes + gstreamer + configs
├── go2_navigation/           # Nav2/SLAM params + 12 maps
├── go2_bringup/              # Launch files
└── pointcloud2_aggregator/   # External submodule (lidar aggregation)
```

### Data Flow

```
Go2 Robot (192.168.123.161) ──── Ethernet/CycloneDDS ──── Jetson or PC

 NATIVE TOPICS (standard ROS2, zero conversion)
 ─────────────────────────────────────────────
 /utlidar/robot_odom  (nav_msgs/Odometry, 150Hz)
     ├─► Go2TfBroadcaster ──► TF odom→base_link
     └─► Nav2 (controller_server, bt_navigator, velocity_smoother)

 /utlidar/imu  (sensor_msgs/Imu, 250Hz)
     └─► Available to all nodes directly

 /utlidar/cloud_deskewed  (sensor_msgs/PointCloud2, 15Hz)
     └─► aggregator ──► pointcloud_to_laserscan ──► /scan

 BRIDGE TOPICS (robot doesn't provide natively)
 ────────────────────────────────────────────────
 /lf/lowstate  (unitree_go/LowState)
     └─► Go2JointState ──► /joint_states

 /cmd_vel_out  (geometry_msgs/Twist)
     └─► Go2CmdVelBridge ──► /api/sport/request (Move API)

 /joy  (sensor_msgs/Joy)
     └─► Go2CmdVelBridge ──► /api/sport/request (StandUp/Down/Stop)

 CAMERA (GStreamer H.264 RTP multicast)
 ────────────────────────────────────────
 UDP 230.1.1.1:1720
     └─► go2_gstreamer ──► /frontcamera/compressed (CompressedImage)
         (jetson: nvv4l2decoder + reader thread, pc: avdec_h264)
```

### TF Tree

```
map ──► odom ──► base_link ──► [12 leg joints + imu + utlidar_imu + utlidar_lidar + camera]
       ▲           ▲
  AMCL/SLAM    Go2TfBroadcaster + robot_state_publisher
```

---

## Quick Start

### System Dependencies

```bash
sudo apt install \
  ros-humble-rmw-cyclonedds-cpp \
  ros-humble-rosidl-generator-dds-idl \
  ros-humble-navigation2 ros-humble-nav2-bringup \
  ros-humble-slam-toolbox \
  ros-humble-pointcloud-to-laserscan \
  ros-humble-twist-mux ros-humble-teleop-twist-joy ros-humble-joy \
  ros-humble-robot-statepublisher ros-humble-xacro ros-humble-tf2-ros \
  ros-humble-image-transport ros-humble-compressed-image-transport \
  libopencv-dev libyaml-cpp-dev nlohmann-json3-dev \
  gstreamer1.0-plugins-base-apps gstreamer1.0-libav
```

### Build

```bash
source /opt/ros/humble/setup.bash
git submodule update --init --recursive
colcon build --symlink-install --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -sf build/compile_commands.json compile_commands.json
source install/setup.bash
```

### Launch

```bash
# Full bringup (driver + camera + URDF + lidar pipeline + teleop)
ros2 launch go2_bringup go2_bringup.launch.py

# SLAM mapping
ros2 launch go2_bringup go2_mapping.launch.py

# Navigation with default map
ros2 launch go2_bringup go2_navigation.launch.py

# Navigation with custom map
ros2 launch go2_bringup go2_navigation.launch.py map:=$(ros2 pkg prefix go2_navigation)/share/go2_navigation/maps/studio.yaml

# Remote teleop only (no lidar pipeline)
ros2 launch go2_bringup go2_remote.launch.py
```

### Launch Arguments

All launch files accept these args:

| Arg | Default | Values | Effect |
|---|---|---|---|
| `platform` | `jetson` | `jetson`, `pc` | Selects CycloneDDS XML, gstreamer config, and enables/disables RViz |
| `camera` | `true` | `true`, `false` | Enable/disable front camera gstreamer node |
| `mqtt` | `false` | `true`, `false` | When true, disables joy_node and teleop (use MQTT bridge instead) |

`go2_navigation.launch.py` additionally accepts:

| Arg | Default | Description |
|---|---|---|
| `map` | `lobby.yaml` | Path to map yaml file |

**Platform behavior:**

| | `jetson` | `pc` |
|---|---|---|
| CycloneDDS | `cyclonedds_jetson.xml` | `cyclonedds_pc.xml` |
| Gstreamer | `gstreamer_jetson.yaml` (nvv4l2decoder HW) | `gstreamer_pc.yaml` (avdec_h264 SW) |
| RViz | Off | On |

**Examples:**

```bash
# PC deployment
ros2 launch go2_bringup go2_navigation.launch.py platform:=pc

# Jetson, no camera, MQTT control
ros2 launch go2_bringup go2_bringup.launch.py camera:=false mqtt:=true

# PC remote teleop, no camera
ros2 launch go2_bringup go2_remote.launch.py platform:=pc camera:=false
```

---

## Packages

### go2_sdk

Vendor message definitions from the official [unitree_ros2](https://github.com/unitreerobotics/unitree_ros2) repo, trimmed to only what the Go2 needs:
- `unitree_go` — `SportModeState`, `LowState`, `LowCmd`, `WirelessController`, `IMUState`, `MotorState`
- `unitree_api` — `Request`, `Response` (sport API)

### go2_description

URDF (`go2.urdf`), collision meshes (DAE), and a display launch for visualizing the robot model. Sensor frames defined as fixed joints from `base_link`:

| Frame | Offset from base_link |
|---|---|
| `utlidar_lidar` | `(0.289, 0, -0.047)` pitch=2.8782 rad |
| `utlidar_imu` | `(-0.026, 0, 0.042)` |
| `imu` | `(0, 0, 0)` |
| `front_camera` | Head link offset |

### go2_controller

C++ nodes and utilities. OOP class-based design.

**`go2_driver_node`** — Single composition root containing:
- `Go2TfBroadcaster` — Subscribes `/utlidar/robot_odom`, broadcasts TF `odom->base_link`
- `Go2JointState` — Subscribes `/lf/lowstate`, publishes `/joint_states`
- `Go2CmdVelBridge` — Subscribes `/cmd_vel_out` + `/joy`, calls sport API
- `Go2SportClient` — Sport API request builder (Move, StandUp, StandDown, StopMove, etc.)

**`go2_gstreamer`** — Front camera node. Single executable, dual platform:
- `platform:=jetson` — `nvv4l2decoder` hardware decode, dedicated reader thread, low-latency config
- `platform:=pc` — `avdec_h264` software decode, single-threaded

**Configs:**

| File | Purpose |
|---|---|
| `gstreamer_jetson.yaml` / `gstreamer_pc.yaml` | Camera params per platform |
| `cyclonedds_jetson.xml` / `cyclonedds_pc.xml` | CycloneDDS network config per platform |
| `twist_mux.yaml` | Teleop + Nav2 velocity muxing (Nav2 priority=10 > joystick=5) |
| `joystick.yaml` | Joy node device config |

### go2_navigation

SLAM Toolbox and Nav2 configuration. All topic references point to native robot topics:

| Config | Key Settings |
|---|---|
| `nav2_params.yaml` | `odom_topic: /utlidar/robot_odom`, `scan_topic: /scan`, DWB controller, SmacPlanner2D |
| `slam_params.yaml` | `odom_frame: odom`, `scan_topic: /scan`, Ceres solver, loop closing enabled |
| `pointcloud_to_laserscan.yaml` | `target_frame: base_link`, full 360° scan, range 0.1–10m |

**12 maps** included: `lobby`, `studio`, `stair`, `FullMap`, and variants.

### go2_bringup

Launch files using OOP `Go2LaunchConfig` + `Go2NodeFactory` pattern. All config file paths centralized in `Go2LaunchConfig.configs` dict.

| Launch | Description | Brings Up |
|---|---|---|
| `go2_bringup.launch.py` | Base bringup | Driver + camera + URDF + aggregator + laserscan + teleop |
| `go2_mapping.launch.py` | SLAM mapping | Bringup + SLAM Toolbox + RViz (pc only) |
| `go2_navigation.launch.py` | Autonomous nav | Bringup + Nav2 + AMCL + RViz (pc only) |
| `go2_remote.launch.py` | Manual control | Driver + camera + teleop only |

### pointcloud2_aggregator

Git submodule. Aggregates partial scans from `/utlidar/cloud_deskewed` into full 360° scan at `/utlidar/cloud_deskewed_aggregated`.

---

## Topic Reference

### Robot → PC (Native)

| Topic | Type | Hz | Used By |
|---|---|---|---|
| `/utlidar/robot_odom` | `nav_msgs/Odometry` | 150 | TF broadcaster, Nav2 |
| `/utlidar/imu` | `sensor_msgs/Imu` | 250 | Available directly |
| `/utlidar/cloud_deskewed` | `sensor_msgs/PointCloud2` | 15 | Aggregator → laserscan → `/scan` |
| `/lf/lowstate` | `unitree_go/LowState` | 20 | Go2JointState → `/joint_states` |
| `/lf/sportmodestate` | `unitree_go/SportModeState` | 50 | Monitoring only |
| `/wirelesscontroller` | `unitree_go/WirelessController` | — | Monitoring only |

### PC → Robot (Bridge)

| Topic | Type | Published By |
|---|---|---|
| `/api/sport/request` | `unitree_api/Request` | Go2CmdVelBridge |
| `/joint_states` | `sensor_msgs/JointState` | Go2JointState |
| `/frontcamera/compressed` | `sensor_msgs/CompressedImage` | go2_gstreamer |

### Internal Pipeline

| Topic | Description |
|---|---|
| `/cmd_vel_joy` | teleop_twist_joy output |
| `/cmd_vel` | Nav2 velocity_smoother output |
| `/cmd_vel_out` | twist_mux merged output → Go2CmdVelBridge input |
| `/scan` | pointcloud_to_laserscan output → Nav2/SLAM input |

---

## Velocity Flow

```
Nav2 planner ──► /cmd_vel (priority 10) ──┐
                                           ├─► twist_mux ──► /cmd_vel_out ──► Go2CmdVelBridge ──► Robot
Joystick ──► /cmd_vel_joy (priority 5) ───┘
```

---

## Motor & Sensor Reference

- Motor index: 0-2=FL(hip,thigh,calf), 3-5=FR, 6-8=RL, 9-11=RR
- Native odom `frame_id`: `odom`, `child_frame_id`: `base_link`
- Quaternion order: [w, x, y, z]

## Complete DDS Reference

See [GO2_DDS.md](./GO2_DDS.md) for the full list of ~90 robot DDS topics.
