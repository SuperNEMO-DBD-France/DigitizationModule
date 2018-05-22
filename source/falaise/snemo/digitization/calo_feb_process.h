// snemo/digitization/calo_feb_process.h
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_FEB_PROCESS_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_FEB_PROCESS_H

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

// - Falaise:
#include <falaise/snemo/datamodels/sim_digi_data.h>

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
    class calo_feb_process : boost::noncopyable
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

				static const unsigned int DEFAULT_ADC_DYNAMIC = 4096;
				static const double DEFAULT_VOLTAGE_DYNAMIC = 2.5 * CLHEP::volt;
				static const unsigned int DEFAULT_ZERO_ADC_POS = 2048;
				const double VOLT_ADC_VALUE = DEFAULT_VOLTAGE_DYNAMIC / DEFAULT_ADC_DYNAMIC;

				unsigned int adc_dynamic = DEFAULT_ADC_DYNAMIC; //!< ADC dynamic
				bool    external_trigger; //!< External trigger activated
				bool    calo_tp_spare;    //!< Calo TP spare bit activated
				int16_t acquisition_window_length; //<! Number of samples of the acquisition window
				bool    initialized;     //!< Initialization flag
				double  low_threshold;   //!< Calorimeter Low threshold in Volts
				double  high_threshold;  //!< Calorimeter High threshold in Volts
				double  sampling_rate;	 //!< Wavecatcher sampling rate
				double  sampling_step;	 //!< Wavecatcher sampling step (based on sampling rate)
				double  post_trig_window_ns; //!< Time to record after trigger
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
				uint32_t					 low_threshold_CT_25;
				uint32_t					 high_threshold_CT_25;

				circular_buffer    calo_digitized_signal;
			};

			/// Default constructor
      calo_feb_process();

      /// Destructor
      virtual ~calo_feb_process();

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

			/// Return the collection of calo digi working data
			const std::vector<calo_feb_process::calo_digi_working_data> get_calo_digi_working_data_vector() const;

			/// Clear temporary working data
			void clear_working_data();

			/// Process to fill a calo tp data object from signal data
			void trigger_process(const mctools::signal::signal_data & SSD_,
													 calo_tp_data & my_calo_tp_data_);

      /// Process to fill simulated digitized data calo digitized hit collection
			void readout_process(snemo::datamodel::sim_digi_data & SDD_);

    protected:

			/// Set defaults parameters
			void _set_defaults();

      // Increment the running digi ID assigned to the next signal
      void _increment_running_digi_id();

      // Increment the running digi ID assigned to the next signal
      void _increment_running_tp_id();

      // Increment the running digi ID assigned to the next signal
      void _increment_running_readout_id();

			///  Process to fill a calo tp data object from signal data
			void _trigger_process(const mctools::signal::signal_data & SSD_,
														calo_tp_data & my_calo_tp_data_);

      /// Process to fill simulated digitized data calo digitized hit collection
			void _readout_process(snemo::datamodel::sim_digi_data & SDD_);

		private :

			// Configuration:
      bool _initialized_;      //!< Initialization flag
			bool _active_main_wall_; //!< Main wall activation flag
			bool _active_xwall_;		 //!< X-wall activation flag
			bool _active_gveto_;  	 //!< Gamma-veto activation flag

			clock_utils * _clock_utils_;             //!< An external SuperNEMO digitization clock manager
			electronic_mapping * _electronic_mapping_;     //!< Convert geometric ID into electronic ID
			mctools::signal::signal_shape_builder * _ssb_; //!< An external shape builder

			std::string _signal_category_;     //!< The calorimeter signal category
			calo_feb_config _calo_feb_config_; //!< Calorimeter Front-End board configuration

			// Working resources:
			int _running_digi_id_;    //!< Give a new unique hit ID to calo WD
			int _running_tp_id_;      //!< Give a new unique hit ID to calo TP
			int _running_readout_id_; //!< Give a new unique hit ID to calo digitized hit (readout)

			std::vector<calo_digi_working_data> _calo_digi_data_collection_; //!< Temporary collection of calo digitized data

    };

  } // end of namespace digitization

} // end of namespace snemo


#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_CALO_FEB_PROCESS_H

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
