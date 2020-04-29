#
file(GLOB_RECURSE PH2_POWERSUPPLY ${PROJECT_SOURCE_DIR}/../power-supply/src/PowerSupply.cc)
if(PH2_POWERSUPPLY)
      set(PH2_POWERSUPPLY_FOUND TRUE)
      #strip to the blank path
      get_filename_component(PH2_POWERSUPPLY_DIR "${PH2_POWERSUPPLY}" PATH)

endif(PH2_POWERSUPPLY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PH2_PowerSupply DEFAULT_MSG PH2_POWERSUPPLY_DIR)
