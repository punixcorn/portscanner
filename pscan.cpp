#include "pscan.hpp"

namespace pt = ::port_scanner;

int main(int argc, char **argv) {
    if (argc <= 1) {
        pt::help();
        exit(0);
    }

    int c, start, end;
    char *ip_addr;

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
    print("arg for i is {}", ip_addr == NULL ? "" : "");

    pt::thread_handler(start, end);
    for (const auto &[key, pair] : pt::m_services) {
        print("{} : {}\n", key, pair);
    }

    pt::print_port();
}
