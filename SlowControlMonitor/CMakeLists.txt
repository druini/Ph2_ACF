if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR}) 

MESSAGE(STATUS " ")
MESSAGE(STATUS "    ${BoldYellow}========================================================================================================${Reset}")
MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/miniDAQ/CMakeLists.txt${Reset}]. ${BoldRed}Begin...${Reset}")
MESSAGE(STATUS " ")

# Includes
include_directories(${PROJECT_SOURCE_DIR}/NetworkUtils)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Library dirs
link_directories(${UHAL_UHAL_LIB_PREFIX})
link_directories(${PROJECT_SOURCE_DIR/lib})

# Initial set of libraries
set(LIBS ${LIBS} NetworkUtils boost_regex)

# Find root and link against it
if(${ROOT_FOUND})
    include_directories(${ROOT_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${ROOT_LIBRARIES})
    if(NoDataShipping)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{UseRootFlag}")
    endif()

    #check for THttpServer
    if(${ROOT_HAS_HTTP})
        set(LIBS ${LIBS} ${ROOT_RHTTP_LIBRARY})
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{HttpFlag}")
    endif()
endif()

####################################
## EXECUTABLES
####################################

# SlowControlMonitorController
add_executable(SlowControlMonitorController SlowControlMonitorController.cc SlowControlMonitorInterface.cc)
target_link_libraries(SlowControlMonitorController ${LIBS})


# file(GLOB BINARIES RELATIVE ${PROJECT_SOURCE_DIR}/miniDAQ *.cc)

# message("--     ${BoldCyan}#### Building the following executables: ####${Reset}")
# foreach( sourcefile ${BINARIES} )
#     string(REPLACE ".cc" "" name ${sourcefile})
#     message(STATUS "    ${name}")
# endforeach(sourcefile ${BINARIES})
# message("--     ${BoldCyan}#### End ####${Reset}")

MESSAGE(STATUS " ")
MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/miniDAQ/CMakeLists.txt${Reset}]. ${BoldGreen}DONE!${Reset}")
MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}")
MESSAGE(STATUS " ")

else() # ------------------------------- Compilation in the otsdaq environment ---------------------------------------------

    MESSAGE(STATUS " ")
    MESSAGE(STATUS "${BoldBlue}========================================================================================================${Reset}")
    MESSAGE(STATUS "${BoldYellow}MIDDLEWARE${Reset} [otsdaq]: Compilation of the middleware package for OTSDAQ")
    MESSAGE(STATUS "${BoldYellow}MIDDLEWARE${Reset} [otsdaq]: [${BoldCyan}${PH2ACF_BASE_DIR}/CMakeLists.txt${Reset}]")
    MESSAGE(STATUS " ")
    
    MESSAGE(STATUS "NOT DOING ANYTHING!!!")
    MESSAGE(STATUS "NOT DOING ANYTHING!!!")
    MESSAGE(STATUS "NOT DOING ANYTHING!!!")

    MESSAGE(STATUS " ")
    MESSAGE(STATUS "${BoldYellow}MIDDLEWARE${Reset} [otsdaq]: [${BoldCyan}${PH2ACF_BASE_DIR}/CMakeLists.txt${Reset}]")
    MESSAGE(STATUS "${BoldYellow}MIDDLEWARE${Reset} [otsdaq]: Compilation of the middleware package done.")
    MESSAGE(STATUS "${BoldBlue}========================================================================================================${Reset}")
    MESSAGE(STATUS " ")

endif()
