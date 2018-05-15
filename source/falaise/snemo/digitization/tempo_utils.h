// snemo/digitization/tempo_utils.h
// Author(s): G.Oliviéro <goliviero@lpccaen.in2p3.fr>
// Date: 2018-04-17

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_TEMPO_UTILS_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_TEMPO_UTILS_H

// Standard libraries :
#include <string>

// - Bayeux/mctools:
#include <mctools/signal/signal_shape_builder.h>
#include <mctools/signal/base_signal.h>

namespace snemo {

  namespace digitization {

    void build_signal_name(const int id_,
													 std::string & name_);

    void build_private_signal_name(const int id_, std::string & name_);

    bool is_private_signal_name(const std::string & name_);

    void build_shape(mctools::signal::signal_shape_builder & builder_,
										 const mctools::signal::base_signal & signal_);

    void build_multi_signal(mctools::signal::base_signal & a_multi_signal_,
														const std::vector<mctools::signal::base_signal> & list_of_atomic_signal_);

  } // end of namespace asb

} // end of namespace snemo

#endif // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_TEMPO_UTILS_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
