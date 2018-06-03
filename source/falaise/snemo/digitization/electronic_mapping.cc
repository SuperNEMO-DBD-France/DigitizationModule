// snemo/digitization/electronic_mapping.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Standard library :
#include <string>

// - Boost:
#include <boost/scoped_ptr.hpp>

// - Bayeux/datatools:
#include <datatools/exception.h>

// Ourselves :
#include <snemo/digitization/electronic_mapping.h>

namespace snemo {

  namespace digitization {

    const std::set<int32_t> & electronic_mapping::supported_types()
    {
      static boost::scoped_ptr<std::set<int32_t> > _types;
      if (!_types) {
	_types.reset(new std::set<int32_t>);
	_types->insert(mapping::GEIGER_ANODIC_CATEGORY_TYPE);
	_types->insert(mapping::CALO_MAIN_WALL_CATEGORY_TYPE);
	_types->insert(mapping::CALO_XWALL_CATEGORY_TYPE);
	_types->insert(mapping::CALO_GVETO_CATEGORY_TYPE);
      }
      return *_types;
    }

    electronic_mapping::electronic_mapping()
    {
      _initialized_ = false;
      _geo_manager_ = 0;
      _geo_manager_status_ = false;
      _module_number_ = mapping::INVALID_MODULE_NUMBER;
      _module_number_status_ = false;
    }

    electronic_mapping::~electronic_mapping()
    {
      if (is_initialized())
	{
	  reset();
	}
      return;
    }

    void electronic_mapping::add_preconstructed_type(int type_)
    {
      DT_THROW_IF(supported_types().count(type_) == 0, std::logic_error, "Type ["<< type_ << "] is not supported ! ");
      _pre_constructed_types_.insert(type_);
      return;
    }

    void electronic_mapping::set_geo_manager(const geomtools::manager & mgr_ )
    {
      _geo_manager_ = & mgr_;
      _geo_manager_status_ = true;
      _ID_convertor_.set_geo_manager(mgr_);
    }

    bool electronic_mapping::geo_manager_is_set() const
    {
      return _geo_manager_status_;
    }

    void electronic_mapping::set_module_number(int module_number_)
    {
      _module_number_ = module_number_;
      _module_number_status_ = true;
      _ID_convertor_.set_module_number(module_number_);
      return;
    }

    bool electronic_mapping::module_number_is_set() const
    {
      return _module_number_status_;
    }

    void electronic_mapping::initialize()
    {
      DT_THROW_IF(!geo_manager_is_set() && !module_number_is_set(), std::logic_error, "Geo manager or module number is not set ! ");
      datatools::properties dummy_config;
      initialize(dummy_config);
      return;
    }

    void electronic_mapping::initialize(const datatools::properties & config_)
    {
      DT_THROW_IF(!geo_manager_is_set() && !module_number_is_set(), std::logic_error, "Geo manager or module number is not set ! ");

      _initialize(config_);
      return;
    }

    bool electronic_mapping::is_initialized() const
    {
      return _initialized_;
    }

    void electronic_mapping::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized, it can't be reset ! ");
      _geo_manager_ = 0;
      _initialized_ = false;
      _logging_ = datatools::logger::PRIO_FATAL;
      _module_number_ = mapping::INVALID_MODULE_NUMBER;
      _pre_constructed_types_.clear();
      _geiger_id_bimap_.clear();
      _mcalo_id_bimap_.clear();
      _xcalo_id_bimap_.clear();
      _gveto_id_bimap_.clear();
      _ID_convertor_.reset();
      return;
    }

    void electronic_mapping::convert_GID_to_EID(const bool tracker_trigger_mode_,
						const geomtools::geom_id & geom_id_,
						geomtools::geom_id & electronic_id_) const
    {
      const_cast<electronic_mapping *> (this) -> _convert_GID_to_EID(tracker_trigger_mode_,
								     geom_id_,
								     electronic_id_);
      return;
    }

