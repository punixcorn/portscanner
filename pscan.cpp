#include "pscan.hpp"
namespace pt = ::port_scanner;

int main(int argc, char **argv) {
    if (argc <= 1) {
        pt::help();
        exit(0);
    }

    int c, start, end;

    while ((c = getopt(argc, argv, "supah")) != 1) {
        switch (c) {
            case 's':
                start = 0;
                end = 1023;
                run(pt::thread_handler(start, end));
                break;
            case 'u':
                start = 1024;
                end = 49151;
                run(pt::thread_handler(start, end));
                break;
            case 'p':
                start = 49151;
                end = PORT_MAX_T;
                run(pt::thread_handler(start, end));
                break;
            case 'a':
                start = 0;
                end = PORT_MAX_T;
                run(pt::thread_handler(start, end));
                break;
            case 'h':
                pt::help();
                break;
            default:
                break;
        }

        // for(int i = 0; i < 5; i++){
        //   p::thread_handler(start,end);
        // }
    }
}
