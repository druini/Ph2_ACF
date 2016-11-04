#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)

cet_set_compiler_flags(
 EXTRA_FLAGS -Wno-reorder
)
 
cet_make(LIBRARY_NAME Ph2_Description
        LIBRARIES
	Ph2_Utils
#	pthread
#	${Boost_FILESYSTEM_LIBRARY}
#	${Boost_SYSTEM_LIBRARY}
#	${Boost_THREAD_LIBRARY}
#	${Boost_REGEX_LIBRARY}
#	cactus_uhal_grammars
#	cactus_uhal_log
#	$ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/lib/libcactus_extern_pugixml.so
        )

install_headers()
install_source()
#g++ -std=c++11  -g -O1 -Wall -fPIC   -I/opt/cactus/include -I../ -c -o FrontEndDescription.o FrontEndDescription.cc
#g++ -std=c++11  -g -O1 -Wall -fPIC   -I/opt/cactus/include -I../ -c -o BeBoard.o BeBoard.cc
#g++ -std=c++11  -g -O1 -Wall -fPIC   -I/opt/cactus/include -I../ -c -o Cbc.o Cbc.cc
#g++ -std=c++11  -g -O1 -Wall -fPIC   -I/opt/cactus/include -I../ -c -o Module.o Module.cc
#gcc -shared -o libPh2_Description.so FrontEndDescription.o BeBoard.o Cbc.o Module.o 