    void electronic_mapping::convert_EID_to_GID(const bool tracker_trigger_mode_,
						const geomtools::geom_id & electronic_id_,
						geomtools::geom_id & geom_id_) const
    {
      const_cast<electronic_mapping *> (this) -> _convert_EID_to_GID(tracker_trigger_mode_,
								     electronic_id_,
								     geom_id_);
      return;
    }

    void electronic_mapping::dump_geiger_bimap(std::ostream & out_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized ! ");

      for (ID_bimap::const_iterator it = _geiger_id_bimap_.begin(); it != _geiger_id_bimap_.end(); it++)
	{
	  out_ << it->left << " <=> " << it->right << std::endl;
	}
    }

    void electronic_mapping::dump_main_calo_bimap(std::ostream & out_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized ! ");

      for (ID_bimap::const_iterator it = _mcalo_id_bimap_.begin(); it != _mcalo_id_bimap_.end(); it++)
	{
	  out_ << it->left << " <=> " << it->right << std::endl;
	}
    }

    void electronic_mapping::dump_xwall_bimap(std::ostream & out_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized ! ");

      for (ID_bimap::const_iterator it = _xcalo_id_bimap_.begin(); it != _xcalo_id_bimap_.end(); it++)
	{
	  out_ << it->left << " <=> " << it->right << std::endl;
	}
    }

    void electronic_mapping::dump_gveto_bimap(std::ostream & out_) const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized ! ");

