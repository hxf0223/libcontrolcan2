#include "UDPServer.h"

int main() {
    try {
        boost::asio::io_service io_service;
        UDPServer server(io_service, "1111");
        io_service.run();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
    return 0;
}
