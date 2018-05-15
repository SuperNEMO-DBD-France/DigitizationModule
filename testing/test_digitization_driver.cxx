// test_digitization_driver.cxx

// Standard libraries :
#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <cstdio>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/temporary_files.h>
#include <datatools/clhep_units.h>
// - Bayeux/mctools:
#include <mctools/signal/signal_data.h>

// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>
// - Bayeux/mygsl:
#include <mygsl/parameter_store.h>
#include <mygsl/i_unary_function_with_derivative.h>

// - Bayeux/geomtools:
#include <geomtools/geomtools_config.h>
#include <geomtools/gnuplot_draw.h>
#if GEOMTOOLS_WITH_GNUPLOT_DISPLAY == 1
#include <geomtools/gnuplot_i.h>
#include <geomtools/gnuplot_drawer.h>
#endif // GEOMTOOLS_WITH_GNUPLOT_DISPLAY

// Boost :
#include <boost/program_options.hpp>

// Falaise:
#include <falaise/falaise.h>
#include <falaise/snemo/datamodels/sim_digi_data.h>

// This project :
#include <snemo/digitization/fldigi.h>
#include <snemo/digitization/digitization_driver.h>

struct params_type
{
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;
  bool draw = false;
  int number_of_events = -1;
  std::string input_filename = "";
  std::string config_filename = "";
  std::string output_path = "";
};

void test_driver_1(const params_type &);

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  snemo::digitization::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  try {
    std::clog << "Test program for class 'snemo::asb::analog_signal_builder_module'!" << std::endl;

    // Parameters:
    params_type params;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h", "produce help message")
      ("draw,D", "draw option")
      ("input,i",
       po::value<std::string>(& params.input_filename),
       "set an input file")
      ("config,c",
       po::value<std::string>(& params.config_filename),
       "set a configuration file")
      ("number_of_events,n",
       po::value<int>(& params.number_of_events)->default_value(5),
       "set the maximum number of events")
      ("output-path,o",
       po::value<std::string>(& params.output_path),
       "set an output path")
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

    // Use command line arguments :
    if (vm.count("draw")) {
      params.draw = true;
    }

    std::clog << "First test..." << std::endl;
    test_driver_1(params);

    // std::clog << "Next test..." << std::endl;
    // test_driver_2(params);

    std::clog << "The end." << std::endl;

  } catch (std::exception & error) {
    std::cerr << "error: " << error.what () << std::endl;
    error_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "error: " << "Unexpected error!" << std::endl;
    error_code = EXIT_FAILURE;
  }
  snemo::digitization::terminate();
  falaise::terminate();
  return (error_code);
}

void test_driver_1(const params_type & params_)
{
  std::clog << "[info] test_driver_1..." << std::endl;

  std::string SSD_bank_label   = "SSD"; // Simulated Signal Data "SSD" bank label
  std::string SDD_bank_label   = "SDD"; // Simulated Digitized Data "SDD" bank label

  std::string SSD_filename = "";
  if (params_.input_filename.empty()) SSD_filename = "${FALAISE_DIGI_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SSD_10_events.brio";
  else SSD_filename = params_.input_filename;
  datatools::fetch_path_with_env(SSD_filename);

  // Geom manager :
  std::string manager_config_file;
  manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
  datatools::fetch_path_with_env(manager_config_file);
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
  my_manager.tree_dump(std::clog, "My geometry manager");

  // Event reader :
  dpp::input_module reader;
  reader.set_name("InputSSD");
  datatools::properties reader_config;
  reader_config.store("logging.priority", "notice");
  reader_config.store("max_record_total", params_.number_of_events);
  reader_config.store("files.mode", "single");
  reader_config.store("files.single.filename", SSD_filename);
  reader.initialize_standalone(reader_config);

  // Event record :
  datatools::things ER;

  std::string digi_config_filename = "";
  if (params_.config_filename.empty()) digi_config_filename =  "${FALAISE_DIGI_RESOURCE_DIR}/config/snemo/demonstrator/simulation/digitization/0.1/digitization.conf";
  else digi_config_filename = params_.config_filename;
  datatools::fetch_path_with_env(digi_config_filename);

  std::clog << "Digi config filename = " << digi_config_filename << std::endl;

  datatools::properties general_config;
  general_config.read_configuration(digi_config_filename);
  // general_config.tree_dump(std::clog, "General configuration: ", "[info] ");

  std::string digi_config_key = "driver.digitization.config.";
  datatools::properties digi_config;
  general_config.export_and_rename_starting_with(digi_config, digi_config_key, "");
  // digi_config.tree_dump(std::clog, "Digitization configuration: ", "[info] ");

  snemo::digitization::digitization_driver digi_driver;
  digi_driver.set_geometry_manager(my_manager);
  digi_driver.initialize(digi_config);
  digi_driver.tree_dump(std::clog, "Digitization driver");

  std::clog << "Number of events = " << params_.number_of_events << std::endl;

  int psd_count = 0;
  while (!reader.is_terminated())
    {
      std::clog << "DEBUG : EVENT NUMBER #" << psd_count << std::endl;
      reader.process(ER);
      // A plain `mctools::simulated_data' object is stored here :
      if (ER.has(SSD_bank_label) && ER.is_a<mctools::signal::signal_data>(SSD_bank_label))
	{
	  // Access to the "SD" bank with a stored `mctools::simulated_data' :
	  const mctools::signal::signal_data & SSD = ER.get<mctools::signal::signal_data>(SSD_bank_label);
	  SSD.tree_dump(std::clog, "SSD");

	  snemo::datamodel::sim_digi_data SDD;

	  digi_driver.process(SSD, SDD);
	} // end if (ER.has(SSD_bank_label)...)

      ER.clear();

      psd_count++;
      std::clog << "DEBUG : psd count " << psd_count << std::endl;
      DT_LOG_NOTICE(params_.logging, "Simulated Signal data #" << psd_count);
    } // end of reader

  std::clog << std::endl;
  return;
}
