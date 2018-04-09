/// \file falaise/snemo/digitization/digitization_module.cc

// Third party:
// - Bayeux/datatools:
#include <bayeux/datatools/handle.h>
#include <datatools/service_manager.h>
// - Bayeux/geomtools:
#include <bayeux/geomtools/geometry_service.h>

// This project (Falaise):
#include <falaise/snemo/processing/services.h>
#include <falaise/snemo/datamodels/data_model.h>

// Ourselves:
#include <snemo/digitization/digitization_module.h>

namespace snemo {

  namespace digitization {

    // Registration instantiation macro
    DPP_MODULE_REGISTRATION_IMPLEMENT(digitization_module,
                                      "snemo::digitization::digitization_module")

    void digitization_module::_set_defaults_()
    {
      _SSD_label_.clear();
      _SDD_label_.clear();
      _Geo_label_.clear();
      _Db_label_.clear();
      _abort_at_missing_input_ = true;
      _abort_at_former_output_ = false;
      _preserve_former_output_ = false;
      _driver_->reset();
      return;
    }

    digitization_module::digitization_module(datatools::logger::priority logging_priority_)
      : dpp::base_module(logging_priority_)
    {
      _geometry_manager_ = nullptr;
      _set_defaults_();
      return;
    }

    digitization_module::~digitization_module()
    {
      if (is_initialized()) digitization_module::reset();
      return;
    }

    void digitization_module::initialize(const datatools::properties  & config_,
                                         datatools::service_manager   & service_manager_,
                                         dpp::module_handle_dict_type & /* module_dict_ */)

    {
      DT_THROW_IF (is_initialized(), std::logic_error,
                   "Module '" << get_name() << "' is already initialized ! ");

      dpp::base_module::_common_initialize(config_);

      /// Input SSD bank:
      if (_SSD_label_.empty()) {
        if (config_.has_key("SSD_label")) {
          _SSD_label_ = config_.fetch_string("SSD_label");
        }
      }
      // Default label:
      if (_SSD_label_.empty()) {
        _SSD_label_  = snemo::datamodel::data_info::default_simulated_signal_data_label();
      }

      /// Output SDD bank:
      if (_SDD_label_.empty()) {
        if (config_.has_key("SDD_label")) {
          _SDD_label_ = config_.fetch_string("SDD_label");
        }
      }
      // Default label:
      if (_SDD_label_.empty()) {
        _SDD_label_ = snemo::datamodel::data_info::default_simulated_digitized_data_label();
      }

      /*if (_db_manager_ == nullptr) */ {
        /// Db service:
        if (_Db_label_.empty()) {
          if (config_.has_key("Db_label")) {
            _Db_label_ = config_.fetch_string("Db_label");
          }
        }
        // Default label:
        // if (_Db_label_.empty()) {
        //   _Db_label_ = snemo::datamodel::data_info::default_database_service_label();
        // }
        // DT_THROW_IF(! service_manager_.has(_Db_label_) ||
        //             ! service_manager_.is_a<snemo::XXX::database_service>(_Db_label_),
        //             std::logic_error,
        //             "Module '" << get_name() << "' has no '" << _Db_label_ << "' service !");
        // const snemo::XXX::::database_service & Db = service_manager_.get<snemo::XXX::database_service>(_Db_label_);
        // set_db_manager(Db.get_geom_manager());
      }

      if (_geometry_manager_ == nullptr) {
        /// Geo service:
        if (_Geo_label_.empty()) {
          if (config_.has_key("Geo_label")) {
            _Geo_label_ = config_.fetch_string("Geo_label");
          }
        }
        // Default label:
        if (_Geo_label_.empty()) {
          _Geo_label_ = snemo::processing::service_info::default_geometry_service_label();
        }
        DT_THROW_IF(! service_manager_.has(_Geo_label_) ||
                    ! service_manager_.is_a<geomtools::geometry_service>(_Geo_label_),
                    std::logic_error,
                    "Module '" << get_name() << "' has no '" << _Geo_label_ << "' service !");
        const geomtools::geometry_service & Geo = service_manager_.get<geomtools::geometry_service>(_Geo_label_);
        set_geometry_manager(Geo.get_geom_manager());
      }
      DT_THROW_IF(_geometry_manager_ == nullptr, std::logic_error, "Missing geometry manager !");

      if (config_.has_key("abort_at_missing_input")) {
        set_abort_at_missing_input(config_.fetch_boolean("abort_at_missing_input"));
      }

      if (config_.has_key("abort_at_former_output")) {
        set_abort_at_former_output(config_.fetch_boolean("abort_at_former_output"));
      }

      if (config_.has_key("preserve_former_output")) {
        set_preserve_former_output(config_.fetch_boolean("preserve_former_output"));
      }

      // Driver :
      _driver_->initialize(config_);

      _set_initialized(true);
      return;
    }

