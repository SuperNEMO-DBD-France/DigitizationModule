// snemo/digitization/signal_to_calo_tp_algo.h
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_SIGNAL_TO_CALO_TP_ALGO_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_SIGNAL_TO_CALO_TP_ALGO_H

// Standard library :
#include <stdexcept>

// Third party:
// - Bayeux/datatools :
#include <datatools/logger.h>
// - Bayeux/mctools:
#include <mctools/signal/signal_data.h>
#include <mctools/signal/signal_shape_builder.h>
// - Bayeux/geomtools:
#include <geomtools/manager.h>
// - Bayeux/mygsl:
#include <mygsl/i_unary_function.h>

// Boost :
#include <boost/circular_buffer.hpp>

// This project :
#include <snemo/digitization/calo_tp_data.h>
#include <snemo/digitization/signal_data.h>
#include <snemo/digitization/electronic_mapping.h>
#include <snemo/digitization/clock_utils.h>

namespace snemo {

  namespace digitization {

    /// \brief Algorithm processing. Take signal data and fill calo trigger primitive data object.
    class signal_to_calo_tp_algo : boost::noncopyable
    {
    public :
			typedef boost::circular_buffer<double> circular_buffer;

			class calo_feb_config
			{
			public:
				calo_feb_config();
				virtual ~calo_feb_config();
				void initialize(const datatools::properties & config_);
				bool is_initialized() const;
				void reset();
				virtual void tree_dump(std::ostream & out_         = std::clog,
															 const std::string & title_  = "",
															 const std::string & indent_ = "",
															 bool inherit_               = false) const;

				int16_t acquisition_window_length; //<! Number of samples of the acquisition window
				bool   initialized;    //!< Initialization flag
				double low_threshold;  //!< Calorimeter Low threshold in Volts
				double high_threshold; //!< Calorimeter High threshold in Volts
				double sampling_rate;	 //!< Wavecatcher sampling rate
				double sampling_step;	 //!< Wavecatcher sampling step (based on sampling rate)
				double post_trig_window_ns; //!< Time to record after trigger
				unsigned int post_trig_window_samples; //!< Number of samples to record after trigger

			protected :
				void _set_defaults();

			};

			class calo_digi_working_data
			{
			public:
				calo_digi_working_data();
				virtual ~calo_digi_working_data();
				void reset();
 				bool operator<(const calo_digi_working_data &) const;
				void tree_dump(std::ostream & out_         = std::clog,
											 const std::string & title_  = "",
											 bool dump_digi_signal       = false,
											 const std::string & indent_ = "",
											 bool inherit_               = false) const;

				int32_t            hit_id;
				geomtools::geom_id geom_id;
				geomtools::geom_id channel_electronic_id;
				bool							 is_low_threshold_only;

				bool							 is_low_threshold;
				double						 low_threshold_trigger_time;
				bool							 is_high_threshold;
				double						 high_threshold_trigger_time;
				uint32_t					 clocktick_25;
				circular_buffer    calo_digitized_signal;
			};

			/// Default constructor
      signal_to_calo_tp_algo();

      /// Destructor
      virtual ~signal_to_calo_tp_algo();

      /// Initializing
      void initialize(const datatools::properties & config_,
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

		  /// Set the clocktick reference for the algorithm
			void set_clocktick_reference(uint32_t clocktick_ref_);

		  /// Set the clocktick shift
			void set_clocktick_shift(double clocktick_shift_);

      /// Process to fill a calo tp data object from signal data
			void process(const mctools::signal::signal_data & signal_data_,
									 calo_tp_data & my_calo_tp_data_);

    protected:

			// unsigned int _existing_same_electronic_id(const geomtools::geom_id & electronic_id_,
			// 																					calo_tp_data & my_calo_tp_data_);

			/// Set defaults parameters
			void _set_defaults();

      // Increment the running signal ID assigned to the next signal
      void _increment_running_digi_id();

			///  Process to fill a calo tp data object from signal data
			void _process(const mctools::signal::signal_data & signal_data_,
										calo_tp_data & my_calo_tp_data_);

    private :

			// Configuration:
      bool _initialized_;      //!< Initialization flag
      bool _active_main_wall_; //!< Main wall activation flag
      bool _active_xwall_;     //!< X-wall activation flag
      bool _active_gveto_;     //!< Gamma-veto activation flag

			uint32_t _clocktick_ref_;  //!< Clocktick reference of the algorithm
			double  _clocktick_shift_; //!< Clocktick shift between [0:25]
			electronic_mapping * _electronic_mapping_;     //!< Convert geometric ID into electronic ID
			mctools::signal::signal_shape_builder * _ssb_; //!< An external shape builder

			std::string _signal_category_;     //!< The calorimeter signal category
			calo_feb_config _calo_feb_config_; //!< Calorimeter Front-End board configuration

			// Working resources:
			int _running_digi_id_ = -1; //!< Give a new unique hit ID to calo WD
			std::vector<calo_digi_working_data> _calo_digi_data_collection_; //!< Temporary collection of calo digitized data


    };

  } // end of namespace digitization

} // end of namespace snemo


#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_SIGNAL_TO_CALO_TP_ALGO_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
