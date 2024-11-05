cmake_minimum_required(VERSION 3.10)
project(NASA_satellite_data_analysis)

set(CURL_LIBRARY "-lcurl")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCURL_STATICLIB") 
set(CURL_INCLUDE_DIR " ${CMAKE_SOURCE_DIR}/lib/curl/include")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows)

find_package(ZeroMQ CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(PythonLibs 3.12 REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/gtk-3.0
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/atk-1.0
    ${PROJECT_SOURCE_DIR}/lib/gtk/include
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/fribidi
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/cairo
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/gdk-pixbuf-2.0
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/glib-2.0
    ${PROJECT_SOURCE_DIR}/lib/gtk/lib/glib-2.0/include
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/pango-1.0
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/libpng16
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/freetype2
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/pixman-1
    ${PROJECT_SOURCE_DIR}/lib/gtk/include/harfbuzz
	${PYTHON_INCLUDE_DIRS}
)

link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/include)

# Create binaries
add_executable(message_publisher message_publisher.c 
								${PROJECT_SOURCE_DIR}/include/custom.c 
								${PROJECT_SOURCE_DIR}/lib/slre.c) 

add_executable(mongo_app		mongo_app.c 
								${PROJECT_SOURCE_DIR}/include/custom.c 
								${PROJECT_SOURCE_DIR}/lib/slre.c
								${PROJECT_SOURCE_DIR}/include/db_connector.c )

add_executable(satellite_viewer WIN32 satellite_viewer.c
								${PROJECT_SOURCE_DIR}/include/db_connector.c
								${PROJECT_SOURCE_DIR}/include/custom.c 
								${PROJECT_SOURCE_DIR}/include/analyzer_core.c
								${PROJECT_SOURCE_DIR}/lib/slre.c)
add_definitions(${GTK3_CFLAGS_OTHER})

set(DLL_FILES_MSG_PUB
    "${PROJECT_SOURCE_DIR}/runtime_libs/message_publisher/libzmq-mt-4_3_5.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/message_publisher/cjson.dll"
)
set(DLL_FILES_MNG_APP
    "${PROJECT_SOURCE_DIR}/runtime_libs/mongo_app/libzmq-mt-4_3_5.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/mongo_app/cjson.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/mongo_app/mongoc-1.0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/mongo_app/bson-1.0.dll"
)
set(DLL_FILES_SAT_VWR
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/mongoc-1.0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/bson-1.0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/asprintf.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/atk-1.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/cairo-2.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/cairo-gobject-2.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/cairo-script-interpreter-2.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/epoxy-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/ffi-8.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/freetype-6.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/fribidi-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gailutil-3-vs17.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gdk-3-vs17.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gdk_pixbuf-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gettextlib-0.21.0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gettextpo.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gettextsrc-0.21.0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gio-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/girepository-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/glib-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gmodule-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gobject-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gthread-2.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/gtk-3-vs17.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/harfbuzz-cairo.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/harfbuzz-gobject.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/harfbuzz-subset.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/harfbuzz.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/iconv.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/intl.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/jpeg62.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/libpng16.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pango-1.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pangocairo-1.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pangowin32-1.0-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pcre2-16-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pcre2-32-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pcre2-8-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pcre2-posix-3.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pixman-1-0.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/pkgconf-5.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/python3.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/python312.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/textstyle.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/tiff.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/turbojpeg.dll"
    "${PROJECT_SOURCE_DIR}/runtime_libs/satellite_viewer/zlib1.dll"
)

# Copy the DLLs to the output directory
foreach(DLL ${DLL_FILES_MSG_PUB})
    add_custom_command(TARGET message_publisher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL}"
        "$<TARGET_FILE_DIR:message_publisher>"
        #COMMENT "Copying DLL: ${DLL}"
    )
endforeach()

foreach(DLL ${DLL_FILES_MNG_APP})
    add_custom_command(TARGET mongo_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL}"
        "$<TARGET_FILE_DIR:mongo_app>"
    )
endforeach()

foreach(DLL ${DLL_FILES_SAT_VWR})
    add_custom_command(TARGET satellite_viewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DLL}"
        "$<TARGET_FILE_DIR:satellite_viewer>"
    )
endforeach()

# Find libs
find_library(CURL_LIBRARY NAMES libcurl_a HINTS "${PROJECT_SOURCE_DIR}/lib/curl")
find_library(ZMQ_LIBRARY NAMES libzmq-mt-4_3_5 HINTS "${PROJECT_SOURCE_DIR}/lib")
find_library(PY_LIBRARY NAMES python312 HINTS "${PROJECT_SOURCE_DIR}/lib")

# MESSAGE PUBLISHER
target_link_libraries(message_publisher PUBLIC libzmq-mt-4_3_5 cjson ws2_32 iphlpapi Normaliz Wldap32 Crypt32 advapi32)
target_include_directories(message_publisher PUBLIC "${PROJECT_SOURCE_DIR}/lib"
												    "${PROJECT_SOURCE_DIR}/lib/curl"
											        "${PROJECT_SOURCE_DIR}/include"
											        "${PROJECT_SOURCE_DIR}/vcpkg/installed/x64-windows/include")

# DATABASE HANDLER
target_link_libraries(mongo_app PUBLIC libzmq-mt-4_3_5 mongoc-1.0 bson-1.0 cjson ws2_32 iphlpapi Normaliz Wldap32 Crypt32 advapi32)
target_include_directories(mongo_app PUBLIC "${PROJECT_SOURCE_DIR}/lib" 
												   "${PROJECT_SOURCE_DIR}/lib/curl"
												   "${PROJECT_SOURCE_DIR}/lib/mongo-c-driver-master/src/libmongoc/src/mongoc"
												   "${PROJECT_SOURCE_DIR}/lib/mongo-c-driver-master/src/libbson/src/bson"
												   "${PROJECT_SOURCE_DIR}/include"
                                                   "${PROJECT_SOURCE_DIR}/vcpkg/installed/x64-windows/include")

# MESSAGE ANALYZER
target_link_libraries(satellite_viewer PUBLIC cjson mongoc-1.0 bson-1.0 ws2_32 iphlpapi Normaliz Wldap32 Crypt32 advapi32 
    gtk-3.lib gdk-3.lib gdi32.lib imm32.lib shell32.lib ole32.lib winmm.lib dwmapi.lib setupapi.lib cfgmgr32.lib hid.lib 
    winspool.lib comctl32.lib comdlg32.lib atk-1.0.lib gio-2.0.lib pangowin32-1.0.lib pangocairo-1.0.lib gdk_pixbuf-2.0.lib 
    cairo-gobject.lib pango-1.0.lib harfbuzz.lib cairo.lib gobject-2.0.lib glib-2.0.lib intl.lib ${PYTHON_LIBRARIES})

target_include_directories(satellite_viewer PUBLIC "${PROJECT_SOURCE_DIR}/lib" 
												   "${PROJECT_SOURCE_DIR}/lib/curl" 
												   "${PROJECT_SOURCE_DIR}/lib/mongo-c-driver-master/src/libmongoc/src/mongoc"
												   "${PROJECT_SOURCE_DIR}/lib/mongo-c-driver-master/src/libbson/src/bson"
												   "${PROJECT_SOURCE_DIR}/include"
												   "${PROJECT_SOURCE_DIR}/include/python")

# Misc										       
if(MSVC)
    set_target_properties(satellite_viewer PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()	

				   