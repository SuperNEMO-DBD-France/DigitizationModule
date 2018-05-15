// test_signal_to_calo_tp_process.cxx

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
#include <snemo/digitization/tempo_utils.h>
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/signal_to_calo_tp_algo.h>

int main(int argc_, char** argv_)
{
  falaise::initialize(argc_, argv_);
  int error_code = EXIT_SUCCESS;
  datatools::logger::priority logging = datatools::logger::PRIO_FATAL;

  // Parse options:
  namespace po = boost::program_options;
  po::options_description opts("Allowed options");
  opts.add_options()
    ("help,h", "produce help message")
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
    std::clog << "Test program for class 'snemo::digitization::signal_to_calo_tp_process' !" << std::endl;
    int32_t seed = 314157;
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

    std::string SSD_bank_label = "SSD";
    std::string calo_signal_category = "sigcalo";

    snemo::digitization::electronic_mapping my_e_mapping;
    my_e_mapping.set_geo_manager(my_manager);
    my_e_mapping.set_module_number(0);
    my_e_mapping.initialize();

    snemo::digitization::clock_utils my_clock_manager;
    my_clock_manager.initialize();

    mctools::signal::signal_shape_builder calo_ssb;
    calo_ssb.set_logging_priority(datatools::logger::PRIO_DEBUG);
    calo_ssb.set_category("sigcalo");
    calo_ssb.add_registered_shape_type_id("mctools::signal::triangle_signal_shape");
    calo_ssb.add_registered_shape_type_id("mctools::signal::multi_signal_shape");
    calo_ssb.initialize_simple();
    // calo_ssb.tree_dump(std::clog, "Calorimeter signal shape builder", "[info] ");

    datatools::properties algos_config;

    mctools::signal::signal_data SSD;

    const double rise_time = 6; // in ns
    const double fall_time = 50; // in ns
    const double energy_amplitude_factor = 0.3 * CLHEP::volt; // 1 MeV <=> 300 mV
    const double event_time_ref = 0;

    unsigned int hit_id = 0;

    // Fill the SSD bank with mock calorimeter signals:
    std::vector<mctools::signal::base_signal> atomic_signals;
    {
      mctools::signal::base_signal & a_calo_signal  = SSD.add_signal(calo_signal_category);
      geomtools::geom_id calo_gid(1302, 0, 0, 7, 6, geomtools::geom_id::ANY_ADDRESS);
      a_calo_signal.set_hit_id(hit_id);
      a_calo_signal.set_geom_id(calo_gid);
      a_calo_signal.set_category(calo_signal_category);
      a_calo_signal.set_time_ref(event_time_ref);
      a_calo_signal.set_shape_type_id("mctools::signal::triangle_signal_shape");
      a_calo_signal.set_shape_string_parameter("polarity", "-");
      const double t0 = 1 + event_time_ref;
      const double t1 = t0 + rise_time;
      const double t2 = t1 + fall_time;
      const double amplitude = 0.7 * energy_amplitude_factor;
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("t0", t0, "ns");
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("t1", t1, "ns");
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("t2", t2, "ns");
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("amplitude", amplitude, "V");
      a_calo_signal.initialize_simple();
      a_calo_signal.tree_dump(std::clog, "A calo signal");
      hit_id++;
      atomic_signals.push_back(a_calo_signal);
    }

    {
      mctools::signal::base_signal & a_calo_signal  = SSD.add_signal(calo_signal_category);
      geomtools::geom_id calo_gid(1302, 0, 0, 7, 5, geomtools::geom_id::ANY_ADDRESS);
      a_calo_signal.set_hit_id(hit_id);
      a_calo_signal.set_geom_id(calo_gid);
      a_calo_signal.set_category(calo_signal_category);
      a_calo_signal.set_time_ref(event_time_ref);
      a_calo_signal.set_shape_type_id("mctools::signal::triangle_signal_shape");
      a_calo_signal.set_shape_string_parameter("polarity", "-");
      const double t0 = 890 + event_time_ref;
      const double t1 = t0 + rise_time;
      const double t2 = t1 + fall_time;
      const double amplitude = 0.06 * energy_amplitude_factor;
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("t0", t0, "ns");
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("t1", t1, "ns");
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("t2", t2, "ns");
      a_calo_signal.set_shape_real_parameter_with_explicit_unit("amplitude", amplitude, "V");
      a_calo_signal.initialize_simple();
      a_calo_signal.tree_dump(std::clog, "A calo signal");
      hit_id++;
      atomic_signals.push_back(a_calo_signal);
    }

    mctools::signal::base_signal & a_multi_signal = SSD.add_signal(calo_signal_category);
    geomtools::geom_id calo_gid(1302, 0, 0, 12, 2, geomtools::geom_id::ANY_ADDRESS);
    a_multi_signal.set_hit_id(hit_id);
    a_multi_signal.set_geom_id(calo_gid);
    a_multi_signal.set_category(calo_signal_category);
    a_multi_signal.set_time_ref(event_time_ref);
    a_multi_signal.set_shape_type_id("mctools::signal::multi_signal_shape");
    snemo::digitization::build_multi_signal(a_multi_signal, atomic_signals);
    a_multi_signal.tree_dump(std::clog, "A multi signal");

    SSD.tree_dump(std::clog, "SSD");

    my_clock_manager.compute_clockticks_ref(random_generator);
    int32_t clocktick_25_reference  = my_clock_manager.get_clocktick_25_ref();
    double  clocktick_25_shift      = my_clock_manager.get_shift_25();
    my_clock_manager.tree_dump(std::clog, "Clocktick manager");


    snemo::digitization::signal_to_calo_tp_algo signal_2_calo_tp;
    signal_2_calo_tp.initialize(algos_config,
				my_e_mapping,
				calo_ssb);
    signal_2_calo_tp.set_clocktick_reference(clocktick_25_reference);
    signal_2_calo_tp.set_clocktick_shift(clocktick_25_shift);

    snemo::digitization::calo_tp_data my_calo_tp_data;

    if (SSD.has_signals(calo_signal_category))
      {
	signal_2_calo_tp.process(SSD, my_calo_tp_data);
	my_calo_tp_data.tree_dump(std::clog, "Calorimeter TP(s) data : ", "INFO : ");

	for(uint32_t i = my_calo_tp_data.get_clocktick_min(); i <= my_calo_tp_data.get_clocktick_max(); i++)
	  {
	    for(unsigned int j = 0 ; j <= snemo::digitization::mapping::NUMBER_OF_CRATES ; j++)
	      {
		std::vector<datatools::handle<snemo::digitization::calo_tp> > calo_tp_list_per_clocktick_per_crate;
		my_calo_tp_data.get_list_of_tp_per_clocktick_per_crate(i, j, calo_tp_list_per_clocktick_per_crate);
		if(!calo_tp_list_per_clocktick_per_crate.empty())
		  {
		    for(unsigned int k = 0; k < calo_tp_list_per_clocktick_per_crate.size(); k++)
		      {
			const snemo::digitization::calo_tp & my_calo_tp =  calo_tp_list_per_clocktick_per_crate[k].get();
			my_calo_tp.tree_dump(std::clog, "a_calo_tp : ", "INFO : ");
		      }
		  }
	      }
	  }
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

  falaise::terminate();
  return error_code;
}
