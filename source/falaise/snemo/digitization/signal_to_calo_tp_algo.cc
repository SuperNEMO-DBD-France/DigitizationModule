// snemo/digitization/signal_to_calo_tp_algo.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// This project :
#include <snemo/digitization/clock_utils.h>

// Ourselves:
#include <snemo/digitization/signal_to_calo_tp_algo.h>
#include <snemo/digitization/tempo_utils.h>

namespace snemo {

  namespace digitization {

    signal_to_calo_tp_algo::signal_to_calo_tp_algo()
    {
      _initialized_ = false;
      _electronic_mapping_ = 0;
      _clocktick_ref_ = clock_utils::INVALID_CLOCKTICK;
      datatools::invalidate(_clocktick_shift_);
      return;
    }

    signal_to_calo_tp_algo::~signal_to_calo_tp_algo()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void signal_to_calo_tp_algo::initialize(const datatools::properties & config_,
					    electronic_mapping & my_electronic_mapping_,
					    mctools::signal::signal_shape_builder & my_ssb_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Signal to calo tp algorithm is already initialized ! ");
      _set_defaults();
      _electronic_mapping_ = & my_electronic_mapping_;
      _ssb_ = & my_ssb_;

      if (config_.has_key("signal_category")) {
	std::string signal_category = config_.fetch_string("signal_category");
        _signal_category_ = signal_category;
      }

      if (config_.has_key("low_threshold")) {
        double low_threshold = config_.fetch_real_with_explicit_dimension("low_threshold", "electric_potential");
        _low_threshold_ = low_threshold;
      }

      if (config_.has_key("high_threshold")) {
        double high_threshold = config_.fetch_real_with_explicit_dimension("high_threshold", "electric_potential");
        _high_threshold_ = high_threshold;
      }

      _initialized_ = true;
      return;
    }

    bool signal_to_calo_tp_algo::is_initialized() const
    {
      return _initialized_;
    }

