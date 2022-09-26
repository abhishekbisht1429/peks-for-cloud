#include <iostream>
#include "tb_lib/http/http_server.cpp"
#include "tb_lib/net_socket/ipv4_address.h"
#include "tb_lib/util.h"
#include <vector>
#include <fstream>

http::response handle_data_owner(std::vector<std::string> path_components, tb_util::bytes body) {
    tb_util::output_bytes(std::cout, body);
    // store the bytes in a file
    std::ofstream store("temp/store");
    store.write(reinterpret_cast<const char *>(body.c_str()), body.length());

//    std::vector<tb_util::bytes> data = tb_util::deserialize_bytes_vec(body);
    return http::response(http::status::OK, "OK");
}

http::response handle_data_consumer(std::vector<std::string> path_components, tb_util::bytes body) {
    return http::response(http::status::OK, "OK");
}

http::response handler(http::request req, net_socket::sock_address addr) {
    std::cout << "Request received from " << addr.ip.to_string() << " " << addr.port << "\n";
    auto path_components = tb_util::tokenize(req.get_resource(), "/");
    std::cout<<"component 1 "<<path_components[1]<<"\n";
    if(path_components.size() < 2)
        return http::response(http::status::NOT_FOUND, "Resource not found");
    if(path_components[1] == "data_owner") {
        return handle_data_owner(path_components, req.get_body());
    } else if (path_components[1] == "data_consumer") {
        return handle_data_consumer(path_components, req.get_body());
    } else {
        return http::response(http::status::NOT_FOUND, "Resource not found");
    }
}

int main() {
    auto addr = net_socket::ipv4_address("0.0.0.0");
    uint16_t port = 8500;

    auto server = http::http_server(addr, port);
    server.serve(handler);
    return 0;
}

