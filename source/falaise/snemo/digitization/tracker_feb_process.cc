// snemo/digitization/tracker_feb_process.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Standard library :
#include <math.h>

// - Bayeux/mctools:
#include <mctools/signal/utils.h>

// This project :
#include <snemo/digitization/clock_utils.h>

// Ourselves:
#include <snemo/digitization/tracker_feb_process.h>

namespace snemo {

  namespace digitization {

    tracker_feb_process::geiger_feb_config::geiger_feb_config()
    {
      reset();
    }

    tracker_feb_process::geiger_feb_config::~geiger_feb_config()
    {
      if (is_initialized())
	{
	  reset();
	}
    }

    void tracker_feb_process::geiger_feb_config::initialize(const datatools::properties & config_)
    {
      _set_defaults();

      if (config_.has_key("VLNT")) {
        double VLNT_ = config_.fetch_real_with_explicit_dimension("VLNT", "electric_potential");
        this->VLNT = VLNT_;
      }

      if (config_.has_key("VHNT")) {
        double VHNT_ = config_.fetch_real_with_explicit_dimension("VHNT", "electric_potential");
        this->VHNT = VHNT_;
      }

      if (config_.has_key("VHPT")) {
        double VHPT_ = config_.fetch_real_with_explicit_dimension("VHPT", "electric_potential");
        this->VHPT = VHPT_;
      }

      if (config_.has_key("VCPT")) {
        double VCPT_ = config_.fetch_real_with_explicit_dimension("VCPT", "electric_potential");
        this->VCPT = VCPT_;
      }

      initialized = true;

      return;
    }

    bool tracker_feb_process::geiger_feb_config::is_initialized() const
    {
      return initialized;
    }

    void tracker_feb_process::geiger_feb_config::reset()
    {
      datatools::invalidate(VLNT);
      datatools::invalidate(VHNT);
      datatools::invalidate(VHPT);
      datatools::invalidate(VCPT);
      initialized = false;
      return;
    }

    void tracker_feb_process::geiger_feb_config::_set_defaults()
    {
      this->VLNT = -0.015 * CLHEP::volt;
      this->VHNT = -0.12 * CLHEP::volt;
      this->VHPT = +0.12 * CLHEP::volt;
      this->VCPT = +0.0019 * CLHEP::volt;
      return;
    }

    void tracker_feb_process::geiger_feb_config::tree_dump(std::ostream & out_,
								const std::string & title_,
								const std::string & indent_,
								bool inherit_) const
    {
      if (!title_.empty()) out_ << indent_ << title_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "VLNT : " << this->VLNT << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "VHNT : " << this->VHNT << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "VHPT  : " << this->VHPT << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag (inherit_)
           << "VCPT  : " << this->VCPT << std::endl;

      return;
    }

    tracker_feb_process::geiger_digi_working_data::geiger_digi_working_data()
    {
      reset();
    }


    tracker_feb_process::geiger_digi_working_data::~geiger_digi_working_data()
    {
      reset();
      return;
    }

    void tracker_feb_process::geiger_digi_working_data::reset()
    {
      signal_ref = 0;
      signal_deriv.reset();
      hit_id = -1;

      anodic_gid.reset();
      cathodic_bottom_gid.reset();
      cathodic_top_gid.reset();

      anodic_eid.reset();
      cathodic_bottom_eid.reset();
      cathodic_top_eid.reset();

      datatools::invalidate(trigger_time);
      datatools::invalidate(anodic_R0);
      datatools::invalidate(anodic_R1);
      datatools::invalidate(anodic_R2);
      datatools::invalidate(anodic_R3);
      datatools::invalidate(anodic_R4);
      datatools::invalidate(cathodic_R5);
      datatools::invalidate(cathodic_R6);
      cathode_top_register = "";
      cathode_bottom_register = "";
      clocktick_800 = clock_utils::INVALID_CLOCKTICK;
    }

    bool tracker_feb_process::geiger_digi_working_data::operator<(const geiger_digi_working_data & other_) const
    {
      return this-> clocktick_800 < other_.clocktick_800;
    }

