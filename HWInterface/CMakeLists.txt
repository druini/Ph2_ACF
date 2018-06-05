if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})
    #includes
    include_directories(${UHAL_UHAL_INCLUDE_PREFIX})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/HWDescription)
    include_directories(${PROJECT_SOURCE_DIR})
    
#replace this with find_package
    link_directories(${UHAL_UHAL_LIB_PREFIX})
    
    #find source files
    file(GLOB HEADERS *.h)
    file(GLOB SOURCES *.cc)
    
    #add the library
    add_library(Ph2_Interface SHARED ${SOURCES} ${HEADERS})
    set(LIBS ${LIBS} Ph2_Description cactus_uhal_uhal )
    TARGET_LINK_LIBRARIES(Ph2_Interface ${LIBS})
    
else()

    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)
    
    cet_set_compiler_flags(
     EXTRA_FLAGS -Wno-reorder -Wl,--undefined
    )
     
    cet_make(LIBRARY_NAME Ph2_Interface_${Ph2_ACF_Master}
            LIBRARIES
            Ph2_Description_${Ph2_ACF_Master}
            #pthread
            cactus_uhal_uhal
            )
    
    install_headers()
    install_source()

endif()
