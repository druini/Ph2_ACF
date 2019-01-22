if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})
cmake_minimum_required(VERSION 2.8)
project(Ph2_ACF)

#set the output directory
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

# ---------- Setup output Directories -------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY 
    ${PROJECT_SOURCE_DIR}/lib 
    CACHE PATH
   "Single Directory for all Libraries")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
   ${PROJECT_SOURCE_DIR}/bin
   CACHE PATH
   "Single Directory for all Executables."
   )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
   ${PROJECT_SOURCE_DIR}/bin
   CACHE PATH
   "Single Directory for all static libraries."
   )
# ---------- Setup output Directories -------------------------


#set the cmakeL module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# compiler flags
set (CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wcpp -pthread -pedantic -Wall -w -g -fPIC ${CMAKE_CXX_FLAGS}")

#check for external dependences
message("#### Checking for external Dependencies ####")
#ROOT
find_package(ROOT COMPONENTS RHTTP)
if(ROOT_FOUND)
    #check for THttpServer
    if(EXISTS ${ROOT_RHTTP_LIBRARY})
        message(STATUS "Found THttp Server support - enabling compiler flags")
        set(ROOT_HAS_HTTP TRUE)
    elseif()
        message(STATUS "ROOT built without THttp Server - disabling compiler flags")
    endif()
    message(STATUS "Using ROOT version ${ROOT_VERSION}")
endif(ROOT_FOUND)

find_package(CACTUS)
if(${CACTUS_AMC13_FOUND})
    message(STATUS "    enabling AMC13 component")
else(${CACTUS_AMC13_FOUND})
    message(STATUS "    continuing without AMC13 components")
endif(${CACTUS_AMC13_FOUND})

find_package(PH2_ANTENNA)
if(PH2_ANTENNA_FOUND)
    message(STATUS "    building the Antenna components")
else(PH2_ANTENNA_FOUND)
    message(STATUS "    continuing without Antenna components")
endif(PH2_ANTENNA_FOUND)

#ZeroMQ optional
find_package(ZMQ)
if(ZMQ_FOUND)
    #message(STATUS "Found ZMQ Library - checking for Ph2_UsbInstLib")

    #libPh2_UsbInstLib
    find_package(PH2_USBINSTLIB)
    if(PH2_USBINSTLIB_FOUND)
        #message(STATUS "Found Ph2_UsbInstLib - enabling support")
    endif(PH2_USBINSTLIB_FOUND)
endif(ZMQ_FOUND)

#Boost
file(GLOB_RECURSE uhal_boost /opt/cactus/*version.hpp)
if(uhal_boost)
    MESSAGE(STATUS "Found boost installation that comes with uHAL, using this version") 
	set(Boost_NO_SYSTEM_PATHS TRUE)
	if(Boost_NO_SYSTEM_PATHS)
	    if(${CACTUS_FOUND})
	        set(BOOST_ROOT ${CACTUS_ROOT})
	    else(${CACTUS_FOUND})
	        set(BOOST_ROOT /opt/cactus)
	    endif(${CACTUS_FOUND})
	    set(BOOST_INCLUDE_DIRS ${BOOST_ROOT}/include)
	    set(BOOST_LIBRARY_DIRS ${BOOST_ROOT}/lib)
	endif(Boost_NO_SYSTEM_PATHS)
else(uhal_boost)
    MESSAGE(STATUS "No boost headers found with uHAL (you are most likely using CC7) - make sure to install boost 1.53 (default version on CC7)")
endif(uhal_boost)

find_package(Boost 1.53 REQUIRED system filesystem thread program_options)
#find_package(Boost 1.53 REQUIRED filesystem system-mt )

message("#### Done ####")

#all the subdirs
if(${CACTUS_AMC13_FOUND})
    add_subdirectory(AMC13)
endif()

    add_subdirectory(tools)
    add_subdirectory(RootWeb)
    add_subdirectory(src)
    add_subdirectory(miniDAQ)
    add_subdirectory(Utils)
    add_subdirectory(HWDescription)
    add_subdirectory(HWInterface)
    add_subdirectory(System)

message("#### Don't forget to set the path and ld_library_path by sourcing setup.sh before running")
#set(ENV{PATH} "$ENV{PATH}:${PROJECT_SOURCE_DIR}/bin")
#set(ENV{LD_LIBRARY_PATH} "${PROJECT_SOURCE_DIR}/lib:$ENV(LD_LIBRARY_PATH)")

else()
add_subdirectory(Utils)
add_subdirectory(HWDescription)
add_subdirectory(HWInterface)
add_subdirectory(System)
add_subdirectory(tools)
endif()

