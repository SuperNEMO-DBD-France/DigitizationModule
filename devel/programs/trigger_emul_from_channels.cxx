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


// Ourselves
#include <snemo/digitization/mapping.h>
#include <snemo/digitization/electronic_mapping.h>


void parse_config_file(const std::string & filename,
		       std::vector<std::pair<geomtools::geom_id, double> > & list_of_eid_time_to_generate);



void generate_eid_for_a_channel(const std::string hit_type,
				const std::size_t crate_id,
				const std::size_t board_id,
				const std::size_t feast_id,
				const std::size_t channel_id,
				geomtools::geom_id & eid_channel);

void generate_BSH_and_fill_SD(const snemo::digitization::electronic_mapping & my_e_mapping,
			      const snemo::geometry::calo_locator  & CL,
			      const snemo::geometry::gveto_locator & GVL,
			      const snemo::geometry::gg_locator    & GL,
			      const std::vector<std::pair<geomtools::geom_id, double> > & list_of_eid_time_to_generate,
			      mctools::simulated_data & output_SD);


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

    // Electronic mapping :
    datatools::properties electronic_config;
    electronic_config.store_integer("module_number", snemo::digitization::mapping::DEMONSTRATOR_MODULE_NUMBER);
    electronic_config.store_string("feast_channel_mapping", "${FALAISE_DIGI_RESOURCE_DIR}/config/snemo/demonstrator/simulation/digitization/0.1/feast_channel_mapping.csv");
    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.initialize(electronic_config);

    datatools::things ER;
    mctools::simulated_data * ptr_simu_data = 0;
    ptr_simu_data = &(ER.add<mctools::simulated_data>(SD_bank_label));
    mctools::simulated_data & output_SD = *ptr_simu_data;
    output_SD.add_step_hits("calo", 300);
    output_SD.add_step_hits("xcalo", 150);
    output_SD.add_step_hits("gveto", 70);
    output_SD.add_step_hits("gg", 300);


    std::vector<std::pair<geomtools::geom_id,double> > list_of_eid_time_to_generate;

    parse_config_file(config_file,
		      list_of_eid_time_to_generate);

    // std::clog << "Size of EID/time pair list = " << list_of_eid_time_to_generate.size() << std::endl;

    generate_BSH_and_fill_SD(my_e_mapping,
			     calo_locator,
			     gveto_locator,
			     gg_locator,
			     list_of_eid_time_to_generate,
			     output_SD);

    output_SD_writer.process(ER);


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

