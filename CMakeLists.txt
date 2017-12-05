cmake_minimum_required (VERSION 2.8.2)
project(RayTracerVR)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include(ExternalProject)
ExternalProject_Add(
    MinVR
    GIT_REPOSITORY https://github.com/MinVR/MinVR.git
    PREFIX ${CMAKE_SOURCE_DIR}/ext/MinVR
	CMAKE_ARGS -DMINVR_VRPN_PLUGIN=ON
)

file(GLOB SRC_FILES src/*.cpp)
set(SRC_FILES
	${SRC_FILES}
	${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/third-party/GLEW/glew.c
)
message(${SRC_FILES})
include_directories(
	ext/
	src/
	#${MinVR_INCLUDE_DIR}
)

include_directories(src/)
include_directories(ext/)
include_directories(${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/src/)
include_directories(${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/third-party/GLEW/)

#find_package(GLEW REQUIRED)
#if (GLEW_FOUND)
#    include_directories(${GLEW_INCLUDE_DIRS})
#    link_libraries(${GLEW_LIBRARIES})
#endif()

find_package(OpenGL REQUIRED)
#find_package(GLUT)
#find_package(MinVR REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})
add_dependencies(${PROJECT_NAME} MinVR)

#${MinVR_LIBRARIES}
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
