if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})

#includes
include_directories(${UHAL_UHAL_INCLUDE_PREFIX})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/RootUtils)
include_directories(${PROJECT_SOURCE_DIR}/HWDescription)
include_directories(${PROJECT_SOURCE_DIR})

#last but not least, find root and link against it
if(${ROOT_FOUND})
    include_directories(${ROOT_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${ROOT_LIBRARIES})
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__NO_USE_ROOT__")

    #check for THttpServer
    if(${ROOT_HAS_HTTP})
        set(LIBS ${LIBS} ${ROOT_RHTTP_LIBRARY})
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__HTTP__")
    endif()
endif()


#find source files
file(GLOB SOURCES *.cc)
add_library(Ph2_DQMUtils SHARED ${SOURCES})

else()

include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF/RootUtils)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)

cet_set_compiler_flags(
 EXTRA_FLAGS -Wno-reorder -Wl,--undefined -D__OTSDAQ__
)
 
cet_make(LIBRARY_NAME Ph2_DQMUtils_${Ph2_ACF_Master}
        LIBRARIES
#    Ph2_Utils_${Ph2_ACF_Master}
#    pthread
        )

install_headers()
install_source()
endif()


