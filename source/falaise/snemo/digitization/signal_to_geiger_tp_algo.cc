// snemo/digitization/signal_to_geiger_tp_algo.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Standard library :
#include <math.h>

// - Bayeux/mctools:
#include <mctools/signal/utils.h>

// This project :
#include <snemo/digitization/clock_utils.h>

// Ourselves:
#include <snemo/digitization/signal_to_geiger_tp_algo.h>

namespace snemo {

  namespace digitization {

    signal_to_geiger_tp_algo::geiger_feb_config::geiger_feb_config()
    {
      reset();
    }

    signal_to_geiger_tp_algo::geiger_feb_config::~geiger_feb_config()
    {
      if (is_initialized())
	{
	  reset();
	}
    }

    void signal_to_geiger_tp_algo::geiger_feb_config::initialize(const datatools::properties & config_)
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

      initialized = true;

      return;
    }

    bool signal_to_geiger_tp_algo::geiger_feb_config::is_initialized() const
    {
      return initialized;
    }

    void signal_to_geiger_tp_algo::geiger_feb_config::reset()
    {
      datatools::invalidate(VLNT);
      datatools::invalidate(VHNT);
      datatools::invalidate(VHPT);
      initialized = false;
      return;
    }

    void signal_to_geiger_tp_algo::geiger_feb_config::_set_defaults()
    {
      this->VLNT = -0.015 * CLHEP::volt;
      this->VHNT = -0.12 * CLHEP::volt;
      this->VHPT = +0.12 * CLHEP::volt;
      return;
    }

    void signal_to_geiger_tp_algo::geiger_feb_config::tree_dump(std::ostream & out_,
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

      return;
    }

    signal_to_geiger_tp_algo::geiger_digi_working_data::geiger_digi_working_data()
    {
      reset();
    }


    signal_to_geiger_tp_algo::geiger_digi_working_data::~geiger_digi_working_data()
    {
      reset();
      return;
    }

    void signal_to_geiger_tp_algo::geiger_digi_working_data::reset()
    {
      signal_ref = 0;
      signal_deriv.reset();
      hit_id = -1;
      geom_id.reset();
      cell_electronic_id.reset();
      datatools::invalidate(trigger_time);
      datatools::invalidate(anodic_R0);
      datatools::invalidate(anodic_R1);
      datatools::invalidate(anodic_R2);
      datatools::invalidate(anodic_R3);
      datatools::invalidate(anodic_R4);
      datatools::invalidate(cathodic_R5);
      datatools::invalidate(cathodic_R6);
      clocktick_800 = clock_utils::INVALID_CLOCKTICK;
    }

    bool signal_to_geiger_tp_algo::geiger_digi_working_data::operator<(const geiger_digi_working_data & other_) const
    {
      return this-> clocktick_800 < other_.clocktick_800;
    }

    void signal_to_geiger_tp_algo::geiger_digi_working_data::tree_dump(std::ostream & out_,
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
           << "Event time reference : " << signal_ref->get_time_ref() << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Geometric ID         : " << geom_id << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
           << "Electronic cell ID   : " << cell_electronic_id << std::endl;

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

      out_ << indent_ << datatools::i_tree_dumpable::inherit_tag (inherit_)
           << "Clocktick 800 ns     : " << clocktick_800  << std::endl;

      return;
    }

    signal_to_geiger_tp_algo::signal_to_geiger_tp_algo()
    {
      _initialized_   = false;
      _electronic_mapping_  = nullptr;
      _ssb_ = nullptr;
      _signal_category_ = "";

      return;
    }

    signal_to_geiger_tp_algo::~signal_to_geiger_tp_algo()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void signal_to_geiger_tp_algo::initialize(const datatools::properties & config_,
					      clock_utils & my_clock_utils_,
					      electronic_mapping & my_electronic_mapping_,
					      mctools::signal::signal_shape_builder & my_ssb_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Signal to geiger tp algorithm is already initialized ! ");
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

      _initialized_ = true;
      return;
    }

    bool signal_to_geiger_tp_algo::is_initialized() const
    {
      return _initialized_;
    }

