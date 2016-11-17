include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)

cet_set_compiler_flags(
 EXTRA_FLAGS -Wno-reorder -Wl,--undefined
)
 
cet_make(LIBRARY_NAME Ph2_Interface
        LIBRARIES
        Ph2_Description
		pthread
		cactus_uhal_grammars
		cactus_uhal_log
		cactus_uhal_uhal
        )

install_headers()
install_source()
