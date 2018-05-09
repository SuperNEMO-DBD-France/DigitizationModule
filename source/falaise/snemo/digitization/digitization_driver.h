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
 *   A driver  class that  wraps the Digitization  algorithms, trigger
 *   and readout.
 *
 * History:
 *
 */

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DIGITIZATION_DRIVER_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DIGITIZATION_DRIVER_H 1

// Standard library:
#include <string>

// Third party:
// - Bayeux/geomtools:
#include <bayeux/geomtools/manager.h>
// - Bayeux/mctools:
#include <mctools/signal/signal_data.h>
// - Falaise:
#include <falaise/snemo/datamodels/sim_digi_data.h>

// This project:
#include <falaise/snemo/digitization/signal_to_calo_tp_algo.h>
#include <falaise/snemo/digitization/signal_to_geiger_tp_algo.h>
#include <falaise/snemo/digitization/calo_tp_to_ctw_algo.h>
#include <falaise/snemo/digitization/geiger_tp_to_ctw_algo.h>
#include <falaise/snemo/digitization/trigger_algorithm.h>

namespace snemo {

  namespace digitization {

    /// Driver for the digitization algorithms (trigger+readout)
    class digitization_driver
    {
    public:

      /// Constructor
      digitization_driver();

      /// Destructor
      virtual ~digitization_driver();

      /// Set logging priority level
      void set_logging_priority(datatools::logger::priority logging_priority_);

      /// Get logging priority
      datatools::logger::priority get_logging_priority() const;

      /// Check the geometry manager
      bool has_geometry_manager() const;

      /// Address the geometry manager
      void set_geometry_manager(const geomtools::manager & gmgr_);

      /// Return a non-mutable reference to the geometry manager
      const geomtools::manager & get_geometry_manager() const;

			/// Check the electronic mapping
      bool has_electronic_mapping() const;

      /// Address the electronic mapping
      void set_electronic_mapping(electronic_mapping & emap_);

      /// Return a non-mutable reference to the electronic mapping
      const electronic_mapping & get_electronic_mapping() const;

			/// Return a mutable reference to the electronic mapping
			electronic_mapping & grab_electronic_mapping() const;

			/// Check the clock utils
      bool has_clock_utils() const;

      /// Address the clock utils
      void set_clock_utils(const clock_utils & cu_);

      /// Return a non-mutable reference to the clock utils
      const clock_utils & get_clock_utils() const;

      /// Initialize the digitization driver through configuration properties
			virtual void initialize(const datatools::properties & config_);

      /// Check the initialization status
      bool is_initialized() const;

      /// Reset the driver
      virtual void reset();

      /// Run the algorithm
      virtual void process(const mctools::signal::signal_data & SSD_,
													 snemo::datamodel::sim_digi_data & SDD_);

      // Smart print
      virtual void tree_dump(std::ostream & out_ = std::clog,
                             const std::string & title_ = "",
                             const std::string & indent_ = "",
                             bool inherit_ = false) const;

		protected:

			/// Set default attribute values
      void _set_defaults();

      /// Process digitization algorithms ('SSD' to trigger for the moment)
      void _process_digitization_algorithms(const mctools::signal::signal_data & SSD_,
																						snemo::datamodel::sim_digi_data & SDD_);

      /// Process readout algorithms (TO DO)
      void _process_readout_algorithms(const mctools::signal::signal_data & SSD_,
																			 snemo::datamodel::sim_digi_data & SDD_);

    private:

      /// Set the initialization flag
      void _set_initialized_(bool);

    private:

      // Configuration:
      bool _initialized_; //!< Initialization status
      datatools::logger::priority _logging_priority_;  //!< Logging priority
      const geomtools::manager * _geometry_manager_;   //!< The SuperNEMO geometry manager
			electronic_mapping * _electronic_mapping_; //!< The SuperNEMO electronic mapping
			const clock_utils * _clock_utils_;               //!< The SuperNEMO digitization clock utils


      // Algorithms:
      snemo::digitization::signal_to_calo_tp_algo   _calo_signal_to_tp_algo_;   //!< Calo signal to calo trigger primitive (TP) algo
      snemo::digitization::signal_to_geiger_tp_algo _geiger_signal_to_tp_algo_; //!< Geiger signal to geiger  TP algo
      snemo::digitization::calo_tp_to_ctw_algo      _calo_tp_to_ctw_algo_;      //!< Calo TP to crate trigger word algo
      snemo::digitization::geiger_tp_to_ctw_algo    _geiger_tp_to_ctw_algo_;    //!< Geiger TP to crate trigger word algo
      snemo::digitization::trigger_algorithm        _trigger_algo_;             //!< The trigger algorithm

    };

  }  // end of namespace digitization

}  // end of namespace snemo


// Declare the OCD interface of the module
#include <datatools/ocd_macros.h>
DOCD_CLASS_DECLARATION(snemo::digitization::digitization_driver)

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DIGITIZATION_DRIVER_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
