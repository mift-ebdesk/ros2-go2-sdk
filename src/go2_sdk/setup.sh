#!/bin/bash

echo "Setting up ROS2-go2 environment"
source /opt/ros/humble/setup.bash
source $HOME/Projects/UnitreeGo2/sdk_ws/install/setup.bash
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces>
                            <NetworkInterface name="enp2s0" priority="default" multicast="default" />
                        </Interfaces></General></Domain></CycloneDDS>'
