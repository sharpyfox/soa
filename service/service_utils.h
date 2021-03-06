/** service_utils.h                                 -*- C++ -*-
    Rémi Attab, 13 Mar 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

    Service utilities

*/

#pragma once

#include "service_base.h"

#include <boost/program_options/options_description.hpp>
#include <vector>
#include <string>
#include <sys/utsname.h>


namespace Datacratic {

/******************************************************************************/
/* SERVICE PROXIES ARGUMENTS                                                  */
/******************************************************************************/

/** Turns command line arguments into a ServiceProxy object */
struct ServiceProxyArguments
{
    boost::program_options::options_description
    makeProgramOptions(const std::string& title = "General Options")
    {
        using namespace boost::program_options;

        options_description options(title);
        options.add_options()
            ("bootstrap,B", value(&bootstrapPath), "path to bootstrap.json")

            ("zookeeper-uri,Z", value(&zookeeperUri), "URI of zookeeper to use")
            ("carbon-connection,c", value<std::vector<std::string> >(&carbonUris),
             "URI of connection to carbon daemon")

            ("installation,I", value(&installation),
             "Name of the installation that is running")
            ("node-name,N", value(&nodeName), "Name of the node we're running");

        return options;
    }

    std::shared_ptr<ServiceProxies> makeServiceProxies()
    {
        auto services = std::make_shared<ServiceProxies>();

        if (!bootstrapPath.empty())
            services->bootstrap(bootstrapPath);

        if (!zookeeperUri.empty()) {
            ExcCheck(!installation.empty(), "installation is required");
            services->useZookeeper(zookeeperUri, installation);
        }

        if (!carbonUris.empty()) {
            ExcCheck(!installation.empty(), "installation is required");

            if (nodeName.empty()) {
                struct utsname s;
                int ret = uname(&s);
                ExcCheckErrno(!ret, "Unable to call uname");

                nodeName = std::string(s.nodename);
            }

            services->logToCarbon(carbonUris, installation + "." + nodeName);
        }

        return services;
    }

    std::string bootstrapPath;

    std::string zookeeperUri;
    std::vector<std::string> carbonUris;

    std::string installation;
    std::string nodeName;
};

} // namespace Datacratic