    void tracker_feb_process::geiger_digi_working_data::tree_dump(std::ostream & out_,
								       const std::string & title_,
								       bool dump_signal_,
								       const std::string & indent_,
								       bool inherit_) const
    {

      if (!title_.empty()) out_ << indent_ << title_ << std::endl;

      if (dump_signal_) signal_ref->tree_dump(out_, title_, indent_, inherit_);

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Hit ID               : " << hit_id << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Event time reference : " << event_time_reference << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic GID           : " << anodic_gid << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic bottom GID  : " << cathodic_bottom_gid << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic top GID     : " << cathodic_top_gid << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic EID           : " << anodic_eid << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic bottom EID  : " << cathodic_bottom_eid << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic top EID     : " << cathodic_top_eid << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Trigger time         : " << trigger_time / CLHEP::microsecond << " us" << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic register 0    : " << anodic_R0 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic register 1    : " << anodic_R1 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic register 2    : " << anodic_R2 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic register 3    : " << anodic_R3 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Anodic register 4    : " << anodic_R4 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic register 5  : " << cathodic_R5 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic register 6  : " << cathodic_R6 / CLHEP::microsecond << " us"  << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Cathodic top reg     : " << cathode_top_register << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Cathodic bot reg     : " << cathode_bottom_register << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag (inherit_)
           << "Clocktick 800 ns     : " << clocktick_800  << std::endl;

      return;
    }

    tracker_feb_process::tracker_feb_process()
    {
      _initialized_   = false;
      _electronic_mapping_  = nullptr;
      _ssb_ = nullptr;
      _signal_category_ = "";

      return;
    }

    tracker_feb_process::~tracker_feb_process()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void tracker_feb_process::initialize(const datatools::properties & config_,
					      clock_utils & my_clock_utils_,
					      electronic_mapping & my_electronic_mapping_,
					      mctools::signal::signal_shape_builder & my_ssb_)
    {
      DT_THROW_IF (is_initialized(), std::logic_error, "Tracker FEB process is already initialized ! ");
      _set_defaults();
      _clock_utils_ = & my_clock_utils_;
      _electronic_mapping_ = & my_electronic_mapping_;
      _ssb_ = & my_ssb_;

      if (config_.has_key("signal_category")) {
	std::string signal_category = config_.fetch_string("signal_category");
        _signal_category_ = signal_category;
      }

      _gg_feb_config_.initialize(config_);
      _gg_feb_config_.tree_dump(std::clog, "Geiger FEB configuration");

      _running_tp_id_ = 0;
      _running_readout_id_ = 0;

      _initialized_ = true;
      return;
    }

    bool tracker_feb_process::is_initialized() const
    {
      return _initialized_;
    }

    void tracker_feb_process::reset()
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Tracker FEB process is not initialized, it can't be reset ! ");
      _initialized_ = false;
      _electronic_mapping_ = 0;
      _running_tp_id_ = -1;
      _running_readout_id_ = -1;
      return;
    }

    bool tracker_feb_process::has_signal_category() const
    {
      return !_signal_category_.empty();
    }

    const std::string & tracker_feb_process::get_signal_category() const
    {
      return _signal_category_;
    }

    void tracker_feb_process::set_signal_category(const std::string & category_)
    {
      _signal_category_ = category_;
      return;
    }

