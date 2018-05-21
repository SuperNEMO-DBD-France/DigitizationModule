//test_bimap.cxx

// Standard libraries :
#include <iostream>

// Boost :
#include <boost/bimap.hpp>
// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>

// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/digitization/fldigi.h>
#include <snemo/digitization/mapping.h>
#include <snemo/digitization/electronic_mapping.h>

int main( int  argc_ , char ** argv_  )
{
  falaise::initialize(argc_, argv_);
  snemo::digitization::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;
  try {
    std::clog << "Test program for class 'snemo::digitization::test_bimap' !" << std::endl;

    std::string manager_config_file;

    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
    datatools::fetch_path_with_env (manager_config_file);
    datatools::properties manager_config;
    datatools::properties::read_config (manager_config_file,
					manager_config);
    geomtools::manager my_manager;

    manager_config.update ("build_mapping", true);
    if (manager_config.has_key ("mapping.excluded_categories"))
      {
	manager_config.erase ("mapping.excluded_categories");
      }
    my_manager.initialize (manager_config);


    std::string geiger_feb_mapping_filename = "@fldigi:config/snemo/demonstrator/simulation/digitization/0.1/feast_channel_mapping.csv";
    datatools::fetch_path_with_env(geiger_feb_mapping_filename);
    std::clog << "GG FEB mapping filename = " << geiger_feb_mapping_filename << std::endl;

    int module_number = 0;
    datatools::properties elec_config;
    elec_config.store_string("feast_channel_mapping", geiger_feb_mapping_filename);
    elec_config.store("module_number", module_number);

    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.initialize(elec_config);


    const geomtools::geom_id GID1(1210, 0, 0, 3, 25);
    const geomtools::geom_id GID2(1210, 0, 0, 6, 30);
    const geomtools::geom_id GID3(1210, 0, 0, 5, 35);
    const geomtools::geom_id GID4(1210, 0, 0, 5, 44);
    const geomtools::geom_id GID5(1210, 0, 0, 5, 33);

    geomtools::geom_id EID1;
    geomtools::geom_id EID2;
    geomtools::geom_id EID3;
    geomtools::geom_id EID4;
    geomtools::geom_id EID5;


    my_e_mapping.convert_GID_to_EID(snemo::digitization::mapping::THREE_WIRES_TRACKER_MODE, GID1, EID1);
    std::clog <<"GID1 : "<<  GID1;
    std::clog <<" ---> EID1 : "<<  EID1<<std::endl;

    my_e_mapping.convert_GID_to_EID(snemo::digitization::mapping::THREE_WIRES_TRACKER_MODE, GID2, EID2);
    std::clog <<"GID2 : "<<  GID2;
    std::clog <<" ---> EID2 : "<<  EID2<<std::endl;

    my_e_mapping.convert_GID_to_EID(snemo::digitization::mapping::THREE_WIRES_TRACKER_MODE, GID3, EID3);
    std::clog <<"GID3 : "<<  GID3;
    std::clog <<" ---> EID3 : "<<  EID3<<std::endl;

    my_e_mapping.convert_GID_to_EID(snemo::digitization::mapping::THREE_WIRES_TRACKER_MODE, GID4, EID4);
    std::clog <<"GID4 : "<<  GID4;
    std::clog <<" ---> EID4 : "<<  EID4<<std::endl;

    my_e_mapping.convert_GID_to_EID(snemo::digitization::mapping::THREE_WIRES_TRACKER_MODE, GID5, EID5);
    std::clog <<"GID5 : "<<  GID5;
    std::clog <<" ---> EID5 : "<<  EID5<<std::endl;

  }
  catch (std::exception & error) {
    DT_LOG_FATAL(logging, error.what());
    error_code = EXIT_FAILURE;
  }

  catch (...) {
    DT_LOG_FATAL(logging, "Unexpected error!");
    error_code = EXIT_FAILURE;
  }

  snemo::digitization::terminate();
  falaise::terminate();
  return error_code;
}
