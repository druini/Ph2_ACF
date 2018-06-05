if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#find source files
file(GLOB SOURCES *.cc)
add_library(Ph2_Description SHARED ${SOURCES})

else()

#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
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
endif()


