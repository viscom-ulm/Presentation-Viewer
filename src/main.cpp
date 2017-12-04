/**
 * @file   main.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.24
 *
 * @brief  Program entry point.
 */

#include "core/main.h"
#include <fstream>
#include <iostream>
#include <g3log/logworker.hpp>
#include <g3log/loglevels.hpp>
#include "core/g3log/filesink.h"
#include "core/ApplicationNodeInternal.h"
#include "core/initialize.h"
inline bool exists_test3(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

int main(int argc, char** argv)
{
    const std::string directory = "./";
    const std::string name = "viscomlabfw.log";
    auto worker = g3::LogWorker::createLogWorker();
    auto handle = worker->addSink(std::make_unique<vku::FileSink>(name, directory, false), &vku::FileSink::fileWrite);

    g3::log_levels::disable(WARNING);

#ifndef NDEBUG
    g3::log_levels::enable(WARNING);
#endif

    initializeLogging(worker.get());

    LOG(INFO) << "Log created.";
    viscom::FWConfiguration config;
    if (argc > 1) config = viscom::LoadConfiguration(argv[1]);
    else config = viscom::LoadConfiguration("framework.cfg");
    auto appNode = Application_Init(config);

    // Main loop
    LOG(INFO) << "Started Rendering.";
    appNode->Render();

    return 0;
}
