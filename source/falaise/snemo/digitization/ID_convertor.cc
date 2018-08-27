// snemo/digitization/ID_convertor.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Standard library :
#include <map>
#include <fstream>

// This project :
#include <falaise/snemo/geometry/xcalo_locator.h>
#include <falaise/snemo/geometry/gveto_locator.h>
#include <falaise/snemo/geometry/calo_locator.h>
#include <falaise/snemo/geometry/gg_locator.h>

// Ourselves
#include <snemo/digitization/ID_convertor.h>

namespace snemo {

  namespace digitization {

    ID_convertor::ID_convertor()
    {
      _logging_ = datatools::logger::PRIO_FATAL;
      _initialized_ = false;
      _module_number_ = mapping::INVALID_MODULE_NUMBER;
      _geo_manager_ = 0;
      _tracker_mapping_filename_ = "";
    }

    ID_convertor::ID_convertor(const geomtools::manager & mgr_,
			       int module_number_)
    {
      _logging_ = datatools::logger::PRIO_FATAL;
      _initialized_ = false;
      _module_number_ = mapping::INVALID_MODULE_NUMBER;
      _geo_manager_ = 0;
      set_module_number(module_number_);
      set_geo_manager(mgr_);
      _tracker_mapping_filename_ = "";
    }

    ID_convertor::~ID_convertor()
    {
      if(is_initialized())
	{
	  reset();
	}
    }

    bool ID_convertor::is_initialized() const
    {
      return _initialized_;
    }

    void ID_convertor::initialize(const datatools::properties & config_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Convertor is already initialized ! ");
      DT_THROW_IF(_module_number_ != mapping::DEMONSTRATOR_MODULE_NUMBER, std::logic_error, "Missing module number ! ");
      DT_THROW_IF(_geo_manager_ == 0, std::logic_error, "Missing geometry manager ! ");

      if (config_.has_key("feast_channel_mapping")) {
	std::string tracker_mapping_filename = config_.fetch_string("feast_channel_mapping");
	set_geiger_feb_mapping_file(tracker_mapping_filename);
      }

      DT_THROW_IF(_tracker_mapping_filename_.empty(), std::logic_error, "Missing tracker FEB internal mapping config file ! ");

      build_geiger_feb_mapping();

      _gg_locator_.reset(new geometry::gg_locator);
      _gg_locator_->set_geo_manager(*_geo_manager_);
      _gg_locator_->set_module_number(mapping::DEMONSTRATOR_MODULE_NUMBER);
      _gg_locator_->initialize();

      _calo_locator_.reset(new geometry::calo_locator);
      _calo_locator_->set_geo_manager(*_geo_manager_);
      _calo_locator_->set_module_number(mapping::DEMONSTRATOR_MODULE_NUMBER);
      _calo_locator_->initialize();

      _xcalo_locator_.reset(new geometry::xcalo_locator);
      _xcalo_locator_->set_geo_manager(*_geo_manager_);
      _xcalo_locator_->set_module_number(mapping::DEMONSTRATOR_MODULE_NUMBER);
      _xcalo_locator_->initialize();

      _gveto_locator_.reset(new geometry::gveto_locator);
      _gveto_locator_->set_geo_manager(*_geo_manager_);
      _gveto_locator_->set_module_number(mapping::DEMONSTRATOR_MODULE_NUMBER);
      _gveto_locator_->initialize();

      _initialized_ = true;
      return;
    }

    void ID_convertor::reset()
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Convertor is not initialized ! ");
      _initialized_ = false;
      _gg_locator_.reset();
      _calo_locator_.reset();
      _xcalo_locator_.reset();
      _gveto_locator_.reset();
      _logging_ = datatools::logger::PRIO_FATAL;
      _initialized_ = false;
      _module_number_ = mapping::INVALID_MODULE_NUMBER;
      _geo_manager_ = 0;
      _tracker_mapping_filename_ = "";
      return;
    }

    void ID_convertor::set_geiger_feb_mapping_file(const std::string & filename_)
    {
      _tracker_mapping_filename_ = filename_;
      datatools::fetch_path_with_env(_tracker_mapping_filename_);
      return;
    }

    void ID_convertor::set_logging(datatools::logger::priority prio_)
    {
      _logging_ = prio_;
      return;
    }