    void tracker_feb_process::add_geiger_tp(const geiger_digi_working_data & my_wd_data_,
						 uint32_t signal_clocktick_,
						 geiger_tp_data & my_geiger_tp_data_)
    {
      snemo::digitization::geiger_tp & gg_tp = my_geiger_tp_data_.add();
      unsigned int gg_tp_hit_id = _running_tp_id_;
      _increment_running_tp_id();

      geomtools::geom_id temporary_feb_id;
      temporary_feb_id.set_type(my_wd_data_.anodic_eid.get_type());
      temporary_feb_id.set_depth(mapping::BOARD_DEPTH);
      my_wd_data_.anodic_eid.extract_to(temporary_feb_id);
      gg_tp.set_header(gg_tp_hit_id,
		       temporary_feb_id,
		       signal_clocktick_,
		       mapping::THREE_WIRES_TRACKER_MODE,
		       mapping::SIDE_MODE,
		       mapping::NUMBER_OF_CONNECTED_ROWS);

      // Find bit index function of the WD Anodic GID directly:

      int side  = my_wd_data_.anodic_gid.get(mapping::SIDE_INDEX);
      int layer = my_wd_data_.anodic_gid.get(mapping::LAYER_INDEX);
      int row   = my_wd_data_.anodic_gid.get(mapping::ROW_INDEX);
      int tp_channel = -1;
      if (side == 0)
	{
	  if (row % 2 == 0)
	    {
	      tp_channel = layer;
	    }
	  if (row % 2 == 1)
	    {
	      tp_channel = mapping::NUMBER_OF_GEIGER_LAYERS + layer;
	    }
	}
      if (side == 1)
	{
	  if (row % 2 == 0)
	    {
	      tp_channel = 2 * mapping::NUMBER_OF_GEIGER_LAYERS + layer;
	    }
	  if (row % 2 == 1)
	    {
	      tp_channel = 2 * mapping::NUMBER_OF_GEIGER_LAYERS + mapping::NUMBER_OF_GEIGER_LAYERS + layer;
	    }
	}

      gg_tp.set_gg_tp_active_bit(tp_channel);
      // gg_tp.set_auxiliaries(my_wd_data_.auxiliaries);
      _activated_bits_[tp_channel] = 1;
      // gg_tp.tree_dump(std::clog, "***** Geiger TP creation : *****", "INFO : ");

      return;
    }

    void tracker_feb_process::update_gg_tp(const geiger_digi_working_data & my_wd_data_,
						geiger_tp & my_geiger_tp_)
    {
      int side  = my_wd_data_.anodic_gid.get(mapping::SIDE_INDEX);
      int layer = my_wd_data_.anodic_gid.get(mapping::LAYER_INDEX);
      int row   = my_wd_data_.anodic_gid.get(mapping::ROW_INDEX);
      int tp_channel = -1;
      if (side == 0)
	{
	  if (row % 2 == 0)
	    {
	      tp_channel = layer;
	    }
	  if (row % 2 == 1)
	    {
	      tp_channel = mapping::NUMBER_OF_GEIGER_LAYERS + layer;
	    }
	}
      if (side == 1)
	{
	  if (row % 2 == 0)
	    {
	      tp_channel = 2 * mapping::NUMBER_OF_GEIGER_LAYERS + layer;
	    }
	  if (row % 2 == 1)
	    {
	      tp_channel = 2 * mapping::NUMBER_OF_GEIGER_LAYERS + mapping::NUMBER_OF_GEIGER_LAYERS + layer;
	    }
	}

      my_geiger_tp_.set_gg_tp_active_bit(tp_channel);
      // gg_tp.set_auxiliaries(my_wd_data_.auxiliaries);
      _activated_bits_[tp_channel] = 1;
      // my_geiger_tp_.tree_dump(std::clog, "***** Geiger TP Update : *****", "INFO : ");
      return;
    }

