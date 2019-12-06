if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})

    MESSAGE(STATUS " ") 
    MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}") 
    MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/HWDescription/CMakeLists.txt${Reset}]. ${BoldRed}Begin...${Reset}")
    MESSAGE(STATUS " ") 

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    #find source files
    file(GLOB SOURCES *.cc)
    add_library(Ph2_Description STATIC ${SOURCES})

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

    #include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
    #include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
    #include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
    #include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsinnertracker/Ph2_ACF)
    #include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)

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


