//! \file    snemo/digitization/flasb.h
//! \brief  Utilities for intialization of the Digitization Falaise plugin at load
//
// Copyright (c) 2018 by  F. Mauger <mauger@lpccaen.in2p3.fr>
//
// This file is part of Digitization Falaise plugin.
//
// Digitization Falaise plugin is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Digitization Falaise plugin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Digitization Falaise plugin.  If not, see <http://www.gnu.org/licenses/>.

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_FLDIGI_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_FLDIGI_H

// Standard Library
#include <cstdint>

namespace snemo {

  namespace digitization {

    /// Initialize resources of the Digitization Falaise library
    void initialize(int argc_ = 0, char* argv_[] = 0, uint32_t flags_ = 0);

    /// Terminate resources of the Digitization Falaise library
    void terminate();

  }  // namespace digitization

}  // namespace snemo

#endif  // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_FLDIGI_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
