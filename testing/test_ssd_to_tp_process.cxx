// test_ssd_to_tp_process.cxx

// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
// - Bayeux/mctools:
#include <mctools/signal/signal_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
// - Bayeux/mygsl:
#include <bayeux/mygsl/rng.h>

// Falaise:
#include <falaise/falaise.h>

// Third part :
// Boost :
#include <boost/program_options.hpp>

// This project :
#include <snemo/digitization/fldigi.h>
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/signal_to_calo_tp_algo.h>
#include <snemo/digitization/signal_to_geiger_tp_algo.h>

int main(int argc_, char** argv_)
{
  falaise::initialize(argc_, argv_);
  snemo::digitization::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  std::string input_filename = "";
  int max_events = 0;

  // Parse options:
  namespace po = boost::program_options;
  po::options_description opts("Allowed options");
  opts.add_options()
    ("help,h", "produce help message")
    ("input,i",
     po::value<std::string>(& input_filename),
     "set an input file")
    ("event_number,n",
     po::value<int>(& max_events)->default_value(10),
     "set the maximum number of events")
    ; // end of options description

  // Describe command line arguments :
  po::variables_map vm;
  po::store(po::command_line_parser(argc_, argv_)
	    .options(opts)
	    .run(), vm);
  po::notify(vm);

  // Use command line arguments :
  if (vm.count("help")) {
    std::cout << "Usage : " << std::endl;
    std::cout << opts << std::endl;
    return(error_code);
  }

  try {
    std::clog << "Test program for class 'snemo::digitization::ssd_to_tp_process' !" << std::endl;
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

    std::string pipeline_simulated_data_filename;
    std::string SSD_bank_label = "SSD";
    std::string tracker_signal_category = "sigtracker";
    std::string calo_signal_category = "sigcalo";

    if(!input_filename.empty()){
      pipeline_simulated_data_filename = input_filename;
    }else{
      pipeline_simulated_data_filename = "${FALAISE_DIGI_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SSD_10_events.brio";
    }

    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store ("logging.priority", "debug");
    reader_config.store ("max_record_total", max_events);
    reader_config.store ("files.mode", "single");
    reader_config.store ("files.single.filename", pipeline_simulated_data_filename);
    reader.initialize_standalone (reader_config);
    reader.tree_dump (std::clog, "Simulated data reader module");

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

    snemo::digitization::clock_utils my_clock_manager;
    my_clock_manager.initialize();

    mctools::signal::signal_shape_builder gg_ssb;
    gg_ssb.set_logging_priority(datatools::logger::PRIO_DEBUG);
    gg_ssb.set_category("sigtracker");
    gg_ssb.add_registered_shape_type_id("mctools::signal::triangle_signal_shape");
    gg_ssb.add_registered_shape_type_id("mctools::signal::triangle_gate_signal_shape");
    gg_ssb.add_registered_shape_type_id("mctools::signal::multi_signal_shape");
    gg_ssb.initialize_simple();
    // gg_ssb.tree_dump(std::clog, "Geiger signal shape builder", "[info] ");

    mctools::signal::signal_shape_builder calo_ssb;
    calo_ssb.set_logging_priority(datatools::logger::PRIO_DEBUG);
    calo_ssb.set_category("sigcalo");
    calo_ssb.add_registered_shape_type_id("mctools::signal::triangle_signal_shape");
    calo_ssb.add_registered_shape_type_id("mctools::signal::multi_signal_shape");
    calo_ssb.initialize_simple();
    // calo_ssb.tree_dump(std::clog, "Calorimeter signal shape builder", "[info] ");

    datatools::properties algos_config;

    int psd_count = 0;
    while (!reader.is_terminated())
      {
	reader.process(ER);
	// A plain `mctools::simulated_data' object is stored here :
	if (ER.has(SSD_bank_label) && ER.is_a<mctools::signal::signal_data>(SSD_bank_label))
	  {
	    // Access to the "SD" bank with a stored `mctools::simulated_data' :
	    const mctools::signal::signal_data & SSD = ER.get<mctools::signal::signal_data>(SSD_bank_label);
	    SSD.tree_dump(std::clog, "SSD");

	    my_clock_manager.compute_clockticks_ref(random_generator);

	    snemo::digitization::signal_to_calo_tp_algo signal_2_calo_tp;
	    signal_2_calo_tp.initialize(algos_config,
					my_clock_manager,
					my_e_mapping,
					calo_ssb);

	    snemo::digitization::signal_to_geiger_tp_algo signal_2_geiger_tp;
	    signal_2_geiger_tp.initialize(algos_config,
					  my_clock_manager,
					  my_e_mapping,
					  gg_ssb);

	    snemo::digitization::geiger_tp_data my_geiger_tp_data;
	    snemo::digitization::calo_tp_data my_calo_tp_data;

	    if (SSD.has_signals(tracker_signal_category))
	      {
		mctools::signal::base_signal a_signal = SSD.get_signal(tracker_signal_category, 0);
		// a_signal.tree_dump(std::clog, "A GG signal");

	    	signal_2_geiger_tp.process(SSD, my_geiger_tp_data);
		my_geiger_tp_data.tree_dump(std::clog, "Geiger TP(s) data : ", "INFO : ");
	      }

	    if (SSD.has_signals(calo_signal_category))
	      {
		mctools::signal::base_signal a_signal = SSD.get_signal(calo_signal_category, 0);
		a_signal.tree_dump(std::clog, "A Calo signal");

		// signal_2_calo_tp.process(SSD, my_calo_tp_data);
		// my_calo_tp_data.tree_dump(std::clog, "Calorimeter TP(s) data : ", "INFO : ");


		// Create fake calo signals :




	      }
	    std::clog << std::endl;
	  }
	// CF README.RST pour display graphique avec loader de manager.conf
	// -> /home/guillaume/data/Bayeux/Bayeux-trunk/source/bxmctools/examples/ex00
	ER.clear();

	psd_count++;
	std::clog << "DEBUG : psd count " << psd_count << std::endl;
	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);
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
