cmake_minimum_required (VERSION 2.8.2)
project(RayTracerVR)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include(ExternalProject)
ExternalProject_Add(
    MinVR
    GIT_REPOSITORY https://github.com/MinVR/MinVR.git
    PREFIX ${CMAKE_SOURCE_DIR}/ext/MinVR
)

file(GLOB SRC_FILES src/*.cpp)

include_directories(
	ext/
	src/
	${MinVR_INCLUDE_DIR}
)
message(${MinVR_INCLUDE_DIR})
include_directories(src/)
include_directories(ext/)
include_directories(${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/src/)
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif()

find_package(OpenGL REQUIRED)
find_package(GLUT)
find_package(MinVR REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(
    ${PROJECT_NAME}
	${MinVR_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${GLUT_LIBRARIES}
	${CMAKE_DL_LIBS}
)
#add_executable(RayTracerVR ${SRC_FILES})
