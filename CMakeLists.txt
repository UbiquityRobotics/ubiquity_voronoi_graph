cmake_minimum_required(VERSION 3.0.2)
project(ubiquity_voronoi_graph)

## Compile as C++14
add_compile_options(-std=c++14)
add_compile_options(-Wall)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  geometry_msgs
  nav_msgs
  message_generation
  dynamic_reconfigure
  cv_bridge
  tf
  rviz
)

## System dependencies are found with CMake's conventions
find_package(Boost COMPONENTS serialization filesystem program_options system REQUIRED)
find_package(Eigen3 REQUIRED)

set(_opencv_version 4)
find_package(OpenCV 4 QUIET)
if(NOT OpenCV_FOUND)
  message(STATUS "Did not find OpenCV 4, trying OpenCV 3")
  set(_opencv_version 3)
endif()

find_package(OpenCV ${_opencv_version} REQUIRED
  COMPONENTS
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
  CONFIG
)

find_library(DXF_LIB dxflib)
if(NOT DXF_LIB)
  message(WARNING "dxf library not found")
  message(WARNING "run 'sudo apt install libdxflib-dev' to install the dxf library")
endif()

################################################
## Declare ROS messages, services and actions ##
################################################

## Generate messages in the 'msg' folder
add_message_files(
  FILES
  Vertex.msg
  Graph.msg
)

## Generate added messages and services with any dependencies listed here
generate_messages(
  DEPENDENCIES
  std_msgs
  geometry_msgs
)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if your package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS include
#  LIBRARIES ubiquity_voronoi_graph
  CATKIN_DEPENDS roscpp std_msgs geometry_msgs nav_msgs message_runtime dynamic_reconfigure cv_bridge tf
#  DEPENDS OpenCV
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR} 
  ${Boost_INCLUDE_DIRS}
)

link_directories(${catkin_LIBRARY_DIRS})


## This setting causes Qt's "MOC" generation to happen automatically.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

## This plugin includes Qt widgets, so we must include Qt.
## We'll use the version that rviz used so they are compatible.
if(rviz_QT_VERSION VERSION_LESS "5")
  message(STATUS "Using Qt4 based on the rviz_QT_VERSION: ${rviz_QT_VERSION}")
  find_package(Qt4 ${rviz_QT_VERSION} EXACT REQUIRED QtCore QtGui)
  ## pull in all required include dirs, define QT_LIBRARIES, etc.
  include(${QT_USE_FILE})
else()
  message(STATUS "Using Qt5 based on the rviz_QT_VERSION: ${rviz_QT_VERSION}")
  find_package(Qt5 ${rviz_QT_VERSION} EXACT REQUIRED Core Widgets)
  ## make target_link_libraries(${QT_LIBRARIES}) pull in all required dependencies
  set(QT_LIBRARIES Qt5::Widgets)
endif()

## I prefer the Qt signals and slots to avoid defining "emit", "slots",
## etc because they can conflict with boost signals, so define QT_NO_KEYWORDS here.
add_definitions(-DQT_NO_KEYWORDS)



## Declare a C++ library
add_library(graph_serialization_lib
  src/serializer.cpp
)
target_link_libraries(graph_serialization_lib 
${catkin_LIBRARIES} 
${OpenCV_LIBRARIES} 
${Boost_SERIALIZATION_LIBRARY}
)

add_library(voronoi_path_lib
  src/voronoi_path_generator.cpp
  src/thinning.cpp
)
target_link_libraries(voronoi_path_lib 
${catkin_LIBRARIES} 
${OpenCV_LIBRARIES}
)

add_library(voronoi_graph_lib
  src/voronoi_graph_generator.cpp
  src/segment.cpp
  src/segment_expander.cpp
  src/crossing.cpp
)
target_link_libraries(voronoi_path_lib graph_serialization_lib voronoi_graph_lib 
${catkin_LIBRARIES} 
${OpenCV_LIBRARIES}
)

add_library(ubiquity_voronoi_rviz_lib
  include/ubiquity_voronoi_rviz/VoronoiGraphVisual.h
  include/ubiquity_voronoi_rviz/VoronoiGraphDisplay.h
  src/ubiquity_voronoi_rviz/VoronoiGraphVisual.cpp
  src/ubiquity_voronoi_rviz/VoronoiGraphDisplay.cpp
  include/ubiquity_voronoi_rviz/VoronoiSegmentVisual.h
  include/ubiquity_voronoi_rviz/VoronoiSegmentDisplay.h
  src/ubiquity_voronoi_rviz/VoronoiSegmentVisual.cpp
  src/ubiquity_voronoi_rviz/VoronoiSegmentDisplay.cpp
)
target_link_libraries(ubiquity_voronoi_rviz_lib 
${QT_LIBRARIES} 
${catkin_LIBRARIES} 
${Boost_LIBRARIES}
)


## Add cmake target dependencies of the library
add_dependencies(voronoi_path_lib ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
add_dependencies(ubiquity_voronoi_rviz_lib ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Declare a C++ executable
add_executable(ubiquity_voronoi_graph_node src/voronoi_graph_node.cpp)
target_link_libraries(ubiquity_voronoi_graph_node voronoi_path_lib)
add_dependencies(ubiquity_voronoi_graph_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})