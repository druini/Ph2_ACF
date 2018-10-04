#includes
include_directories(${UHAL_UHAL_INCLUDE_PREFIX})
include_directories(${PROJECT_SOURCE_DIR}/HWDescription)
include_directories(${PROJECT_SOURCE_DIR}/HWInterface)
include_directories(${PROJECT_SOURCE_DIR}/Utils)
include_directories(${PROJECT_SOURCE_DIR}/System)
include_directories(${PROJECT_SOURCE_DIR}/tools)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#library dirs
link_directories(${UHAL_UHAL_LIB_PREFIX})
link_directories(${PROJECT_SOURCE_DIR/lib})

#initial set of libraries
set(LIBS ${LIBS} Ph2_Description Ph2_Interface Ph2_Utils Ph2_System Ph2_Tools cactus_uhal_uhal)

#check for ZMQ installed
if(ZMQ_FOUND)
    #here, now check for UsbInstLib
    if(PH2_USBINSTLIB_FOUND)

        #add include directoreis for ZMQ and USBINSTLIB
        include_directories(${PH2_USBINSTLIB_INCLUDE_DIRS})
        link_directories(${PH2_USBINSTLIB_LIBRARY_DIRS})
        include_directories(${ZMQ_INCLUDE_DIRS})

        #and link against the libs
        set(LIBS ${LIBS} ${ZMQ_LIBRARIES} ${PH2_USBINSTLIB_LIBRARIES})
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ZMQ__ -D__USBINST__")
    endif()
endif()

#check for AMC13 libraries
if(${CACTUS_AMC13_FOUND})
    include_directories(${PROJECT_SOURCE_DIR}/AMC13)
    include_directories(${UHAL_AMC13_INCLUDE_PREFIX})
    link_directories(${UHAL_AMC13_LIB_PREFIX})
    set(LIBS ${LIBS} cactus_amc13_amc13 Ph2_Amc13)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__AMC13__")
endif()

#check for AntennaDriver
if(${PH2_ANTENNA_FOUND})
    include_directories(${PH2_ANTENNA_INCLUDE_DIRS})
    link_directories(${PH2_ANTENNA_LIBRARY_DIRS})
    set(LIBS ${LIBS} usb ${PH2_ANTENNA_LIBRARIES})
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ANTENNA__")
endif()

#boost also needs to be linked
if(Boost_FOUND)
    set(LIBS ${LIBS} ${Boost_LIBRARIES})
endif()

#last but not least, find root and link against it
if(${ROOT_FOUND})
    include_directories(${ROOT_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${ROOT_LIBRARIES})

    #check for THttpServer
    if(${ROOT_HAS_HTTP})
        set(LIBS ${LIBS} ${ROOT_RHTTP_LIBRARY})
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__HTTP__")
    endif()
endif()

####################################
## EXECUTABLES
####################################

file(GLOB BINARIES RELATIVE ${PROJECT_SOURCE_DIR}/src *.cc)
#to remove binaries
list(REMOVE_ITEM BINARIES test.cc)

message("#### Building the following executables: ####")
foreach( sourcefile ${BINARIES} )
    string(REPLACE ".cc" "" name ${sourcefile})
    message(STATUS "    ${name}")
    add_executable(${name} ${sourcefile})
    target_link_libraries(${name} ${LIBS})
endforeach(sourcefile ${BINARIES})
message("#### End ####")
