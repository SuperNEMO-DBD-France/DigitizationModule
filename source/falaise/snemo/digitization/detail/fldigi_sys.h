//! \file    snemo/digitization/detail/fldigi_sys.h
//! \brief   Provide Digitization Falaise plugin system singleton
//! \details
//
// Copyright (c) 2018 by Francois Mauger <mauger@lpccaen.in2p3.fr>
// Copyright (c) 2018 by Université de Caen Normandie
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

#ifndef FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DETAIL_FLDIGI_SYS_H
#define FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DETAIL_FLDIGI_SYS_H

// Standard Library
// #include <string>

// Third party:
#include <boost/core/noncopyable.hpp>

// This project:
#include <datatools/logger.h>
#include <datatools/service_manager.h>
#include <datatools/i_tree_dump.h>

namespace snemo {

  namespace digitization {

    namespace detail {

      //! \brief Digitization module system singleton
      class fldigi_sys
        : public datatools::i_tree_dumpable
        , private boost::noncopyable {

      public:

        /// Return the name of the Digitization Falaise plugin URN database service for supported setups (geometry,
        /// simulation...)
        static const std::string & fldigi_setup_db_name();

        /// Return the name of the Digitization Falaise plugin URN to resource path resolver service
        static const std::string & fldigi_resource_resolver_name();

        /// Extract the verbosity from the FLDIGI_SYS_LOGGING environment variable (if any)
        static datatools::logger::priority process_logging_env();

        /// Default constructor
        fldigi_sys();

        /// Destructor
        virtual ~fldigi_sys();

        /// Return the logging priority
        datatools::logger::priority get_logging() const;

        /// Set the logging priority
        void set_logging(const datatools::logger::priority);

        /// Check initialization flag
        bool is_initialized() const;

        /// Initialize
        void initialize();

        /// Shutdown
        void shutdown();

        /// Return a mutable reference to the embedded service manager
        datatools::service_manager & grab_services();

        /// Return a non mutable reference to the embedded service manager
        const datatools::service_manager & get_services() const;

        /// Check if the Digitization Falaise plugin system singleton is instantiated
        static bool is_instantiated();

        /// Return a mutable reference to the Digitization Falaise plugin system singleton instance
        static fldigi_sys & instance();

        /// Return a non-mutable reference to the Digitization Falaise plugin system singleton instance
        static const fldigi_sys & const_instance();

        /// Instantiate the Digitization Falaise plugin system singleton
        static fldigi_sys & instantiate();

        // Smart print
        void print_tree(std::ostream & out_ = std::clog,
                        const boost::property_tree::ptree & options_ = empty_options()) const;

      private:

        void _libinfo_registration_();

        void _libinfo_deregistration_();

        void _initialize_urn_services_();

        void _shutdown_urn_services_();

      private:

        // Management:
        bool _initialized_ = false;             //!< Initialization flag
        datatools::logger::priority _logging_;  //!< Logging priority threshold

        // Working internal data:
        datatools::service_manager _services_;  //!< Embedded services

        // Singleton:
        static fldigi_sys * _instance_;  //!< Digitization Falaise plugin system singleton handle

      };

    }  // end of namespace detail

  }  // end of namespace digitization

}  // namespace snemo

#endif  // FALAISE_DIGITIZATION_PLUGIN_SNEMO_DIGITIZATION_DETAIL_FLDIGI_SYS_H

// Local Variables: --
// mode: c++ --
// c-file-style: "gnu" --
// tab-width: 2 --
// End: --
