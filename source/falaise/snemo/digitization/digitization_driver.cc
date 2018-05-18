/// \file falaise/snemo/digitization/digitization_driver.cc

// Ourselves:
#include <snemo/digitization/digitization_driver.h>
// #include <snemo/digitization/geiger_tp_data.h>
// #include <snemo/digitization/calo_tp_data.h>
// #include <snemo/digitization/geiger_ctw_data.h>
// #include <snemo/digitization/calo_ctw_data.h>

namespace snemo {

  namespace digitization {

    digitization_driver::digitization_driver()
    {
      _initialized_ = false;
      return;
    }

    digitization_driver::~digitization_driver()
    {
      if (is_initialized()) {
        reset();
      }
      return;
    }

    datatools::logger::priority digitization_driver::get_logging_priority() const
    {
      return _logging_priority_;
    }

    void digitization_driver::set_logging_priority(datatools::logger::priority priority_)
    {
      _logging_priority_ = priority_;
      return;
    }

    bool digitization_driver::has_geometry_manager() const
    {
      return _geometry_manager_ != 0;
    }

    void digitization_driver::set_geometry_manager(const geomtools::manager & geo_mgr_)
    {
      DT_THROW_IF (is_initialized(), std::logic_error, "Already initialized/locked !");
      _geometry_manager_ = & geo_mgr_;
      return;
    }

    const geomtools::manager & digitization_driver::get_geometry_manager() const
    {
      DT_THROW_IF (!has_geometry_manager(), std::logic_error, "No geometry manager is setup !");
      return *_geometry_manager_;
    }

    bool digitization_driver::has_electronic_mapping() const
    {
      return _electronic_mapping_.is_initialized();
    }

    const electronic_mapping & digitization_driver::get_electronic_mapping() const
    {
      DT_THROW_IF (!has_electronic_mapping(), std::logic_error, "No electronic mapping is setup !");
      return _electronic_mapping_;
    }

    electronic_mapping & digitization_driver::grab_electronic_mapping()
    {
      DT_THROW_IF (!has_electronic_mapping(), std::logic_error, "No electronic mapping is setup !");
      return _electronic_mapping_;
    }

    bool digitization_driver::has_clock_utils() const
    {
      return _clock_utils_.is_initialized();
    }

    const clock_utils & digitization_driver::get_clock_utils() const
    {
      DT_THROW_IF (!has_clock_utils(), std::logic_error, "No clock utils is setup !");
      return _clock_utils_;
    }

