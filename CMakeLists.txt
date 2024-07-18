cmake_minimum_required(VERSION 3.0)
project(SensorDataProcessing)

# Set the path to the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

# Add the installation prefix of ZeroMQ
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows)

# Find the ZeroMQ package
find_package(ZeroMQ CONFIG REQUIRED)

# Define executables
add_executable(publisher publisher.c)
add_executable(data_storage data_storage.c)
add_executable(anomaly_detection anomaly_detection.c)

find_library(ZMQ_LIBRARY NAMES libzmq-mt-4_3_5 HINTS "${PROJECT_SOURCE_DIR}/lib")

target_link_libraries(publisher PRIVATE libzmq libzmq-mt-4_3_5)
target_link_libraries(data_storage PRIVATE libzmq libzmq-mt-4_3_5)
target_include_directories(data_storage PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(anomaly_detection PRIVATE libzmq libzmq-mt-4_3_5)
target_include_directories(anomaly_detection PRIVATE ${PROJECT_SOURCE_DIR}/include)

install(TARGETS anomaly_detection data_storage publisher DESTINATION launch_bin) 