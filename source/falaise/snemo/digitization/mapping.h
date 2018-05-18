// snemo/digitization/mapping.h
// Author(s): Yves LEMIERE <lemiere@lpccaen.in2p3.fr>
// Author(s): Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_MAPPING_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_MAPPING_H

// Boost :
#include <boost/cstdint.hpp>

namespace snemo {

  namespace digitization {

    class mapping
    {
    public :
			// Geometric ID index:
			enum geom_ID_index {
				MODULE_INDEX     = 0,
				SIDE_INDEX       = 1,
				LAYER_INDEX      = 2,
				COLUMN_INDEX     = 2,
				WALL_INDEX       = 2,
				ROW_INDEX        = 3,
				XWALL_ROW_INDEX  = 4,
				GEIGER_CELL_PART_INDEX = 4
			};

      /// Electronic id index coming from a geom id
      enum electronic_ID_index {
				// MODULE_INDEX   = 0, already declared
				CRATE_INDEX    = 1,
				BOARD_INDEX    = 2,
				CALO_CHANNEL_INDEX   = 3,
				GEIGER_FEAST_INDEX   = 3,
				GEIGER_CHANNEL_INDEX = 4
      };


			enum electronic_ID_depth {
				MODULE_DEPTH   = 1,
				CRATE_DEPTH    = 2,
				BOARD_DEPTH    = 3,
				CALO_CHANNEL_DEPTH   = 4,
				GEIGER_FEAST_DEPTH   = 4,
				GEIGER_CHANNEL_DEPTH = 5
			};

			enum calo_crate {
				MAIN_CALO_SIDE_0_CRATE = 0,
				MAIN_CALO_SIDE_1_CRATE = 1,
				XWALL_GVETO_CALO_CRATE = 2
			};

			enum geiger_crate {
				GEIGER_CRATE_0 = 0,
				GEIGER_CRATE_1 = 1,
				GEIGER_CRATE_2 = 2
			};

			enum tp_tracker_wire_mode {
				// INVALID_WIRES_TRACKER_MODE = -1,
				// THREE_WIRES_TRACKER_MODE   = 0,
				// TWO_WIRES_TRACKER_MODE     = 1
			};

			enum tp_tracker_side_mode {
				INVALID_TRACKER_SIDE_MODE = -1,
				TRACKER_ONE_SIDE_MODE  = 0,
				TRACKER_TWO_SIDES_MODE = 1
			};

			static const bool THREE_WIRES_TRACKER_MODE = 0;
			static const bool TWO_WIRES_TRACKER_MODE   = 1;
			static const bool SIDE_MODE = 1;

			static const std::string  & geiger_type();
			static const std::string  & main_calo_type();
			static const std::string  & x_wall_type();
			static const std::string  & gveto_type();

			static const uint32_t GEIGER_ANODIC_CATEGORY_TYPE   = 1210;
			static const uint32_t GEIGER_CATHODIC_CATEGORY_TYPE = 1211;
			static const uint32_t CALO_MAIN_WALL_CATEGORY_TYPE = 1302;
			static const uint32_t CALO_XWALL_CATEGORY_TYPE  = 1232;
			static const uint32_t CALO_GVETO_CATEGORY_TYPE  = 1252;

			static const uint32_t CALO_CRATE_CATEGORY_TYPE   = 300;
			static const uint32_t GEIGER_CRATE_CATEGORY_TYPE = 301;
			static const uint32_t CALO_FEB_CATEGORY_TYPE     = 400;
			static const uint32_t GEIGER_FEB_CATEGORY_TYPE   = 401;
			static const uint32_t CALO_HV_CATEGORY_TYPE      = 500;
			static const uint32_t GEIGER_HV_CATEGORY_TYPE    = 501;
			static const uint32_t CALO_CONTROL_BOARD_TYPE    = 600;
			static const uint32_t GEIGER_CONTROL_BOARD_TYPE  = 601;
			static const uint32_t TRIGGER_BOARD_TYPE         = 700;

			static const int32_t  INVALID_MODULE_NUMBER = -1;
			static const uint32_t DEMONSTRATOR_MODULE_NUMBER = 0;

			static const uint32_t NUMBER_OF_SIDES = 2;
			static const uint32_t NUMBER_OF_WALLS = 2;

			static const uint32_t NUMBER_OF_MAIN_CALO_ROWS = 13;
			static const uint32_t NUMBER_OF_MAIN_CALO_COLUMNS = 20;
			static const uint32_t NUMBER_OF_XWALL_ROWS = 16;
			static const uint32_t NUMBER_OF_XWALL_COLUMNS = 2;
			static const uint32_t NUMBER_OF_XWALL_TRIGGER_ZONES = 4;
			static const uint32_t NUMBER_OF_GVETO_COLUMNS = 16;

			static const uint32_t NUMBER_OF_GEIGER_ROWS = 113;
			static const uint32_t NUMBER_OF_GEIGER_LAYERS = 9;
			static const uint32_t NUMBER_OF_GEIGER_CELL_PARTS = 3;

			static const uint32_t GEIGER_CELL_BOTTOM_CATHODE_PART = 0;
			static const uint32_t GEIGER_CELL_TOP_CATHODE_PART = 1;
			static const uint32_t GEIGER_CELL_ANODIC_PART = 2;
			static const uint32_t NUMBER_OF_GEIGER_FEAST_PER_FEB = 2;
			static const uint32_t NUMBER_OF_CONNECTED_ROWS = 7;

			static const uint32_t NUMBER_OF_TRACKER_TRIGGER_INTERZONES = 9;
			static const uint32_t NUMBER_OF_TRACKER_TRIGGER_SUBZONES = 4;
			static const uint32_t NUMBER_OF_TRACKER_TRIGGER_SUBZONES_PER_SIDE = 40;
			static const uint32_t NUMBER_OF_TRIGGER_ZONES = 10;

			static const uint32_t NUMBER_OF_CRATES_PER_TYPE  = 3;
			static const uint32_t NUMBER_OF_GEIGER_FEBS_PER_CRATE = 19;
			static const uint32_t NUMBER_OF_CALO_FEBS_PER_CRATE = 20;

			static const uint32_t TRIGGER_CRATE_INDEX = 2;
			static const uint32_t CONTROL_BOARD_INDEX = 10;
			static const uint32_t TRIGGER_BOARD_INDEX = 20;

			/// Row limit to take account, there is no FEB ID 10 for every crate
			static const uint32_t BOARD_ID_SHIFT_CRATE_0_LIMIT = 19;
			static const uint32_t BOARD_ID_SHIFT_CRATE_2_LIMIT = 94;
			/// Shift to take account, there is no FEB ID 10 (place of the Control Board)
			static const uint32_t NO_FEB_NUMBER_10_SHIFT = 2;

			static const uint32_t THREE_WIRES_CRATE_0_LIMIT     = 37;
			static const uint32_t THREE_WIRES_CRATE_1_BEGINNING = 38;
			static const uint32_t THREE_WIRES_LONELY_ROW        = 56;
			static const uint32_t THREE_WIRES_CRATE_1_LIMIT     = 74;
			static const uint32_t THREE_WIRES_CRATE_2_BEGINNING = 75;
			static const uint32_t THREE_WIRES_CRATE_2_LIMIT     = 112;

    };

  } // end of namespace digitization

} // end of namespace snemo

#endif /* FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_MAPPING_H */

/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
