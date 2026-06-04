cmake_minimum_required(VERSION 3.10)
project(go2_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(unitree_go REQUIRED)
find_package(unitree_api REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)

add_library(go2_sport_client SHARED
  src/go2_sport_client.cpp
)
target_include_directories(go2_sport_client PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(go2_sport_client rclcpp unitree_api nlohmann_json)

add_executable(go2_driver_node
  src/main.cpp
  src/go2_driver_node.cpp
  src/go2_tf_broadcaster.cpp
  src/go2_joint_state.cpp
  src/go2_cmd_vel_bridge.cpp
)
target_include_directories(go2_driver_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(go2_driver_node
  rclcpp nav_msgs sensor_msgs geometry_msgs tf2 tf2_ros unitree_go unitree_api nlohmann_json
)
target_link_libraries(go2_driver_node go2_sport_client)

add_executable(go2_gstreamer
  src/go2_gstreamer_main.cpp
  src/go2_gstreamer.cpp
)
target_include_directories(go2_gstreamer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(go2_gstreamer rclcpp sensor_msgs)
target_link_libraries(go2_gstreamer ${OpenCV_LIBS})

add_executable(go2_gstreamer_jetson
  src/go2_gstreamer_jetson_main.cpp
  src/go2_gstreamer_jetson.cpp
)
target_include_directories(go2_gstreamer_jetson PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(go2_gstreamer_jetson rclcpp sensor_msgs)
target_link_libraries(go2_gstreamer_jetson ${OpenCV_LIBS})

install(TARGETS go2_driver_node go2_sport_client go2_gstreamer go2_gstreamer_jetson
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include)

install(DIRECTORY config/ DESTINATION share/${PROJECT_NAME}/config)
install(DIRECTORY rviz/ DESTINATION share/${PROJECT_NAME}/rviz)
install(DIRECTORY scripts/
  DESTINATION lib/${PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
)

ament_package()
