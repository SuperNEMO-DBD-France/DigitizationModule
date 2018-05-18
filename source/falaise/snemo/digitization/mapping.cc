// snemo/digitization/mapping.cc
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

// Standard library :
#include <string>

// Boost :
#include <boost/scoped_ptr.hpp>

// Ourselves:
#include <falaise/snemo/digitization/mapping.h>

namespace snemo {

  namespace digitization {

    const bool     mapping::THREE_WIRES_TRACKER_MODE;
    const bool     mapping::TWO_WIRES_TRACKER_MODE;
    const bool     mapping::SIDE_MODE;

    const uint32_t mapping::GEIGER_ANODIC_CATEGORY_TYPE;
    const uint32_t mapping::GEIGER_CATHODIC_CATEGORY_TYPE;
    const uint32_t mapping::CALO_MAIN_WALL_CATEGORY_TYPE;
    const uint32_t mapping::CALO_XWALL_CATEGORY_TYPE;
    const uint32_t mapping::CALO_GVETO_CATEGORY_TYPE;


    const uint32_t mapping::CALO_CRATE_CATEGORY_TYPE;
    const uint32_t mapping::GEIGER_CRATE_CATEGORY_TYPE;
    const uint32_t mapping::CALO_FEB_CATEGORY_TYPE;
    const uint32_t mapping::GEIGER_FEB_CATEGORY_TYPE;
    const uint32_t mapping::CALO_HV_CATEGORY_TYPE;
    const uint32_t mapping::GEIGER_HV_CATEGORY_TYPE;
    const uint32_t mapping::CALO_CONTROL_BOARD_TYPE;
    const uint32_t mapping::GEIGER_CONTROL_BOARD_TYPE;
    const uint32_t mapping::TRIGGER_BOARD_TYPE;

    const int32_t  mapping::INVALID_MODULE_NUMBER;
    const uint32_t mapping::DEMONSTRATOR_MODULE_NUMBER;

    const uint32_t mapping::NUMBER_OF_SIDES;
    const uint32_t mapping::NUMBER_OF_WALLS;

    const uint32_t mapping::NUMBER_OF_MAIN_CALO_ROWS;
    const uint32_t mapping::NUMBER_OF_MAIN_CALO_COLUMNS;
    const uint32_t mapping::NUMBER_OF_XWALL_ROWS;
    const uint32_t mapping::NUMBER_OF_XWALL_COLUMNS;
    const uint32_t mapping::NUMBER_OF_XWALL_TRIGGER_ZONES;
    const uint32_t mapping::NUMBER_OF_GVETO_COLUMNS;

    const uint32_t mapping::NUMBER_OF_GEIGER_ROWS;
    const uint32_t mapping::NUMBER_OF_GEIGER_LAYERS;
    const uint32_t mapping::NUMBER_OF_GEIGER_CELL_PARTS;

    const uint32_t mapping::GEIGER_CELL_BOTTOM_CATHODE_PART;
    const uint32_t mapping::GEIGER_CELL_TOP_CATHODE_PART;
    const uint32_t mapping::GEIGER_CELL_ANODIC_PART;
    const uint32_t mapping::NUMBER_OF_GEIGER_FEAST_PER_FEB;
    const uint32_t mapping::NUMBER_OF_CONNECTED_ROWS;

    const uint32_t mapping::NUMBER_OF_TRACKER_TRIGGER_INTERZONES;
    const uint32_t mapping::NUMBER_OF_TRACKER_TRIGGER_SUBZONES;
    const uint32_t mapping::NUMBER_OF_TRACKER_TRIGGER_SUBZONES_PER_SIDE;
    const uint32_t mapping::NUMBER_OF_TRIGGER_ZONES;

    const uint32_t mapping::NUMBER_OF_CRATES_PER_TYPE;
    const uint32_t mapping::NUMBER_OF_GEIGER_FEBS_PER_CRATE;
    const uint32_t mapping::NUMBER_OF_CALO_FEBS_PER_CRATE;

    const uint32_t mapping::TRIGGER_CRATE_INDEX;
    const uint32_t mapping::CONTROL_BOARD_INDEX;
    const uint32_t mapping::TRIGGER_BOARD_INDEX;

    const uint32_t mapping::BOARD_ID_SHIFT_CRATE_0_LIMIT;
    const uint32_t mapping::BOARD_ID_SHIFT_CRATE_2_LIMIT;
    const uint32_t mapping::NO_FEB_NUMBER_10_SHIFT;
    const uint32_t mapping::THREE_WIRES_CRATE_0_LIMIT;
    const uint32_t mapping::THREE_WIRES_CRATE_1_BEGINNING;
    const uint32_t mapping::THREE_WIRES_LONELY_ROW;
    const uint32_t mapping::THREE_WIRES_CRATE_1_LIMIT;
    const uint32_t mapping::THREE_WIRES_CRATE_2_BEGINNING;
    const uint32_t mapping::THREE_WIRES_CRATE_2_LIMIT;

    const std::string & mapping::geiger_type()
    {
      static boost::scoped_ptr<std::string> _label;
      if (!_label) {
	_label.reset(new std::string("geiger"));
      }
      return *_label;
    }
    const std::string & mapping::main_calo_type()
    {
      static boost::scoped_ptr<std::string> _label;
      if (!_label) {
	_label.reset(new std::string("calo"));
      }
      return *_label;
    }
    const std::string & mapping::x_wall_type()
    {
      static boost::scoped_ptr<std::string> _label;
      if (!_label) {
	_label.reset(new std::string("xwall"));
      }
      return *_label;
    }
    const std::string & mapping::gveto_type()
    {
    static boost::scoped_ptr<std::string> _label;
      if (!_label) {
	_label.reset(new std::string("gveto"));
      }
      return *_label;
    }

  } // end of namespace digitization

} // end of namespace snemo