void parse_config_file(const std::string & filename,
		       std::vector<std::pair<geomtools::geom_id,double> > & list_of_eid_time_to_generate)
{
  std::ifstream configstream;
  configstream.open(filename);

  std::string line;
  std::size_t line_number = 0;
  if (configstream.is_open()) {
    while (getline(configstream, line)) {
      if (!line.empty() && line[0] != '#') {
	// std::clog << "Line #" << line_number << " : " << line << std::endl;
	std::vector<std::string> splitted_lines;

	// Line string parsing
	boost::split(splitted_lines, line, [](char c){return c == ':';});

	std::string hit_type = splitted_lines[0];
	std::string crate_with_letter =  splitted_lines[1];
	std::string board_with_letter =  splitted_lines[2];

	std::string feast_with_letter = "";
	std::string channel_with_letter = "";
	std::string time_with_letter = ""; // in microsecond

	if (hit_type == "TRACKER")
	  {
	    feast_with_letter   = splitted_lines[3];
	    channel_with_letter = splitted_lines[4];
	    time_with_letter    = splitted_lines[5];
	  }
	else
	  {
	    channel_with_letter = splitted_lines[3];
	    time_with_letter    = splitted_lines[4];
	  }

	std::size_t crate_token = crate_with_letter.find('C'); // Pos of the letter
	std::size_t crate_id = std::stoi(crate_with_letter.substr(crate_token + 1));

	bool multiple_boards = false;
	std::size_t found_mult_board = board_with_letter.find('-');
	if (found_mult_board != std::string::npos) multiple_boards = true;

	bool multiple_feasts = false;
	std::size_t found_mult_feast = feast_with_letter.find('-');
	if (found_mult_feast != std::string::npos) multiple_feasts = true;

	bool multiple_channels = false;
	std::size_t found = channel_with_letter.find('-');
	if (found != std::string::npos) multiple_channels = true;

	std::size_t board_token_begin = board_with_letter.find('S');
	std::size_t board_token_last = board_with_letter.find('-');

	std::size_t board_id_begin = -1;
	std::size_t board_id_end = -1;

	if (multiple_boards) {
	  board_id_begin = std::stoi(board_with_letter.substr(board_token_begin + 1, board_token_last));
	  board_id_end = std::stoi(board_with_letter.substr(board_token_last + 1));
	}
	else {
	  board_id_begin = std::stoi(board_with_letter.substr(board_token_begin + 1, board_token_last));
	  board_id_end = board_id_begin;
	}

	std::size_t feast_token_begin = feast_with_letter.find("F");
	std::size_t feast_token_last = feast_with_letter.find('-');

	int feast_id_begin = -1;
	int feast_id_end = -1;

	if (hit_type == "TRACKER") {
	  if (multiple_feasts) {
	    feast_id_begin = std::stoi(feast_with_letter.substr(feast_token_begin + 1, feast_token_last));
	    feast_id_end = std::stoi(feast_with_letter.substr(feast_token_last + 1));
	  }
	  else {
	    feast_id_begin = std::stoi(feast_with_letter.substr(feast_token_begin + 1, feast_token_last));
	    feast_id_end = feast_id_begin;
	  }
	}

	std::size_t channel_token_begin = channel_with_letter.find("CH");
	std::size_t channel_token_last = channel_with_letter.find('-');

	std::size_t channel_id_begin = -1;
	std::size_t channel_id_end = -1;

	if (multiple_channels) {
	  channel_id_begin = std::stoi(channel_with_letter.substr(channel_token_begin + 2, channel_token_last));
	  channel_id_end = std::stoi(channel_with_letter.substr(channel_token_last + 1));
	}
	else {
	  channel_id_begin = std::stoi(channel_with_letter.substr(channel_token_begin + 2, channel_token_last));
	  channel_id_end = channel_id_begin;
	}

	std::size_t time_token = time_with_letter.find("T");
	double a_time = std::stoi(time_with_letter.substr(time_token + 1)) * CLHEP::microsecond;

	for (std::size_t board_id = board_id_begin; board_id <= board_id_end; board_id++)
	  {
	    for (int feast_id = feast_id_begin; feast_id <= feast_id_end; feast_id++)
	      {
		for (std::size_t channel_id = channel_id_begin; channel_id <= channel_id_end; channel_id++)
		  {
		    std::pair<geomtools::geom_id, double> a_eid_time_pair;
		    geomtools::geom_id a_eid;
		    generate_eid_for_a_channel(hit_type, crate_id, board_id, feast_id, channel_id, a_eid);

		    a_eid_time_pair = std::make_pair(a_eid, a_time);

		    list_of_eid_time_to_generate.push_back(a_eid_time_pair);
		  }
	      }
	  }

	line_number++;
      }
    }
    configstream.close();
  }

  // std::clog << "parse_config_file():end of parse method" << std::endl;

  return;
}



void generate_eid_for_a_channel(const std::string hit_type,
				const std::size_t crate_id,
				const std::size_t board_id,
				const std::size_t feast_id,
				const std::size_t channel_id,
				geomtools::geom_id & eid_channel)
{
  std::size_t module_id = 0;
  if (hit_type ==  "TRACKER") {
    eid_channel.set_depth(5);
    // EID=[GG_FEB_TYPE:module_ID:crate_ID:board_ID:feast_ID:channel_ID]

    eid_channel.set_type(snemo::digitization::mapping::GEIGER_FEB_CATEGORY_TYPE);
    eid_channel.set(0, module_id);
    eid_channel.set(1, crate_id);
    eid_channel.set(2, board_id);
    eid_channel.set(3, feast_id);
    eid_channel.set(4, channel_id);
  }

  if (hit_type ==  "CALO")  {
    eid_channel.set_depth(4);
    // EID=[CALO_FEB_TYPE:module_ID:crate_ID:board_ID:channel_ID]

    eid_channel.set_type(snemo::digitization::mapping::CALO_FEB_CATEGORY_TYPE);
    eid_channel.set(0, module_id);
    eid_channel.set(1, crate_id);
    eid_channel.set(2, board_id);
    eid_channel.set(3, channel_id);
  }

  return;
}

