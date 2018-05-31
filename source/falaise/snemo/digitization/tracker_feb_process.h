// snemo/digitization/tracker_feb_process.h
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Francois MAUGER <mauger@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_TRACKER_FEB_PROCESS_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_TRACKER_FEB_PROCESS_H

// Standard library :
#include <stdexcept>
#include <algorithm>

// Third party:
// - Bayeux/datatools :
#include <datatools/logger.h>
// - Bayeux/mctools:
#include <mctools/signal/signal_data.h>
#include <mctools/signal/signal_shape_builder.h>
#include <mctools/signal/utils.h>
// - Bayeux/geomtools:
#include <geomtools/manager.h>
// - Bayeux/mygsl:
#include <mygsl/i_unary_function_with_derivative.h>

// - Falaise:
#include <falaise/snemo/datamodels/sim_digi_data.h>
#include <falaise/snemo/datamodels/sim_tracker_digi_hit.h>

// This project :
#include <snemo/digitization/electronic_mapping.h>
#include <snemo/digitization/tempo_utils.h>
#include <snemo/digitization/mapping.h>
#include <snemo/digitization/clock_utils.h>
#include <snemo/digitization/geiger_tp_data.h>
#include <snemo/digitization/geiger_tp_constants.h>
#include <snemo/digitization/trigger_structures.h>

namespace snemo {

  namespace digitization {

    /// \brief Algorithm processing. Take Simulated Signal Data 'SSD' bank and fill geiger trigger primitive 'TP' data object.
    class tracker_feb_process : boost::noncopyable
    {
    public :

			class geiger_feb_config
			{
			public:
				geiger_feb_config();
				virtual ~geiger_feb_config();
				void initialize(const datatools::properties & config_);
				bool is_initialized() const;
				void reset();
				virtual void tree_dump(std::ostream & out_         = std::clog,
															 const std::string & title_  = "",
															 const std::string & indent_ = "",
															 bool inherit_               = false) const;

				bool   initialized;    //!< Initialization flag
				double VLNT; //!< Anodic low negative threshold in Volts
				double VHNT; //!< Anodic high negative threshold in Volts
				double VHPT; //!< Anodic high positive threshold in Volts
				double VCPT; //!< Cathodic positive threshold in Volts

			protected :
				void _set_defaults();

			};

			class geiger_digi_working_data
			{
			public:
				geiger_digi_working_data();
        virtual ~geiger_digi_working_data();
				void reset();
				void readout(snemo::datamodel::sim_tracker_digi_hit & STDH_);
 				bool operator<(const geiger_digi_working_data &) const;
				void tree_dump(std::ostream & out_         = std::clog,
											 const std::string & title_  = "",
											 bool dump_signal_           = false,
											 const std::string & indent_ = "",
											 bool inherit_               = false) const;

				// Probably attributes to remove :
				// Keep it for the moment for debug / visualization
				const mctools::signal::base_signal * signal_ref;
				mygsl::unary_function_promoted_with_numeric_derivative signal_deriv;
				///////

				int32_t            hit_id;
				geomtools::geom_id anodic_gid;
				geomtools::geom_id cathodic_bottom_gid;
				geomtools::geom_id cathodic_top_gid;

				geomtools::geom_id anodic_eid;
				geomtools::geom_id cathodic_bottom_eid;
				geomtools::geom_id cathodic_top_eid;

				double event_time_reference;
				double trigger_time;
				double anodic_R0;
				double anodic_R1;
				double anodic_R2;
				double anodic_R3;
				double anodic_R4;
				double cathodic_R5;
				double cathodic_R6;

				std::string cathode_top_register;
				std::string cathode_bottom_register;

				uint32_t  clocktick_800;
				bool has_been_readout;
			};

			typedef std::vector<geiger_digi_working_data> gg_digi_working_data_collection_type;

      /// Default constructor
      tracker_feb_process();

      /// Destructor
      virtual ~tracker_feb_process();

      /// Initializing
      void initialize(const datatools::properties & config_,
											clock_utils & my_clock_utils_,
											electronic_mapping & my_electronic_mapping_,
											mctools::signal::signal_shape_builder & my_ssb_);

      /// Check if the algorithm is initialized
      bool is_initialized() const;

      /// Reset the object
      void reset();

      /// Check the signal category
      bool has_signal_category() const;

      // Set the signal category
      void set_signal_category(const std::string & category_);

      /// Return the signal category
      const std::string & get_signal_category() const;

			/// Add a geiger tp from a working data
			void add_geiger_tp(const geiger_digi_working_data & my_wd_data_,
												 uint32_t signal_clocktick_,
												 geiger_tp_data & my_geiger_tp_data_);

			/// Update a geiger tp
			void update_gg_tp(const geiger_digi_working_data & my_wd_data_,
												geiger_tp & my_geiger_tp_);

			/// Clear temporary working data
			void clear_working_data();

      /// Process to fill a geiger tp data object from simulated data
      void trigger_process(const mctools::signal::signal_data & SSD_,
													 geiger_tp_data & my_geiger_tp_data_);

      /// Process to fill simulated digitized data tracker digitized hit collection
			void readout_process(const trigger_structures::L2_decision_gate & L2_,
													 snemo::datamodel::sim_digi_data & SDD_);

    protected:

			/// Set internal parameters to default values
			void _set_defaults();

			/// Prepare the working data collection (sort by clocktick)
			void _prepare_working_data(const mctools::signal::signal_data & SSD_,
																 gg_digi_working_data_collection_type & wd_collection_);

			/// Sort working data by clocktick
			void _sort_working_data(gg_digi_working_data_collection_type & wd_collection_);

			/// Create geiger tp from working data collection
			void _geiger_tp_process(const gg_digi_working_data_collection_type & wd_collection_,
															geiger_tp_data & my_geiger_tp_data_);

      // Increment the running TP ID assigned to the next GG TP
      void _increment_running_tp_id();

      // Increment the running readout ID assigned to the next tracker digi signal
      void _increment_running_readout_id();

      ///  Process to fill a geiger tp data object from signal data
      void _trigger_process(const mctools::signal::signal_data & SSD_,
														geiger_tp_data & my_geiger_tp_data_);

      /// Process to fill simulated digitized data tracker digitized hit collection
			void _readout_process(const trigger_structures::L2_decision_gate & L2_,
														snemo::datamodel::sim_digi_data & SDD_);

    private :

			// Configuration :
      bool    _initialized_;      //!< Initialization flag

			clock_utils * _clock_utils_;                   //!< The SuperNEMO digitization clock utils
			electronic_mapping * _electronic_mapping_;     //!< Convert geometric ID into electronic ID
			mctools::signal::signal_shape_builder * _ssb_; //!< An external shape builder

			std::string _signal_category_; //!< Identifier of the input tracker signal category
			geiger_feb_config _gg_feb_config_; //!< The Geiger FEB configuration

			// Working resources :
			int _running_tp_id_; //!< Give a new unique hit ID to calo TP
      int _running_readout_id_;

			uint64_t last_readout_CT_1600;


			bool _activated_bits_[geiger::tp::TP_SIZE]; //!< Table of booleans to see which bits were activated
			gg_digi_working_data_collection_type _gg_digi_data_collection_; //!< Temporary collection of tracker digitized data

    };

  } // end of namespace digitization

} // end of namespace snemo


#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_TRACKER_FEB_PROCESS_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
