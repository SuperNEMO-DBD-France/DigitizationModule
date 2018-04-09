/// \file falaise/snemo/digitization/digitization_driver.cc

// Ourselves:
#include <snemo/digitization/digitization_driver.h>

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

    void digitization_driver::initialize(const datatools::properties & /* setup_ */ )
    {
      DT_THROW_IF (is_initialized(), std::logic_error, "Driver is already initialized !");
      DT_THROW_IF (!has_geometry_manager(), std::logic_error, "No geometry manager is setup !");

      // do something

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
      return;
    }

    void digitization_driver::process(const mctools::signal::signal_data & SSD_,
				      snemo::datamodel::sim_digi_data & SDD_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");
      _process_digitization_algorithms(SSD_, SDD_);
      _process_readout_algorithms(SSD_, SDD_);

      return;
    }

    void digitization_driver::_process_digitization_algorithms(const mctools::signal::signal_data & /*SSD_*/,
                                                              snemo::datamodel::sim_digi_data & /*SDD_*/)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Not initialized !");

      // todo


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