    void digitization_driver::initialize(const datatools::properties & config_)
    {
      DT_THROW_IF (is_initialized(),          std::logic_error, "Driver is already initialized !");
      DT_THROW_IF (!has_geometry_manager(),   std::logic_error, "No geometry manager is setup !");

      _logging_priority_ = datatools::logger::extract_logging_configuration(config_);

      // Fetch configuration
      int32_t prng_seed = 0;

      if (config_.has_key("prng_seed")) {
	prng_seed = config_.fetch_integer("prng_seed");
      }

      // Initialize internal object
      _rdm_gen_.initialize(prng_seed);
      _clock_utils_.initialize();

      std::string elec_mapping_key = "electronic_mapping.config.";
      datatools::properties elec_mapping_config;
      config_.export_and_rename_starting_with(elec_mapping_config, elec_mapping_key, "");
      _electronic_mapping_.set_geo_manager(get_geometry_manager());
      _electronic_mapping_.initialize(elec_mapping_config);
      elec_mapping_config.tree_dump(std::clog, "Electronic mapping configuration");

      _gg_ssb_.set_logging_priority(_logging_priority_);
      _calo_ssb_.set_logging_priority(_logging_priority_);

      std::string gg_ssb_key = "gg_ssb.config.";
      datatools::properties gg_ssb_config;
      config_.export_and_rename_starting_with(gg_ssb_config, gg_ssb_key, "");
      _gg_ssb_.initialize(gg_ssb_config);
      _gg_ssb_.tree_dump(std::clog, "Geiger Signal Shape Builder");

      std::string calo_ssb_key = "calo_ssb.config.";
      datatools::properties calo_ssb_config;
      config_.export_and_rename_starting_with(calo_ssb_config, calo_ssb_key, "");
      _calo_ssb_.initialize(calo_ssb_config);
      _calo_ssb_.tree_dump(std::clog, "Calo Signal Shape Builder");

      std::string gg_to_tp_algo_key = "gg_to_tp_algo.config.";
      datatools::properties gg_to_tp_algo_config;
      config_.export_and_rename_starting_with(gg_to_tp_algo_config, gg_to_tp_algo_key, "");
      _geiger_signal_to_tp_algo_.initialize(gg_to_tp_algo_config,
					    _clock_utils_,
					    grab_electronic_mapping(),
					    _gg_ssb_);

      std::string calo_to_tp_algo_key = "calo_to_tp_algo.config.";
      datatools::properties calo_to_tp_algo_config;
      config_.export_and_rename_starting_with(calo_to_tp_algo_config, calo_to_tp_algo_key, "");
      _calo_signal_to_tp_algo_.initialize(calo_to_tp_algo_config,
					  _clock_utils_,
					  grab_electronic_mapping(),
					  _calo_ssb_);

      std::string calo_tp_to_ctw_algo_key = "calo_tp_to_ctw_algo.config.";
      datatools::properties calo_tp_to_ctw_algo_config;
      config_.export_and_rename_starting_with(calo_tp_to_ctw_algo_config, calo_tp_to_ctw_algo_key, "");
      _calo_tp_to_ctw_algo_.initialize(calo_tp_to_ctw_algo_config);


      std::string geiger_tp_to_ctw_algo_key = "geiger_tp_to_ctw_algo.config.";
      datatools::properties geiger_tp_to_ctw_algo_config;
      config_.export_and_rename_starting_with(geiger_tp_to_ctw_algo_config, geiger_tp_to_ctw_algo_key, "");
      _geiger_tp_to_ctw_algo_.initialize(geiger_tp_to_ctw_algo_config);


      std::string trigger_config_filename;
      if (config_.has_key("trigger_filename")) {
	trigger_config_filename = config_.fetch_string("trigger_filename");
      }

      datatools::fetch_path_with_env(trigger_config_filename);
      std::clog << "Trigger configuration filename = " << trigger_config_filename << std::endl;

      datatools::multi_properties trigger_multi_prop("name", "type", "Trigger parameters multi section configuration");
      trigger_multi_prop.read(trigger_config_filename);

      trigger_multi_prop.tree_dump(std::clog, "Trigger multi properties");

      _trigger_algo_.set_clock_manager(_clock_utils_);
      // Electronic mapping is needed for tracker trigger algorithm
      _trigger_algo_.set_electronic_mapping(_electronic_mapping_);
      _trigger_algo_.initialize(trigger_multi_prop);

      _initialized_ = true;
      return;
    }

    bool digitization_driver::is_initialized() const
    {
      return _initialized_;
    }

    void digitization_driver::reset()
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Driver is not initialized !");
      _initialized_ = false;
      _geometry_manager_ = nullptr;
      _electronic_mapping_.reset();
      _rdm_gen_.reset();
      _clock_utils_.reset();
      _gg_ssb_.reset();
      _calo_ssb_.reset();
      _calo_signal_to_tp_algo_.reset();
      _geiger_signal_to_tp_algo_.reset();
      _calo_tp_to_ctw_algo_.reset();
      _geiger_tp_to_ctw_algo_.reset();
      _trigger_algo_.reset();
      return;
    }

    void digitization_driver::process(const mctools::signal::signal_data & SSD_,
				      snemo::datamodel::sim_digi_data & SDD_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");


      _process_digitization_algorithms(SSD_, SDD_);
      _process_readout_algorithms(SSD_, SDD_);


      _gg_ssb_.clear_functors();
      _calo_ssb_.clear_functors();

      return;
    }

    void digitization_driver::_process_digitization_algorithms(const mctools::signal::signal_data & SSD_,
							       snemo::datamodel::sim_digi_data & /*SDD_*/)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");

      // For each event, clocktick reference and shifts have to be recalculated and set in the algos:
      _clock_utils_.compute_clockticks_ref(_rdm_gen_);
      // _clock_utils_.tree_dump(std::clog, "Clock utils");

