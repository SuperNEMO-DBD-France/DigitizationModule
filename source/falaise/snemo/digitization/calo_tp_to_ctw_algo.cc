// snemo/digitization/calo_tp_to_ctw_algo.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Ourselves:
#include <snemo/digitization/calo_tp_to_ctw_algo.h>

// Third party:
// - Bayeux/datatools:
#include <datatools/exception.h>

namespace snemo {

  namespace digitization {

    calo_tp_to_ctw_algo::calo_tp_to_ctw_algo()
    {
      _initialized_ = false;
      return;
    }

    calo_tp_to_ctw_algo::~calo_tp_to_ctw_algo()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    bool calo_tp_to_ctw_algo::is_initialized() const
    {
      return _initialized_;
    }

    void calo_tp_to_ctw_algo::initialize(const datatools::properties & config_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Calo tp to ctw algo is already initialized ! ");

      _initialized_ = true;
      return;
    }

    void calo_tp_to_ctw_algo::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Calo tp to ctw algo is not initialized, it can't be reset ! ");
      _initialized_ = false;
      return;
    }

    void calo_tp_to_ctw_algo::set_ctw_htm(const calo_tp & my_calo_tp_,
					  calo_ctw & my_ctw_)
    {
      if (my_ctw_.is_main_wall()) // Mode == Main Wall
	{
	  if (my_ctw_.is_htm_main_wall())
	    {
	      unsigned int ctw_multiplicity = my_ctw_.get_htm_main_wall_info();
	      my_ctw_.set_htm_main_wall(ctw_multiplicity + my_calo_tp_.get_htm());
	    }
	  else
	    {
	      my_ctw_.set_htm_main_wall(my_calo_tp_.get_htm());
	    }
	}

      if (!my_ctw_.is_main_wall()) // Mode == XWall Gveto
	{
	  unsigned int board_id = my_calo_tp_.get_geom_id().get(mapping::BOARD_INDEX);
	  if (board_id == 4 || board_id == 5 || board_id == 15 || board_id == 16)
	    { // Filling gamma veto multiplicity
	      if (my_ctw_.is_htm_gveto())
		{
		  unsigned int ctw_multiplicity = my_ctw_.get_htm_gveto_info();
		  my_ctw_.set_htm_gveto(ctw_multiplicity + my_calo_tp_.get_htm());
		}
	      else
		{
		  my_ctw_.set_htm_gveto(my_calo_tp_.get_htm());
		}
	    } // end of gamma veto board

	  if (board_id == 6 || board_id == 7 || board_id == 8 || board_id == 9)
	    { // Filling xwall side 0 multiplicity
	      if (my_ctw_.is_htm_xwall_side_0())
		{
		  unsigned int ctw_multiplicity = my_ctw_.get_htm_xwall_side_0_info();
		  my_ctw_.set_htm_xwall_side_0(ctw_multiplicity + my_calo_tp_.get_htm());
		}
	      else
		{
		  my_ctw_.set_htm_xwall_side_0(my_calo_tp_.get_htm());
		}
	    } // end of wall side 0 board

	  if (board_id == 11 || board_id == 12 || board_id == 13 || board_id == 14)
	    { // Filling xwall side 1 multiplicity
	      if (my_ctw_.is_htm_xwall_side_1())
		{
		  unsigned int ctw_multiplicity = my_ctw_.get_htm_xwall_side_1_info();
		  my_ctw_.set_htm_xwall_side_1(ctw_multiplicity + my_calo_tp_.get_htm());
		}
	      else
		{
		  my_ctw_.set_htm_xwall_side_1(my_calo_tp_.get_htm());
		}
	    }
	}
      return;
    }

    void calo_tp_to_ctw_algo::set_ctw_zone_bit_htm(const calo_tp & my_calo_tp_,
						   calo_ctw & my_ctw_)
    {
      unsigned int activated_zone_id = 0;

      if (my_ctw_.is_main_wall()) // Mode == Main Wall
	{
	  if (my_calo_tp_.is_htm())
	    {
	      if (my_calo_tp_.get_geom_id().get(mapping::BOARD_INDEX) > 9)
		{
		  activated_zone_id = (my_calo_tp_.get_geom_id().get(mapping::BOARD_INDEX) + BOARD_SHIFT_INDEX) / 2;
		}
	      else
		{
		  activated_zone_id = my_calo_tp_.get_geom_id().get(mapping::BOARD_INDEX) / 2;
		}
	      my_ctw_.set_zoning_bit(calo::ctw::W_ZW_BIT0 + activated_zone_id, true);
	    }
	}

      if (!my_ctw_.is_main_wall()) // Mode == XWall Gveto
	{
	  if (my_calo_tp_.is_htm())
	    {
	      unsigned int board_id = my_calo_tp_.get_geom_id().get(mapping::BOARD_INDEX);
	      if (board_id == 4 || board_id == 5 || board_id == 15 || board_id == 16)
		{
		}
	      else
		{
		  if (board_id == 6 || board_id == 7)
		    {
		      activated_zone_id = 0;
		    }
		  else if (board_id == 8 || board_id == 9)
		    {
		      activated_zone_id = 1;
		    }

		  else if (board_id == 11 || board_id == 12)
		    {
		      activated_zone_id = 3;
		    }

		  else if (board_id == 13 || board_id == 14)
		    {
		      activated_zone_id = 2;
		    }
		  my_ctw_.set_zoning_bit(calo::ctw::ZONING_XWALL_BIT0 + activated_zone_id, true);
		} // end of else
	    } // end of if is htm
	} // end of if is main wall

      return ;
    }

