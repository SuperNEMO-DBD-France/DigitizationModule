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

    signal_to_calo_tp_algo::calo_feb_config::calo_feb_config()
    {
      reset();
    }

    signal_to_calo_tp_algo::calo_feb_config::~calo_feb_config()
    {
      if (is_initialized())
	{
	  reset();
	}
    }

    void signal_to_calo_tp_algo::calo_feb_config::initialize(const datatools::properties & config_)
    {
      _set_defaults();
      if (config_.has_key("low_threshold")) {
        double low_threshold = config_.fetch_real_with_explicit_dimension("low_threshold", "electric_potential");
        this->low_threshold = low_threshold;
      }

      if (config_.has_key("high_threshold")) {
        double high_threshold = config_.fetch_real_with_explicit_dimension("high_threshold", "electric_potential");
	this->high_threshold = high_threshold;
      }

      if (config_.has_key("sampling_frequency")) {
	std::string sampling_frequency = config_.fetch_string("sampling_frequency");

	if (sampling_frequency == "SNEMO_DEFAULT_CALO_FEB_FREQUENCY")
	  {
	    this->sampling_rate = 2.56 * 1e9 * CLHEP::hertz;
	  }

	this->sampling_step = 1. / this->sampling_rate;
      }

      if (config_.has_key("readout_post_trig_window")) {
        double post_trig_window_ns = config_.fetch_real_with_explicit_dimension("readout_post_trig_window", "time");
        this->post_trig_window_ns = post_trig_window_ns;
	this->post_trig_window_samples = post_trig_window_ns / sampling_step;
      }

      if (config_.has_key("acquisition_window_length")) {
        int16_t acquisition_window_length = config_.fetch_integer("acquisition_window_length");
	this->acquisition_window_length = acquisition_window_length;
      }

      initialized = true;

      return;
    }

    bool signal_to_calo_tp_algo::calo_feb_config::is_initialized() const
    {
      return initialized;
    }

    void signal_to_calo_tp_algo::calo_feb_config::reset()
    {
      initialized = false;
      acquisition_window_length = 0;
      datatools::invalidate(low_threshold);
      datatools::invalidate(high_threshold);
      datatools::invalidate(sampling_rate);
      datatools::invalidate(sampling_step);
      datatools::invalidate(post_trig_window_ns);
      post_trig_window_samples = 0;
      return;
    }

    void signal_to_calo_tp_algo::calo_feb_config::_set_defaults()
    {
      acquisition_window_length = 1000;
      low_threshold = -0.015 * CLHEP::volt;
      high_threshold = -0.035 * CLHEP::volt;
      sampling_rate = 2.56 * 1e9 * CLHEP::hertz;
      sampling_step = 1. / sampling_rate;
      post_trig_window_ns = 290 * CLHEP::ns;
      post_trig_window_samples = post_trig_window_ns / sampling_step;
      return;
    }

    void signal_to_calo_tp_algo::calo_feb_config::tree_dump(std::ostream & out_,
							    const std::string & title_,
							    const std::string & indent_,
							    bool inherit_) const
    {
      if (!title_.empty()) out_ << indent_ << title_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Low threshold : " << low_threshold << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "High threshold : " << high_threshold << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Sampling rate  : " << sampling_rate << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Sampling step  : " << sampling_step << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag (inherit_)
	   << "Post trigger time  : " << post_trig_window_ns << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag (inherit_)
	   << "Post trigger samples  : " << post_trig_window_samples << std::endl;

      return;
    }

    signal_to_calo_tp_algo::calo_digi_working_data::calo_digi_working_data()
    {
      reset();
    }

    signal_to_calo_tp_algo::calo_digi_working_data::~calo_digi_working_data()
    {
      reset();
      return;
    }

    void signal_to_calo_tp_algo::calo_digi_working_data::reset()
    {
      hit_id = -1;
      geom_id.reset();
      channel_electronic_id.reset();
      is_low_threshold_only = false;
      is_low_threshold = false;
      datatools::invalidate(low_threshold_trigger_time);
      is_high_threshold = false;
      datatools::invalidate(high_threshold_trigger_time);
      clocktick_25 = clock_utils::INVALID_CLOCKTICK;
    }

    bool signal_to_calo_tp_algo::calo_digi_working_data::operator<(const calo_digi_working_data & other_) const
    {
      return this-> clocktick_25 < other_.clocktick_25;
    }

    void signal_to_calo_tp_algo::calo_digi_working_data::tree_dump(std::ostream & out_,
								   const std::string & title_,
								   bool dump_signal_,
								   const std::string & indent_,
								   bool inherit_) const
    {

      if (!title_.empty()) out_ << indent_ << title_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Hit ID     : " << hit_id << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Geometric ID     : " << geom_id << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Electronic channel ID    : " << channel_electronic_id << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Is LTO     : " << is_low_threshold_only << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Is LT     : " << is_low_threshold << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "LT trigger time : " << low_threshold_trigger_time << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Is HT     : " << is_high_threshold << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "HT trigger time : " << high_threshold_trigger_time << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Calo digitized signal capacity : " << calo_digitized_signal.capacity() << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Calo digitized signal size : " << calo_digitized_signal.size() << std::endl;

      if (dump_signal_) {

	out_ << indent_ << datatools::i_tree_dumpable::tag
	     << "Calo digitized signal (in volt) : " << std::endl;

	for (int i = 0; i < calo_digitized_signal.size(); i++)
	  {
	    std::clog << calo_digitized_signal[i] / CLHEP::volt << ' ';
	  }
	std::clog << std::endl;
      }

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag (inherit_)
           << "Clocktick 25 ns : " << clocktick_25  << std::endl;
      return;
    }

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

      _calo_feb_config_.initialize(config_);
      _calo_feb_config_.tree_dump(std::clog, "Calo FEB configuration");

      _running_digi_id_ = 0;

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
      _calo_feb_config_.reset();
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
      _running_digi_id_ = -1;
      return;
    }

    void signal_to_calo_tp_algo::_increment_running_digi_id()
    {
      _running_digi_id_++;
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
      		<< " LT = " << _calo_feb_config_.low_threshold / CLHEP::volt
      		<< " HT = " << _calo_feb_config_.high_threshold / CLHEP::volt << std::endl;
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

	  // a_mutable_signal.tree_dump(std::clog, "Mutable signal");


	  const mygsl::i_unary_function * signal_functor = & a_mutable_signal.get_shape();
	  double xmin = signal_functor->get_non_zero_domain_min();
	  double xmax = signal_functor->get_non_zero_domain_max();

	  std::clog << "Isignal = " << i <<  " Xmin = " << xmin << " Xmax = " << xmax << " Step = " << _calo_feb_config_.sampling_step << std::endl;

	  if (i == 2) {
	    const std::string filename = "/tmp/calo_signal.dat";
	    signal_functor->write_ascii_file(filename, xmin, xmax,  _calo_feb_config_.acquisition_window_length * _calo_feb_config_.sampling_step);
	  }

	  bool high_threshold_trigger = false;
	  double high_threshold_time;
	  datatools::invalidate(high_threshold_time);

	  bool   low_threshold_trigger = false;
	  double low_threshold_time;
	  datatools::invalidate(low_threshold_time);
	  int    low_threshold_sample_begin = -1;

	  uint32_t sample_index = 0;

	  circular_buffer online_buffer{static_cast<long unsigned int>(_calo_feb_config_.acquisition_window_length)};

	  geomtools::geom_id signal_GID = a_mutable_signal.get_geom_id();
	  geomtools::geom_id channel_electronic_id;
	  _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE,
						   signal_GID,
						   channel_electronic_id);
	  geomtools::geom_id feb_electronic_id;
	  feb_electronic_id.set_depth(mapping::BOARD_DEPTH);
	  feb_electronic_id.set_type(channel_electronic_id.get_type());
	  channel_electronic_id.extract_to(feb_electronic_id);

	  for (double x = xmin - (350. * _calo_feb_config_.sampling_step); x < xmax + (850.  * _calo_feb_config_.sampling_step); x+= _calo_feb_config_.sampling_step)
	    {
	      double y = signal_functor->eval(x);
	      online_buffer.push_back(y);
	      // std::clog << "Online buffer size = " << online_buffer.size() << std::endl;

	      // Low threshold crossing:
	      if (y <= _calo_feb_config_.low_threshold
		  && !low_threshold_trigger)
		{
		  low_threshold_trigger = true;
		  low_threshold_time = x;
		  low_threshold_sample_begin = sample_index;
		  calo_digi_working_data a_calo_wd;
		  a_calo_wd.calo_digitized_signal.set_capacity(static_cast<long unsigned int>(_calo_feb_config_.acquisition_window_length));
		  a_calo_wd.calo_digitized_signal = online_buffer;
		  std::clog << "Wd buffer capacity = " << a_calo_wd.calo_digitized_signal.capacity() << std::endl;
		  a_calo_wd.hit_id = _running_digi_id_;
		  _increment_running_digi_id();
		  a_calo_wd.geom_id = signal_GID;
		  a_calo_wd.channel_electronic_id = channel_electronic_id;
		  a_calo_wd.is_low_threshold = low_threshold_trigger;
		  a_calo_wd.low_threshold_trigger_time = low_threshold_time;
		  _calo_digi_data_collection_.push_back(a_calo_wd);
		}

	      if (y <= _calo_feb_config_.high_threshold
		  && !high_threshold_trigger)
		{
		  high_threshold_trigger = true;
		  high_threshold_time = x;
		  calo_digi_working_data & a_calo_wd = _calo_digi_data_collection_.back();
		  a_calo_wd.is_high_threshold = true;
		  a_calo_wd.high_threshold_trigger_time = high_threshold_time;
		}

	      if (low_threshold_trigger
		  && sample_index < low_threshold_sample_begin + _calo_feb_config_.post_trig_window_samples)
		{
		  calo_digi_working_data & a_calo_wd = _calo_digi_data_collection_.back();
		  a_calo_wd.calo_digitized_signal.push_back(y);
		}

	      // If outside POST_TRIG window, can create a new calo digitized hit
	      // To do : Open readout gate where the channel is locked until readout
	      // (depending of trigger algorithm, if calo only the gate is shorter than if it is CARACO mode)
	      if (sample_index >= low_threshold_sample_begin + _calo_feb_config_.post_trig_window_samples)
	    	{
		  calo_digi_working_data & a_calo_wd = _calo_digi_data_collection_.back();
		  if (a_calo_wd.is_low_threshold && !a_calo_wd.is_high_threshold) a_calo_wd.is_low_threshold_only = true;
		  // Calculate CT 25 but for LT or HT or both ?

		  low_threshold_trigger = false;
		  low_threshold_sample_begin = -1;
		  high_threshold_trigger = false;
		  datatools::invalidate(low_threshold_time);
		  datatools::invalidate(high_threshold_time);
		}

	      sample_index++;
	    }

	  std::clog << "Calo working data collection size = " << _calo_digi_data_collection_.size() << std::endl;
	  for (int i = 0; i < _calo_digi_data_collection_.size(); i++)
	    {
	      _calo_digi_data_collection_[i].tree_dump(std::clog, "A calo WD", true);
	    }







	  //   for (double x = xmin - (50. * _calo_feb_config_.sampling_step); x < xmax + (50.  * _calo_feb_config_.sampling_step); x+= _calo_feb_config_.sampling_step)
	  //     {
	  //       double y = signal_functor->eval(x);
	  //       y *= 1. / CLHEP::volt;

	  //       // if (i == 0) std::clog << "x = " << x << " y = " << y << std::endl;
	  //       // std::clog << "x = " << x << " y = " << y << std::endl;

	  //       if (y <= _calo_feb_config_.high_threshold / CLHEP::volt)
	  // 	{
	  // 	  high_threshold_trigger = true;
	  // 	  high_threshold_time = x;
	  // 	}

	  //       if (!low_threshold_only_trigger
	  // 	  && y <= _calo_feb_config_.low_threshold / CLHEP::volt
	  // 	  && y >  _calo_feb_config_.high_threshold / CLHEP::volt
	  // 	  && !high_threshold_trigger)
	  // 	{
	  // 	  low_threshold_only_trigger = true;
	  // 	  low_threshold_time = x;
	  // 	}

	  //       // std::clog << "Is HT  : " << high_threshold_trigger << " Time = " << high_threshold_time << std::endl;
	  //       // std::clog << "Is LTO : " << low_threshold_only_trigger << " Time = " << low_threshold_time << std::endl;

	  //       if (high_threshold_trigger) {
	  // 	low_threshold_only_trigger = false;
	  // 	datatools::invalidate(low_threshold_time);
	  // 	break;
	  //       }
	  //     }

	  //   // std::clog << "HT / LTO status for the signal :  HT ["<< high_threshold_trigger <<"] time = " << high_threshold_time << std::endl;
	  //   // std::clog << "LTO ["<< low_threshold_only_trigger <<"] time = " << low_threshold_time << std::endl;

	  //   const geomtools::geom_id & geom_id = a_mutable_signal.get_geom_id();
	  //   geomtools::geom_id channel_electronic_id;
	  //   _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE,
	  // 					   geom_id,
	  // 					   channel_electronic_id);

	  //   geomtools::geom_id feb_electronic_id;
	  //   feb_electronic_id.set_depth(mapping::BOARD_DEPTH);
	  //   feb_electronic_id.set_type(channel_electronic_id.get_type());
	  //   channel_electronic_id.extract_to(feb_electronic_id);
	  //   std::clog << "GID  = " << geom_id << " EID channel = " << channel_electronic_id << " EID FEB = " << feb_electronic_id << std::endl;

	  //   uint32_t a_calo_signal_clocktick = _clocktick_ref_ + clock_utils::CALO_FEB_SHIFT_CLOCKTICK_NUMBER;

	  //   // These bits have to be checked
	  //   bool calo_xt_bit    = 0;
	  //   bool calo_spare_bit = 0;

	  //   // Signal is LTO :
	  //   if (low_threshold_only_trigger)
	  //     {
	  //       if (low_threshold_time > 25) // nanoseconds
	  // 	{
	  // 	  a_calo_signal_clocktick += static_cast<uint32_t>(low_threshold_time / 25);
	  // 	}

	  //       bool already_existing_tp = my_calo_tp_data_.existing_tp(feb_electronic_id,
	  // 							      a_calo_signal_clocktick);

	  //       if (already_existing_tp)
	  // 	{
	  // 	  unsigned int existing_index = my_calo_tp_data_.get_existing_tp_index(feb_electronic_id,
	  // 									       a_calo_signal_clocktick);
	  // 	  // Update existing calo TP
	  // 	  snemo::digitization::calo_tp & existing_calo_tp = my_calo_tp_data_.grab_calo_tps()[existing_index].grab();
	  // 	  existing_calo_tp.set_lto_bit(1);
	  // 	}
	  //       else
	  // 	{
	  // 	  // Create new calo TP
	  // 	  snemo::digitization::calo_tp & calo_tp = my_calo_tp_data_.add();
	  // 	  calo_tp.set_header(a_mutable_signal.get_hit_id(),
	  // 			     feb_electronic_id,
	  // 			     a_calo_signal_clocktick);
	  // 	  calo_tp.set_lto_bit(1);
	  // 	  calo_tp.set_xt_bit(calo_xt_bit);
	  // 	  calo_tp.set_spare_bit(calo_spare_bit);
	  // 	}


	  //     }

	  //   // Signal is HT :
	  //   if (high_threshold_trigger)
	  //     {

	  //       if (high_threshold_time > 25) // nanoseconds
	  // 	{
	  // 	  a_calo_signal_clocktick += static_cast<uint32_t>(high_threshold_time / 25);
	  // 	}

	  //       bool already_existing_tp = my_calo_tp_data_.existing_tp(feb_electronic_id,
	  // 							      a_calo_signal_clocktick);

	  //       if (already_existing_tp)
	  // 	{
	  // 	  unsigned int existing_index = my_calo_tp_data_.get_existing_tp_index(feb_electronic_id,
	  // 									       a_calo_signal_clocktick);
	  // 	  // Update existing calo TP
	  // 	  snemo::digitization::calo_tp & existing_calo_tp = my_calo_tp_data_.grab_calo_tps()[existing_index].grab();
	  // 	  unsigned int existing_multiplicity = existing_calo_tp.get_htm();
	  // 	  existing_multiplicity += 1;
	  // 	  existing_calo_tp.set_htm(existing_multiplicity);
	  // 	}
	  //       else
	  // 	{

	  // 	  // Create new calo TP
	  // 	  snemo::digitization::calo_tp & calo_tp = my_calo_tp_data_.add();
	  // 	  calo_tp.set_header(a_mutable_signal.get_hit_id(),
	  // 			     feb_electronic_id,
	  // 			     a_calo_signal_clocktick);
	  // 	  calo_tp.set_htm(1);
	  // 	  calo_tp.set_xt_bit(calo_xt_bit);
	  // 	  calo_tp.set_spare_bit(calo_spare_bit);
	  // 	}
	  //     }

	} // end of i signal

      // my_calo_tp_data_.tree_dump(std::clog, "CALO TP DATA");

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