    void digitization_module::reset()
    {
      DT_THROW_IF(! is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is not initialized !");

      _set_initialized(false);
      if (_driver_.get() != 0) {
        if (_driver_->is_initialized()) {
          _driver_->reset();
        }
        _driver_.reset();
      }
      _geometry_manager_ = nullptr;
      // _database_manager_ = nullptr;
      _set_defaults_();
      return;
    }

    void digitization_module::set_ssd_label(const std::string & lbl_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _SSD_label_ = lbl_;
      return;
    }

    const std::string & digitization_module::get_ssd_label() const
    {
      return _SSD_label_;
    }

    void digitization_module::set_sdd_label(const std::string & lbl_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _SDD_label_ = lbl_;
      return;
    }

    const std::string & digitization_module::get_sdd_label() const
    {
      return _SDD_label_;
    }

    void digitization_module::set_geo_label(const std::string & lbl_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _Geo_label_ = lbl_;
      return;
    }

    const std::string & digitization_module::get_geo_label() const
    {
      return _Geo_label_;
    }

    void digitization_module::set_db_label(const std::string & lbl_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _Db_label_ = lbl_;
      return;
    }

    const std::string & digitization_module::get_db_label() const
    {
      return _Db_label_;
    }

    bool digitization_module::has_database_manager() const
    {
      // return _database_manager_ != nullptr;
      return false;
    }

    // void digitization_module::set_database_manager(const database::manager & gmgr_)
    // {
    //   DT_THROW_IF(is_initialized(), std::logic_error,
    //               "Module '" << get_name() << "' is already initialized ! ");
    //   _geometry_manager_ = &gmgr_;
    //   return;
    // }

    // const database::manager & digitization_module::get_database_manager() const
    // {
    //   DT_THROW_IF(! is_initialized(), std::logic_error,
    //               "Module '" << get_name() << "' is not initialized ! ");
    //   return *_database_manager_;
    // }

    bool digitization_module::has_geometry_manager() const
    {
      return _geometry_manager_ != nullptr;
    }

    void digitization_module::set_geometry_manager(const geomtools::manager & geo_mgr_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _geometry_manager_ = & geo_mgr_;
      return;
    }

    const geomtools::manager & digitization_module::get_geometry_manager() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is not initialized ! ");
      return * _geometry_manager_;
    }

    bool digitization_module::is_abort_at_missing_input() const
    {
      return _abort_at_missing_input_;
    }

    void digitization_module::set_abort_at_missing_input(bool a_)
    {
      _abort_at_missing_input_ = a_;
      return;
    }

    bool digitization_module::is_abort_at_former_output() const
    {
      return _abort_at_former_output_;
    }

    void digitization_module::set_abort_at_former_output(bool a_)
    {
      _abort_at_former_output_ = a_;
      return;
    }

    bool digitization_module::is_preserve_former_output() const
    {
      return _preserve_former_output_;
    }

    void digitization_module::set_preserve_former_output(bool p_)
    {
      _preserve_former_output_ = p_;
      return;
    }

