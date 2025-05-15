/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include <iostream>
#include <csignal>
#include "Server.h"

Server *server;
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    delete server;
    signal(SIGINT, SIG_IGN);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);
    server = new Server(8889);
    server->startListen();

    return 0;
}
