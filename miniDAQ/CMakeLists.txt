MESSAGE(STATUS " ") 
MESSAGE(STATUS "    ${BoldYellow}========================================================================================================${Reset}") 
MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/miniDAQ/CMakeLists.txt${Reset}]. ${BoldRed}Begin...${Reset}") 
MESSAGE(STATUS " ") 

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
set(LIBS ${LIBS} Ph2_Description Ph2_Interface Ph2_Utils Ph2_System)

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
if(ROOT_FOUND)
    include_directories(${ROOT_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${ROOT_LIBRARIES})

    #check for THttpServer
    if(${ROOT_HAS_HTTP})
        set(LIBS ${LIBS} ${ROOT_RHTTP_LIBRARY})
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__HTTP__")
    endif()

    #check ROOT version
    if(${ROOT_VERSION} VERSION_GREATER 5)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ROOT5__")
    else()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ROOT6__")
    endif()
endif()

####################################
## EXECUTABLES
####################################

#CMSIT_miniDAQ
add_executable(CMSIT_miniDAQ CMSIT_miniDAQ.cc)
target_link_libraries(CMSIT_miniDAQ ${LIBS} RootWeb)

#miniDAQ
add_executable(miniDAQ miniDAQ.cc publisher.cc publisher.h ../DQMUtils/DQMEvent.h ../DQMUtils/DQMEvent.cc ../DQMUtils/SLinkDQMHistogrammer.cc ../DQMUtils/SLinkDQMHistogrammer.cc)
target_link_libraries(miniDAQ ${LIBS} RootWeb)

#miniDQM
add_executable(miniDQM miniDQM.cc publisher.cc publisher.h ../DQMUtils/DQMHistogrammer.cc ../DQMUtils/DQMHistogrammer.h)
target_link_libraries(miniDQM ${LIBS} Ph2_Tools RootWeb)

#miniSLinkDQM
add_executable(miniSLinkDQM miniSLinkDQM.cc publisher.cc publisher.h ../DQMUtils/DQMEvent.h ../DQMUtils/DQMEvent.cc ../DQMUtils/SLinkDQMHistogrammer.cc ../DQMUtils/SLinkDQMHistogrammer.h)
target_link_libraries(miniSLinkDQM ${LIBS} RootWeb)

#RunController
add_executable(RunController RunController.cc MiddlewareController.cc)
target_link_libraries(RunController ${LIBS} Ph2_Tools)

#DQMController
add_executable(DQMController DQMController.cc DQMHistogramController.cc)
target_link_libraries(DQMController ${LIBS} Ph2_Tools)


file(GLOB BINARIES RELATIVE ${PROJECT_SOURCE_DIR}/miniDAQ *.cc)

message("--     ${BoldCyan}#### Building the following executables: ####${Reset}")
foreach( sourcefile ${BINARIES} )
    string(REPLACE ".cc" "" name ${sourcefile})
    message(STATUS "    ${name}")
endforeach(sourcefile ${BINARIES})
message("--     ${BoldCyan}#### End ####${Reset}")

MESSAGE(STATUS " ") 
MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/miniDAQ/CMakeLists.txt${Reset}]. ${BoldGreen}DONE!${Reset}") 
MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}") 
MESSAGE(STATUS " ") 