      for (ID_bimap::const_iterator it = _gveto_id_bimap_.begin(); it != _gveto_id_bimap_.end(); it++)
	{
	  out_ << it->left << " <=> " << it->right << std::endl;
	}
    }

    void electronic_mapping::_initialize(const datatools::properties & config_)
    {
      _initialized_ = true;
      add_preconstructed_type(mapping::GEIGER_ANODIC_CATEGORY_TYPE);
      add_preconstructed_type(mapping::CALO_MAIN_WALL_CATEGORY_TYPE);
      add_preconstructed_type(mapping::CALO_XWALL_CATEGORY_TYPE);
      add_preconstructed_type(mapping::CALO_GVETO_CATEGORY_TYPE);

      if (config_.has_key("module_number")) {
	int module_number = config_.fetch_integer("module_number");
	set_module_number(module_number);
      }

      _ID_convertor_.initialize(config_);
      for (std::set<int32_t>::iterator it = _pre_constructed_types_.begin();
	   it != _pre_constructed_types_.end();
	   it++)
	{
	  if (*it == mapping::GEIGER_ANODIC_CATEGORY_TYPE)  _init_geiger();
	  if (*it == mapping::CALO_MAIN_WALL_CATEGORY_TYPE) _init_mcalo();
	  if (*it == mapping::CALO_XWALL_CATEGORY_TYPE)     _init_xwall();
	  if (*it == mapping::CALO_GVETO_CATEGORY_TYPE)     _init_gveto();
	}


      return;
    }


    void electronic_mapping::_init_geiger()
    {
      uint32_t geiger_gid_type = -1;
      geomtools::geom_id GID(geiger_gid_type, 0, 0, 0, 0, 0);

      for (unsigned int side = 0; side < mapping::NUMBER_OF_SIDES; side++)
	{
	  GID.set(mapping::SIDE_INDEX, side);
	  for (unsigned int layer = 0; layer < mapping::NUMBER_OF_GEIGER_LAYERS; layer++)
	    {
	      GID.set(mapping::LAYER_INDEX, layer);
	      for (unsigned int row = 0; row < mapping::NUMBER_OF_GEIGER_ROWS; row++)
		{
		  GID.set(mapping::ROW_INDEX, row);
		  for (unsigned int part = 0; part < mapping::NUMBER_OF_GEIGER_CELL_PARTS; part++)
		    {
		      geomtools::geom_id EID;
		      GID.set(mapping::GEIGER_CELL_PART_INDEX, part);
		      if (part == mapping::GEIGER_CELL_ANODIC_PART)
			{
			  geiger_gid_type = mapping::GEIGER_ANODIC_CATEGORY_TYPE;
			  GID.set_type(mapping::GEIGER_ANODIC_CATEGORY_TYPE);
			}
		      else
			{
			  geiger_gid_type = mapping::GEIGER_CATHODIC_CATEGORY_TYPE;
			  GID.set_type(mapping::GEIGER_CATHODIC_CATEGORY_TYPE);
			}
		      EID = _ID_convertor_.convert_GID_to_EID(GID);
		      _geiger_id_bimap_.insert(ID_doublet(GID , EID)) ;
		    } // end of part loop
		} // end of row loop
	    } // end of layer loop
	} // end of side loop
      return ;
    }

    void electronic_mapping::_init_mcalo()
    {
      geomtools::geom_id GID(mapping::CALO_MAIN_WALL_CATEGORY_TYPE, 0, 0, 0, 0);
      geomtools::geom_id EID;
      for(unsigned int side = 0; side < mapping::NUMBER_OF_SIDES; side++)
	{
	  GID.set(mapping::SIDE_INDEX, side);
	  for(unsigned int column = 0; column < mapping::NUMBER_OF_MAIN_CALO_COLUMNS; column++)
	    {
	      GID.set(mapping::COLUMN_INDEX, column);
	      for(unsigned int row = 0; row < mapping::NUMBER_OF_MAIN_CALO_ROWS; row++)
		{
		  GID.set(mapping::ROW_INDEX, row);
		  EID = _ID_convertor_.convert_GID_to_EID(GID);
		  _mcalo_id_bimap_.insert( ID_doublet(GID , EID) ) ;
		} // end of row loop
	    } // end of column loop
	} // end of side loop
      return ;
    }

    void electronic_mapping::_init_xwall()
    {
      geomtools::geom_id GID(mapping::CALO_XWALL_CATEGORY_TYPE, 0, 0, 0, 0, 0);
      geomtools::geom_id EID;
      for(unsigned int side = 0; side < mapping::NUMBER_OF_SIDES; side++)
	{
	  GID.set(mapping::SIDE_INDEX, side);
	  for (unsigned int wall = 0; wall < mapping::NUMBER_OF_WALLS; wall ++)
	    {
	      GID.set(mapping::WALL_INDEX, wall);
	      for(unsigned int column = 0; column < mapping::NUMBER_OF_XWALL_COLUMNS; column++)
		{
		  GID.set(mapping::XWALL_COLUMN_INDEX, column);
		  for(unsigned int row = 0; row < mapping::NUMBER_OF_XWALL_ROWS; row++)
		    {
		      GID.set(mapping::XWALL_ROW_INDEX, row);
		      EID = _ID_convertor_.convert_GID_to_EID(GID);
		      _xcalo_id_bimap_.insert( ID_doublet(GID , EID) ) ;
		    } // end of row loop
		} // end of column loop
	    } // end of wall loop
	} // end of side loop
      return;
    }

    void electronic_mapping::_init_gveto()
    {
      geomtools::geom_id GID(mapping::CALO_GVETO_CATEGORY_TYPE, 0, 0, 0, 0);
      geomtools::geom_id EID;
      for(unsigned int side = 0; side < mapping::NUMBER_OF_SIDES; side++)
	{
	  GID.set(mapping::SIDE_INDEX, side);
	  for (unsigned int wall = 0; wall < mapping::NUMBER_OF_WALLS; wall ++)
	    {
	      GID.set(mapping::WALL_INDEX, wall);
	      for(unsigned int column = 0; column < mapping::NUMBER_OF_GVETO_COLUMNS; column++)
		{
		  GID.set(mapping::GVETO_COLUMN_INDEX, column);
		  EID = _ID_convertor_.convert_GID_to_EID(GID);
		  _gveto_id_bimap_.insert(ID_doublet(GID, EID));
		} // end of column loop
	    } // end of wall loop
	} // end of side loop
      return;
    }

    void electronic_mapping::_convert_EID_to_GID(const bool tracker_trigger_mode_,
						 const geomtools::geom_id & elec_id_,
						 geomtools::geom_id & geom_id_)
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized ! ");
      DT_THROW_IF(!(elec_id_.get_type() == mapping::GEIGER_FEB_CATEGORY_TYPE
		    || elec_id_.get_type() == mapping::CALO_FEB_CATEGORY_TYPE),
		  std::logic_error, "Electronic ID [" << elec_id_ << "] incorrect type [" << elec_id_.get_type() << "] !");
      DT_THROW_IF(tracker_trigger_mode_ != mapping::THREE_WIRES_TRACKER_MODE,
		  std::logic_error,
		  "Give a correct tracker trigger mode (Two wires mode is not supported yet) ! ");
      geom_id_.reset();

      unsigned int input_type = elec_id_.get_type();

      // Geiger case:
      if (input_type == mapping::GEIGER_FEB_CATEGORY_TYPE)
	{
	  ID_bimap::right_const_iterator right_iter = _geiger_id_bimap_.right.end();
      	  right_iter = _geiger_id_bimap_.right.find(elec_id_);
      	  if (right_iter != _geiger_id_bimap_.right.end())
      	    {
      	      geom_id_ = right_iter->second;
      	    }
	}

      // Calo case:
      if (input_type == mapping::CALO_FEB_CATEGORY_TYPE)
	{
	  // Search in Main Calo bimap:
	  if (elec_id_.get(mapping::CRATE_INDEX) == mapping::MAIN_CALO_SIDE_0_CRATE || elec_id_.get(mapping::CRATE_INDEX) == mapping::MAIN_CALO_SIDE_1_CRATE)
	    {
	      ID_bimap::right_const_iterator right_iter = _mcalo_id_bimap_.right.end();
	      right_iter = _mcalo_id_bimap_.right.find(elec_id_);
	      if (right_iter != _mcalo_id_bimap_.right.end())
		{
		  geom_id_ = right_iter->second;
		}
	    }


	  if (elec_id_.get(mapping::CRATE_INDEX) == mapping::XWALL_GVETO_CALO_CRATE)
	    {
	      // Search in Calo Gveto bimap:
	      if (elec_id_.get(mapping::BOARD_INDEX) == 4
		  || elec_id_.get(mapping::BOARD_INDEX) == 5
		  || elec_id_.get(mapping::BOARD_INDEX) == 15
		  || elec_id_.get(mapping::BOARD_INDEX) == 16)
		{
		  ID_bimap::right_const_iterator right_iter = _gveto_id_bimap_.right.end();
		  right_iter = _gveto_id_bimap_.right.find(elec_id_);
		  if (right_iter != _gveto_id_bimap_.right.end())
		    {
		      geom_id_ = right_iter->second;
		    }
		}
	      else
		{
		  // Search in Calo XWall bimap:
		  ID_bimap::right_const_iterator right_iter = _xcalo_id_bimap_.right.end();
		  right_iter = _xcalo_id_bimap_.right.find(elec_id_);
		  if (right_iter != _xcalo_id_bimap_.right.end())
		    {
		      geom_id_ = right_iter->second;
		    }
		}
	    }
	}

      return;
    }

    void electronic_mapping::_convert_GID_to_EID(const bool tracker_trigger_mode_,
						 const geomtools::geom_id & geom_id_,
						 geomtools::geom_id & electronic_id_)
    {
      DT_THROW_IF(tracker_trigger_mode_ != mapping::THREE_WIRES_TRACKER_MODE, std::logic_error, " Give a correct tracker trigger mode (Two wires mode is not supported yet) ! ");
      DT_THROW_IF(!is_initialized(), std::logic_error, "Electronic mapping is not initialized ! ");
      electronic_id_.reset();

      switch (geom_id_.get_type())
	{
	case mapping::GEIGER_ANODIC_CATEGORY_TYPE :
	  {
	    // In geometry, cell GID is for anodic but it does not have the 'part' index
	  // Little 'hack' on this GID in order to add the ANODIC_PART at the last index.
	    geomtools::geom_id hacked_gid;
	    hacked_gid.set_type(mapping::GEIGER_ANODIC_CATEGORY_TYPE);
	    // Miss anodic Part on geom ID ( == 2)
	    if (geom_id_.get_depth() == mapping::GEIGER_ROW_DEPTH) {
	      hacked_gid.set_depth(mapping::GEIGER_CELL_PART_DEPTH);
	      hacked_gid.set_address(geom_id_.get(mapping::MODULE_INDEX),
				     geom_id_.get(mapping::SIDE_INDEX),
				     geom_id_.get(mapping::LAYER_INDEX),
				     geom_id_.get(mapping::ROW_INDEX));
	      hacked_gid.set(mapping::GEIGER_CELL_PART_INDEX,
			     mapping::GEIGER_CELL_ANODIC_PART);
	    }
	    else hacked_gid = geom_id_;
	    ID_bimap::left_const_iterator left_iter = _geiger_id_bimap_.left.end();
	    left_iter = _geiger_id_bimap_.left.find(hacked_gid);
	    if (left_iter != _geiger_id_bimap_.left.end() )
	      {
		electronic_id_ = left_iter->second;
	      }
	    break;
	  }

	case mapping::GEIGER_CATHODIC_CATEGORY_TYPE :
	  {
	    ID_bimap::left_const_iterator left_iter = _geiger_id_bimap_.left.end();
	    left_iter = _geiger_id_bimap_.left.find(geom_id_);
	    if (left_iter != _geiger_id_bimap_.left.end() )
	      {
		electronic_id_ = left_iter->second;
	      }
	    break;
	  }

	case mapping::CALO_MAIN_WALL_CATEGORY_TYPE :
	  {
	    // Hack because the depth is too much (there is an '*' part at the end for the OM)
	    // Reduce the depth at [Module.Side.Column.Row] for Main wall OM
	    geomtools::geom_id hacked_gid;
	    hacked_gid.set_type(mapping::CALO_MAIN_WALL_CATEGORY_TYPE);
	    if (geom_id_.get_depth() == mapping::MAIN_CALO_ROW_DEPTH + 1) {
	      hacked_gid.set_depth(mapping::MAIN_CALO_ROW_DEPTH);
	      geom_id_.extract_to(hacked_gid);
	    }
	    else hacked_gid = geom_id_;
	    ID_bimap::left_const_iterator left_iter = _mcalo_id_bimap_.left.end();
	    left_iter = _mcalo_id_bimap_.left.find(hacked_gid);
	    if (left_iter != _mcalo_id_bimap_.left.end() )
	      {
		electronic_id_ = left_iter->second;
	      }
	    break;
	  }

	case mapping::CALO_XWALL_CATEGORY_TYPE :
	  {
	    ID_bimap::left_const_iterator left_iter = _xcalo_id_bimap_.left.end();
	    left_iter = _xcalo_id_bimap_.left.find(geom_id_);
	    if (left_iter != _xcalo_id_bimap_.left.end() )
	      {
		electronic_id_ = left_iter->second;
	      }

	    break;
	  }

	case mapping::CALO_GVETO_CATEGORY_TYPE :
	  {
	    ID_bimap::left_const_iterator left_iter = _gveto_id_bimap_.left.end();
	    left_iter = _gveto_id_bimap_.left.find(geom_id_);
	    if (left_iter != _gveto_id_bimap_.left.end() )
	      {
		electronic_id_ = left_iter->second;
	      }
	    break;
	  }

	default :
	  break;
	}
      return ;
    }

  } // end of namespace digitization

} // end of namespace snemo
