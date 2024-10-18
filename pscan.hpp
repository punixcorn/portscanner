#include <arpa/inet.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <algorithm>
#include <climits>
#include <cstring>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

using fmt::format;
using fmt::print;
using std::mutex;
using std::scoped_lock;
using std::sort;
using std::thread;
using std::vector;

#define PORT_MAX_T USHRT_MAX
#define str(x) #x

namespace port_scanner {
using port_t = unsigned short;
static vector<port_t> open_ports;
static mutex mtx;
static std::string ip = "127.0.0.1";
static char *server_reply = new char[200];
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
    print("port scanner by punixcorn v0.0.1\nopen ports:\n");
    if (open_ports.empty()) {
        print(fmt::fg(fmt::color::red), "No open ports found\n");
    }
    port_t current = 0;

    for (const port_t &i : open_ports) {
        if (current == i) {
            continue;
        } else {
            std::string_view x;

            if (services.find(i) != services.end())
                x = services.find(i)->second;
            else
                x = "unknown service";

            print(fmt::fg(fmt::color::green), "\t{}\t\t{:>5}\n", x, i);
            current = i;
        }
    }
    exit(0);
}

inline void scan(port_t start, port_t end) {
#ifdef TEST
    print(fmt::fg(fmt::color::yellow),
          "THREAD::BEGIN\t{} | start: {} end: {} chunksize: {}\n", start, start,
          end, end - start);
#endif
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
            continue;
        }

        // if connected check which service
        if (connect(sockfd, (struct sockaddr *)&soc, sizeof(soc)) == 0) {
            // lock muted
            std::scoped_lock<mutex> lock(mtx);
            open_ports.push_back(i);

            // Make an HTTP REQUEST TO SEE IF IT'S HTTP
            std::string request = format(
                "OPTIONS / HTTP/1.1\r\n"
                "Host: {}:{}\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n\r\n",
                ip.c_str(), i);
            send(sockfd, request.c_str(), request.size(), 0);

            // async io
            struct pollfd *pfds = new struct pollfd;
            pfds->fd = sockfd;
            pfds->events = POLLIN;
            int num_of_events = poll(pfds, 1, 100);

            // read data
            if ((num_of_events != 0) && (pfds->revents & POLLIN)) {
                if (recv(pfds->fd, server_reply, 199, 0) > 0) {
                    std::string temp{server_reply};
                    if (!services.contains(i)) {
                        // IF IT GETS BACK HTTP
                        if (temp.contains("HTTP")) {
                            services.insert({i, "HTTP"});
                        } else if (temp.contains("MPD")) {
                            services.insert({i, "MPD"});
                        } else {
                            // it's something else => remove \r\n if there's any
                            // form the request
                            std::erase(temp, '\n');
                            std::erase(temp, '\r');
                            services.insert({i, temp});
                        }
                    }
                }
            }
            std::memset(server_reply, 0, 199);
        }

        close(sockfd);
    }
#ifdef TEST
    print(fmt::fg(fmt::color::red), "THREAD::ENDED\t{} |\n", start);
#endif
}

inline void thread_handler(port_t start, port_t end) {
    port_t max_threads = thread::hardware_concurrency();
    std::jthread thread_list[max_threads];
    port_t interval_size = (end - start + 1) / max_threads;
    port_t thread_num;

    for (thread_num = 0; thread_num < max_threads; thread_num++) {
        port_t port_chunk = start + interval_size;
        if (start > port_chunk) {
            port_chunk = PORT_MAX_T;
        }
        thread_list[thread_num] = std::jthread{scan, start, port_chunk};
        start = port_chunk + 1;
    }

    for (port_t i = 0; i < max_threads; i++) {
        thread_list[i].join();
    }

    sort(open_ports.begin(), open_ports.end());
}

}  // namespace port_scanner