    dpp::base_module::process_status digitization_module::process(datatools::things & data_record_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is not initialized !");

      /*******************************
       * Check simulated signal data *
       *******************************/
      // Check if some 'mctools::signal::signal_data' are available in the data model:
      if (!data_record_.has(_SSD_label_))
        {
          DT_THROW_IF(is_abort_at_missing_input(), std::logic_error,
		      "Missing signal simulated data to be processed !");
          // leave the data unchanged.
          return dpp::base_module::PROCESS_ERROR;
        }
      // Get the 'mctools::signal::signal_data' entry from the data model :
      const mctools::signal::signal_data & the_signal_data = data_record_.get<mctools::signal::signal_data>(_SSD_label_);


      /**********************************
       * Check simulated digitized data *
       *********************************/

      // Add the new SDD bank:
      snemo::datamodel::sim_digi_data * ptr_sim_signal_data = nullptr;
      if (! data_record_.has(_SDD_label_)) {
	ptr_sim_signal_data = &(data_record_.add<snemo::datamodel::sim_digi_data>(_SDD_label_));
      } else {
	ptr_sim_signal_data = &(data_record_.grab<snemo::datamodel::sim_digi_data>(_SDD_label_));
      }

      snemo::datamodel::sim_digi_data & the_digi_data = *ptr_sim_signal_data;

      /********************
       * Process the data *
       ********************/

      // Main processing method :
      try {
	_process_(the_signal_data, the_digi_data);
      } catch (std::exception & error) {
        DT_LOG_ERROR(get_logging_priority(), error.what());
        return dpp::base_module::PROCESS_ERROR;
      }

      return dpp::base_module::PROCESS_SUCCESS;
    }

    void digitization_module::_process_(const mctools::signal::signal_data & SSD_,
					snemo::datamodel::sim_digi_data & SDD_)
    {
      DT_LOG_TRACE(get_logging_priority(), "Entering...");

      _driver_->process(SSD_, SDD_);

      DT_LOG_TRACE(get_logging_priority(), "Exiting.");
      return;
    }

  } // end of namespace digitization

} // end of namespace snemo

/* OCD support */
#include <datatools/object_configuration_description.h>
DOCD_CLASS_IMPLEMENT_LOAD_BEGIN(snemo::digitization::digitization_module, ocd_)
{
  ocd_.set_class_name("snemo::digitization::digitization_module");
  ocd_.set_class_description("A module that performs a selection on simulated data based on the number of PMT and GG cells hit");
  ocd_.set_class_library("Falaise_Digitization");
  ocd_.set_class_documentation(" \n"
                               " \n"
                               );

  // Invoke OCD support from parent class :
  dpp::base_module::common_ocd(ocd_);

  {
    // Description of the 'SSD_label' configuration property :
    datatools::configuration_property_description & cpd
      = ocd_.add_property_info();
    cpd.set_name_pattern("SSD_label")
      .set_terse_description("The label/name of the 'simulated signal data' bank")
      .set_traits(datatools::TYPE_STRING)
      .set_mandatory(false)
      .set_long_description("This is the name of the input bank to be used  \n"
                            "after a simulation. \n")
      .set_default_value_string(snemo::datamodel::data_info::default_simulated_signal_data_label())
      .add_example("Use an alternative name for the \n"
                   "'signal data' bank::            \n"
                   "                                \n"
                   "  SSD_label : string = \"SSD2\" \n"
                   "                                \n"
                   )
      ;
  }

  {
    // Description of the 'SSD_label' configuration property :
    datatools::configuration_property_description & cpd
      = ocd_.add_property_info();
    cpd.set_name_pattern("SDD_label")
      .set_terse_description("The label/name of the 'simulated digitized data' bank")
      .set_traits(datatools::TYPE_STRING)
      .set_mandatory(false)
      .set_long_description("This is the name of the output bank. \n")
      .set_default_value_string(snemo::datamodel::data_info::default_simulated_digitized_data_label())
      .add_example("Use an alternative name for the \n"
                   "'signal data' bank::            \n"
                   "                                \n"
                   "  SDD_label : string = \"SDD2\" \n"
                   "                                \n"
                   )
      ;
  }

  ocd_.set_validation_support(true);
  ocd_.lock();

  return;
}

DOCD_CLASS_IMPLEMENT_LOAD_END() // Closing macro for implementation
DOCD_CLASS_SYSTEM_REGISTRATION(snemo::digitization::digitization_module,
                               "snemo::digitization::digitization_module")

// end of falaise/snemo/digitization/digitization_module.cc
