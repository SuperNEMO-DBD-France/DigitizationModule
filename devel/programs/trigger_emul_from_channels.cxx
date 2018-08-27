// produce_self_trigger_SD.cxx

// Standard libraries :
#include <iostream>

// - Bayeux/datatools:
#include <datatools/utils.h>
#include <datatools/io_factory.h>
#include <datatools/clhep_units.h>
#include <datatools/temporary_files.h>
#include <datatools/io_factory.h>
#include <datatools/properties.h>
// - Bayeux/geomtools:
#include <geomtools/manager.h>
#include <geomtools/base_hit.h>
// - Bayeux/mctools:
#include <mctools/simulated_data.h>
// - Bayeux/dpp:
#include <dpp/input_module.h>
#include <dpp/output_module.h>

// Falaise:
#include <falaise/falaise.h>
#include <falaise/snemo/geometry/calo_locator.h>
#include <falaise/snemo/geometry/gveto_locator.h>
#include <falaise/snemo/geometry/gg_locator.h>

// Third part :
// GSL:
#include <bayeux/mygsl/rng.h>

// Boost :
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>


void generate_pool_of_calo_spurious_SD(mygsl::rng * rdm_gen_,
				       const snemo::geometry::calo_locator & CL_,
				       const datatools::properties & config_,
				       const std::vector<geomtools::geom_id> & gid_collection_,
				       std::vector<mctools::base_step_hit> & calo_tracker_spurious_pool_);

void generate_pool_of_geiger_spurious_SD(mygsl::rng * rdm_gen_,
					 const snemo::geometry::gg_locator & GL_,
					 const datatools::properties & config_,
					 const std::vector<geomtools::geom_id> & gid_collection_,
					 std::vector<mctools::base_step_hit> & calo_tracker_spurious_pool_);

void parse_config_file(const std::string & filename,
		       std::vector<geomtools::geom_id> & list_of_gid_to_generate);