    void signal_to_geiger_tp_algo::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to geiger tp algorithm is not initialized, it can't be reset ! ");
      _initialized_ = false;
      _electronic_mapping_ = 0;
      return;
    }

    bool signal_to_geiger_tp_algo::has_signal_category() const
    {
      return !_signal_category_.empty();
    }

    const std::string & signal_to_geiger_tp_algo::get_signal_category() const
    {
      return _signal_category_;
    }

    void signal_to_geiger_tp_algo::set_signal_category(const std::string & category_)
    {
      _signal_category_ = category_;
      return;
    }

    void signal_to_geiger_tp_algo::add_geiger_tp(const geiger_digi_working_data & my_wd_data_,
						 uint32_t signal_clocktick_,
 						 int32_t hit_id_,
						 geiger_tp_data & my_geiger_tp_data_)
    {
      snemo::digitization::geiger_tp & gg_tp = my_geiger_tp_data_.add();
      geomtools::geom_id temporary_feb_id;
      temporary_feb_id.set_type(my_wd_data_.cell_electronic_id.get_type());
      temporary_feb_id.set_depth(mapping::BOARD_DEPTH);
      my_wd_data_.cell_electronic_id.extract_to(temporary_feb_id);
      gg_tp.set_header(hit_id_,
		       temporary_feb_id,
		       signal_clocktick_,
		       mapping::THREE_WIRES_TRACKER_MODE,
		       mapping::SIDE_MODE,
		       mapping::NUMBER_OF_CONNECTED_ROWS);
      gg_tp.set_gg_tp_active_bit(my_wd_data_.cell_electronic_id.get(mapping::CHANNEL_INDEX));
      // gg_tp.set_auxiliaries(my_wd_data_.auxiliaries);
      _activated_bits_[my_wd_data_.cell_electronic_id.get(mapping::CHANNEL_INDEX)] = 1;
      // gg_tp.tree_dump(std::clog, "***** Geiger TP creation : *****", "INFO : ");

      return;
    }

    void signal_to_geiger_tp_algo::update_gg_tp(const geiger_digi_working_data & my_wd_data_,
						geiger_tp & my_geiger_tp_)
    {
      my_geiger_tp_.set_gg_tp_active_bit(my_wd_data_.cell_electronic_id.get(mapping::CHANNEL_INDEX));
      _activated_bits_[my_wd_data_.cell_electronic_id.get(mapping::CHANNEL_INDEX)] = 1;
      // my_geiger_tp_.tree_dump(std::clog, "***** Geiger TP Update : *****", "INFO : ");
      return;
    }

    void signal_to_geiger_tp_algo::_set_defaults()
    {
      for (unsigned int i = 0; i < geiger::tp::TP_SIZE; i++)
	{
	  _activated_bits_[i] = 0;
	}
      _signal_category_ = "sigtracker";
      _running_tp_id_   = -1;

      return;
    }

    void signal_to_geiger_tp_algo::_increment_running_tp_id()
    {
      _running_tp_id_++;
      return;
    }

    void signal_to_geiger_tp_algo::_prepare_working_data(const mctools::signal::signal_data & SSD_,
							gg_digi_working_data_collection_type & wd_collection_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
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

	    double event_time_ref = a_wd.signal_ref->get_time_ref();

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

	    std::size_t number_of_samples = 0;
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
		    trigger_time = x;
		    a_wd.anodic_R0 = x;
		    former_y = y;
		    number_of_samples++;
		    continue;
		  }

		if (y < 0
		    && former_y >= _gg_feb_config_.VHNT / CLHEP::volt
		    && y <= _gg_feb_config_.VHNT / CLHEP::volt
		    && R0_crossed
		    && !R1_crossed)
		  {
		    R1_crossed = true;
		    a_wd.anodic_R1 = x;
		    former_y = y;
		    number_of_samples++;
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
		    a_wd.anodic_R3 = x;
		    former_y = y;
		    number_of_samples++;
		    continue;
		  }

		if (y > 0
		    && former_y <= _gg_feb_config_.VHPT / CLHEP::volt
		    && y >= _gg_feb_config_.VHPT / CLHEP::volt
		    && R0_crossed
		    && !R2_crossed)
		  {
		    R2_crossed = true;
		    a_wd.anodic_R2 = x;
		    former_y = y;
		    number_of_samples++;
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
		    a_wd.anodic_R4 = x;
		    former_y = y;
		    number_of_samples++;
		    continue;
		  }

		// if (first_trigger) break;

		former_y = y;
		number_of_samples++;
	      }
	    // std::clog << "Number of samples = " << number_of_samples << std::endl;


	    const geomtools::geom_id & geom_id = a_wd.signal_ref->get_geom_id();
	    geomtools::geom_id electronic_id;

	    _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE, geom_id, electronic_id);
	    // std::clog << "GID = " << geom_id << " EID = " << electronic_id << std::endl;

	    double relative_time = trigger_time - event_time_ref ;
	    uint32_t a_geiger_signal_clocktick = _clock_utils_->compute_clocktick_800ns_from_time(relative_time);
	    //std::floor(event_time_ref / 800) +  _clocktick_ref_ + clock_utils::TRACKER_FEB_SHIFT_CLOCKTICK_NUMBER;

	    // if (relative_time > 800)
	    //   {
	    // 	a_geiger_signal_clocktick += static_cast<int32_t>(relative_time) / 800;
	    //   }


	    a_wd.trigger_time = trigger_time;
	    a_wd.geom_id      = geom_id;
	    a_wd.cell_electronic_id = electronic_id;
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
	      // Find the anodic working data already created:
	      geomtools::geom_id cathodic_reduced_gid;
	      cathodic_reduced_gid.set_depth(4);
	      cathodic_reduced_gid.set_type(mapping::GEIGER_CATEGORY_TYPE);
	      a_signal.get_geom_id().extract_to(cathodic_reduced_gid);
	      geomtools::geom_id electronic_id;
	      _electronic_mapping_->convert_GID_to_EID(mapping::THREE_WIRES_TRACKER_MODE, cathodic_reduced_gid, electronic_id);
	      std::clog << "GID = " << a_signal.get_geom_id() << " REDUCED_GID = " << cathodic_reduced_gid << " EID = " << electronic_id << std::endl;







	      mctools::signal::base_signal a_mutable_signal = a_signal;
	      std::string signal_name;
	      snemo::digitization::build_signal_name(a_mutable_signal.get_hit_id(),
						     signal_name);

	      a_mutable_signal.build_signal_shape(*_ssb_,
						  signal_name,
						  a_mutable_signal);

	      // a_mutable_signal.tree_dump(std::clog, "Mutable cathodic signal with shape instantiated");

	      const mygsl::i_unary_function * signal_functor = & a_mutable_signal.get_shape();

	      bool R5_crossed = false;
	      bool R6_crossed = false;

	      unsigned int nsamples = 1000;
	      double xmin = signal_functor->get_non_zero_domain_min();
	      double xmax = signal_functor->get_non_zero_domain_max();

	      if (i == 71) {
		const std::string filename = "/tmp/gg_signal_cathodic.dat";
		signal_functor->write_ascii_file(filename, xmin, xmax, nsamples);
	      }

	  }

	} // end of number_of_hits


      return ;
    }

    void signal_to_geiger_tp_algo::_sort_working_data(gg_digi_working_data_collection_type & wd_collection_)
    {
      std::sort(wd_collection_.begin(), wd_collection_.end());
      return;
    }

    void signal_to_geiger_tp_algo::_geiger_tp_process(const gg_digi_working_data_collection_type & wd_collection_,
						      geiger_tp_data & my_geiger_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
      int32_t geiger_tp_hit_id = 0;
      for (unsigned int i = 0; i < wd_collection_.size(); i++)
	{
	  uint32_t signal_clocktick  = wd_collection_[i].clocktick_800;
	  int existing_index = -1;
	  bool existing_eid  = false;

	  for (unsigned int j = 0; j < clock_utils::ACTIVATED_GEIGER_CELLS_NUMBER; j++)
	    {
	      bool existing_ct = false;
	      std::vector<datatools::handle<geiger_tp> > my_list_of_gg_tp_per_eid;

	      my_geiger_tp_data_.get_list_of_gg_tp_per_eid(wd_collection_[i].cell_electronic_id , my_list_of_gg_tp_per_eid);

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
				geiger_tp_hit_id,
				my_geiger_tp_data_);
		}
	      signal_clocktick++;
	    } // end of for (j < 10)

	} //end of for (i < wd_size)

      return;
    }

    void signal_to_geiger_tp_algo::process(const mctools::signal::signal_data & SSD_,
					   geiger_tp_data & my_geiger_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");

      /////////////////////////////////
      // Check simulated signal data //
      /////////////////////////////////

      // // Check if some 'simulated_data' are available in the data model:
      if (SSD_.has_signals(get_signal_category())) {
	/********************
	 * Process the data *
	 ********************/

	// Main processing method :
	_process(SSD_, my_geiger_tp_data_);
      }

      return;
    }

    void signal_to_geiger_tp_algo::_process(const mctools::signal::signal_data & SSD_,
					    geiger_tp_data & my_geiger_tp_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Signal to geiger TP algorithm is not initialized ! ");
      _prepare_working_data(SSD_, _gg_digi_data_collection_);
      _sort_working_data(_gg_digi_data_collection_);
      _geiger_tp_process(_gg_digi_data_collection_, my_geiger_tp_data_);

      // my_geiger_tp_data_.tree_dump(std::clog, "GG TP DATA");

      // for(uint32_t i = my_geiger_tp_data_.get_clocktick_min(); i <= my_geiger_tp_data_.get_clocktick_max(); i++)
      // 	{
      // 	  for(unsigned int j = 0 ; j <= mapping::NUMBER_OF_CRATES ; j++)
      // 	    {
      // 	      std::vector<datatools::handle<geiger_tp> > geiger_tp_list_per_clocktick_per_crate;
      // 	      my_geiger_tp_data_.get_list_of_gg_tp_per_clocktick_per_crate(i, j, geiger_tp_list_per_clocktick_per_crate);
      // 	      if(!geiger_tp_list_per_clocktick_per_crate.empty())
      // 		{
      // 		  for(unsigned int k = 0; k < geiger_tp_list_per_clocktick_per_crate.size(); k++)
      // 		    {
      // 		      const geiger_tp & my_geiger_tp =  geiger_tp_list_per_clocktick_per_crate[k].get();
      // 		      my_geiger_tp.tree_dump(std::clog, "a_geiger_tp : ", "INFO : ");
      // 		    }
      // 		}
      // 	    }
      // 	}

      return;
    }

  } // end of namespace digitization

} // end of namespace snemo
