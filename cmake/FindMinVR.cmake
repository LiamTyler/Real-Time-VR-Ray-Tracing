set(MinVR_build ${CMAKE_SOURCE_DIR}/ext/MinVR/src/MinVR/build)

find_path(MinVR_INCLUDE_DIR VRMain.h
          HINTS ${MinVR_build}/install/include/main)

if(MinVR_INCLUDE_DIR)
	set(MinVR_INCLUDE_DIR ${MinVR_INCLUDE_DIR}/..)
endif()

find_library(MinVR_OPT_LIBRARIES NAMES libMinVR.a MinVR.lib MinVR
        HINTS ${MinVR_build} 
        ${MinVR_build}/install/lib
		${MinVR_build}/Release/lib)
          
find_library(MinVR_DEBUG_LIBRARIES NAMES libMinVRd.a MinVRd.lib MinVRd
        HINTS ${MinVR_build} 
        ${MinVR_build}/install/lib
	  	${MinVR_build}/Debug/lib)

if(MinVR_OPT_LIBRARIES AND MinVR_DEBUG_LIBRARIES)
	set(MinVR_OPT_LIBRARIES optimized ${MinVR_OPT_LIBRARIES} )
	set(MinVR_DEBUG_LIBRARIES debug ${MinVR_DEBUG_LIBRARIES} )
#if only opt is found, use it for both
elseif(MinVR_OPT_LIBRARIES AND NOT MinVR_DEBUG_LIBRARIES)
	set(MinVR_DEBUG_LIBRARIES debug ${MinVR_OPT_LIBRARIES} )
	set(MinVR_OPT_LIBRARIES optimized ${MinVR_OPT_LIBRARIES} )
#if only debug is found, use it for both
elseif(NOT MinVR_OPT_LIBRARIES AND MinVR_DEBUG_LIBRARIES)
	set(MinVR_OPT_LIBRARIES optimized ${MinVR_DEBUG_LIBRARIES} )
	set(MinVR_DEBUG_LIBRARIES debug ${MinVR_DEBUG_LIBRARIES} )
endif()

set(MinVR_LIBRARIES ${MinVR_OPT_LIBRARIES} ${MinVR_DEBUG_LIBRARIES})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MinVR  DEFAULT_MSG
                                  MinVR_INCLUDE_DIR MinVR_OPT_LIBRARIES MinVR_DEBUG_LIBRARIES MinVR_LIBRARIES)

mark_as_advanced(MinVR_INCLUDE_DIR MinVR_OPT_LIBRARIES MinVR_DEBUG_LIBRARIES MinVR_LIBRARIES)
