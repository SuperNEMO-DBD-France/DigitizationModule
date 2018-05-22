//test_signal_to_tp_process.cxx

// Standard libraries :
#include <iostream>

// GSL:
#include <bayeux/mygsl/rng.h>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>

// Falaise:
#include <falaise/falaise.h>

// This project :
#include <snemo/digitization/fldigi.h>
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/signal_data.h>
#include <snemo/digitization/tracker_feb_process.h>
#include <snemo/digitization/electronic_mapping.h>
#include <snemo/digitization/mapping.h>

int main( int  argc_ , char ** argv_ )
{
  falaise::initialize(argc_, argv_);
  snemo::digitization::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    std::clog << "Test program for class 'snemo::digitization::tracker_feb_process' !" << std::endl;
    int32_t seed = 314159;
    mygsl::rng random_generator;
    random_generator.initialize(seed);

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

    snemo::digitization::clock_utils my_clock_manager;
    my_clock_manager.initialize();
    my_clock_manager.compute_clockticks_ref(random_generator);

    std::string geiger_feb_mapping_filename = "@fldigi:config/snemo/demonstrator/simulation/digitization/0.1/feast_channel_mapping.csv";
    datatools::fetch_path_with_env(geiger_feb_mapping_filename);
    std::clog << "Geiger FEB mapping filename = " << geiger_feb_mapping_filename << std::endl;

    int module_number = 0;
    datatools::properties elec_config;
    elec_config.store_string("feast_channel_mapping", geiger_feb_mapping_filename);
    elec_config.store("module_number", module_number);

    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.initialize(elec_config);

    datatools::things ER;

    snemo::digitization::tracker_feb_process tracker_feb_process;
    // tracker_feb_process.initialize(my_e_mapping, my_clock_manager);

    const geomtools::geom_id GID1(1210, 0, 0, 3, 106);
    const geomtools::geom_id GID2(1210, 0, 0, 6, 95);
    const geomtools::geom_id GID3(1210, 0, 0, 5, 57);
    const double anode_avalanche_time1 = 1200 * CLHEP::nanosecond;
    const double anode_avalanche_time2 = 850 * CLHEP::nanosecond;
    const double anode_avalanche_time3 = 4500 * CLHEP::nanosecond;

    snemo::digitization::signal_data signal_data;
    snemo::digitization::geiger_signal & my_gg_signal = signal_data.add_geiger_signal();
    my_gg_signal.set_header(0, GID1);
    my_gg_signal.set_anode_avalanche_time(anode_avalanche_time1);

    snemo::digitization::geiger_signal & my_gg_signal2 = signal_data.add_geiger_signal();
    my_gg_signal2.set_header(1, GID2);
    my_gg_signal2.set_anode_avalanche_time(anode_avalanche_time2);

    snemo::digitization::geiger_signal & my_gg_signal3 = signal_data.add_geiger_signal();
    my_gg_signal3.set_header(3, GID3);
    my_gg_signal3.set_anode_avalanche_time(anode_avalanche_time3);


    std::clog << "DEBUG : size of signal data : " << signal_data.get_geiger_signals().size() << std::endl;
    snemo::digitization::geiger_tp_data my_geiger_tp_data;

    if( signal_data.has_geiger_signals())
      {
	// tracker_feb_process.trigger_process(signal_data, my_geiger_tp_data);
	my_geiger_tp_data.tree_dump(std::clog, "Geiger TP(s) data : ", "INFO : ");
      }

    std::clog << "The end." << std::endl;
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