    void tracker_feb_process::clear_working_data()
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
      _set_defaults();
      _gg_digi_data_collection_.clear();
      return;
    }

    void tracker_feb_process::_set_defaults()
    {
      for (unsigned int i = 0; i < geiger::tp::TP_SIZE; i++)
	{
	  _activated_bits_[i] = 0;
	}
      _signal_category_ = "sigtracker";
      _running_tp_id_   = -1;

      return;
    }

    void tracker_feb_process::_increment_running_tp_id()
    {
      _running_tp_id_++;
      return;
    }

    void tracker_feb_process::_prepare_working_data(const mctools::signal::signal_data & SSD_,
							 gg_digi_working_data_collection_type & wd_collection_)
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
      std::size_t number_of_hits = SSD_.get_number_of_signals(get_signal_category());

      // Build the shape for each anodic signal first to create working data:
      for (std::size_t i = 0; i < number_of_hits; i++)
	{
	  const mctools::signal::base_signal a_signal = SSD_.get_signals(get_signal_category())[i].get();

	  // Process for an anodic signal :
	  if (a_signal.get_auxiliaries().fetch_string("subcategory") == "anodic")
	    {
	      mctools::signal::base_signal a_mutable_signal = a_signal;
	      std::string signal_name;
	      snemo::digitization::build_signal_name(a_mutable_signal.get_hit_id(),
						     signal_name);

	      a_mutable_signal.build_signal_shape(*_ssb_,
						  signal_name,
						  a_mutable_signal);
	      //a_mutable_signal.tree_dump(std::clog, "Mutable anodic signal with shape instantiated");

	      geiger_digi_working_data a_wd;
	      a_wd.signal_ref = & a_mutable_signal;

	      a_wd.event_time_reference = a_wd.signal_ref->get_time_ref();

	      const mygsl::i_unary_function * signal_functor = & a_mutable_signal.get_shape();
	      a_wd.signal_deriv.set_functor(*signal_functor);

	      unsigned int nsamples = 1000;
	      double xmin = a_wd.signal_deriv.get_non_zero_domain_min();
	      double xmax = a_wd.signal_deriv.get_non_zero_domain_max();

	      if (i == 72) {
		const std::string filename = "/tmp/gg_signal.dat";
		a_wd.signal_deriv.write_ascii_file(filename, xmin, xmax, nsamples);
		std::ofstream gg_deriv_stream;
		const std::string gg_deriv_filename = "/tmp/gg_signal_deriv.dat";
		gg_deriv_stream.open(gg_deriv_filename);
		for (double x = xmin - (50 * xmin / nsamples); x < xmax + (50 * xmin / nsamples); x+= (xmax - xmin) / nsamples) {
		  double y = a_wd.signal_deriv.eval_df(x);
		  gg_deriv_stream << x  << ' ' << y << std::endl;
		}
		gg_deriv_stream.close();
	      }

	      // std::clog << "Xmin = " << xmin << " Xmax = " << xmax << " (Xmax - Xmin) / nsamples = " << (xmax - xmin) / nsamples << std::endl;

	      double trigger_time;
	      datatools::invalidate(trigger_time);

	      bool R0_crossed = false;
	      bool R1_crossed = false;
	      bool R2_crossed = false;
	      bool R3_crossed = false;
	      bool R4_crossed = false;

	      double former_y = 0;

	      for (double x = xmin - (50 * xmin / nsamples); x < xmax + (50 * xmin / nsamples); x+= (xmax - xmin) / nsamples)
		{
		  double y = a_wd.signal_deriv.eval_df(x);
		  y *= CLHEP::meter / CLHEP::volt;
		  // if (i == 72) std::clog << "x = " << x << " y = " << y << " former y = " << former_y << std::endl;

		  if (y < 0
		      && former_y >= _gg_feb_config_.VLNT / CLHEP::volt
		      && y <= _gg_feb_config_.VLNT / CLHEP::volt
		      && !R0_crossed)
		    {
		      R0_crossed = true;
		      trigger_time = x - a_wd.event_time_reference;
		      a_wd.anodic_R0 = x - a_wd.event_time_reference;
		      former_y = y;
		      continue;
		    }

		  if (y < 0
		      && former_y >= _gg_feb_config_.VHNT / CLHEP::volt
		      && y <= _gg_feb_config_.VHNT / CLHEP::volt
		      && R0_crossed
		      && !R1_crossed)
		    {
		      R1_crossed = true;
		      a_wd.anodic_R1 = x - a_wd.event_time_reference;
		      former_y = y;
		      continue;
		    }

		  if (y < 0
		      && former_y >= _gg_feb_config_.VHNT / CLHEP::volt
		      && y <= _gg_feb_config_.VHNT / CLHEP::volt
		      && R0_crossed
		      && R1_crossed
		      && !R3_crossed)
		    {
		      R3_crossed = true;
		      a_wd.anodic_R3 = x - a_wd.event_time_reference;
		      former_y = y;
		      continue;
		    }

		  if (y > 0
		      && former_y <= _gg_feb_config_.VHPT / CLHEP::volt
		      && y >= _gg_feb_config_.VHPT / CLHEP::volt
		      && R0_crossed
		      && !R2_crossed)
		    {
		      R2_crossed = true;
		      a_wd.anodic_R2 = x - a_wd.event_time_reference;
		      former_y = y;
		      continue;
		    }

		  if (y > 0
		      && former_y <= _gg_feb_config_.VHPT / CLHEP::volt
		      && y >= _gg_feb_config_.VHPT / CLHEP::volt
		      && R0_crossed
		      && R2_crossed
		      && !R4_crossed)
		    {
		      R4_crossed = true;
		      a_wd.anodic_R4 = x - a_wd.event_time_reference;
		      former_y = y;
		      continue;
		    }

		  former_y = y;
		}

	      const geomtools::geom_id & geom_id = a_wd.signal_ref->get_geom_id();
	      geomtools::geom_id electronic_id;
	      _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE, geom_id, electronic_id);

	      uint32_t a_geiger_signal_clocktick = _clock_utils_->compute_clocktick_800ns_from_time(trigger_time);
	      // See with Jihane and Thierry to add shift computation
	      // a_geiger_signal_clocktick += clock_utils::TRACKER_FEB_SHIFT_CLOCKTICK_NUMBER;

	      a_wd.hit_id = a_wd.signal_ref->get_hit_id();
	      a_wd.trigger_time = trigger_time;
	      a_wd.anodic_gid   = geom_id;
	      a_wd.anodic_eid   = electronic_id;
	      a_wd.clocktick_800 = a_geiger_signal_clocktick;

	      // a_wd.tree_dump(std::clog, "A working data #" + std::to_string(i));
	      wd_collection_.push_back(a_wd);

	    } // end of anodic

	} // end of number_of_hits


      // Loop on cathodic signal once their GG WD anodic signal is already created
      // Readout will be done on cathodic signals only if anodic exists:
      for (std::size_t i = 0; i < number_of_hits; i++)
	{
	  const mctools::signal::base_signal a_signal = SSD_.get_signals(get_signal_category())[i].get();

	  // Process for a cathodic :
	  if (a_signal.get_auxiliaries().fetch_string("subcategory") == "cathodic")
	    {
	      mctools::signal::base_signal a_mutable_signal = a_signal;
	      std::string signal_name;
	      snemo::digitization::build_signal_name(a_mutable_signal.get_hit_id(),
						     signal_name);

	      a_mutable_signal.build_signal_shape(*_ssb_,
						  signal_name,
						  a_mutable_signal);

	      const mygsl::i_unary_function * signal_functor = & a_mutable_signal.get_shape();

	      unsigned int nsamples = 500;
	      double xmin = signal_functor->get_non_zero_domain_min();
	      double xmax = signal_functor->get_non_zero_domain_max();

	      if (i == 71) {
		const std::string filename = "/tmp/gg_signal_cathodic.dat";
		signal_functor->write_ascii_file(filename, xmin, xmax, nsamples);
	      }

	      // Find the anodic working data already created:
	      const geomtools::geom_id & geom_id = a_signal.get_geom_id();
	      geomtools::geom_id cathode_electronic_id;
	      _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE,
						       geom_id,
						       cathode_electronic_id);

	      // Find the working data already created for the ANODIC signal
	      int existing_index = -1;
	      bool is_existing = false;

	      geomtools::geom_id anodic_reduced_gid;
	      anodic_reduced_gid.set_depth(4);
	      anodic_reduced_gid.set_type(mapping::GEIGER_ANODIC_CATEGORY_TYPE);
	      a_signal.get_geom_id().extract_to(anodic_reduced_gid);

	      for (unsigned int j = 0; j < _gg_digi_data_collection_.size(); j++)
		{
		  if (anodic_reduced_gid == _gg_digi_data_collection_[j].anodic_gid
		      && (xmin + _gg_digi_data_collection_[j].trigger_time) < 200 * CLHEP::microsecond)
		    {
		      existing_index = j;
		      is_existing = true;
		    }
		}

	      if (is_existing)
		{
		  geiger_digi_working_data & a_wd = _gg_digi_data_collection_[existing_index];
		  // a_wd.tree_dump(std::clog, "Existing WD");
		  if (i < 6) {
		    //std::clog << "GID = " << a_signal.get_geom_id() << " REDUCED_GID = " << anodic_reduced_gid << " EID = " << cathode_electronic_id << std::endl;
		    // a_mutable_signal.tree_dump(std::clog, "Cathodic signal");
		  }

		  bool is_cathodic_top = false;
		  if (geom_id.get(mapping::GEIGER_CELL_PART_INDEX) == mapping::GEIGER_CELL_TOP_CATHODE_PART) is_cathodic_top = true;
		  if (is_cathodic_top)
		    {
		      a_wd.cathodic_top_gid = geom_id;
		      a_wd.cathodic_top_eid = cathode_electronic_id;
		    }
		  else
		    {
		      a_wd.cathodic_bottom_gid = geom_id;
		      a_wd.cathodic_bottom_eid = cathode_electronic_id;
		    }

		  double former_y = 0;

		  for (double x = xmin - (50 * xmin / nsamples); x < xmax + (50 * xmin / nsamples); x+= (xmax - xmin) / nsamples)
		    {
		      double y = signal_functor->eval(x);
		      y *= 1. / CLHEP::volt;


		      if (y > 0
			  && former_y <= _gg_feb_config_.VCPT / CLHEP::volt
			  && y >= _gg_feb_config_.VCPT / CLHEP::volt
			  && !datatools::is_valid(a_wd.cathodic_R5))
			{
			  if (is_cathodic_top) a_wd.cathode_top_register = "R5";
			  else a_wd.cathode_bottom_register = "R5";

			  a_wd.cathodic_R5 = x - a_wd.event_time_reference;
			  former_y = y;
			  break;
			}

		      if (y > 0
			  && former_y <= _gg_feb_config_.VCPT / CLHEP::volt
			  && y >= _gg_feb_config_.VCPT / CLHEP::volt
			  && datatools::is_valid(a_wd.cathodic_R5)
			  && !datatools::is_valid(a_wd.cathodic_R6))
			{
			  // Check if actual crossing time is before previous crossing time threshold R5
			  // (1st cathode but maybe not int time)

			  if (x < a_wd.cathodic_R5)
			    {
			      a_wd.cathodic_R6 = a_wd.cathodic_R5;
			      a_wd.cathodic_R5 = x - a_wd.event_time_reference;

			      if (is_cathodic_top)
				{
				  a_wd.cathode_top_register = "R5";
				  a_wd.cathode_bottom_register = "R6";
				}
			      else
				{
				  a_wd.cathode_bottom_register = "R5";
				  a_wd.cathode_top_register = "R6";
				}
			    }
			  else
			    {
			      a_wd.cathodic_R6 = x;
			      if (is_cathodic_top) a_wd.cathode_top_register = "R6";
			      else a_wd.cathode_bottom_register = "R6";
			    }

			  former_y = y;
			  break;
			}
		    }
		}

	    } // end of cathodics

	} // end of number_of_hits


      // Print Working Data colleciton
      for (unsigned int j = 0; j < _gg_digi_data_collection_.size(); j++)
	{
	  //if (_gg_digi_data_collection_[j].cathode_top_register == "R6")
	  // _gg_digi_data_collection_[j].tree_dump(std::clog, "GG WD #" + std::to_string(_gg_digi_data_collection_[j].hit_id));
	}

      return ;
    }

    void tracker_feb_process::_sort_working_data(gg_digi_working_data_collection_type & wd_collection_)
    {
      std::sort(wd_collection_.begin(), wd_collection_.end());
      return;
    }

    void tracker_feb_process::_geiger_tp_process(const gg_digi_working_data_collection_type & wd_collection_,
						      geiger_tp_data & my_geiger_tp_data_)
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
      for (unsigned int i = 0; i < wd_collection_.size(); i++)
	{
	  int32_t geiger_tp_hit_id   = wd_collection_[i].hit_id;
	  uint32_t signal_clocktick  = wd_collection_[i].clocktick_800;
	  int existing_index = -1;
	  bool existing_eid  = false;

	  for (unsigned int j = 0; j < clock_utils::ACTIVATED_GEIGER_CELLS_NUMBER; j++)
	    {
	      bool existing_ct = false;
	      std::vector<datatools::handle<geiger_tp> > my_list_of_gg_tp_per_eid;

	      my_geiger_tp_data_.get_list_of_gg_tp_per_eid(wd_collection_[i].anodic_eid , my_list_of_gg_tp_per_eid);

	      if (my_list_of_gg_tp_per_eid.empty())
		{
		  existing_eid = false;
		  existing_index = -1;
		}
	      else
		{
		  existing_eid = true;
		  unsigned int iterator_list = 0;
		  while (existing_ct == false && iterator_list < my_list_of_gg_tp_per_eid.size())
		    {
		      if (signal_clocktick == my_list_of_gg_tp_per_eid[iterator_list].get().get_clocktick_800ns())
			{
			  existing_ct = true;
			  existing_index = iterator_list;
			}
		      iterator_list++;
		    } //end of while existing_ct == false
		}
	      // Eid is existing, clocktick is existing, gg tp update
	      if (existing_eid == true && existing_ct == true)
	      	{
		  update_gg_tp(wd_collection_[i], my_list_of_gg_tp_per_eid[existing_index].grab());
	      	}

	      // Eid is not existing or clocktick is different, geiger TP first creation
	      else
		{
		  add_geiger_tp(wd_collection_[i],
				signal_clocktick,
				my_geiger_tp_data_);
		}
	      signal_clocktick++;
	    } // end of for (j < 10)

	} //end of for (i < wd_size)

      return;
    }

    void tracker_feb_process::trigger_process(const mctools::signal::signal_data & SSD_,
					      geiger_tp_data & my_geiger_tp_data_)
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");

      /////////////////////////////////
      // Check simulated signal data //
      /////////////////////////////////

      // // Check if some 'simulated_data' are available in the data model:
      if (SSD_.has_signals(get_signal_category())) {
	/********************
	 * Process the data *
	 ********************/

	// Main processing method :
	_trigger_process(SSD_, my_geiger_tp_data_);
      }

      return;
    }

    void tracker_feb_process::readout_process(snemo::datamodel::sim_digi_data & SDD_)
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");

      // Check if Calo digi working data is not empty :
      if (_gg_digi_data_collection_.size() != 0)
	{
	  /********************
	   * Process the data *
	   ********************/

	  // Main processing method :
	  _readout_process(SDD_);
	}

      return;
    }


    void tracker_feb_process::_trigger_process(const mctools::signal::signal_data & SSD_,
					       geiger_tp_data & my_geiger_tp_data_)
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
      _prepare_working_data(SSD_, _gg_digi_data_collection_);
      _sort_working_data(_gg_digi_data_collection_);
      _geiger_tp_process(_gg_digi_data_collection_, my_geiger_tp_data_);

      // my_geiger_tp_data_.tree_dump(std::clog, "GG TP DATA");

      // for (uint32_t i = my_geiger_tp_data_.get_clocktick_min(); i <= my_geiger_tp_data_.get_clocktick_max(); i++)
      // 	{
      // 	  for (unsigned int j = 0 ; j <= mapping::NUMBER_OF_CRATES_PER_TYPE; j++)
      // 	    {
      // 	      std::vector<datatools::handle<geiger_tp> > geiger_tp_list_per_clocktick_per_crate;
      // 	      my_geiger_tp_data_.get_list_of_gg_tp_per_clocktick_per_crate(i, j, geiger_tp_list_per_clocktick_per_crate);
      // 	      if (!geiger_tp_list_per_clocktick_per_crate.empty())
      // 		{
      // 		  for (unsigned int k = 0; k < geiger_tp_list_per_clocktick_per_crate.size(); k++)
      // 		    {
      // 		      const geiger_tp & my_geiger_tp =  geiger_tp_list_per_clocktick_per_crate[k].get();
      // 		      my_geiger_tp.tree_dump(std::clog, "a_geiger_tp : ", "INFO : ");
      // 		    }
      // 		}
      // 	    }
      // 	}

      return;
    }

    void tracker_feb_process::_readout_process(snemo::datamodel::sim_digi_data & SDD_)
    {
      DT_THROW_IF (!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");

      return;
    }

  } // end of namespace digitization

} // end of namespace snemo