    void calo_tp_to_ctw_algo::set_ctw_lto(const calo_tp & my_calo_tp_,
					  calo_ctw & my_ctw_)
    {
      if (my_ctw_.is_main_wall()) // Mode == Main Wall
	{
	  if (my_calo_tp_.is_lto()) my_ctw_.set_lto_main_wall_bit(true);
	}

      if (!my_ctw_.is_main_wall()) // Mode == XWall Gveto
	{
	  unsigned int board_id = my_calo_tp_.get_geom_id().get(mapping::BOARD_INDEX);
	  if (board_id == 4 || board_id == 5 || board_id == 15 || board_id == 16)
	    { // Filling gamma veto multiplicity
	      if (my_calo_tp_.is_lto()) my_ctw_.set_lto_gveto_bit(true);
	    }

	  if (board_id == 6 || board_id == 7 || board_id == 8 || board_id == 9)
	    { // Filling xwall side 0 multiplicity
	      if (my_calo_tp_.is_lto()) my_ctw_.set_lto_xwall_side_0_bit(true);
	    }

	  if (board_id == 11 || board_id == 12 || board_id == 13 || board_id == 14)
	    { // Filling xwall side 1 multiplicity
	      if (my_calo_tp_.is_lto()) my_ctw_.set_lto_xwall_side_1_bit(true);
	    }
	}

      return;
    }

    void calo_tp_to_ctw_algo::_fill_a_main_wall_ctw(const calo_tp & my_calo_tp_,
						    calo_ctw & a_calo_ctw_)
    {
      geomtools::geom_id temporary_feb_id;
      temporary_feb_id.set_type(mapping::CALO_CONTROL_BOARD_TYPE);
      temporary_feb_id.set_depth(mapping::BOARD_DEPTH);
      my_calo_tp_.get_geom_id().extract_to(temporary_feb_id);
      temporary_feb_id.set(mapping::BOARD_INDEX, mapping::CONTROL_BOARD_INDEX);

      uint32_t clocktick_with_internal_shift = my_calo_tp_.get_clocktick_25ns() + clock_utils::CALO_CB_SHIFT_CLOCKTICK_NUMBER;

      a_calo_ctw_.set_header(my_calo_tp_.get_hit_id(),
			     temporary_feb_id,
			     clocktick_with_internal_shift);

      set_ctw_htm(my_calo_tp_, a_calo_ctw_);
      set_ctw_zone_bit_htm(my_calo_tp_, a_calo_ctw_);
      set_ctw_lto(my_calo_tp_, a_calo_ctw_);
      return;
    }


    void calo_tp_to_ctw_algo::_fill_a_xwall_gveto_ctw(const calo_tp & my_calo_tp_,
						      calo_ctw & a_calo_ctw_)
    {
      geomtools::geom_id temporary_feb_id;
      temporary_feb_id.set_type(mapping::CALO_CONTROL_BOARD_TYPE);
      temporary_feb_id.set_depth(mapping::BOARD_DEPTH);
      my_calo_tp_.get_geom_id().extract_to(temporary_feb_id);
      temporary_feb_id.set(mapping::BOARD_INDEX, mapping::CONTROL_BOARD_INDEX);

      uint32_t clocktick_with_internal_shift = my_calo_tp_.get_clocktick_25ns() + clock_utils::CALO_CB_SHIFT_CLOCKTICK_NUMBER;

      a_calo_ctw_.set_header(my_calo_tp_.get_hit_id(),
			     temporary_feb_id,
			     clocktick_with_internal_shift);
      set_ctw_htm(my_calo_tp_, a_calo_ctw_);
      set_ctw_zone_bit_htm(my_calo_tp_, a_calo_ctw_);
      set_ctw_lto(my_calo_tp_, a_calo_ctw_);
      return;
    }

    void calo_tp_to_ctw_algo::_process_for_a_ctw_for_a_clocktick_for_main_wall(const std::vector<datatools::handle<calo_tp> > & my_list_of_calo_tp_,
									       calo_ctw & a_calo_ctw_)
    {
      for (unsigned int i = 0; i < my_list_of_calo_tp_.size(); i++)
	{
	  const calo_tp & my_calo_tp = my_list_of_calo_tp_[i].get();
	  _fill_a_main_wall_ctw(my_calo_tp, a_calo_ctw_);
	}
      return;
    }