    datatools::logger::priority ID_convertor::get_logging() const
    {
      return _logging_;
    }

    void ID_convertor::set_geo_manager(const geomtools::manager & mgr_ )
    {
      _geo_manager_ = & mgr_;
    }

    void ID_convertor::set_module_number(int module_number_)
    {
      _module_number_ = module_number_;
      return;
    }

    void ID_convertor::build_geiger_feb_mapping()
    {
      if (!_tracker_mapping_filename_.empty())
	{
	  std::ifstream file_stream(_tracker_mapping_filename_.c_str(), std::ifstream::in);
	  if (file_stream) {
	    std::string line = "";
	    std::size_t line_counter = 0;
	    while (std::getline(file_stream, line)) {
	      // std::clog << "Line = " << line << std::endl;

	      if (line_counter != 0)  // ignore the header
		{
		  std::stringstream ss(line);
		  uint16_t module_number  = _module_number_;
		  uint16_t side_number    = -1;
		  uint16_t layer_number   = -1;
		  uint16_t row_number     = -1;
		  uint16_t part_number    = -1;
		  uint16_t feast_number   = -1;
		  uint16_t channel_number = -1;

		  ss >> side_number >> layer_number >> row_number >> part_number >> feast_number >> channel_number;

		  uint32_t gg_category_type = -1;
		  if (part_number == 2) gg_category_type = mapping::GEIGER_ANODIC_CATEGORY_TYPE;
		  else gg_category_type =  mapping::GEIGER_CATHODIC_CATEGORY_TYPE;


		  // Geometric ID
		  geomtools::geom_id gg_gid(gg_category_type,
					    module_number,
					    side_number,
					    layer_number,
					    row_number,
					    part_number);

		  // Electronic ID
		  geomtools::geom_id gg_eid(mapping::GEIGER_FEB_CATEGORY_TYPE,
					    feast_number,
					    channel_number);

		  _geiger_feb_mapping_.insert(std::pair<geomtools::geom_id, geomtools::geom_id>(gg_gid, gg_eid));
		}
	      line_counter++;
	    }
	  } // end of file stream

	}
      return;
    }

    std::map<geomtools::geom_id, geomtools::geom_id>  ID_convertor::get_geiger_feb_mapping() const
    {
      DT_THROW_IF(!is_initialized(), std::logic_error, "Convertor is not initialized ! ");
      return _geiger_feb_mapping_;
    }

