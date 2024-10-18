#include "pscan.hpp"

#include <arpa/inet.h>

namespace pt = ::port_scanner;

int main(int argc, char **argv) {
    if (argc <= 1) {
        pt::help();
        exit(0);
    }

    int c, start, end;
    static char *ip_addr = NULL;

    while ((c = getopt(argc, argv, "i:supah")) != -1) {
        switch (c) {
            case 's':
                start = 0;
                end = 1023;
                break;
            case 'u':
                start = 1024;
                end = 49151;
                break;
            case 'p':
                start = 49151;
                end = PORT_MAX_T;
                break;
            case 'a':
                start = 0;
                end = PORT_MAX_T;
                break;
            case 'h':
                pt::help();
                break;
            case 'i':
                ip_addr = optarg;
                break;
            default:
                break;
        }
    }

    if (ip_addr != NULL) {
        if (inet_aton(ip_addr, NULL) == 0) {
            print("invalid ip address passed\n");
            exit(1);
        }
        pt::ip = ip_addr;
    }

    // 2 x for a good charm
    pt::thread_handler(start, end);
    pt::thread_handler(start, end);

    pt::print_port();
}
