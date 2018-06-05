if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})
    #includes
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${UHAL_UHAL_INCLUDE_PREFIX})
    #include_directories(${EXTERN_PUGIXML_INCLUDE_PREFIX})
    
    #library directories
    link_directories(${UHAL_UHAL_LIB_PREFIX})

    #find source files
    file(GLOB HEADERS *.h)
    file(GLOB SOURCES *.cc)
    #message("Sources " ${SOURCES})
    #but withhold UsbUtilities for the moment until we know we have 
    #Ph2_UsbInstLib installed
    #message("Sources " ${SOURCES})
    
    #add the library
    add_library(Ph2_Utils SHARED ${SOURCES} ${HEADERS})
    set(LIBS ${LIBS} cactus_extern_pugixml)
    TARGET_LINK_LIBRARIES(Ph2_Utils ${LIBS})
    
    #check for ZMQ installed
    if(ZMQ_FOUND)
        #here, now check for UsbInstLib
        if(PH2_USBINSTLIB_FOUND)
            #add include directoreis for ZMQ and USBINSTLIB
            include_directories(${PH2_USBINSTLIB_INCLUDE_DIRS})
            include_directories(${ZMQ_INCLUDE_DIRS})
            link_directories(${PH2_USBINSTLIB_LIBRARY_DIRS})
            #link_directories(${ZMQ_LIBRARIES})
    
            #and link against the libs
            set(LIBS ${LIBS} ${ZMQ_LIBRARIES} ${PH2_USBINSTLIB_LIBRARIES})
            TARGET_LINK_LIBRARIES(Ph2_Utils ${LIBS})
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ZMQ__ -D__USBINST__")
        endif(PH2_USBINSTLIB_FOUND)
    else(ZMQ_FOUND)
            list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/UsbUtilities.cc)
            list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/UsbUtilities.h)
    endif(ZMQ_FOUND)
    
else()

    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
    include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)
    

    cet_set_compiler_flags(
     EXTRA_FLAGS -Wno-reorder -Wl,--undefined
     )
    
    cet_make(LIBRARY_NAME Ph2_Utils_${Ph2_ACF_Master}
            LIBRARIES
    	pthread
    	${Boost_SYSTEM_LIBRARY}
    	EXCLUDE UsbUtilities.h UsbUtilities.cc
            )
    
    install_headers()
    install_source()

endif()