      // For the moment only CT ref are used. Shifts plays the role of a new '0'. It will be add later:
      // int32_t clocktick_25_reference  = _clock_utils_.get_clocktick_25_ref();
      // double  clocktick_25_shift      = _clock_utils_.get_shift_25();
      // _calo_signal_to_tp_algo_.set_clocktick_reference(clocktick_25_reference);
      std::clog << "Debug 0" << std::endl;
      calo_tp_data calo_tp_data;
      _calo_signal_to_tp_algo_.process(SSD_, calo_tp_data);

      std::clog << "Debug 1" << std::endl;
      calo_ctw_data calo_ctw_data;
      _calo_tp_to_ctw_algo_.process(calo_tp_data,
				    calo_ctw_data);
      std::clog << "Debug 2" << std::endl;

      // int32_t clocktick_800_reference = _clock_utils_.get_clocktick_800_ref();
      // double  clocktick_800_shift     = _clock_utils_.get_shift_800();
      // _geiger_signal_to_tp_algo_.set_clocktick_reference(clocktick_800_reference);

      geiger_tp_data gg_tp_data;
      _geiger_signal_to_tp_algo_.process(SSD_,
					 gg_tp_data);

      std::clog << "Debug 3" << std::endl;

      geiger_ctw_data gg_ctw_data;
      _geiger_tp_to_ctw_algo_.process(gg_tp_data, gg_ctw_data);


      calo_tp_data.tree_dump(std::clog, "Calorimeter TP(s) data : ", "INFO : ");
      calo_ctw_data.tree_dump(std::clog, "Calorimeter CTW(s) data : ", "INFO : ");

      gg_tp_data.tree_dump(std::clog, "Geiger TP(s) data : ", "INFO : ");
      gg_ctw_data.tree_dump(std::clog, "Geiger CTW(s) data : ", "INFO : ");

      _trigger_algo_.process(calo_ctw_data,
			     gg_ctw_data);

      return;
    }

    void digitization_driver::_process_readout_algorithms(const mctools::signal::signal_data & /*SSD_*/,
							 snemo::datamodel::sim_digi_data & /*SDD_*/)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");

      // todo

      return;
    }

    void digitization_driver::tree_dump(std::ostream & out_,
                                                 const std::string & title_,
                                                 const std::string & indent_,
                                                 bool inherit_) const
    {
      if (!title_.empty()) {
        out_ << indent_ << title_ << std::endl;
      }

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Logging : '"
           << datatools::logger::get_priority_label(_logging_priority_) << "'"
           << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Geometry manager : ";
      if (has_geometry_manager()) {
        out_ << "<yes>";
      } else {
        out_ << "<no>";
      }
      out_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Random generator initialized : ";
      if (_rdm_gen_.is_initialized()) {
	out_ << "<yes>";
      } else {
	out_ << "<no>";
      }
      out_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Electronic mapping initialized : ";
      if (_electronic_mapping_.is_initialized()) {
	out_ << "<yes>";
      } else {
	out_ << "<no>";
      }
      out_ << std::endl;




      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag(inherit_)
           << "Initialized : " << std::boolalpha << is_initialized() << std::endl;

      return;
    }

  }  // end of namespace digitization

}  // end of namespace snemo

/* OCD support */
#include <datatools/object_configuration_description.h>
DOCD_CLASS_IMPLEMENT_LOAD_BEGIN(snemo::digitization::digitization_driver, ocd_)
{
  ocd_.set_class_name("snemo::digitization::digitization_driver");
  ocd_.set_class_description("A driver class for the Digitization algorithm");
  ocd_.set_class_library("Falaise_Digitization");
  ocd_.set_class_documentation("The driver manager for the Digitization algorithms \n");

  ocd_.set_validation_support(true);
  ocd_.lock();
  return;
}
DOCD_CLASS_IMPLEMENT_LOAD_END() // Closing macro for implementation
DOCD_CLASS_SYSTEM_REGISTRATION(snemo::digitization::digitization_driver,
                               "snemo::digitization::digitization_driver")
