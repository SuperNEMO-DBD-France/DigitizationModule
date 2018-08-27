# - List of test programs:
set(FalaiseDigitizationPlugin_DEVEL_PROGRAMS
  produce_geiger_sd_modified.cxx
  produce_self_trigger_hits.cxx
  produce_self_trigger_SD.cxx
  test_geiger_self_trigger.cxx
  # test_tracker_trigger_algorithm_load_from_file.cxx
  trigger_algorithm_efficiency_analysis.cxx
  trigger_algorithm_efficiency_validation.cxx
  trigger_board_implementation.cxx
  trigger_program.cxx
  trigger_program_on_signals.cxx
  #  thesis_trigger_program.cxx
  trigger_emul_from_channels.cxx
  )

# Use C++11
#set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ROOT_INCLUDE_DIR})

# DEBUG :  message( STATUS "ROOT_LIBRARIES='${ROOT_LIBRARIES}'")

#${ROOT_INCLUDES})
foreach(_progsource ${FalaiseDigitizationPlugin_DEVEL_PROGRAMS})
  get_filename_component(_progname ${_progsource} NAME_WE)
  set(_progname "falaise-digitization-plugin-devel-${_progname}")
  add_executable(${_progname} ${_progsource})
  target_link_libraries(${_progname} Falaise_Digitization
    ${ROOT_LIBRARIES}
    Falaise::Falaise)
  # - On Apple, ensure dynamic_lookup of undefined symbols
  if(APPLE)
    set_target_properties(${_progname} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  endif()
  # - For now, dump them into the testing output directory
  set_target_properties(${_progname}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fl-digitization-devel/
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fl-digitization-devel/
    )

endforeach()

# end of CMakeLists.txt
