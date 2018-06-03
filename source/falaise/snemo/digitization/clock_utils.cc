// snemo/digitization/clock_utils.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Standard Library :
#include <math.h>

// - Bayeux/datatools:
#include <datatools/properties.h>
#include <datatools/exception.h>

// Ourselves
#include <snemo/digitization/clock_utils.h>

namespace snemo {

  namespace digitization {


    const uint32_t clock_utils::NUMBER_OF_25_CLOCK_IN_800;
    const uint32_t clock_utils::MAIN_CLOCKTICK;
    const uint32_t clock_utils::TRACKER_CLOCKTICK;
    const uint32_t clock_utils::TRIGGER_CLOCKTICK;
    const uint32_t clock_utils::INVALID_CLOCKTICK;
    const uint32_t clock_utils::ACTIVATED_GEIGER_CELLS_NUMBER;
    const uint32_t clock_utils::CALO_FEB_SHIFT_CLOCKTICK_NUMBER;
    const uint32_t clock_utils::CALO_CB_SHIFT_CLOCKTICK_NUMBER;

    const uint32_t clock_utils::TRACKER_FEB_SHIFT_CLOCKTICK_NUMBER;
    const uint32_t clock_utils::TRACKER_CB_SHIFT_CLOCKTICK_NUMBER;
    const uint32_t clock_utils::TRACKER_FEB_TO_CB_SHIFT_CLOCKTICK_NUMBER;
    const uint32_t clock_utils::TRACKER_CB_TO_TB_SHIFT_CLOCKTICK_NUMBER;

    const uint32_t clock_utils::TRIGGER_FEB_TO_TB_SHIFT_CLOCKTICK_1600NS;
    const uint32_t clock_utils::TRIGGER_COMPUTING_SHIFT_CLOCKTICK_1600NS;
    const uint32_t clock_utils::PREVIOUS_EVENT_RECORD_LIVING_NUMBER_OF_CLOCKTICK;

    clock_utils::clock_utils()
    {
      _initialized_ = false;
      _clocktick_ref_ = 0;
    }

    clock_utils::~clock_utils()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void clock_utils::initialize()
    {
      DT_THROW_IF(_clocktick_ref_ != 0, std::logic_error, "Clocktick reference have to be equal to 0 ! ");
      _initialized_ = true;
      return;
    }

    bool clock_utils::is_initialized() const
    {
      return _initialized_;
    }

    void clock_utils::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized, it can't be reset ! ");
      _initialized_ = false;
      return;
    }

    int32_t clock_utils::get_clocktick_ref() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      return _clocktick_ref_;
    }

    double clock_utils::get_shift_1600() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      return _shift_1600_;
    }

    int32_t clock_utils::get_clocktick_25_ref() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      return _clocktick_25_ref_;
    }

    int32_t clock_utils::get_clocktick_800_ref() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      return _clocktick_800_ref_;
    }
    double clock_utils::get_shift_25() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      return _shift_25_;
    }
    double clock_utils::get_shift_800() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      return _shift_800_;
    }

    int32_t clock_utils::compute_clocktick_25ns_from_time(const double time_)
    {
      // TO DO : add shift 25 ns to time and to compute CT 25.
      // (not for the moment because it is easier to not use it for tests purpose
      if (datatools::is_valid(time_)) {
      	int32_t ct_25ns = _clocktick_25_ref_;

	if (time_ > 25 * CLHEP::ns)
	  {
	    ct_25ns += static_cast<uint32_t>(time_ / 25);
	  }

	return ct_25ns;
      }
      else return INVALID_CLOCKTICK;
    }

    int32_t clock_utils::compute_clocktick_800ns_from_time(const double time_)
    {
      if (datatools::is_valid(time_)) {
      	int32_t ct_800ns = _clocktick_800_ref_;

	if (time_ > 800 * CLHEP::ns)
	  {
	    ct_800ns += static_cast<uint32_t>(time_ / 800);
	  }

	return ct_800ns;
      }
      else return INVALID_CLOCKTICK;
    }

    int32_t clock_utils::compute_clocktick_1600ns_from_time(const double time_)
    {
      if (datatools::is_valid(time_)) {
      	int32_t ct_1600ns = _clocktick_ref_;

	if (time_ > 1600 * CLHEP::ns)
	  {
	    ct_1600ns += static_cast<uint32_t>(time_ / 1600);
	  }

	return ct_1600ns;
      }
      else return INVALID_CLOCKTICK;
    }

    void clock_utils::compute_clocktick_25ns_to_1600ns(const uint32_t clocktick_25ns_,
						       uint32_t & clocktick_1600ns_) const
    {
      clocktick_1600ns_ = (clocktick_25ns_ * MAIN_CLOCKTICK) / TRIGGER_CLOCKTICK;
      return;
    }

    void clock_utils::compute_clocktick_1600ns_to_25ns(const uint32_t clocktick_1600ns_,
						       uint32_t & clocktick_25ns_) const
    {
      clocktick_25ns_ = (clocktick_1600ns_ * TRIGGER_CLOCKTICK) / MAIN_CLOCKTICK;
      return;
    }

    void clock_utils::compute_clocktick_800ns_to_1600ns(const uint32_t clocktick_800ns_,
							uint32_t & clocktick_1600ns_) const
    {
      clocktick_1600ns_ = (clocktick_800ns_ * TRACKER_CLOCKTICK) / TRIGGER_CLOCKTICK;
      return;
    }

    void clock_utils::compute_clocktick_1600ns_to_800ns(const uint32_t clocktick_1600ns_,
							uint32_t & clocktick_800ns_) const
    {
      clocktick_800ns_ = (clocktick_1600ns_ * TRIGGER_CLOCKTICK) / TRACKER_CLOCKTICK;
      return;
    }

    void clock_utils::compute_clockticks_ref(mygsl::rng & prng_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      _randomize_shift(prng_);
      _clocktick_25_ref_ = _shift_1600_ / MAIN_CLOCKTICK;
      _shift_25_ = fmod(_shift_1600_, MAIN_CLOCKTICK);
      _clocktick_800_ref_ = _shift_1600_ / TRACKER_CLOCKTICK;
      _shift_800_ = fmod(_shift_1600_, TRACKER_CLOCKTICK);
      return;
    }

    void clock_utils::_randomize_shift(mygsl::rng & prng_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Clock utils is not initialized ! ");
      _shift_1600_ = prng_.flat(0.0, TRIGGER_CLOCKTICK);
      return;
    }

    void clock_utils::tree_dump (std::ostream & out_,
				 const std::string & title_,
				 const std::string & indent_,
				 bool /*inherit_*/) const
    {

      out_ << indent_ << title_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Shift [0;1600] : " << _shift_1600_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "CT Ref 25 : " << _clocktick_25_ref_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Shift 25 : " << _shift_25_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "CT Ref 800 : " << _clocktick_800_ref_ << std::endl;

      out_ << indent_ << datatools::i_tree_dumpable::tag
	   << "Shift 800 : " << _shift_800_ << std::endl;

      return;
    }

  } // end of namespace digitization

} // end of namespace snemo
