#include <arpa/inet.h>
#include <fmt/args.h>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ranges.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <execution>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using fmt::format;
using fmt::print;
using std::cin;
using std::cout;
using std::lock_guard;
using std::mutex;
using std::sort;
using std::thread;
using std::vector;

#define PORT_MAX_T USHRT_MAX

namespace port_scanner {
using port_t = unsigned short;
static vector<port_t> open_ports;
static mutex vec_mtx;
static std::string ip = "127.0.0.1";
static std::unordered_map<port_t, std::string> m_services{};
static std::unordered_map<port_t, std::string> services{
    {443, "HTTPS"}, {3306, "MYSQL"}, {25, "SMTP"}, {23, "TELNET"},
    {20, "FTP"},    {21, "FTP"},     {80, "HTTP"}, {9050, "TORSOCK"},
    {22, "SSH"},    {8080, "HTTP"}};

inline void help(void) {
    print(
        "{} : usage\n"
        "\t-s  Scan system ports\n"
        "\t-u  Scan user ports\n"
        "\t-p  Scan private ports\n"
        "\t-a  Scan all ports\n"
        "\t-h  Print this messsage\n",
        program_invocation_name);
}

inline void print_port() {
    print("open ports:\n");
    port_t current = 0;

    for (const port_t &i : open_ports) {
        if (current == i) {
            continue;
        } else {
            std::string x = (services.find(i) != services.end())
                                ? services.find(i)->second
                                : "unknown service";
            print("{}\t{}\n", x, i);
            current = i;
        }
    }
    exit(0);
}

inline void scan(port_t start, port_t end) {
    int sockfd;
    struct sockaddr_in soc;

    memset(&soc, 0, sizeof(soc));

    soc.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.c_str(), &soc.sin_addr) < 1) {
        print(stderr, "problem loading ip adddress\n");
        exit(1);
    }

    for (port_t i = start; i < end; i++) {
        soc.sin_port = htons(i);
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            print(stderr, "failed to create socket for on port {}\n", i);
            close(sockfd);
            i--;
            continue;
        }

        // if connected check which service
        if (connect(sockfd, (struct sockaddr *)&soc, sizeof(soc)) == 0) {
            lock_guard<mutex> guard(vec_mtx);
            open_ports.push_back(i);

            // char *server_reply = new char[200];
            // int reply = recv(sockfd, server_reply, 199, 0);
            // if (reply > 0) {
            //     if (m_services.find(i) == m_services.end())
            //         m_services.insert({i, server_reply});
            // }
        }

        close(sockfd);
    }
}

inline void thread_handler(port_t start, port_t end) {
    port_t max_threads = thread::hardware_concurrency();
    thread thread_list[max_threads];
    int interval_size = (end - start + 1) / max_threads;
    int thread_num;

    for (thread_num = 0; thread_num < max_threads; thread_num++) {
        int port_chunk = start + interval_size;
        thread_list[thread_num] = thread(scan, start, port_chunk);
        start = port_chunk + 1;
    }

    for (port_t i = 0; i < max_threads; i++) {
        thread_list[i].join();
    }
    sort(std::execution::par, open_ports.begin(), open_ports.end());
}

}  // namespace port_scanner