    void calo_tp_to_ctw_algo::_process_for_a_ctw_for_a_clocktick_for_xwall_gveto(const std::vector<datatools::handle<calo_tp> > & my_list_of_calo_tp_,
										 calo_ctw & a_calo_ctw_)
    {
      for(unsigned int i = 0; i < my_list_of_calo_tp_.size(); i++)
	{
	  const calo_tp & my_calo_tp = my_list_of_calo_tp_[i].get();
	  _fill_a_xwall_gveto_ctw(my_calo_tp, a_calo_ctw_);
	}
      return;
    }

    void calo_tp_to_ctw_algo::process(const calo_tp_data & calo_tp_data_,
				      calo_ctw_data & calo_ctw_data_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Calo tp to ctw algo is not initialized, it can't process ! ");

      for (uint32_t i = calo_tp_data_.get_clocktick_min(); i <= calo_tp_data_.get_clocktick_max(); i++)
	{
	  std::vector<datatools::handle<calo_tp> > tp_list_per_clocktick_for_crate_0;
	  calo_tp_data_.get_list_of_tp_per_clocktick_per_crate(i,
							       mapping::MAIN_CALO_SIDE_0_CRATE,
							       tp_list_per_clocktick_for_crate_0);

	  if (!tp_list_per_clocktick_for_crate_0.empty())
	    {
	      calo_ctw & a_ctw_ = calo_ctw_data_.add();
	      _process_for_a_ctw_for_a_clocktick_for_main_wall(tp_list_per_clocktick_for_crate_0, a_ctw_);
	      // a_ctw_.tree_dump(std::clog, "a_calo_ctw : ", "INFO : ");
	    }

	  std::vector<datatools::handle<calo_tp> > tp_list_per_clocktick_for_crate_1;
	  calo_tp_data_.get_list_of_tp_per_clocktick_per_crate(i,
							       mapping::MAIN_CALO_SIDE_1_CRATE,
							       tp_list_per_clocktick_for_crate_1);

	  if (!tp_list_per_clocktick_for_crate_1.empty())
	    {
	      calo_ctw & a_ctw_ = calo_ctw_data_.add();
	      _process_for_a_ctw_for_a_clocktick_for_main_wall(tp_list_per_clocktick_for_crate_1, a_ctw_);
	      // a_ctw_.tree_dump(std::clog, "a_calo_ctw : ", "INFO : ");
	    }

	  std::vector<datatools::handle<calo_tp> > tp_list_per_clocktick_for_crate_2;
	  calo_tp_data_.get_list_of_tp_per_clocktick_per_crate(i,
							       mapping::XWALL_GVETO_CALO_CRATE,
							       tp_list_per_clocktick_for_crate_2);

	  if (!tp_list_per_clocktick_for_crate_2.empty())
	    {
	      calo_ctw & a_ctw_ = calo_ctw_data_.add();
	      _process_for_a_ctw_for_a_clocktick_for_xwall_gveto(tp_list_per_clocktick_for_crate_2, a_ctw_);
	      // a_ctw_.tree_dump(std::clog, "a_calo_ctw : ", "INFO : ");
	    }
	}


      // //std::clog << "Actual CT = " << i << std::endl;
      // if (_mode_ == MODE_MAIN_WALL)
      //   {
      //     std::vector<datatools::handle<calo_tp> > tp_list_per_clocktick_per_crate;
      //     calo_tp_data_.get_list_of_tp_per_clocktick_per_crate(i, _crate_number_, tp_list_per_clocktick_per_crate);

      //     if (!tp_list_per_clocktick_per_crate.empty())
      // 	{
      // 	  calo_ctw & a_ctw_ = calo_ctw_data_.add();
      // 	  _process_for_a_ctw_for_a_clocktick_for_main_wall(tp_list_per_clocktick_per_crate, a_ctw_);
      // 	  // a_ctw_.tree_dump(std::clog, "a_calo_ctw : ", "INFO : ");
      // 	}
      //   }

      // if (_mode_ == MODE_XWALL_GVETO)
      //   {
      //     std::vector<datatools::handle<calo_tp> > tp_list_per_clocktick_per_crate;
      //     calo_tp_data_.get_list_of_tp_per_clocktick_per_crate(i, _crate_number_, tp_list_per_clocktick_per_crate);

      //     if (!tp_list_per_clocktick_per_crate.empty())
      // 	{
      // 	  calo_ctw & a_ctw_ = calo_ctw_data_.add();
      // 	  _process_for_a_ctw_for_a_clocktick_for_xwall_gveto(tp_list_per_clocktick_per_crate, a_ctw_);
      // 	  // a_ctw_.tree_dump(std::clog, "a_calo_ctw : ", "INFO : ");
      // 	}
      //   }
      // }

      return;
    }

  } // end of namespace digitization

} // end of namespace snemo
