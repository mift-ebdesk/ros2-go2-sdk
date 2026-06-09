# Go2 Controller - Joystick Mapping

## Mode X

### Axes

| Index | Control | Range |
|-------|---------|-------|
| 0 | Left stick roll | -1 to 1 |
| 1 | Left stick pitch | -1 (forward) to 1 (backward) |
| 2 | L2 (analog trigger) | -1 to 1 |
| 3 | Right stick roll | -1 (strafe right) to 1 (strafe left) |
| 4 | Right stick pitch | -1 to 1 |
| 5 | R2 (analog trigger) | -1 to 1 |
| 6 | Arrow left/right | -1 (right) to 1 (left) |
| 7 | Arrow up/down | -1 to 1 |

### Buttons

| Index | Button | Action |
|-------|--------|--------|
| 0 | A | StandDown (sit) |
| 1 | B | Content (happy pose) |
| 2 | X | Hello (wave) |
| 3 | Y | StandUp (stand) |
| 4 | L1 | - |
| 5 | R1 | - |
| 6 | Select | Disable joystick (nav takes over) |
| 7 | Start | Enable joystick (override nav) |
| 8 | Unknown | - |
| 9 | L3 | - |
| 10 | R3 | - |

### ROS Config (Mode X)

```yaml
# teleop_twist_joy
axis_linear:
  x: 1
  y: 3
scale_linear:
  x: 0.5
  y: 0.5
axis_angular:
  yaw: 6
scale_angular:
  yaw: 1.0
```

```cpp
// go2_cmd_vel_bridge.cpp
buttons[7]  // Start  → Enable joystick
buttons[6]  // Select → Disable joystick
buttons[3]  // Y → StandUp
buttons[0]  // A → StandDown
buttons[2]  // X → Hello
buttons[1]  // B → Content
```

---

## Mode D

### Axes

| Index | Control | Range |
|-------|---------|-------|
| 0 | Left stick roll | -1 to 1 |
| 1 | Left stick pitch | -1 (forward) to 1 (backward) |
| 2 | Right stick roll | -1 (strafe right) to 1 (strafe left) |
| 3 | Right stick pitch | -1 to 1 |
| 4 | Arrow left/right | -1 (right) to 1 (left) |
| 5 | Arrow up/down | -1 to 1 |

### Buttons

| Index | Button | Action |
|-------|--------|--------|
| 0 | X | - |
| 1 | A | StandDown (sit) |
| 2 | B | - |
| 3 | Y | StandUp (stand) |
| 4 | L1 | - |
| 5 | R1 | - |
| 6 | L2 | Disable joystick (nav takes over) |
| 7 | R2 | Enable joystick (override nav) |
| 8 | Select | - |
| 9 | Start | - |
| 10 | L3 | - |
| 11 | R3 | - |

### ROS Config (Mode D - NOT TESTED YET)

```yaml
# teleop_twist_joy
axis_linear:
  x: 1
  y: 2
scale_linear:
  x: 0.5
  y: 0.5
axis_angular:
  yaw: 4
scale_angular:
  yaw: 1.0
```

```cpp
// go2_cmd_vel_bridge.cpp (Mode D button indices)
buttons[7]  // R2     → Enable joystick
buttons[6]  // L2     → Disable joystick
buttons[3]  // Y → StandUp
buttons[1]  // A → StandDown
```

---

## Control Reference

| Function | Mode X | Mode D |
|----------|--------|--------|
| Move forward/backward | Left stick pitch (axis 1) | Left stick pitch (axis 1) |
| Strafe left/right | Right stick roll (axis 3) | Right stick roll (axis 2) |
| Yaw left/right | Arrow keys (axis 6) | Arrow keys (axis 4) |
| Stand up | Y (button 3) | Y (button 3) |
| Sit down | A (button 0) | A (button 1) |
| Hello (wave) | X (button 2) | X (button 0) |
| Content (happy) | B (button 1) | B (button 2) |
| Enable joystick | Start (button 7) | R2 (button 7) |
| Disable joystick | Select (button 6) | L2 (button 6) |

---

## Joystick Toggle Behavior

| State | Default at launch | Press Start | Press Select |
|-------|-------------------|-------------|--------------|
| Joystick | Disabled | Enabled | Disabled |
| Navigation | Active | Paused | Active |

- Start/Select are edge-triggered (press once, not hold)
- Zero velocity is sent once on each state transition for safety
- A/B/X buttons work regardless of joystick toggle state
