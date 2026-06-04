# Unitree Go2 Pro - DDS Topic Reference

All topics published natively by the Go2 Pro via **CycloneDDS** when connected over Ethernet.
Publisher node appears as `_CREATED_BY_BARE_DDS_APP_` (bare DDS, not a ROS2 node).

**Connection:** Ethernet, PC IP `192.168.123.99`, Robot IP `192.168.123.161` (default).
**Requirement:** `RMW_IMPLEMENTATION=rmw_cyclonedds_cpp`

---

## Table of Contents

1. [Robot State](#1-robot-state)
2. [LiDAR & Mapping (utlidar)](#2-lidar--mapping-utlidar)
3. [Navigation & SLAM (uslam)](#3-navigation--slam-uslam)
4. [API Services](#4-api-services)
5. [Video & Audio](#5-video--audio)
6. [Controller & Input](#6-controller--input)
7. [System & Status](#7-system--status)
8. [Low-Level Control](#8-low-level-control)
9. [Arm Control](#9-arm-control)
10. [WebRTC](#10-webrtc)
11. [Misc / Internal](#11-misc--internal)

---

## 1. Robot State

### `/lf/sportmodestate` (Low Frequency ~20 Hz)

**Type:** `unitree_go/msg/SportModeState`

High-level robot motion state at ~20 Hz. Primary source for odometry, IMU, gait, and foot data.

| Field | Type | Description |
|---|---|---|
| `stamp` | `TimeSpec` | Timestamp (sec + nanosec) |
| `error_code` | `uint32` | Error code (100 = normal) |
| `imu_state.quaternion` | `float32[4]` | IMU orientation quaternion [w, x, y, z] (body frame) |
| `imu_state.gyroscope` | `float32[3]` | Angular velocity [x, y, z] rad/s |
| `imu_state.accelerometer` | `float32[3]` | Linear acceleration [x, y, z] m/s^2 |
| `imu_state.rpy` | `float32[3]` | Roll, pitch, yaw in radians |
| `imu_state.temperature` | `int8` | IMU temperature in Celsius |
| `mode` | `uint8` | Sport mode (see table below) |
| `progress` | `float32` | Dance/action progress (0.0 = not executing, 1.0 = complete) |
| `gait_type` | `uint8` | Current gait type (see table below) |
| `foot_raise_height` | `float32` | Current foot raise height in meters |
| `position` | `float32[3]` | Robot position [x, y, z] in odometry frame (meters) |
| `body_height` | `float32` | Body height from ground (meters) |
| `velocity` | `float32[3]` | Velocity [vx, vy, vz] m/s |
| `yaw_speed` | `float32` | Yaw angular velocity rad/s |
| `range_obstacle` | `float32[4]` | Obstacle range [front, right, rear, left] meters |
| `foot_force` | `int16[4]` | Foot contact force [FL, FR, RL, RR] |
| `foot_position_body` | `float32[12]` | Foot positions [x,y,z] x4 relative to body (meters) |
| `foot_speed_body` | `float32[12]` | Foot velocities [vx,vy,vz] x4 relative to body (m/s) |

**Sport Mode values:**

| Value | Mode |
|---|---|
| 0 | Idle (default stand) |
| 1 | Balance stand |
| 2 | Pose |
| 3 | Locomotion |
| 5 | Lie down |
| 6 | Joint lock |
| 7 | Damping |
| 8 | Recovery stand |
| 10 | Sit |
| 11 | Front flip |
| 12 | Front jump |
| 13 | Front pounce |

**Gait Type values:**

| Value | Gait |
|---|---|
| 0 | Idle |
| 1 | Trot |
| 2 | Run (trot running) |
| 3 | Climb stair |
| 4 | Forward down stair |
| 9 | Adjust |

---

### `/sportmodestate` (High Frequency ~500 Hz)

**Type:** `unitree_go/msg/SportModeState`

Same message structure as `/lf/sportmodestate` but published at high frequency (~500 Hz).
Use `/lf/` variant unless you specifically need high-rate updates.

---

### `/lf/lowstate` (Low Frequency ~20 Hz)

**Type:** `unitree_go/msg/LowState`

Complete low-level robot state at ~20 Hz. Contains motor states, battery, IMU, wireless controller raw data.

| Field | Type | Description |
|---|---|---|
| `head` | `uint8[2]` | Packet header [0xFE, 0xEF] |
| `level_flag` | `uint8` | Control level flag (0 = sport mode) |
| `imu_state` | `IMU` | Same IMU structure as SportModeState |
| `motor_state[0..11]` | `MotorState[20]` | 12 active motors (index 12-19 unused, all zeros) |
| `bms_state` | `BmsState` | Battery management system state |
| `foot_force` | `int16[4]` | Foot contact force [FL, FR, RL, RR] |
| `foot_force_est` | `int16[4]` | Estimated foot force |
| `tick` | `uint32` | System tick counter |
| `wireless_remote` | `uint8[40]` | Raw wireless controller data |
| `bit_flag` | `uint8` | Status bit flags (36 = normal standing) |
| `power_v` | `float32` | Battery voltage (V) |
| `power_a` | `float32` | Battery current (A) |
| `temperature_ntc1` | `int8` | NTC temperature 1 (Celsius) |
| `temperature_ntc2` | `int8` | NTC temperature 2 (Celsius) |
| `crc` | `uint32` | CRC checksum |

**MotorState structure (per motor):**

| Field | Type | Description |
|---|---|---|
| `mode` | `uint8` | Motor mode (0 = stopped, 1 = FOC/active) |
| `q` | `float32` | Joint angle (radians) |
| `dq` | `float32` | Joint angular velocity (rad/s) |
| `ddq` | `float32` | Joint angular acceleration (rad/s^2) |
| `tau_est` | `float32` | Estimated joint torque (N.m) |
| `temperature` | `int8` | Motor temperature (Celsius) |
| `lost` | `uint32` | Communication lost count |

**Motor index mapping (12 active motors):**

| Index | Joint | Leg |
|---|---|---|
| 0 | FL_hip (hip roll) | Front Left |
| 1 | FL_thigh (hip pitch) | Front Left |
| 2 | FL_calf (knee) | Front Left |
| 3 | FR_hip | Front Right |
| 4 | FR_thigh | Front Right |
| 5 | FR_calf | Front Right |
| 6 | RL_hip | Rear Left |
| 7 | RL_thigh | Rear Left |
| 8 | RL_calf | Rear Left |
| 9 | RR_hip | Rear Right |
| 10 | RR_thigh | Rear Right |
| 11 | RR_calf | Rear Right |
| 12-19 | (unused) | - |

**BmsState structure:**

| Field | Type | Description |
|---|---|---|
| `version_high/low` | `uint8` | BMS firmware version |
| `status` | `uint8` | Battery status (8 = discharging) |
| `soc` | `uint8` | State of charge percentage (0-100) |
| `current` | `int16` | Battery current (mA) |
| `cycle` | `uint16` | Charge cycle count |
| `bq_ntc[2]` | `uint8` | BQ NTC temperatures (Celsius) |
| `mcu_ntc[2]` | `uint8` | MCU NTC temperatures (Celsius) |
| `cell_vol[15]` | `uint16` | Individual cell voltages (mV) |

---

### `/lowstate` (High Frequency ~500 Hz)

**Type:** `unitree_go/msg/LowState`

Same message as `/lf/lowstate` but at ~500 Hz. Use for low-level motor control feedback.

---

## 2. LiDAR & Mapping (utlidar)

These topics are from the onboard **Mid-360 LiDAR** and its built-in SLAM/processing pipeline.

### `/utlidar/cloud` (~15 Hz)

**Type:** `sensor_msgs/msg/PointCloud2`
**Frame:** `utlidar_lidar`

Raw 3D point cloud from the LiDAR sensor. Fields: x, y, z, intensity, ring, time.

### `/utlidar/cloud_deskewed` (~15 Hz)

**Type:** `sensor_msgs/msg/PointCloud2`
**Frame:** `odom`

Motion-compensated (deskewed) point cloud. This is the **recommended input for SLAM** and mapping. Points are corrected for robot motion during the scan. Fields: x, y, z, intensity.

### `/utlidar/cloud_base`

**Type:** `sensor_msgs/msg/PointCloud2`

Point cloud transformed to `base_link` frame.

### `/utlidar/robot_odom` (~150 Hz)

**Type:** `nav_msgs/msg/Odometry`
**Frame:** `odom` -> `base_link`

LiDAR-based odometry from the onboard SLAM. Standard `nav_msgs/Odometry` format.
**This is the primary odometry source for Nav2/SLAM.** Provides position, orientation, and twist at ~150 Hz.

Note: The robot publishes this as a topic message but does **NOT** publish the corresponding TF transform. A TF broadcaster node is needed to convert this to `odom -> base_link` TF.

### `/utlidar/robot_pose` (~150 Hz)

**Type:** `geometry_msgs/msg/PoseStamped`
**Frame:** `odom`

Robot pose from the onboard LiDAR SLAM. Same data as `robot_odom` but in `PoseStamped` format.

### `/utlidar/imu` (~250 Hz)

**Type:** `sensor_msgs/msg/Imu`
**Frame:** `utlidar_imu`

Standard ROS2 IMU message from the LiDAR's built-in IMU. Provides orientation, angular velocity, and linear acceleration at 250 Hz.

### `/utlidar/voxel_map_compressed`

**Type:** `unitree_go/msg/VoxelMapCompressed`

Compressed voxel map of the surroundings. Contains:
- `resolution`: Voxel size in meters (0.05m)
- `origin[3]`: Map origin [x, y, z]
- `width[3]`: Map dimensions in voxels [x, y, z]
- `src_size`: Original data size
- `data[]`: Compressed voxel data (requires WASM decoder)

### `/utlidar/height_map_array`

**Type:** `unitree_go/msg/HeightMap`

Grid-based height map representation of the terrain.

### `/utlidar/range_info`

**Type:** `geometry_msgs/msg/PointStamped`
**Frame:** `base_link`

Range information - likely the bounding box or detection range of the LiDAR.
- `x`: Range forward
- `y`: Range lateral
- `z`: Range vertical

### `/utlidar/grid_map`

**Type:** `sensor_msgs/msg/PointCloud2`

2D grid map representation as a point cloud.

### `/utlidar/height_map`

**Type:** `sensor_msgs/msg/PointCloud2`

Height map as a point cloud representation.

### `/utlidar/range_map`

**Type:** `sensor_msgs/msg/PointCloud2`

Range map visualization.

### `/utlidar/voxel_map`

**Type:** `sensor_msgs/msg/PointCloud2`

Uncompressed voxel map as a point cloud.

### `/utlidar/lidar_state`

**Type:** `unitree_go/msg/LidarState`

LiDAR sensor status including firmware versions, rotation speed, error state, frequencies.

### `/utlidar/switch`

**Type:** `std_msgs/msg/String`

LiDAR enable/disable control (JSON string).

### `/utlidar/mapping_cmd`

**Type:** `std_msgs/msg/String`

Commands to control the LiDAR mapping process.

### `/utlidar/client_cmd`

**Type:** `std_msgs/msg/String`

Client commands to the LiDAR subsystem.

### `/utlidar/server_log`

**Type:** `std_msgs/msg/String`

Log output from the LiDAR processing server.

---

## 3. Navigation & SLAM (uslam)

The onboard autonomous navigation and SLAM system.

### `/uslam/cloud_map`

**Type:** `sensor_msgs/msg/PointCloud2`

Full 3D map point cloud built by the onboard SLAM system.

### `/uslam/frontend/cloud_world_ds`

**Type:** `sensor_msgs/msg/PointCloud2`

Frontend downsampled world point cloud. (Subscribed by robot, external input expected.)

### `/uslam/frontend/odom`

**Type:** `nav_msgs/msg/Odometry`

Frontend odometry input. (Subscribed by robot, external input expected.)

### `/uslam/localization/cloud_world`

**Type:** `sensor_msgs/msg/PointCloud2`

Localized world point cloud. (Subscribed by robot, external input expected.)

### `/uslam/localization/odom`

**Type:** `nav_msgs/msg/Odometry`

Localization odometry. (Subscribed by robot, external input expected.)

### `/uslam/navigation/global_path`

**Type:** `sensor_msgs/msg/PointCloud2`

Global navigation path. (Subscribed by robot, external input expected.)

### `/uslam/client_command`

**Type:** `std_msgs/msg/String`

Commands to the onboard SLAM/navigation system (JSON).

### `/uslam/server_log`

**Type:** `std_msgs/msg/String`

Log output from the SLAM/navigation server.

---

## 4. API Services

All API services follow a **request/response pattern** using `unitree_api/msg/Request` and `unitree_api/msg/Response`.

**Request structure:**

| Field | Type | Description |
|---|---|---|
| `header.identity.id` | `int64` | Unique request ID |
| `header.identity.api_id` | `int64` | API function ID |
| `header.lease.id` | `int64` | Lease token |
| `header.policy.priority` | `int32` | Request priority |
| `header.policy.noreply` | `bool` | If true, no response will be sent |
| `parameter` | `string` | JSON-encoded parameters |
| `binary` | `uint8[]` | Optional binary payload |

**Response structure:**

| Field | Type | Description |
|---|---|---|
| `header.identity.id` | `int64` | Matching request ID |
| `header.identity.api_id` | `int64` | Matching API function ID |
| `header.status.code` | `int32` | Status code |
| `data` | `string` | JSON-encoded response data |
| `binary` | `int8[]` | Optional binary response |

### `/api/sport/request` + `/api/sport/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Sport mode control API. This is the **primary control interface** for high-level robot commands.

**Key API IDs:**

| API ID | Function | Parameters (JSON) |
|---|---|---|
| 1001 | Damp (damping mode) | - |
| 1002 | BalanceStand | - |
| 1003 | StopMove | - |
| 1004 | StandUp | - |
| 1005 | StandDown | - |
| 1006 | RecoveryStand | - |
| 1007 | Euler (body orientation) | `{"x": roll, "y": pitch, "z": yaw}` |
| 1008 | **Move** (velocity command) | `{"x": vx, "y": vy, "z": vyaw}` |
| 1009 | Sit | - |
| 1010 | RiseSit (stand from sit) | - |
| 1015 | SpeedLevel | `{"data": level}` (0, 1, 2) |
| 1016 | Hello (wave) | - |
| 1017 | Stretch | - |
| 1020 | Content (happy) | - |
| 1022 | Dance1 | - |
| 1023 | Dance2 | - |
| 1027 | SwitchJoystick | `{"data": bool}` |
| 1028 | Pose | `{"data": bool}` |
| 1029 | Scrape | - |
| 1030 | FrontFlip | - |
| 1031 | FrontJump | - |
| 1032 | FrontPounce | - |
| 1036 | Heart (heartbeat) | - |
| 1061 | StaticWalk | - |
| 1062 | TrotRun | - |
| 1063 | EconomicGait | - |
| 2041 | LeftFlip | - |
| 2043 | BackFlip | - |
| 2044 | HandStand | `{"data": bool}` |
| 2045 | FreeWalk | - |
| 2046 | FreeBound | `{"data": bool}` |
| 2047 | FreeJump | `{"data": bool}` |
| 2048 | FreeAvoid | `{"data": bool}` |
| 2049 | ClassicWalk | `{"data": bool}` |
| 2050 | WalkUpright | `{"data": bool}` |
| 2051 | CrossStep | `{"data": bool}` |
| 2054 | AutoRecoverySet | `{"data": bool}` |
| 2055 | AutoRecoveryGet | - |
| 2058 | SwitchAvoidMode | - |

**Example - Send Move command:**
```
Request:
  header.identity.api_id = 1008
  parameter = '{"x": 0.3, "y": 0.0, "z": 0.1}'
```
Where x = forward velocity (m/s), y = lateral velocity (m/s), z = yaw angular velocity (rad/s).

### `/api/sport_lease/request` + `/api/sport_lease/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Sport mode lease management. Required for exclusive control of the robot in some configurations.

### `/api/obstacles_avoid/request` + `/api/obstacles_avoid/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Obstacle avoidance AI control.

| API ID | Function |
|---|---|
| 1001 | SwitchSet - enable/disable obstacle avoidance `{"enable": bool}` |
| 1002 | SwitchGet - query obstacle avoidance state |
| 1003 | Move - move with obstacle avoidance `{"x": vx, "y": vy, "yaw": vyaw, "mode": 0}` |
| 1004 | UseRemoteCommandFromApi `{"is_remote_commands_from_api": bool}` |

### `/api/robot_state/request` + `/api/robot_state/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Robot state query API. Used to get service states and configurations.

### `/api/vui/request` + `/api/vui/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

VUI (Voice User Interface) control. Brightness, volume, etc.

### `/api/videohub/request` + `/api/videohub/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Video hub control. Start/stop video streams.

### `/api/audiohub/request` + `/api/audiohub/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Audio hub control. Play/stop audio.

### `/api/config/request` + `/api/config/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Robot configuration API.

### `/api/motion_switcher/request` + `/api/motion_switcher/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Motion mode switching (between sport mode and other motion modes).

### `/api/gpt/request` + `/api/gpt/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

GPT/LLM integration API for AI-powered commands.

### `/api/gesture/request`

**Type:** `unitree_api/msg/Request`

Gesture recognition control.

### `/api/pet/request` + `/api/pet/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Pet mode (follow, interactive behavior) control.

### `/api/assistant_recorder/request` + `/api/assistant_recorder/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Voice assistant recorder control.

### `/api/bashrunner/request` + `/api/bashrunner/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Remote bash command execution (developer/advanced use).

### `/api/programming_actuator/request` + `/api/programming_actuator/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Programming actuator interface for custom motion sequences.

### `/api/fourg_agent/request` + `/api/fourg_agent/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

4G connectivity agent control.

### `/api/gas_sensor/request` + `/api/gas_sensor/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

Gas sensor interface (if sensor module installed).

### `/api/uwbswitch/request` + `/api/uwbswitch/response`

**Type:** `unitree_api/msg/Request` / `unitree_api/msg/Response`

UWB (Ultra-Wideband) positioning switch control.

---

## 5. Video & Audio

### `/frontvideostream`

**Type:** `unitree_go/msg/Go2FrontVideoData`

Raw front camera H.264 video stream. Contains:
- `time_frame`: Timestamp
- `resolution`: Video resolution (e.g., 360)
- `data[]`: H.264 encoded frame data

**Alternative access:** Camera is also available via UDP multicast at `230.1.1.1:1720` (H.264 stream, use GStreamer to decode).

### `/audiosender`

**Type:** `unitree_go/msg/AudioData`

Audio data sent by the robot (microphone).
- `time_frame`: Timestamp
- `data[]`: Audio sample data

### `/audioreceiver`

**Type:** `unitree_go/msg/AudioData`

Audio data received by the robot (speaker). Write to this to play audio.
- `time_frame`: Timestamp
- `data[]`: Audio sample data

### `/audiohub/player/state`

**Type:** `std_msgs/msg/String`

Audio player state as JSON: `{"play_state": "not_in_use", "is_playing": false, "current_audio_unique_id": "", "current_audio_custom_name": ""}`

### `/videohub/inner`

**Type:** `std_msgs/msg/String`

Internal video hub status (JSON).

---

## 6. Controller & Input

### `/wirelesscontroller`

**Type:** `unitree_go/msg/WirelessController`

Processed wireless controller (joystick) state.

| Field | Type | Description |
|---|---|---|
| `lx` | `float32` | Left stick X axis [-1.0 to 1.0] |
| `ly` | `float32` | Left stick Y axis [-1.0 to 1.0] |
| `rx` | `float32` | Right stick X axis [-1.0 to 1.0] |
| `ry` | `float32` | Right stick Y axis [-1.0 to 1.0] |
| `keys` | `uint16` | Button bitmask |

### `/wirelesscontroller_unprocessed`

**Type:** `unitree_go/msg/WirelessController`

Raw wireless controller data before processing.

---

## 7. System & Status

### `/multiplestate`

**Type:** `std_msgs/msg/String`

Aggregated system state as JSON: `{"brightness": 0, "obstaclesAvoidSwitch": true, "uwbSwitch": true, "volume": 8}`

### `/servicestate`

**Type:** `std_msgs/msg/String`

Service state information (JSON).

### `/servicestateactivate`

**Type:** `std_msgs/msg/String`

Service activation status.

### `/selftest`

**Type:** `std_msgs/msg/String`

Self-test diagnostic results.

### `/rtc/state`

**Type:** `std_msgs/msg/String`

WebRTC connection state as JSON: `{"connection_state": "not_connected"}`

### `/rtc_status`

**Type:** `std_msgs/msg/String`

WebRTC status information.

### `/public_network_status`

**Type:** `std_msgs/msg/String`

Network status as JSON: `{"network_status": "NetworkStatus.ON_WIFI_CONNECTED"}`

### `/config_change_status`

**Type:** `unitree_go/msg/ConfigChangeStatus`

Configuration change notification.

### `/uwbstate`

**Type:** `unitree_go/msg/UwbState`

UWB (Ultra-Wideband) positioning state. Contains tag orientation, distance, joystick data.

### `/uwbswitch`

**Type:** `unitree_go/msg/UwbSwitch`

UWB switch state.

### `/gnss`

**Type:** `std_msgs/msg/String`

GNSS/GPS data (if module installed).

### `/gas_sensor`

**Type:** `std_msgs/msg/String`

Gas sensor data (if module installed).

---

## 8. Low-Level Control

### `/lowcmd`

**Type:** `unitree_go/msg/LowCmd`

Low-level motor command. **Write to this topic** to control individual motors.
Uses the same structure as `LowState` but with `MotorCmd` instead of `MotorState`.

| Field | Type | Description |
|---|---|---|
| `level_flag` | `uint8` | Control level |
| `motor_cmd[0..19]` | `MotorCmd[20]` | Motor commands (12 active) |

**MotorCmd per motor:**

| Field | Type | Description |
|---|---|---|
| `mode` | `uint8` | 0x01 = FOC (active), 0x00 = stop |
| `q` | `float32` | Target position (radians) |
| `dq` | `float32` | Target velocity (rad/s) |
| `tau` | `float32` | Target torque (N.m) |
| `kp` | `float32` | Position gain |
| `kd` | `float32` | Velocity gain |

---

## 9. Arm Control

### `/arm_Command`

**Type:** `unitree_arm/msg/ArmString`

Commands to the robotic arm (if Z1 arm attached). JSON string payload.

### `/arm_Feedback`

**Type:** `unitree_arm/msg/ArmString`

Feedback from the robotic arm.

---

## 10. WebRTC

Topics for WebRTC signaling (used when connecting remotely over WiFi/4G instead of wired CycloneDDS).

### `/webrtcreq` + `/webrtcres`

**Type:** `std_msgs/msg/String`

WebRTC request and response signaling. JSON-encoded SDP/ICE data.

### `/xfk_webrtcreq` + `/xfk_webrtcres`

**Type:** `std_msgs/msg/String`

Secondary WebRTC signaling channel (typically for video stream).

---

## 11. Misc / Internal

### `/gesture/result`

**Type:** `std_msgs/msg/String`

Gesture recognition result.

### `/gpt_cmd`

**Type:** `std_msgs/msg/String`

GPT AI command output.

### `/gptflowfeedback`

**Type:** `std_msgs/msg/String`

GPT flow execution feedback.

### `/pet/flowfeedback`

**Type:** `std_msgs/msg/String`

Pet mode behavior flow feedback.

### `/programming_actuator/command`

**Type:** `std_msgs/msg/String`

Programming actuator commands.

### `/programming_actuator/feedback`

**Type:** `std_msgs/msg/String`

Programming actuator feedback.

### `/qt_command`

**Type:** `unitree_interfaces/msg/QtCommand`

Qt UI command.

### `/qt_add_node`

**Type:** `unitree_interfaces/msg/QtNode`

Add node to Qt UI graph.

### `/qt_add_edge`

**Type:** `unitree_interfaces/msg/QtEdge`

Add edge to Qt UI graph.

### `/qt_notice`

**Type:** `std_msgs/msg/String`

Qt UI notification.

### `/query_result_node`

**Type:** `unitree_interfaces/msg/QtNode`

Qt node query result.

### `/query_result_edge`

**Type:** `unitree_interfaces/msg/QtEdge`

Qt edge query result.

### `/pctoimage_local`

**Type:** `unitree_interfaces/msg/PcToImage`

Point cloud to image conversion interface.

---

## Quick Reference: Topics for SLAM + Nav2

| What You Need | Topic | Type | Bridge Required? |
|---|---|---|---|
| Odometry | `/utlidar/robot_odom` | `nav_msgs/Odometry` | Just remap to `/odom` |
| TF odom->base_link | - | - | **Yes**: broadcast from `/utlidar/robot_odom` |
| IMU | `/utlidar/imu` | `sensor_msgs/Imu` | Just remap to `/imu/data` |
| LiDAR (3D) | `/utlidar/cloud_deskewed` | `sensor_msgs/PointCloud2` | No (pass to aggregator) |
| Laser scan (2D) | - | `sensor_msgs/LaserScan` | **Yes**: `pointcloud_to_laserscan` node |
| Velocity command | `/cmd_vel` (Twist) | `geometry_msgs/Twist` | **Yes**: convert to `/api/sport/request` (API 1008) |
| Static TF base->lidar | - | - | `static_transform_publisher` |
| Robot description | - | URDF | `robot_state_publisher` + xacro |
| Motor states | `/lf/lowstate` | `unitree_go/LowState` | Optional: convert to `JointState` |

---

## Topic Frequencies Summary

| Topic | Approx. Frequency |
|---|---|
| `/lf/sportmodestate` | 20 Hz |
| `/sportmodestate` | ~500 Hz |
| `/lf/lowstate` | 20 Hz |
| `/lowstate` | ~500 Hz |
| `/utlidar/cloud` | ~15 Hz |
| `/utlidar/cloud_deskewed` | ~15 Hz |
| `/utlidar/robot_odom` | ~150 Hz |
| `/utlidar/robot_pose` | ~150 Hz |
| `/utlidar/imu` | ~250 Hz |
| `/wirelesscontroller` | Event-driven |
| `/frontvideostream` | ~20 Hz |
| API request/response | On demand |