void generate_BSH_and_fill_SD(const snemo::digitization::electronic_mapping & my_e_mapping,
			      const snemo::geometry::calo_locator  & CL,
			      const snemo::geometry::gveto_locator & GVL,
			      const snemo::geometry::gg_locator    & GL,
			      const std::vector<std::pair<geomtools::geom_id, double> > & list_of_eid_time_to_generate,
			      mctools::simulated_data & output_SD)
{
  std::size_t hit_count = 0;
  for (std::size_t i = 0; i < list_of_eid_time_to_generate.size(); i++)
    {
      geomtools::geom_id a_eid = list_of_eid_time_to_generate[i].first;
      double hit_time_start = list_of_eid_time_to_generate[i].second;

      // See the GID and produce corresponding BSH (calo / xcalo / gveto / geiger) :
      geomtools::geom_id a_gid;
      my_e_mapping.convert_EID_to_GID(false, a_eid, a_gid);

      std::clog << "EID : " << a_eid << " <=> GID : " << a_gid << std::endl;

      // Calo type
      if (a_gid.get_type() == snemo::digitization::mapping::CALO_MAIN_WALL_CATEGORY_TYPE
	  || a_gid.get_type() == snemo::digitization::mapping::CALO_XWALL_CATEGORY_TYPE
	  || a_gid.get_type() == snemo::digitization::mapping::CALO_GVETO_CATEGORY_TYPE) {

	geomtools::vector_3d calo_block_position;
	geomtools::geom_id calo_gid;

	if (a_gid.get_type() == snemo::digitization::mapping::CALO_MAIN_WALL_CATEGORY_TYPE) {
	  calo_gid.set_type(snemo::digitization::mapping::CALO_MAIN_WALL_CATEGORY_TYPE);
	  calo_gid.set_depth(5);
	  calo_gid.inherits_from(a_gid);
	  calo_gid.set_any(4);
	  CL.get_block_position(calo_gid, calo_block_position);
	}

	if (a_gid.get_type() == snemo::digitization::mapping::CALO_XWALL_CATEGORY_TYPE) {
	  calo_gid = a_gid;
	  CL.get_block_position(calo_gid, calo_block_position);
	}

	if (a_gid.get_type() == snemo::digitization::mapping::CALO_GVETO_CATEGORY_TYPE) {
	  calo_gid = a_gid;
	  GVL.get_block_position(a_gid, calo_block_position);
	}

	geomtools::vector_3d calo_hit_position;
	calo_hit_position.setX(calo_block_position.x()); // - 100 * CLHEP::millimeter);
	calo_hit_position.setY(calo_block_position.y()); // - 10 * CLHEP::millimeter);
	calo_hit_position.setZ(calo_block_position.z());

	// Create a new calo hit in the middle of the scintillator block step :
	mctools::base_step_hit a_calo_hit;
	a_calo_hit.set_hit_id(hit_count);
	a_calo_hit.set_geom_id(calo_gid);
	a_calo_hit.set_position_start(calo_hit_position);
	a_calo_hit.set_position_stop(calo_hit_position);

	a_calo_hit.set_time_start(hit_time_start);
	a_calo_hit.set_time_stop(hit_time_start);
	const double energy = 1 * CLHEP::MeV;
	a_calo_hit.set_energy_deposit(energy);
	a_calo_hit.set_particle_name("e-");

	if (calo_gid.get_type() == snemo::digitization::mapping::CALO_MAIN_WALL_CATEGORY_TYPE) output_SD.add_step_hit("calo") = a_calo_hit;
	if (calo_gid.get_type() == snemo::digitization::mapping::CALO_XWALL_CATEGORY_TYPE) output_SD.add_step_hit("xcalo") = a_calo_hit;
	if (calo_gid.get_type() == snemo::digitization::mapping::CALO_GVETO_CATEGORY_TYPE) output_SD.add_step_hit("gveto") = a_calo_hit;
      }

      // Geiger cell hit
      if (a_gid.get_type() == snemo::digitization::mapping::GEIGER_ANODIC_CATEGORY_TYPE) {

	geomtools::vector_3d geiger_cell_position;
	GL.get_cell_position(a_gid, geiger_cell_position);
	geomtools::vector_3d geiger_hit_position;

	geiger_hit_position.setX(geiger_cell_position.x() + 3 * CLHEP::millimeter);
	geiger_hit_position.setY(geiger_cell_position.y() + 3 * CLHEP::millimeter);
	geiger_hit_position.setZ(geiger_cell_position.z());

	mctools::base_step_hit a_geiger_hit;
	a_geiger_hit.set_hit_id(hit_count);
	a_geiger_hit.set_geom_id(a_gid);
	a_geiger_hit.set_position_start(geiger_hit_position);
	a_geiger_hit.set_position_stop(geiger_cell_position);

	double anodic_time = hit_time_start;
	a_geiger_hit.set_time_start(anodic_time);
	a_geiger_hit.set_particle_name("e-");

	output_SD.add_step_hit("gg") = a_geiger_hit;
      }


      std::clog << "EID = " << a_eid << " Time = " << hit_time_start << " GID = " << a_gid << std::endl;
      hit_count++;
    }
  std::clog << "Number of  EID / time pair : " << hit_count << std::endl;

  std::clog << "Main calo hits  : " << output_SD.get_number_of_step_hits("calo") << std::endl;
  std::clog << "Xwall calo hits : " << output_SD.get_number_of_step_hits("xcalo") << std::endl;
  std::clog << "Gveto calo hits : " << output_SD.get_number_of_step_hits("gveto") << std::endl;
  std::clog << "Geiger hits     : " << output_SD.get_number_of_step_hits("gg") << std::endl;


  return;
}
