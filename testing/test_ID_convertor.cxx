// Standard libraries :
#include <iostream>
#include <exception>
#include <cstdlib>

// Third party:
// - Bayeux/datatools:
#include <datatools/logger.h>
#include <datatools/io_factory.h>
// - Bayeux/geomtools:
#include <geomtools/manager.h>
// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/digitization/fldigi.h>
#include <snemo/digitization/calo_tp.h>
#include <snemo/digitization/geiger_tp.h>
#include <snemo/digitization/ID_convertor.h>
#include <snemo/digitization/mapping.h>


int main(int argc_, char ** argv_)
{
  falaise::initialize(argc_, argv_);
  snemo::digitization::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;
  try {

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

    std::string geiger_feb_mapping_filename = "@fldigi:config/snemo/demonstrator/simulation/digitization/0.1/feast_channel_mapping.csv";
    datatools::fetch_path_with_env(geiger_feb_mapping_filename);
    std::clog << "GG FEB mapping filename = " << geiger_feb_mapping_filename << std::endl;


    // std::vector<std::string> only_categories;
    // only_categories.push_back ("module");
    // only_categories.push_back ("drift_cell_core");
    // only_categories.push_back ("xcalo_block");
    // only_categories.push_back ("gveto_block");
    // only_categories.push_back ("calorimeter_block");
    // only_categories.push_back ("calorimeter_optical_module");
    // set the 'only' property:
    // manager_config.update ("mapping.only_categories", only_categories);
    my_manager.initialize (manager_config);

    snemo::digitization::ID_convertor my_convertor;
    my_convertor.set_geo_manager(my_manager);
    my_convertor.set_module_number(0);
    my_convertor.set_geiger_feb_mapping_file(geiger_feb_mapping_filename);
    my_convertor.initialize(manager_config);

    std::map<geomtools::geom_id, geomtools::geom_id> gid_eid_feb_map = my_convertor.get_geiger_feb_mapping();

    for (auto it=gid_eid_feb_map.begin(); it!=gid_eid_feb_map.end(); it++)
      {
	std::cout << it->first << " => " << it->second << std::endl;
      }

    geomtools::geom_id GID;
    GID.set_type(snemo::digitization::mapping::GEIGER_ANODIC_CATEGORY_TYPE);
    int module = 0;
    int side = 0;
    int layer = 2;
    int row = 4;
    GID.set_address(module, side, layer, row);

    geomtools::geom_id EID;
    EID = my_convertor.convert_GID_to_EID(GID);
    std::clog <<" Anodic GID : " << GID << " <=> EID : " << EID << std::endl;

    int part = 0; // bottom
    geomtools::geom_id GID2(snemo::digitization::mapping::GEIGER_CATHODIC_CATEGORY_TYPE,
			    module,
			    side,
			    layer,
			    row,
			    part);
    EID = my_convertor.convert_GID_to_EID(GID2);
    std::clog <<" Cathodic GID : " << GID2 << " <=> EID : " << EID << std::endl;



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
