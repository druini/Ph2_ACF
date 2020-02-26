file(GLOB_RECURSE EUDAQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../main/lib/core/include/eudaq/Configuration.hh)
if(EUDAQ_SOURCE_DIR)
         set(EUDAQ_FOUND TRUE)
         #strip to the blank path
         get_filename_component(EUDAQ_SOURCE_DIR "${EUDAQ_SOURCE_DIR}" PATH)

         find_library(EUDAQ_LIBRARY_DIRS
         NAMES
           libeudaq_core.so
         PATHS
         ${EUDAQ_SOURCE_DIR}
         )

         #strip away the path
         get_filename_component(EUDAQ_LIBRARY_DIRS "${EUDAQ_LIBRARY_DIRS}" PATH)
         #strip again to get the path to the Ph2_USBInstDriver directory from the root of the FS
         get_filename_component(EUDAQ_SOURCE_DIR "${EUDAQ_SOURCE_DIR}" PATH)

         set(EUDAQ_INCLUDE_DIRS
             ${EUDAQ_SOURCE_DIR})

         file(GLOB_RECURSE EUDAQ_LIBRARIES ${EUDAQ_LIBRARY_DIRS}/*.so)
else(EUDAQ_SOURCE_DIR)
         set(EUDAQ_FOUND FALSE)
endif(EUDAQ_SOURCE_DIR)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(EUDAQ DEFAULT_MSG EUDAQ_SOURCE_DIR)

  # show the PH2_USBINSTLIB_INCLUDE_DIRS and PH2_USBINSTLIB_LIBRARIES variables only in the advanced view
  mark_as_advanced(EUDAQ_INCLUDE_DIRS EUDAQ_LIBRARY_DIRS)