    void signal_to_calo_tp_algo::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "SD to calo tp algorithm is not initialized, it can't be reset ! ");
      _initialized_ = false;
      _electronic_mapping_ = 0;
      _clocktick_ref_ = clock_utils::INVALID_CLOCKTICK;
      datatools::invalidate(_clocktick_shift_);
      return;
    }

    bool signal_to_calo_tp_algo::has_signal_category() const
    {
      return !_signal_category_.empty();
    }

    const std::string & signal_to_calo_tp_algo::get_signal_category() const
    {
      return _signal_category_;
    }

    void signal_to_calo_tp_algo::set_signal_category(const std::string & category_)
    {
      _signal_category_ = category_;
      return;
    }

    void signal_to_calo_tp_algo::set_clocktick_reference(uint32_t clocktick_ref_)
    {
      _clocktick_ref_ = clocktick_ref_;
      return;
    }

    void signal_to_calo_tp_algo::set_clocktick_shift(double clocktick_shift_)
    {
      _clocktick_shift_ = clocktick_shift_;
      return;
    }

    void signal_to_calo_tp_algo::_set_defaults()
    {
      _signal_category_ = "sigcalo";
      _low_threshold_  = -0.01 * CLHEP::volt;
      _high_threshold_ = -0.02 * CLHEP::volt;

      return;
    }

    void signal_to_calo_tp_algo::process(const mctools::signal::signal_data & SSD_,
					 calo_tp_data & my_calo_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to calo TP algorithm is not initialized ! ");

      /////////////////////////////////
      // Check simulated signal data //
      /////////////////////////////////

      // // Check if some 'simulated_data' are available in the data model:
      if (SSD_.has_signals(get_signal_category())) {
	/********************
	 * Process the data *
	 ********************/

	// Main processing method :
	_process(SSD_, my_calo_tp_data_);
      }

      return;
    }

    void signal_to_calo_tp_algo::_process(const mctools::signal::signal_data & SSD_,
					  calo_tp_data & my_calo_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to calo TP algorithm is not initialized ! ");
      std::size_t number_of_hits = SSD_.get_number_of_signals(get_signal_category());

      std::clog << "Signal category = " << _signal_category_
		<< " LT = " << _low_threshold_ / CLHEP::volt
		<< " HT = " << _high_threshold_ / CLHEP::volt << std::endl;


      std::clog << "Number of signals = " << number_of_hits << std::endl;

      // Build the shape for each signal and give the unary function promoted with numeric derivative to the working data:
      for (std::size_t i = 0; i < number_of_hits; i++)
	{
	  const mctools::signal::base_signal a_signal = SSD_.get_signals(get_signal_category())[i].get();
	  // a_signal.tree_dump(std::clog, "A calo signal");

	  mctools::signal::base_signal a_mutable_signal = a_signal;
	  std::string signal_name;
	  snemo::digitization::build_signal_name(a_mutable_signal.get_hit_id(),
						 signal_name);

	  a_mutable_signal.build_signal_shape(*_ssb_,
					      signal_name,
					      a_mutable_signal);

	  a_mutable_signal.tree_dump(std::clog, "Mutable signal");


	  const mygsl::i_unary_function * signal_functor = & a_mutable_signal.get_shape();
	  unsigned int nsamples = 1000;
	  double xmin = signal_functor->get_non_zero_domain_min();
	  double xmax = signal_functor->get_non_zero_domain_max();

	  // std::clog << "Isignal = " << i <<  " Xmin = " << xmin << " Xmax = " << xmax << std::endl;

	  if (i == 0) {
	    const std::string filename = "/tmp/calo_signal.dat";
	    signal_functor->write_ascii_file(filename, xmin, xmax,  nsamples);
	  }

	  bool high_threshold_trigger = false;
	  double high_threshold_time;
	  datatools::invalidate(high_threshold_time);

	  bool low_threshold_only_trigger = false;
	  double low_threshold_only_time;
	  datatools::invalidate(low_threshold_only_time);

	  for (double x = xmin - (50 / nsamples); x < xmax + (50 / nsamples); x+= (1. / nsamples))
	    {
	      double y = signal_functor->eval(x);
	      y *= 1. / CLHEP::volt;

	      // if (i == 0) std::clog << "x = " << x << " y = " << y << std::endl;
	      // std::clog << "x = " << x << " y = " << y << std::endl;

	      if (y <= _high_threshold_ / CLHEP::volt)
		{
		  high_threshold_trigger = true;
		  high_threshold_time = x;
		}

	      if (!low_threshold_only_trigger
		  && y <= _low_threshold_ / CLHEP::volt
		  && y > _high_threshold_ / CLHEP::volt
		  && !high_threshold_trigger)
		{
		  low_threshold_only_trigger = true;
		  low_threshold_only_time = x;
		}

	      // std::clog << "Is HT  : " << high_threshold_trigger << " Time = " << high_threshold_time << std::endl;
	      // std::clog << "Is LTO : " << low_threshold_only_trigger << " Time = " << low_threshold_only_time << std::endl;

	      if (high_threshold_trigger) {
		low_threshold_only_trigger = false;
		datatools::invalidate(low_threshold_only_time);
		break;
	      }
	    }

	  std::clog << "HT / LTO status for the signal :  HT ["<< high_threshold_trigger <<"] time = " << high_threshold_time << std::endl;
	  std::clog << "LTO ["<< low_threshold_only_trigger <<"] time = " << low_threshold_only_time << std::endl;

	  const geomtools::geom_id & geom_id = a_mutable_signal.get_geom_id();
	  geomtools::geom_id electronic_id;
	  _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE,
						   geom_id,
						   electronic_id);
	  std::clog << "GID = " << geom_id << " EID = " << electronic_id << std::endl;

	  uint32_t a_calo_signal_clocktick = _clocktick_ref_ + clock_utils::CALO_FEB_SHIFT_CLOCKTICK_NUMBER;

	  // These bits have to be checked
	  bool calo_xt_bit    = 0;
	  bool calo_spare_bit = 0;

	  // Signal is LTO :
	  if (low_threshold_only_trigger)
	    {
	      if (low_threshold_only_time > 25) // nanoseconds
		{
		  a_calo_signal_clocktick += static_cast<uint32_t>(low_threshold_only_time / 25);
		}

	      bool already_existing_tp = my_calo_tp_data_.existing_tp(electronic_id,
								      a_calo_signal_clocktick);

	      if (already_existing_tp)
		{
		  unsigned int existing_index = my_calo_tp_data_.get_existing_tp_index(electronic_id,
										       a_calo_signal_clocktick);
		  // Update existing calo TP
		  snemo::digitization::calo_tp & existing_calo_tp = my_calo_tp_data_.grab_calo_tps()[existing_index].grab();
		  existing_calo_tp.set_lto_bit(1);
		}
	      else
		{
		  // Create new calo TP
		  snemo::digitization::calo_tp & calo_tp = my_calo_tp_data_.add();
		  calo_tp.set_header(a_mutable_signal.get_hit_id(),
				     electronic_id,
				     a_calo_signal_clocktick);
		  calo_tp.set_lto_bit(1);
		  calo_tp.set_xt_bit(calo_xt_bit);
		  calo_tp.set_spare_bit(calo_spare_bit);
		}


	    }

	  // Signal is HT :
	  if (high_threshold_trigger)
	    {

	      if (high_threshold_time > 25) // nanoseconds
		{
		  a_calo_signal_clocktick += static_cast<uint32_t>(high_threshold_time / 25);
		}

	      bool already_existing_tp = my_calo_tp_data_.existing_tp(electronic_id,
								      a_calo_signal_clocktick);

	      if (already_existing_tp)
		{
		  unsigned int existing_index = my_calo_tp_data_.get_existing_tp_index(electronic_id,
										       a_calo_signal_clocktick);
		  // Update existing calo TP
		  snemo::digitization::calo_tp & existing_calo_tp = my_calo_tp_data_.grab_calo_tps()[existing_index].grab();
		  unsigned int existing_multiplicity = existing_calo_tp.get_htm();
		  existing_multiplicity += 1;
		  existing_calo_tp.set_htm(existing_multiplicity);
		}
	      else
		{

		  // Create new calo TP
		  snemo::digitization::calo_tp & calo_tp = my_calo_tp_data_.add();
		  calo_tp.set_header(a_mutable_signal.get_hit_id(),
				     electronic_id,
				     a_calo_signal_clocktick);
		  calo_tp.set_htm(1);
		  calo_tp.set_xt_bit(calo_xt_bit);
		  calo_tp.set_spare_bit(calo_spare_bit);
		}
	    }

	} // end of i signal

      my_calo_tp_data_.tree_dump(std::clog, "CALO TP DATA");

      for(uint32_t i = my_calo_tp_data_.get_clocktick_min(); i <= my_calo_tp_data_.get_clocktick_max(); i++)
      	{
      	  for(unsigned int j = 0 ; j <= mapping::NUMBER_OF_CRATES ; j++)
      	    {
      	      std::vector<datatools::handle<calo_tp> > calo_tp_list_per_clocktick_per_crate;
      	      my_calo_tp_data_.get_list_of_tp_per_clocktick_per_crate(i, j, calo_tp_list_per_clocktick_per_crate);
      	      if(!calo_tp_list_per_clocktick_per_crate.empty())
      		{
      		  for(unsigned int k = 0; k < calo_tp_list_per_clocktick_per_crate.size(); k++)
      		    {
      		      const calo_tp & my_calo_tp =  calo_tp_list_per_clocktick_per_crate[k].get();
      		      my_calo_tp.tree_dump(std::clog, "a_calo_tp : ", "INFO : ");
      		    }
      		}
      	    }
      	}

      return;
    }



  } // end of namespace digitization

} // end of namespace snemo
