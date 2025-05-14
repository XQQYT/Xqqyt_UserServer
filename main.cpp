#include <iostream>
#include "Server.h"
int main(int argc, char *argv[])
{

    Server server(8889);
    server.startListen();

    return 0;
}