    geomtools::geom_id ID_convertor::convert_GID_to_EID(const geomtools::geom_id & geom_id_) const
    {
      DT_THROW_IF(!geom_id_.is_valid (), std::logic_error, "Geom ID to convert is not valid !");

      //DT_THROW_IF(!geom_id_.is_complete(), std::logic_error,
      //   "Geom ID to convert is not complete !");

      geomtools::geom_id electronic_id;
      int module_id   = -1;
      int crate_id    = -1;
      int board_id    = -1;
      int channel_id  = -1;

      uint32_t GID_type = geom_id_.get_type();
      module_id = _module_number_;

      if (GID_type == mapping::GEIGER_ANODIC_CATEGORY_TYPE
	  || GID_type == mapping::GEIGER_CATHODIC_CATEGORY_TYPE)
	{
	  int feast_id    = -1;

	  // Drift cell --> Side [0;1] Layer [0;8] Row [0;112]
	  electronic_id.set_type(mapping::GEIGER_FEB_CATEGORY_TYPE);
	  unsigned int side_index  = _gg_locator_->extract_side(geom_id_);
	  unsigned int layer_index = _gg_locator_->extract_layer(geom_id_);
	  unsigned int row_index   = _gg_locator_->extract_row(geom_id_);
	  // Part [0;2] : 0 is bottom ring, 1 is top ring and 2 is anodic wire
	  int part_index = -1;
	  if (GID_type == mapping::GEIGER_ANODIC_CATEGORY_TYPE) part_index = mapping::GEIGER_CELL_ANODIC_PART; // Anodic
	  if (GID_type == mapping::GEIGER_CATHODIC_CATEGORY_TYPE) part_index = geom_id_.get(mapping::GEIGER_CELL_PART_INDEX); // Retrieve the part on cathodic rings

	  unsigned int shift = 0;
	  unsigned int row_shift = 0;

	  if (row_index <= mapping::BOARD_ID_SHIFT_CRATE_0_LIMIT)
	    {
	      crate_id = mapping::GEIGER_CRATE_0;
	    }

	  if (row_index > mapping::BOARD_ID_SHIFT_CRATE_0_LIMIT && row_index <= mapping::THREE_WIRES_CRATE_0_LIMIT)
	    {
	      crate_id  = mapping::GEIGER_CRATE_0;
	      row_shift = mapping::NO_FEB_NUMBER_10_SHIFT;
	    }

	  if (row_index > mapping::THREE_WIRES_CRATE_0_LIMIT && row_index  <= mapping::THREE_WIRES_LONELY_ROW )
	    {
	      crate_id = mapping::GEIGER_CRATE_1;
	      shift    = mapping::THREE_WIRES_CRATE_1_BEGINNING;
	    }

	  if (row_index > mapping::THREE_WIRES_LONELY_ROW && row_index  <= mapping::THREE_WIRES_CRATE_1_LIMIT )
	    {
	      crate_id  = mapping::GEIGER_CRATE_1;
	      shift     = mapping::THREE_WIRES_CRATE_1_BEGINNING;
	      shift    -= 1; // in order to take into account the lonely row at the middle
	      row_shift = mapping::NO_FEB_NUMBER_10_SHIFT;
	    }

	  if (row_index > mapping::THREE_WIRES_CRATE_1_LIMIT && row_index <= mapping::BOARD_ID_SHIFT_CRATE_2_LIMIT)
	    {
	      crate_id = mapping::GEIGER_CRATE_2;
	      shift    = mapping::THREE_WIRES_CRATE_2_BEGINNING;
	    }

	  if (row_index > mapping::BOARD_ID_SHIFT_CRATE_2_LIMIT)
	    {
	      crate_id  = mapping::GEIGER_CRATE_2;
	      shift     = mapping::THREE_WIRES_CRATE_2_BEGINNING;
	      row_shift = mapping::NO_FEB_NUMBER_10_SHIFT;
	    }

	  board_id = (row_index + row_shift - shift) / 2;

	  geomtools::geom_id generic_gid_with_part = geom_id_;
	  generic_gid_with_part.set_type(geom_id_.get_type());

	  // In geometry, cell GID is for anodic but it does not have the 'part' index
	  // Little 'hack' on this GID in order to add the ANODIC_PART at the last index.
	  if (geom_id_.get_type() == mapping::GEIGER_ANODIC_CATEGORY_TYPE
	      && geom_id_.get_depth() == mapping::GEIGER_ROW_DEPTH)
	    {
	      geomtools::geom_id hacked_gid;
	      hacked_gid.set_type(geom_id_.get_type());
	      hacked_gid.set_depth(mapping::GEIGER_CELL_PART_DEPTH);
	      hacked_gid.set_address(geom_id_.get(mapping::MODULE_INDEX),
				     geom_id_.get(mapping::SIDE_INDEX),
				     geom_id_.get(mapping::LAYER_INDEX),
				     geom_id_.get(mapping::ROW_INDEX));
	      hacked_gid.set(mapping::GEIGER_CELL_PART_INDEX,
			     mapping::GEIGER_CELL_ANODIC_PART);
	      generic_gid_with_part = hacked_gid;
	    }

	  if (geom_id_.get(mapping::ROW_INDEX) % 2 == 0) generic_gid_with_part.set(mapping::ROW_INDEX, 0);
	  if (geom_id_.get(mapping::ROW_INDEX) % 2 == 1) generic_gid_with_part.set(mapping::ROW_INDEX, 1);

	  geomtools::geom_id feast_channel_eid = _geiger_feb_mapping_.find(generic_gid_with_part)->second;
	  feast_id   = feast_channel_eid.get(0);
	  channel_id = feast_channel_eid.get(1);

	  electronic_id.set_address(module_id,
				    crate_id,
				    board_id,
				    feast_id,
				    channel_id);
	} // End of Geiger Category type

      if (GID_type == mapping::CALO_MAIN_WALL_CATEGORY_TYPE)
	{
	  // MCALO -- Side [0;1] Column [0;19] (Row[0;12] )type --> 1302
	  electronic_id.set_type(mapping::CALO_FEB_CATEGORY_TYPE);

	  unsigned int column_index_ = _calo_locator_-> extract_column(geom_id_);
	  unsigned int row_index_    = _calo_locator_-> extract_row(geom_id_);
	  unsigned int side_index_   = _calo_locator_-> extract_side(geom_id_);

	  if (side_index_ == 0)
	    {
	      crate_id = mapping::mapping::MAIN_CALO_SIDE_0_CRATE;
	    }
	  if (side_index_ == 1)
	    {
	      crate_id = mapping::MAIN_CALO_SIDE_1_CRATE;
	    }

	  int32_t shift_no_feb_10_index = 1;

	  board_id = column_index_;
	  // else if (column_index_ >= mapping::CONTROL_BOARD_INDEX) board_id = column_index_ + shift_no_feb_10_index;
	  channel_id = row_index_;

	  electronic_id.set_address(module_id,
				    crate_id,
				    board_id,
				    channel_id);

	} // End of Calo Main Wall Category type

      if (GID_type == mapping::CALO_XWALL_CATEGORY_TYPE)
	{
	  //XCALO  -- Side [0;1 ] Wall [0;1] Column [0;1] (Row[0;15])  type --> 1232
	  electronic_id.set_type(mapping::CALO_FEB_CATEGORY_TYPE);
	  crate_id = mapping::XWALL_GVETO_CALO_CRATE;

	  unsigned int column_index_ = _xcalo_locator_->extract_column(geom_id_);
	  unsigned int wall_index_   = _xcalo_locator_->extract_wall(geom_id_);
	  unsigned int side_index_   = _xcalo_locator_->extract_side(geom_id_);
	  unsigned int row_index_    = _xcalo_locator_->extract_row(geom_id_);

	  if ( side_index_ == 0)
	    {
	      if ( wall_index_ == 0)
		{
		  if ( column_index_ == 0) board_id = 6;
		  if ( column_index_ == 1) board_id = 7;
		}
	      if ( wall_index_ == 1)
		{
		  if ( column_index_ == 0) board_id = 8;
		  if ( column_index_ == 1) board_id = 9;
		}
	    } //end of side == 0
	  if ( side_index_ == 1)
	    {
	      if ( wall_index_ == 0)
		{
		  if ( column_index_ == 0) board_id = 14;
		  if ( column_index_ == 1) board_id = 13;
		}
	      if ( wall_index_ == 1)
		{
		  if ( column_index_ == 0) board_id = 12;
		  if ( column_index_ == 1) board_id = 11;
		}
	    } //end of side == 1
	  channel_id = row_index_;

	  electronic_id.set_address(module_id,
				    crate_id,
				    board_id,
				    channel_id);

	} // End of X-Wall Category type


      if (GID_type == mapping::CALO_GVETO_CATEGORY_TYPE)
	{
	  //GVETO -- Side [0;1 ] Wall [0;1] (Column [0;15])type --> 1252
	  electronic_id.set_type(mapping::CALO_FEB_CATEGORY_TYPE);
	  crate_id = mapping::XWALL_GVETO_CALO_CRATE;

	  unsigned int side_index_   = _gveto_locator_->extract_side(geom_id_);
	  unsigned int wall_index_   = _gveto_locator_->extract_wall(geom_id_);
	  unsigned int column_index_ = _gveto_locator_->extract_column(geom_id_);

	  if ( side_index_ == 0)
	    {
	      if ( wall_index_ == 0)board_id = 4;
	      if ( wall_index_ == 1)board_id = 5;
	    }
	  if ( side_index_ == 1)
	    {
	      if ( wall_index_ == 0)board_id = 16;
	      if ( wall_index_ == 1)board_id = 15;
	    }
	  channel_id = column_index_;

	  electronic_id.set_address(module_id,
				    crate_id,
				    board_id,
				    channel_id);

	} // End of G-Veto Category type

      return  electronic_id;
    }
    /*
      void ID_convertor::tree_dump(std::ostream & out_,
      const std::string & title_ ,
      const std::string & indent_  ,
      bool inherit_){
      }*/

  }// end of namespace digitization

} // end of namespace snemo
