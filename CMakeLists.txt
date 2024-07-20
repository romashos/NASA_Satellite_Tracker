cmake_minimum_required(VERSION 3.0)
project(SensorDataProcessing)

set(CURL_LIBRARY "-lcurl")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCURL_STATICLIB") 
set(CURL_INCLUDE_DIR " ${CMAKE_SOURCE_DIR}/lib/curl/include")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows)

find_package(ZeroMQ CONFIG REQUIRED)
find_package(CURL REQUIRED)

add_executable(publisher publisher.c)
add_executable(data_storage data_storage.c)
add_executable(anomaly_detection anomaly_detection.c)

find_library(CURL_LIBRARY NAMES libcurl_a HINTS "${PROJECT_SOURCE_DIR}/lib/curl")
find_library(ZMQ_LIBRARY NAMES libzmq-mt-4_3_5 HINTS "${PROJECT_SOURCE_DIR}/lib")

target_link_libraries(publisher PUBLIC ws2_32 iphlpapi Normaliz Wldap32 Crypt32 advapi32)
target_include_directories(publisher PUBLIC "${PROJECT_SOURCE_DIR}/lib")
target_include_directories(publisher PUBLIC "${PROJECT_SOURCE_DIR}/lib/curl")

target_include_directories(data_storage PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(anomaly_detection PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/lib)