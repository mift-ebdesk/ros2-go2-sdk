# Unitree Go2 Pro - Complete API Reference

All APIs use `unitree_api/msg/Request` and `unitree_api/msg/Response` via DDS topics.
Protocol: CycloneDDS, `RMW_IMPLEMENTATION=rmw_cyclonedds_cpp`.

---

## Sport API (`/api/sport/request`)

Primary high-level control interface for the Go2 Pro.

### Movement & Control

| API ID | Function | Parameters | Implemented |
|--------|----------|------------|:-----------:|
| 1001 | Damp (motors compliant) | - | Yes |
| 1002 | BalanceStand | - | Yes |
| 1003 | StopMove | - | Yes |
| 1004 | StandUp | - | Yes |
| 1005 | StandDown (sit) | - | Yes |
| 1006 | RecoveryStand | - | Yes |
| 1007 | Euler (body orientation) | `{"x": roll, "y": pitch, "z": yaw}` | Yes |
| 1008 | **Move** (velocity command) | `{"x": vx, "y": vy, "z": vyaw}` | Yes |
| 1009 | Sit | - | No |
| 1010 | RiseSit (stand from sit) | - | No |
| 1015 | SpeedLevel | `{"data": 0\|1\|2}` | Yes |
| 1027 | SwitchJoystick | `{"data": bool}` | Yes |
| 1036 | Heart (heartbeat) | - | Yes |

### Animation & Dance

| API ID | Function | Parameters | Implemented |
|--------|----------|------------|:-----------:|
| 1016 | Hello (wave) | - | Yes |
| 1017 | Stretch | - | No |
| 1020 | Content (happy) | - | Yes |
| 1022 | Dance1 | - | No |
| 1023 | Dance2 | - | No |
| 1028 | Pose | `{"data": bool}` | No |
| 1029 | Scrape | - | No |

### Stunt

| API ID | Function | Parameters | Implemented |
|--------|----------|------------|:-----------:|
| 1030 | FrontFlip | - | No |
| 1031 | FrontJump | - | No |
| 1032 | FrontPounce | - | No |
| 2041 | LeftFlip | - | No |
| 2043 | BackFlip | - | No |
| 2044 | HandStand | `{"data": bool}` | No |

### Gait Modes

| API ID | Function | Parameters | Implemented |
|--------|----------|------------|:-----------:|
| 1061 | StaticWalk | - | No |
| 1062 | TrotRun | - | No |
| 1063 | EconomicGait | - | No |
| 2045 | FreeWalk | - | Yes |
| 2049 | ClassicWalk | `{"data": bool}` | No |
| 2050 | WalkUpright | `{"data": bool}` | No |
| 2051 | CrossStep | `{"data": bool}` | No |

### Advanced

| API ID | Function | Parameters | Implemented |
|--------|----------|------------|:-----------:|
| 2046 | FreeBound | `{"data": bool}` | No |
| 2047 | FreeJump | `{"data": bool}` | No |
| 2048 | FreeAvoid | `{"data": bool}` | No |
| 2054 | AutoRecoverySet | `{"data": bool}` | No |
| 2055 | AutoRecoveryGet | - | No |
| 2058 | SwitchAvoidMode | - | Yes |

---

## Obstacle Avoidance API (`/api/obstacles_avoid/request`)

| API ID | Function | Parameters | Implemented |
|--------|----------|------------|:-----------:|
| 1001 | SwitchSet | `{"enable": bool}` | No |
| 1002 | SwitchGet | - | No |
| 1003 | Move (with avoidance) | `{"x": vx, "y": vy, "yaw": vyaw, "mode": 0}` | No |
| 1004 | UseRemoteCommandFromApi | `{"is_remote_commands_from_api": bool}` | No |

---

## System APIs

| Service | Endpoint | Description |
|---------|----------|-------------|
| Sport Lease | `/api/sport_lease/request` | Exclusive control lease management |
| Robot State | `/api/robot_state/request` | Query service states and configurations |
| VUI | `/api/vui/request` | Voice UI - brightness, volume control |
| Video Hub | `/api/videohub/request` | Start/stop video streams |
| Audio Hub | `/api/audiohub/request` | Play/stop audio |
| Config | `/api/config/request` | Robot configuration |
| Motion Switcher | `/api/motion_switcher/request` | Switch between sport and other motion modes |
| GPT | `/api/gpt/request` | AI-powered commands via LLM |
| Gesture | `/api/gesture/request` | Gesture recognition control |
| Pet Mode | `/api/pet/request` | Follow and interactive behavior |
| Assistant Recorder | `/api/assistant_recorder/request` | Voice assistant recorder |
| Bash Runner | `/api/bashrunner/request` | Remote bash execution (developer) |
| Programming Actuator | `/api/programming_actuator/request` | Custom motion sequences |
| 4G Agent | `/api/fourg_agent/request` | 4G connectivity control |
| Gas Sensor | `/api/gas_sensor/request` | Gas sensor interface |
| UWB Switch | `/api/uwbswitch/request` | Ultra-Wideband positioning switch |

---

## Sport Mode States

Values reported by `mode` field in `SportModeState`:

| Value | Mode |
|-------|------|
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

## Gait Types

Values reported by `gait_type` field in `SportModeState`:

| Value | Gait |
|-------|------|
| 0 | Idle |
| 1 | Trot |
| 2 | Run (trot running) |
| 3 | Climb stair |
| 4 | Forward down stair |
| 9 | Adjust |
