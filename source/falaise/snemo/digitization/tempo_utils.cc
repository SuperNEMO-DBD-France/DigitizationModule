//tempo_utils.cc - Implementation of Falaise Digitization plugin utilities
//
// Copyright (c) 2018  Guillaume OLIVIERO <goliviero@lpccaen.in2p3.fr>
//                     F. Mauger <mauger@lpccaen.in2p3.fr>
//
// This file is part of Falaise/Digitization plugin.
//
// Falaise/Digitization plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Falaise/Digitization plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Falaise/Digitization plugin.  If not, see <http://www.gnu.org/licenses/>.

// Ourselves:
#include <snemo/digitization/tempo_utils.h>

// Standard library:
#include <sstream>
#include <stdexcept>

// Third party:
// - Boost:
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
// - Bayeux/datatools :
#include <bayeux/datatools/properties.h>
#include <bayeux/datatools/exception.h>
#include <bayeux/datatools/clhep_units.h>

namespace snemo {

  namespace digitization {

    void build_signal_name(const int id_,
                           std::string & name_)
    {
      name_ = "shape." + boost::lexical_cast<std::string>(id_);
      return;
    }

    void build_private_signal_name(const int id_,
                                   std::string & name_)
    {
      name_ = "__shape." + boost::lexical_cast<std::string>(id_);
      return;
    }

    bool is_private_signal_name(const std::string & name_)
    {
      return boost::starts_with(name_, "__shape.");
    }

    void build_shape(mctools::signal::signal_shape_builder & builder_,
                     const mctools::signal::base_signal & signal_)
    {
      std::string signal_key;
      build_signal_name(signal_.get_hit_id(), signal_key);
      if (signal_.has_private_shapes_config()) {
        const datatools::multi_properties & priv_mp = signal_.get_private_shapes_config();
        const datatools::multi_properties::entries_ordered_col_type & oentries
          = priv_mp.ordered_entries();
        for (const auto * entry_ptr : oentries) {
          std::string priv_signal_key = entry_ptr->get_key();
          const std::string & priv_signal_meta = entry_ptr->get_meta();
          const datatools::properties & priv_signal_params = entry_ptr->get_properties();
          builder_.create_signal_shape(priv_signal_key,
                                       priv_signal_meta,
                                       priv_signal_params);
        }
      }
      datatools::properties signal_shape_params;
      signal_.get_auxiliaries().export_and_rename_starting_with(signal_shape_params,
                                                                mctools::signal::base_signal::shape_parameter_prefix(),
                                                                "");
      builder_.create_signal_shape(signal_key,
                                   signal_.get_shape_type_id(),
                                   signal_shape_params);
      return;
    }

    void build_multi_signal(mctools::signal::base_signal & a_multi_signal_,
			    const std::vector<mctools::signal::base_signal> & list_of_atomic_signal_)
    {
      datatools::properties multi_signal_config;
      std::vector<std::string> component_labels;

      for (unsigned int i = 0; i < list_of_atomic_signal_.size(); i++)
	{
	  const mctools::signal::base_signal atomic_signal = list_of_atomic_signal_[i];
	  const int comp_sig_id    = atomic_signal.get_hit_id();
	  std::string comp_sig_key = "sig" + boost::lexical_cast<std::string>(comp_sig_id);
	  component_labels.push_back(comp_sig_key);
	  std::string comp_sig_prefix = "components." + comp_sig_key + ".";
	  std::string comp_sig_label;
	  build_private_signal_name(comp_sig_id, comp_sig_label);

	  datatools::properties comp_shape_parameters;
	  atomic_signal.get_auxiliaries().export_and_rename_starting_with(comp_shape_parameters,
									  mctools::signal::base_signal::shape_parameter_prefix(),
									  "");
	  a_multi_signal_.add_private_shape(comp_sig_label,
					   atomic_signal.get_shape_type_id(),
					   comp_shape_parameters);

	  // Private shape
	  std::string key_key        = comp_sig_prefix + "key";
	  std::string time_shift_key = comp_sig_prefix + "time_shift";
	  std::string scaling_key    = comp_sig_prefix + "scaling";
	  multi_signal_config.store(key_key, comp_sig_label);
	  multi_signal_config.store_real_with_explicit_unit(time_shift_key, 0.0 * CLHEP::ns);
	  multi_signal_config.set_unit_symbol(time_shift_key, "ns");
	  multi_signal_config.store_real(scaling_key, 1.0);

	}

      multi_signal_config.store("components", component_labels);
      a_multi_signal_.set_shape_parameters(multi_signal_config);
      a_multi_signal_.initialize(multi_signal_config);

      return;
    }


  } // end of namespace digitization

} // end of namespace snemo
