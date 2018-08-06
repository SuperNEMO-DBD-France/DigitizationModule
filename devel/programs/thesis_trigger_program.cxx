// trigger_program.cxx
// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Falaise:
#include <falaise/falaise.h>

// Third part :
// GSL:
#include <bayeux/mygsl/rng.h>

// Boost :
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>

// Falaise:
#include <falaise/falaise.h>
#include <falaise/snemo/datamodels/sim_digi_data.h>

// This project :
#include <snemo/digitization/fldigi.h>
#include <snemo/digitization/digitization_driver.h>

int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_DEBUG;

  try {
    // Parsing arguments
    bool is_display      = false;

    std::vector<std::string> input_filenames;
    std::string config_filename = "";
    std::string output_path = "";
    std::size_t max_events = 0;

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h",    "produce help message")
      ("display,d", "display mode")
      ("trace,t",   "trace mode for debug purpose")
      ("input,i",
       po::value<std::vector<std::string> >(& input_filenames)->multitoken(),
       "set a list of input files")
      ("output-path,o",
       po::value<std::string>(& output_path),
       "set the output path where produced files are created")
      ("config,c",
       po::value<std::string>(& config_filename),
       "set the configuration file")
      ("event_number,n",
       po::value<std::size_t>(& max_events)->default_value(10),
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

    // Use command line arguments :
    else if (vm.count("display")) {
      is_display = true;
    }

    // Use command line arguments :
    else if (vm.count("trace")) {
      logging = datatools::logger::PRIO_TRACE;
    }

    std::clog << "Test program for class 'snemo::digitization::thesis_trigger_program' !" << std::endl;

    if (input_filenames.size() == 0) {
      DT_LOG_WARNING(logging, "No input file(s) !");

      std::string input_default_file = "${FALAISE_DIGI_TESTING_DIR}/data/Se82_0nubb-source_strips_bulk_SSD_10_events.brio";
      input_filenames.push_back(input_default_file);

      DT_LOG_WARNING(logging, "Default input file : " << input_filenames[0] << " !");
    }

    std::size_t file_counter = 0;
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) {
      std::clog << "File #" << file_counter << ' ' << *file << std::endl;
      file_counter++;
    }

    DT_LOG_INFORMATION(logging, "List of input file(s) : ");
    for (auto file = input_filenames.begin();
	 file != input_filenames.end();
	 file++) std::clog << *file << ' ';

    // boolean for debugging (display etc)
    bool debug = false;
    if (is_display) debug = true;

    std::clog << "Test program for class ' !" << std::endl;
    int32_t seed = 314159;
    mygsl::rng random_generator;
    random_generator.initialize(seed);

    // Configure the geometry manager :
    std::string manager_config_file;
    manager_config_file = "@falaise:config/snemo/demonstrator/geometry/4.0/manager.conf";
    datatools::fetch_path_with_env(manager_config_file);
    datatools::properties manager_config;
    datatools::properties::read_config (manager_config_file,
					manager_config);
    geomtools::manager my_manager;
    manager_config.update ("build_mapping", true);
    if (manager_config.has_key ("mapping.excluded_categories")) {
      manager_config.erase ("mapping.excluded_categories");
    }
    my_manager.initialize (manager_config);

    // Set the default output path :
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    // Simulated  bank label :
    std::string SD_bank_label = "SD";
    std::string SSD_bank_label   = "SSD"; // Simulated Signal Data "SSD" bank label
    std::string SDD_bank_label   = "SDD"; // Simulated Digitized Data "SDD" bank label

    // Event record :
    datatools::things ER;


    std::string digi_config_filename = "";
    if (config_filename.empty()) digi_config_filename =  "${FALAISE_DIGI_RESOURCE_DIR}/config/snemo/demonstrator/simulation/digitization/0.1/digitization.conf";
    else digi_config_filename = config_filename;
    datatools::fetch_path_with_env(digi_config_filename);

    datatools::properties general_config;
    general_config.read_configuration(digi_config_filename);
    // general_config.tree_dump(std::clog, "General configuration: ", "[info] ");

    std::string digi_config_key = "driver.digitization.config.";
    datatools::properties digi_config;
    general_config.export_and_rename_starting_with(digi_config, digi_config_key, "");
    // digi_config.tree_dump(std::clog, "Digitization configuration: ", "[info] ");

    // Number of events :
    int max_record_total = static_cast<int>(max_events) * static_cast<int>(input_filenames.size());
    std::clog << "max_record total = " << max_record_total << std::endl;
    std::clog << "max_events       = " << max_events << std::endl;

    // Event reader :
    dpp::input_module reader;
    datatools::properties reader_config;
    reader_config.store("logging.priority", "debug");
    reader_config.store("files.mode", "list");
    reader_config.store("files.list.filenames", input_filenames);
    reader_config.store("max_record_total", max_record_total);
    reader_config.store("max_record_per_file", static_cast<int>(max_events));
    reader_config.tree_dump(std::clog, "Input module configuration parameters: ");
    reader.initialize_standalone(reader_config);
    reader.tree_dump(std::clog, "Simulated data reader module");

    int psd_count = 0; // Event counter

    // Trigger output writers :
    // Fake trigger writer
    std::string fake_trigger_filename = output_path + "fake_trigger_SD.brio";
    dpp::output_module ft_writer;
    datatools::properties ft_config;
    ft_config.store ("logging.priority", "debug");
    ft_config.store ("files.mode", "single");
    ft_config.store ("files.single.filename", fake_trigger_filename);
    ft_writer.initialize_standalone (ft_config);

    // No real trigger but FT writer
    std::string ft_no_rt_filename = output_path + "ft_no_rt_SD.brio"; // rt : real trigger
    dpp::output_module ft_no_rt_writer;
    datatools::properties ft_no_rt_config;
    ft_no_rt_config.store ("logging.priority", "debug");
    ft_no_rt_config.store ("files.mode", "single");
    ft_no_rt_config.store ("files.single.filename", ft_no_rt_filename);
    ft_no_rt_writer.initialize_standalone (ft_no_rt_config);

    // Caraco trigger writer
    std::string caraco_trigger_filename = output_path + "caraco_trigger_SD.brio";
    dpp::output_module caraco_writer;
    datatools::properties caraco_config;
    caraco_config.store ("logging.priority", "debug");
    caraco_config.store ("files.mode", "single");
    caraco_config.store ("files.single.filename", caraco_trigger_filename);
    caraco_writer.initialize_standalone (caraco_config);

    // Real trigger writer
    std::string alpha_trigger_filename = output_path + "alpha_trigger_SD.brio";
    dpp::output_module alpha_writer;
    datatools::properties alpha_config;
    alpha_config.store ("logging.priority", "debug");
    alpha_config.store ("files.mode", "single");
    alpha_config.store ("files.single.filename", alpha_trigger_filename);
    alpha_writer.initialize_standalone (alpha_config);

    // Statistics for trigger output
    std::size_t total_number_of_events = 0;
    std::size_t total_number_of_fake_trigger_events = 0;
    std::size_t total_number_of_fake_delayed_trigger_events = 0;
    std::size_t total_number_of_fake_trigger_no_real_trigger_events = 0;
    std::size_t total_number_of_real_trigger_events = 0;
    std::size_t total_number_of_caraco_trigger_events = 0;
    std::size_t total_number_of_delayed_trigger_events = 0;
    std::size_t total_number_of_ape_trigger_events = 0;
    std::size_t total_number_of_dave_trigger_events = 0;

    snemo::digitization::digitization_driver digi_driver;
    digi_driver.set_geometry_manager(my_manager);
    digi_driver.initialize(digi_config);
    digi_driver.tree_dump(std::clog, "Digitization driver");

    while (!reader.is_terminated())
      {
    	reader.process(ER);
    	DT_LOG_WARNING(logging, "Event #" << psd_count);

	// A plain `mctools::signal::signal_data' object is stored here :
	if (ER.has(SSD_bank_label) && ER.is_a<mctools::signal::signal_data>(SSD_bank_label))
	  {
	    // Access to the "SSD" bank with a stored `mctools::simulated_data' :
	    const mctools::signal::signal_data & SSD = ER.get<mctools::signal::signal_data>(SSD_bank_label);
	    // SSD.tree_dump(std::clog, "SSD");

	    snemo::datamodel::sim_digi_data SDD;
	    digi_driver.process(SSD, SDD);

	    if (SDD.has_trigger_digi_data()) {

	      // Access to the "SD" bank with a stored `mctools::simulated_data' :
	      const mctools::simulated_data & SD = ER.get<mctools::simulated_data>(SD_bank_label);

	      // For the fake trigger :
	      std::size_t number_of_main_calo_hits = 0;
	      std::size_t number_of_xwall_calo_hits = 0;
	      std::size_t number_of_geiger_hits = 0;
	      bool has_delayed_geiger = false;
	      static const int MAXIMUM_DELAYED_TIME = 10000;  // in ns
	      if (SD.has_step_hits("calo") || SD.has_step_hits("xcalo") || SD.has_step_hits("gveto") || SD.has_step_hits("gg"))
		{
		  if (SD.has_step_hits("calo")) number_of_main_calo_hits = SD.get_number_of_step_hits("calo");
		  if (SD.has_step_hits("xcalo")) number_of_xwall_calo_hits = SD.get_number_of_step_hits("xcalo");
		  if (SD.has_step_hits("gg"))
		    {
		      number_of_geiger_hits = SD.get_number_of_step_hits("gg");
		      // New sd bank
		      mctools::simulated_data flaged_sd = SD;
		      for (size_t ihit = 0; ihit < number_of_geiger_hits; ihit++)
			{
			  mctools::base_step_hit & geiger_hit = flaged_sd.grab_step_hit("gg", ihit);
			  for (size_t jhit = ihit + 1; jhit < number_of_geiger_hits; jhit++)
			    {
			      mctools::base_step_hit & other_geiger_hit = flaged_sd.grab_step_hit("gg", jhit);
			      if (geiger_hit.get_geom_id() == other_geiger_hit.get_geom_id())
				{
				  const double gg_hit_time       = geiger_hit.get_time_start();
				  const double other_gg_hit_time = other_geiger_hit.get_time_start();
				  if (gg_hit_time > other_gg_hit_time)
				    {
				      bool geiger_already_hit = true;
				      if (!geiger_hit.get_auxiliaries().has_flag("geiger_already_hit")) geiger_hit.grab_auxiliaries().store("geiger_already_hit", geiger_already_hit);
				    }
				  else
				    {
				      bool geiger_already_hit = true;
				      if (!other_geiger_hit.get_auxiliaries().has_flag("geiger_already_hit")) other_geiger_hit.grab_auxiliaries().store("geiger_already_hit", geiger_already_hit);
				    }
				}
			    }
			}


		      mctools::simulated_data::hit_handle_collection_type BSHC = flaged_sd.get_step_hits("gg");
		      for (mctools::simulated_data::hit_handle_collection_type::const_iterator i = BSHC.begin();
			   i != BSHC.end();
			   i++)
			{
			  number_of_geiger_hits = flaged_sd.get_number_of_step_hits("gg");
			  const mctools::base_step_hit & BSH = i->get();
			  if (BSH.get_auxiliaries().has_flag("geiger_already_hit") || BSH.get_auxiliaries().has_flag("other_geiger_already_hit")) {}
			  else
			    {
			      double time_start = BSH.get_time_start();
			      if (time_start > MAXIMUM_DELAYED_TIME) has_delayed_geiger = true;
			    }
			}
		    }

		  // Browse TDD struct in SDD :






		} // has SD step hits

	    } // has SDD hits

	  }// end if (ER.has(SSD_bank_label)...)


	ER.clear();

	psd_count++;
	std::clog << "DEBUG : psd count " << psd_count << std::endl;
	DT_LOG_NOTICE(logging, "Simulated Signal data #" << psd_count);
      } // end of reader


    // 	    uint16_t number_of_L2_decision = L2_decision_record.size();
    // 	    bool caraco_decision = false;
    // 	    uint32_t caraco_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
    // 	    bool delayed_decision = false;
    // 	    uint32_t delayed_clocktick_1600ns = snemo::digitization::clock_utils::INVALID_CLOCKTICK;
    // 	    bool already_delayed_trig = false;
    // 	    snemo::digitization::trigger_structures::L2_trigger_mode delayed_trigger_mode = snemo::digitization::trigger_structures::L2_trigger_mode::INVALID;

    // 	    if (number_of_L2_decision != 0)
    // 	      {
    // 		for (unsigned int isize = 0; isize < number_of_L2_decision; isize++)
    // 		  {
    // 		    if (L2_decision_record[isize].L2_decision_bool && L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::CARACO)
    // 		      {
    // 			caraco_decision         = L2_decision_record[isize].L2_decision_bool;
    // 			caraco_clocktick_1600ns = L2_decision_record[isize].L2_ct_decision;
    // 		      }
    // 		    else if (L2_decision_record[isize].L2_decision_bool &&  (L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::APE
    // 									     || L2_decision_record[isize].L2_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::DAVE) && already_delayed_trig == false)
    // 		      {
    // 			delayed_decision         = L2_decision_record[isize].L2_decision_bool;
    // 			delayed_clocktick_1600ns = L2_decision_record[isize].L2_ct_decision;
    // 			delayed_trigger_mode     = L2_decision_record[isize].L2_trigger_mode;
    // 			already_delayed_trig     = true;
    // 		      }
    // 		  }
    // 	      }

    // 	    // for (std::size_t i = 0; i  < calo_collection_records.size(); i++) {
    // 	    //   calo_collection_records[i].display();
    // 	    // }

    // 	    // for (std::size_t i = 0; i  < coincidence_collection_calo_records.size(); i++) {
    // 	    //   coincidence_collection_calo_records[i].display();
    // 	    // }

    // 	    // for (std::size_t i = 0; i  < tracker_collection_records.size(); i++) {
    // 	    //   tracker_collection_records[i].display();
    // 	    // }

    // 	    // for (std::size_t i = 0; i  < coincidence_collection_records.size(); i++) {
    // 	    //   coincidence_collection_records[i].display();
    // 	    // }
    // 	    std::size_t total_number_of_calo_hits = number_of_main_calo_hits + number_of_xwall_calo_hits;
    // 	    bool ft_passed = false;
    // 	    if (total_number_of_calo_hits >= 1 && number_of_geiger_hits >= 3)
    // 	      {
    // 		ft_writer.process(ER);
    // 		total_number_of_fake_trigger_events++;
    // 		ft_passed = true;
    // 	      }
    // 	    if (caraco_decision && has_delayed_geiger)
    // 	      {
    // 		total_number_of_fake_delayed_trigger_events++;
    // 	      }

    // 	    bool real_trigger_decision = false;
    // 	    if (caraco_decision)
    // 	      {
    // 		caraco_writer.process(ER);
    // 		total_number_of_caraco_trigger_events++;
    // 		real_trigger_decision = true;
    // 	      }
    // 	    if (delayed_decision)
    // 	      {
    // 		alpha_writer.process(ER);
    // 		total_number_of_delayed_trigger_events++;
    // 		if (delayed_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::APE) total_number_of_ape_trigger_events++;
    // 		if (delayed_trigger_mode == snemo::digitization::trigger_structures::L2_trigger_mode::DAVE) total_number_of_dave_trigger_events++;

    // 		real_trigger_decision = true;
    // 	      }

    // 	    if (real_trigger_decision) total_number_of_real_trigger_events++;

    // 	    if (ft_passed && !real_trigger_decision)
    // 	      {
    // 		ft_no_rt_writer.process(ER);
    // 		total_number_of_fake_trigger_no_real_trigger_events++;
    // 	      }

    // 	    DT_LOG_INFORMATION(logging, "Number of L2 decision : " << number_of_L2_decision);
    // 	    DT_LOG_INFORMATION(logging, "CARACO decision :       " << caraco_decision);
    // 	    DT_LOG_INFORMATION(logging, "CARACO CT1600ns :       " << caraco_clocktick_1600ns);
    // 	    DT_LOG_INFORMATION(logging, "Delayed decision :      " << delayed_decision);
    // 	    DT_LOG_INFORMATION(logging, "Delayed CT1600ns :      " << delayed_clocktick_1600ns);
    // 	    DT_LOG_INFORMATION(logging, "Delayed trigger mode :  " << delayed_trigger_mode);

    // 	    my_trigger_algo.reset_data();

    // 	  } //end of if has bank label "SD"
    // 	total_number_of_events++;

    // 	ER.clear();
    // 	psd_count++;
    // 	if (debug) std::clog << "DEBUG : psd count " << psd_count << std::endl;
    // 	DT_LOG_NOTICE(logging, "Simulated data #" << psd_count);

    // } // end of reader is terminated

    // Display some stats

    std::string output_stat_filename = output_path + '/' + "output_trigger.stat";
    std::ofstream statstream;
    statstream.open(output_stat_filename);
    statstream << "Welcome on trigger statistic file" << std::endl << std::endl;

    statstream << "Total number of events : " << total_number_of_events << std::endl;
    statstream << "Total number of fake trigger events : " << total_number_of_fake_trigger_events << std::endl;
    statstream << "Total number of fake trigger delayed events : " << total_number_of_fake_delayed_trigger_events << std::endl;
    statstream << "Total number of fake trigger no real trigger events : " << total_number_of_fake_trigger_no_real_trigger_events << std::endl;
    statstream << "Total number of real trigger events : " << total_number_of_real_trigger_events << std::endl;
    statstream << "Total number of caraco trigger events : " << total_number_of_caraco_trigger_events << std::endl;
    statstream << "Total number of delayed trigger events : " << total_number_of_delayed_trigger_events << std::endl;
    statstream << "Total number of ape trigger events : " << total_number_of_ape_trigger_events << std::endl;
    statstream << "Total number of dave trigger events : " << total_number_of_dave_trigger_events << std::endl;
    statstream << "The end." << std::endl;

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

  falaise::terminate();
  return error_code;
}
