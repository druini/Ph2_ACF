if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})

    MESSAGE(STATUS " ")
    MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}")
    MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/HWDescription/CMakeLists.txt${Reset}]. ${BoldRed}Begin...${Reset}")
    MESSAGE(STATUS " ")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})


    file(GLOB CXX20_SOURCES *.cpp)
    add_library(Ph2_Description_cxx20 OBJECT ${CXX20_SOURCES})
    target_compile_options(Ph2_Description_cxx20 PRIVATE -std=c++20)

    # Find source files
    file(GLOB SOURCES *.cc)
    add_library(Ph2_Description STATIC ${SOURCES} $<TARGET_OBJECTS:Ph2_Description_cxx20>)

    # target_link_libraries(Ph2_Description cxx20)

    #check for TestCard USBDriver
    #    if(${PH2_TCUSB_FOUND})
    # include_directories(${PH2_TCUSB_INCLUDE_DIRS})
    # link_directories(${PH2_TCUSB_LIBRARY_DIRS})
    # set(LIBS ${LIBS} ${PH2_TCUSB_LIBRARIES} usb)
    # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{TCUSBFlag}")
    #endif()

    # target_precompile_headers(Ph2_Description PRIVATE RD53BEventDecoder.h RD53BFWEvent.h RD53BFWEventDecoder.h)

    ####################################
    ## EXECUTABLES
    ####################################

    file(GLOB BINARIES RELATIVE ${PROJECT_SOURCE_DIR}/HWDescription *.cc)

    message("--     ${BoldCyan}#### Building the following executables: ####${Reset}")
    foreach( sourcefile ${BINARIES} )
        string(REPLACE ".cc" "" name ${sourcefile})
        message(STATUS "    ${name}")
    endforeach(sourcefile ${BINARIES})
    message("--     ${BoldCyan}#### End ####${Reset}")

    MESSAGE(STATUS " ")
    MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/HWDescription/CMakeLists.txt${Reset}]. ${BoldRed}DONE!${Reset}")
    MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}")
    MESSAGE(STATUS " ")

else()

    MESSAGE(STATUS " ")
    MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}")
    MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [otsdaq/middleware]: [${BoldCyan}Ph2_ACF/HWDescription/CMakeLists.txt${Reset}]. ${BoldRed}Begin...${Reset}")
    MESSAGE(STATUS " ")

    cet_set_compiler_flags(
     EXTRA_FLAGS -Wno-reorder -Wl,--undefined
    )

    cet_make(LIBRARY_NAME Ph2_Description_${Ph2_ACF_Master}
             LIBRARIES
             Ph2_Utils_${Ph2_ACF_Master}
             pthread
            )

    install_headers()
    install_source()

    MESSAGE(STATUS " ")
    MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [otsdaq/middleware]: [${BoldCyan}Ph2_ACF/HWDescription/CMakeLists.txt${Reset}]. ${BoldGreen}DONE!${Reset}")
    MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}")
    MESSAGE(STATUS " ")

endif()
