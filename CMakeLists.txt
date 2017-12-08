cmake_minimum_required (VERSION 2.8.2)
project(RayTracerVR)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

include(ExternalProject)
option(VRPN "If enabled, VRPN plugin will be installed (NEEDED FOR CAVE)" OFF)
option(OPENVR "If enabled, OPENVR plugin will be installed (NEEDED FOR VIVE)" OFF)
ExternalProject_Add(
    MinVR
    GIT_REPOSITORY https://github.com/MinVR/MinVR.git
    PREFIX ${CMAKE_SOURCE_DIR}/ext/MinVR
	UPDATE_COMMAND ""
	CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DMINVR_VRPN_PLUGIN=${VRPN} -DMINVR_OPENVR_PLUGIN=${OPENVR}
)

set(GLEW_CPP ${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/third-party/GLEW/glew.c)
set_source_files_properties(${GLEW_CPP} PROPERTIES GENERATED TRUE)

file(GLOB SRC_FILES src/*.cpp)
set(SRC_FILES
	${SRC_FILES}
	${GLEW_CPP}
)

configure_file(${CMAKE_SOURCE_DIR}/config/config.h.in ${CMAKE_SOURCE_DIR}/src/include/config.h)
include_directories(
	ext/
	src/
)

include_directories(src/)
include_directories(ext/)
include_directories(${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/src/)
include_directories(${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/third-party/GLEW/)

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})
add_dependencies(${PROJECT_NAME} MinVR)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(LIB_FILE ${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/build/install/lib/libMinVR.a)
else()
	set(LIB_FILE ${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/build/install/lib/MinVR.lib)
endif()
target_link_libraries(
    ${PROJECT_NAME}
	${LIB_FILE}
    ${OPENGL_gl_LIBRARY}
	${CMAKE_DL_LIBS}
)
