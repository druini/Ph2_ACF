if(NOT DEFINED ENV{OTSDAQ_CMSOUTERTRACKER_DIR})


MESSAGE(STATUS " ")
MESSAGE(STATUS "    ${BoldYellow}========================================================================================================${Reset}") 
MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/NetworkUtils/CMakeLists.txt${Reset}]. ${BoldRed}Begin...${Reset}") 
MESSAGE(STATUS " ")

#includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#find source files
file(GLOB SOURCES *.cc)
add_library(Ph2_NetworkUtils STATIC ${SOURCES})


message("--     ${BoldCyan}#### End ####${Reset}")

MESSAGE(STATUS " ")
MESSAGE(STATUS "    ${BoldYellow}MIDDLEWARE${Reset} [stand-alone/middleware]: [${BoldCyan}Ph2_ACF/NetworkUtils/CMakeLists.txt${Reset}]. ${BoldGreen}DONE!${Reset}") 
MESSAGE(STATUS "    ${BoldBlue}========================================================================================================${Reset}") 
MESSAGE(STATUS " ")

else()

#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/uhal/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/log/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/uhal/grammars/include)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/otsdaq-cmsoutertracker/Ph2_ACF/RootUtils)
#include_directories($ENV{OTSDAQ_CMSOUTERTRACKER_DIR}/uhal/uhal_2_4_2/cactuscore/extern/pugixml/RPMBUILD/SOURCES/include/)

cet_set_compiler_flags(
 EXTRA_FLAGS -Wno-reorder -Wl,--undefined -D__OTSDAQ__
)
 
cet_make(LIBRARY_NAME Ph2_NetworkUtils_${Ph2_ACF_Master}
        LIBRARIES
        )

install_headers()
install_source()
endif()