int main( int  argc_ , char **argv_  )
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  try {
    bool is_display = false;
    std::string output_path = "";
    std::string config_file = "";

    // Parse options:
    namespace po = boost::program_options;
    po::options_description opts("Allowed options");
    opts.add_options()
      ("help,h",    "produce help message")
      ("display,d", "display mode")
      ("output-path,o",
       po::value<std::string>(& output_path),
       "set the output path where produced files are created")
      ("config,c",
       po::value<std::string>(& config_file),
       "set the list of SD hits to generate from electronic mapping configuration file")
      ; // end of options description

    // Describe command line arguments :
    po::variables_map vm;
    po::store(po::command_line_parser(argc_, argv_)
              .options(opts)
              .run(), vm);
    po::notify(vm);

    // Use command line arguments :
    if (vm.count("help")) {
      std::cout << "Usage :" << std::endl;
      std::cout << opts << std::endl;
      return(error_code);
    }

    if (vm.count("display")) {
      is_display = true;
    }

    // Set the default output path :
    if (output_path.empty()) output_path = "/tmp/";
    DT_LOG_INFORMATION(logging, "Output path : " + output_path);
    datatools::fetch_path_with_env(output_path);

    // Default config file (datatools::properties format):
    if (config_file.empty()) config_file = "${FALAISE_DIGITIZATION_DIR}/devel/resources/trigger_emul_from_channels.conf";
    datatools::fetch_path_with_env(config_file);

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
    my_manager.initialize(manager_config);


    // Self trigger SD brio writer :
    std::string output_SD_filename = output_path + "trigger_emul_from_channels_SD.brio";
    dpp::output_module output_SD_writer;
    datatools::properties output_SD_config;
    output_SD_config.store ("logging.priority", "debug");
    output_SD_config.store ("files.mode", "single");
    output_SD_config.store ("files.single.filename", output_SD_filename);
    output_SD_writer.initialize_standalone(output_SD_config);

    DT_LOG_INFORMATION(logging, "Serialization self trigger output file :" + output_SD_filename);

    std::string SD_bank_label = "SD";
    unsigned int module_number = 0;

    snemo::geometry::calo_locator calo_locator;
    calo_locator.set_geo_manager(my_manager);
    calo_locator.set_module_number(module_number);
    calo_locator.initialize();

    snemo::geometry::gveto_locator gveto_locator;
    gveto_locator.set_geo_manager(my_manager);
    gveto_locator.set_module_number(module_number);
    gveto_locator.initialize();

    snemo::geometry::gg_locator gg_locator;
    gg_locator.set_geo_manager(my_manager);
    gg_locator.set_module_number(module_number);
    gg_locator.initialize();


    // // Select calo main wall GID :
    // geomtools::geom_id main_wall_gid_pattern(1302,
    //                                          module_number,
    //                                          geomtools::geom_id::ANY_ADDRESS, // Side
    //                                          geomtools::geom_id::ANY_ADDRESS, // Column
    //                                          geomtools::geom_id::ANY_ADDRESS, // Row
    // 					     0); // part only 0, to convert into any (to not have part 0 and 1 in the vector double count)
    // std::vector<geomtools::geom_id> collection_of_main_wall_gid;
    // my_manager.get_mapping().compute_matching_geom_id(main_wall_gid_pattern,
    //                                                   collection_of_main_wall_gid);
    // // Convert part 0 into any (*)
    // for (std::size_t i = 0; i < collection_of_main_wall_gid.size(); i++) {
    //   collection_of_main_wall_gid[i].set_any(4);
    // }

    // // Select calo xwall GID :
    // geomtools::geom_id xwall_gid_pattern(1232,
    //                                      module_number,
    //                                      geomtools::geom_id::ANY_ADDRESS,  // Side
    //                                      geomtools::geom_id::ANY_ADDRESS,  // Wall
    //                                      geomtools::geom_id::ANY_ADDRESS,  // Column
    //                                      geomtools::geom_id::ANY_ADDRESS); // Row
    // std::vector<geomtools::geom_id> collection_of_xwall_gid;
    // my_manager.get_mapping().compute_matching_geom_id(xwall_gid_pattern,
    //                                                   collection_of_xwall_gid);


    // // Select geiger GID :
    // geomtools::geom_id geiger_gid_pattern(1210,
    // 					  module_number,
    // 					  geomtools::geom_id::ANY_ADDRESS,  // Side
    // 					  geomtools::geom_id::ANY_ADDRESS,  // Layer
    // 					  geomtools::geom_id::ANY_ADDRESS); // Row
    // std::vector<geomtools::geom_id> collection_of_geiger_gid;
    // my_manager.get_mapping().compute_matching_geom_id(geiger_gid_pattern,
    // 						      collection_of_geiger_gid);


    // std::clog << "Generating pool of calo spurious hits 'SD'..." << std::endl;
    // std::vector<mctools::base_step_hit> calo_tracker_spurious_pool;

    // // Generate pool of main wall spurious signals :
    // generate_pool_of_calo_spurious_SD(&random_generator,
    // 				      calo_locator,
    // 				      self_trigger_config,
    // 				      collection_of_main_wall_gid,
    // 				      calo_tracker_spurious_pool);

    // // Generate pool of xwall spurious signals :
    // generate_pool_of_calo_spurious_SD(&random_generator,
    // 				      calo_locator,
    // 				      self_trigger_config,
    // 				      collection_of_xwall_gid,
    // 				      calo_tracker_spurious_pool);

    // // Generate pool of geiger spurious signals :
    // generate_pool_of_geiger_spurious_SD(&random_generator,
    // 					gg_locator,
    // 					self_trigger_config,
    // 					collection_of_geiger_gid,
    // 					calo_tracker_spurious_pool);


    // std::size_t event_counter = 0;

    // datatools::things ER;
    // mctools::simulated_data * ptr_simu_data = 0;
    // ptr_simu_data = &(ER.add<mctools::simulated_data>(SD_bank_label));
    // mctools::simulated_data & output_SD = *ptr_simu_data;
    // output_SD.add_step_hits("calo", 2000);
    // output_SD.add_step_hits("xcalo", 2000);
    // output_SD.add_step_hits("gveto", 2000);
    // output_SD.add_step_hits("gg", 2000);

    // if (output_SD.has_step_hits("calo")
    // 	|| output_SD.has_step_hits("xcalo")
    // 	|| output_SD.has_step_hits("gveto")
    // 	|| output_SD.has_step_hits("gg"))
    //   {
    // 	output_SD_writer.process(ER);
    // 	// self_trigger_SD.tree_dump();
    // 	event_counter++;
    //   }

    // std::clog << "Number of events = " << event_counter << std::endl;


    std::vector<geomtools::geom_id> list_of_gid_to_generate;

    parse_config_file(config_file,
		      list_of_gid_to_generate);


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


void generate_pool_of_calo_spurious_SD(mygsl::rng * rdm_gen_,
				       const snemo::geometry::calo_locator & CL_,
				       const datatools::properties & config_,
				       const std::vector<geomtools::geom_id> & gid_collection_,
				       std::vector<mctools::base_step_hit> & calo_tracker_spurious_pool_)
{
  int hit_count = 0;
  double time_interval; //= 0.1 * CLHEP::second;
  double calo_self_triggering_frequency; // = 1. / CLHEP::second; // Hertz
  double energy_min;
  double energy_max;
  datatools::invalidate(time_interval);
  datatools::invalidate(calo_self_triggering_frequency);
  datatools::invalidate(energy_min);
  datatools::invalidate(energy_max);

  if (config_.has_key("time_interval")) {
    time_interval = config_.fetch_real("time_interval");
  }

  if (config_.has_key("calo.self_trigger_frequency")) {
    calo_self_triggering_frequency = config_.fetch_real("calo.self_trigger_frequency");
  }

  if (config_.has_key("calo.energy_min")) {
    energy_min = config_.fetch_real("calo.energy_min");
  }

  if (config_.has_key("calo.energy_max")) {
    energy_max = config_.fetch_real("calo.energy_max");
  }

  std::clog << "Calo self-trigger frequency (in Hz) " << calo_self_triggering_frequency / CLHEP::hertz << std::endl;
  std::clog << "Calo energy min (in MeV) " << energy_min << std::endl;
  std::clog << "Calo energy max (in MeV) " << energy_max << std::endl;

  // Create spurious hits during a time interval for each calo GID :
  for (std::size_t i = 0; i < gid_collection_.size(); i++)
    {
      double mean_number = time_interval * calo_self_triggering_frequency;
      double sigma_gauss = std::sqrt(mean_number);
      std::size_t number_of_calo_hit = 0;
      std::string distrib = "";
      // Number of calo hit during time_interval for a calo block (identified by his GID) :
      // If > 15 gaussian distribution
      if (mean_number > 15) {
        number_of_calo_hit = rdm_gen_->gaussian(mean_number, sigma_gauss);
        distrib = "gaussian";
      }
      // Else Poisson distribution
      else {
        number_of_calo_hit = rdm_gen_->poisson(mean_number);
        distrib = "poisson";
      }

      //std::clog << "mean_number " << mean_number << " number_of_calo_hit " << number_of_calo_hit << std::endl;

      geomtools::geom_id calo_gid = gid_collection_[i];

      for (std::size_t j = 0; j < number_of_calo_hit; j++)
	{
	  geomtools::vector_3d calo_block_position;
	  CL_.get_block_position(calo_gid, calo_block_position);

	  // Create a new calo hit in the middle of the scintillator block :
	  mctools::base_step_hit a_calo_hit;
	  a_calo_hit.set_hit_id(hit_count);
	  a_calo_hit.set_geom_id(calo_gid);
	  a_calo_hit.set_position_start(calo_block_position);
	  a_calo_hit.set_position_stop(calo_block_position);
	  const double timestamp = rdm_gen_->flat(0, time_interval);
	  a_calo_hit.set_time_start(timestamp);
	  a_calo_hit.set_time_stop(timestamp);
	  const double energy = rdm_gen_->flat(energy_min, energy_max);
	  a_calo_hit.set_energy_deposit(energy);
	  a_calo_hit.set_particle_name("e-");
	  // a_calo_hit.tree_dump(std::clog, "A main calo hit #" + std::to_string(a_calo_hit.get_hit_id()));

	  calo_tracker_spurious_pool_.push_back(a_calo_hit);
	  hit_count++;
	}
    }

  return;
}

void generate_pool_of_geiger_spurious_SD(mygsl::rng * rdm_gen_,
					 const snemo::geometry::gg_locator & GL_,
					 const datatools::properties & config_,
					 const std::vector<geomtools::geom_id> & gid_collection_,
					 std::vector<mctools::base_step_hit> & calo_tracker_spurious_pool_)
{
  int hit_count = 0;
  double time_interval; //= 0.1 * CLHEP::second;
  double geiger_self_triggering_frequency; // = 1. / CLHEP::second; // Hertz
  double cell_dead_time;
  datatools::invalidate(time_interval);
  datatools::invalidate(geiger_self_triggering_frequency);
  datatools::invalidate(cell_dead_time);

  if (config_.has_key("time_interval")) {
    time_interval = config_.fetch_real("time_interval");
  }

  if (config_.has_key("geiger.self_trigger_frequency")) {
    geiger_self_triggering_frequency = config_.fetch_real("geiger.self_trigger_frequency");
  }

  if (config_.has_key("geiger.dead_time")) {
    cell_dead_time = config_.fetch_real("geiger.dead_time");
  }

  std::clog << "Tracker self-trigger frequency (in Hz) " << geiger_self_triggering_frequency / CLHEP::hertz << std::endl;
  std::clog << "Tracker cell dead time (in ms) " << cell_dead_time / CLHEP::millisecond << std::endl;

  // std::clog << "Time interval = " << time_interval << " GG ST freq = " << geiger_self_triggering_frequency << " GG cell_dead_time = " << cell_dead_time << std::endl;

  // Create spurious hits during a time interval for each geiger cell thanks to GID :
  for (std::size_t i = 0; i < gid_collection_.size(); i++)
    {
      double mean_number = time_interval * geiger_self_triggering_frequency;
      double sigma_gauss = std::sqrt(mean_number);
      std::size_t number_of_geiger_hit = 0;
      std::string distrib = "";

      // Number of geiger hit during time_interval for a geiger cell (identified by his GID) :
      // If nhits > 15 : gaussian distribution
      if (mean_number > 15) {
        number_of_geiger_hit = rdm_gen_->gaussian(mean_number, sigma_gauss);
        distrib = "gaussian";
      }
      // else Poisson distribution
      else {
        number_of_geiger_hit = rdm_gen_->poisson(mean_number);
        distrib = "poisson";
      }

      const geomtools::geom_id geiger_gid = gid_collection_[i];

      // We have to generate 'number_of_geiger_hit' of the same cell in the time [0:time_interval]
      // We have to take care about the dead time of the cell.
      // After a spurious hit, each cell cannot trigger during the cell dead time :

      std::vector<double> timestamp_pool;

      double time_interval_limit = number_of_geiger_hit * cell_dead_time;
      double time_max = time_interval - time_interval_limit;
      // std::clog << "Time interval = " << time_interval << " Time int lim = " << time_interval_limit << " tmax = " << time_max << " Freq = " << geiger_self_triggering_frequency << " Mean = " << mean_number << " GG hits = " << number_of_geiger_hit << std::endl;
      bool particular_case = false;
      if (time_max <= 0) {
  	// Particular case, high frequency / low dead_time or time interval too small
  	// Geiger cell always trigger after dead time recovery, kind of 'saturation'
  	for (std::size_t j = 0; j < number_of_geiger_hit; j++) {
  	  const double timestamp = j * cell_dead_time;
  	  if (timestamp >= 0 && timestamp < time_interval) {
  	    timestamp_pool.push_back(timestamp);
  	  }
  	}
  	particular_case = true;
      }
      else {
  	for (std::size_t j = 0; j < number_of_geiger_hit; j++) {
  	  const double timestamp = rdm_gen_ -> flat(0, time_max);
  	  timestamp_pool.push_back(timestamp);
  	}
  	particular_case = false;
      }
      std::sort(timestamp_pool.begin(), timestamp_pool.end());

      // Fill and add a new Geiger hit
      for (std::size_t j = 0; j < timestamp_pool.size(); j++)
      	{
	  geomtools::vector_3d geiger_cell_position;
	  GL_.get_cell_position(geiger_gid, geiger_cell_position);
	  geomtools::vector_3d geiger_hit_position;

	  geiger_hit_position.setX(geiger_cell_position.x() + 10 * CLHEP::millimeter);
	  geiger_hit_position.setY(geiger_cell_position.y() + 10 * CLHEP::millimeter);
	  geiger_hit_position.setZ(geiger_cell_position.z());

	  mctools::base_step_hit a_geiger_hit;
	  a_geiger_hit.set_hit_id(hit_count);
	  a_geiger_hit.set_geom_id(geiger_gid);
	  a_geiger_hit.set_position_start(geiger_hit_position);
	  a_geiger_hit.set_position_stop(geiger_cell_position);
      	  double anodic_time = 0;
      	  if (particular_case) anodic_time = timestamp_pool[j];
      	  else anodic_time = timestamp_pool[j] + j * cell_dead_time;
	  a_geiger_hit.set_time_start(anodic_time);
	  a_geiger_hit.set_particle_name("e-");
	  //a_geiger_hit.tree_dump(std::clog, "A main geiger hit #" + std::to_string(a_geiger_hit.get_hit_id()));

	  calo_tracker_spurious_pool_.push_back(a_geiger_hit);
	  hit_count++;
	}
    }

  return;
}

void parse_config_file(const std::string & filename,
		       std::vector<geomtools::geom_id> & list_of_gid_to_generate)
{
  std::ifstream configstream;
  configstream.open(filename);

  std::string line;
  std::size_t line_number = 0;
  while (getline(configstream, line)) {
    if (!line.empty()) {
      std::clog << "Line #" << line_number << " : " << line << std::endl;
      std::vector<std::string> splitted_lines;

      boost::split(splitted_lines, line, [](char c){return c == ':';});

      std::string hit_type = splitted_lines[0];
      std::string crate_with_letter =  splitted_lines[1];
      std::string boards_with_letter =  splitted_lines[2];

      std::string feast_with_letter = "";
      std::string channel_with_letter = "";
      if (hit_type == "TRACKER")
	{
	  feast_with_letter = splitted_lines[3];
	  channel_with_letter = splitted_lines[4];
	}
      else channel_with_letter = splitted_lines[3];

      if (line_number == 0) std::clog << hit_type << ' '
				      << crate_with_letter << ' '
				      << boards_with_letter << ' '
				      << feast_with_letter << ' '
				      << channel_with_letter << ' ' << std::endl;


      line_number++;
    }
  }




  return;
}
