// -*- mode: c++ ; -*-
/** \file falaise/snemo/digitization/digitization_module.h
 * Author(s) :    Yves Lemiere <lemiere@lpccaen.in2p3.fr>
 *                Francois Mauger <mauger@lpccaen.in2p3.fr>
 *                Guillaume Oliviero <goliviero@lpccaen.in2p3.fr>
 * Creation date: 2016
 * Last modified: 2018
 *
 * Copyright (C) 2018 Guillaume Oliviero <goliviero@lpccaen.in2p3.fr>
 *
 * This  program is  free  software; you  can  redistribute it  and/or
 * modify it  under the  terms of  the GNU  General Public  License as
 * published by the Free Software  Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or FITNESS  FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 *
 * You should have  received a copy of the GNU  General Public License
 * along  with  this program;  if  not,  write  to the  Free  Software
 * Foundation,  Inc.,  51 Franklin  Street,  Fifth  Floor, Boston,  MA
 * 02110-1301, USA.
 *
 * Description:
 *
 *   Module  for digitization  within  SuperNEMO.  It takes  Simulated
 *   Signal Data 'SSD' bank and produce Simulated Digitized Data 'SDD'
 *   bank
 *
 * History:
 *
 */


#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DIGITIZATION_MODULE_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DIGITIZATION_MODULE_H

// Third party :
// - Bayeux/dpp:
#include <dpp/base_module.h>

// - Bayeux/mctools:
#include <mctools/signal/signal_data.h>

// - Bayeux/geomtools:
#include <bayeux/geomtools/manager.h>

// - Falaise:
#include <falaise/snemo/datamodels/sim_digi_data.h>

// This project:
#include <snemo/digitization/digitization_driver.h>


namespace snemo {

  namespace digitization {

    /// \brief Digitization module takes simulated signal data bank as input
    // and construct a simulated digitized data bank which contains trigger
    // data and readout data
    class digitization_module
      : public dpp::base_module
    {
    public :

      /// Constructor
      digitization_module(datatools::logger::priority logging_priority_ = datatools::logger::PRIO_FATAL);

      /// Destructor
      virtual ~digitization_module();

      /// Initialization
      virtual void initialize(const datatools::properties  & config_,
                              datatools::service_manager   & service_manager_,
                              dpp::module_handle_dict_type & /* module_dict_ */);

      /// Reset
      virtual void reset();

      /// Data record processing
      virtual process_status process(datatools::things & data_);

      /// Set the 'simulated signal data' bank label
      void set_ssd_label(const std::string &);

      /// Return the 'simulated signal data' bank label
      const std::string & get_ssd_label() const;

      /// Set the 'simulated digitized data' bank label
      void set_sdd_label(const std::string &);

      /// Return the 'simulated digitized data' bank label
      const std::string & get_sdd_label() const;

      /// Set the 'geometry service' label
      void set_geo_label(const std::string &);

      /// Return the 'geometry service' label
      const std::string & get_geo_label() const;

      /// Set the 'database service' label
      void set_db_label(const std::string &);

      /// Return the 'database service' label
      const std::string & get_db_label() const;

      /// Check database manager
      bool has_database_manager() const;

      // /// Setting database manager
      // void set_database_manager(const database::manager & gmgr_);

      // /// Getting database manager
      // const database::manager & get_database_manager() const;

      /// Check the geometry manager
      bool has_geometry_manager() const;

      /// Address the geometry manager
      void set_geometry_manager(const geomtools::manager & gmgr_);

      /// Return a non-mutable reference to the geometry manager
      const geomtools::manager & get_geometry_manager() const;

      bool is_abort_at_missing_input() const;
      void set_abort_at_missing_input(bool);
      bool is_abort_at_former_output() const;
      void set_abort_at_former_output(bool);
      bool is_preserve_former_output() const;
      void set_preserve_former_output(bool);

    private :

      /// Process the simulated signal data and build the simulated digitized data
      void _process_(const mctools::signal::signal_data & SSD_,
		     snemo::datamodel::sim_digi_data & SDD_);

      /// Give default values to specific class members.
      void _set_defaults_();

    private :

      // Configuration:
      std::string _SSD_label_; //!< The label of the simulated data bank
      std::string _SDD_label_; //!< The label of the simulated digitized data bank (output)
      std::string _Geo_label_; //!< The label of the geometry service
      std::string _Db_label_;  //!< The label of the database service
      bool _abort_at_missing_input_ = true;
      bool _abort_at_former_output_ = false;
      bool _preserve_former_output_ = false;

      const geomtools::manager * _geometry_manager_;  //!< The SuperNEMO geometry manager
      // const snemo::XXX::manager * _database_manager_ = nullptr; //!< The database manager
      boost::scoped_ptr<snemo::digitization::digitization_driver> _digi_driver_; //!< Digitization driver :

      // Macro to automate the registration of the module :
      DPP_MODULE_REGISTRATION_INTERFACE(digitization_module)
    };

  } // end of namespace digitization

} // end of namespace snemo

#include <datatools/ocd_macros.h>

// Declare the OCD interface of the module
DOCD_CLASS_DECLARATION(snemo::digitization::digitization_module)

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DIGITIZATION_MODULE_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
