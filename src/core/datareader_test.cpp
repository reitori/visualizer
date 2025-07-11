#include <iostream>
#include <string>
#include <csignal>

#include "cli.h"

namespace
{
    auto logger = logging::make_log("MainLoop");
}

sig_atomic_t signaled = 0;
int main(int argc, char** argv) {

    int cli_status;
    VisualizerCli cli;

    cli_status = cli.init(argc, argv); if(cli_status <= 0) return cli_status;
    cli_status = cli.configure(); if(cli_status < 0) return cli_status;

    // Printout of FE statuses
    cli.listFEs();

    // Start CLI fes
    cli_status = cli.start(); if(cli_status < 0) return cli_status;

    signal(SIGINT, [](int signum){signaled = 1;});
    signal(SIGTERM, [](int signum){signaled = 1;});
    signal(SIGSTOP, [](int signum){signaled = 1;});

    // for(int i = 0; i < cli.getSize(); i++) {
    //     const json& config = cli.getConfig(i);
    //     std::vector<float> position = config["position"].get<std::vector<float>>();
    //     std::vector<float> angle = config["angle"].get<std::vector<float>>();
    //     logger->info("FE with ID {}: position {},{},{}", i, position[0], position[1], position[2]);
    // }

    // while we don't see CTRL-C or SIGUSR1 
    const json& allConf = cli.getMasterConfig();
    unsigned sleepTime = 100;
    if(allConf.contains("viz_config")) {
        if(allConf["viz_config"].contains("frame_time")) {
            sleepTime = allConf["viz_config"]["frame_time"];
        }
    }
    while(signaled == 0) {

        // MAIN EVENT LOOP HERE
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));

        // collect and count data
        long int size = 0;
        int nfe= 0;
        
        for(int i = 0; i < cli.getTotalFEs(); i++) {
            std::unique_ptr<std::vector<pixelHit>> data = cli.getData(i, true);
            if(data){
                size += data->size();
                data.reset();
                nfe++;
            }
        }
        if(size > 0)
            logger->debug("Total hit event size for this frame, with {} FEs: {}", nfe, size);
    }
    
    std::cout << "\r";
    logger->info("Caught interrupt, stopping threads");
    
    cli_status = cli.stop(); if(cli_status < 0) return cli_status;
    logger->info("End run");
    return 0;
}
