#include <iostream>
#include "tb_lib/http/http_server.cpp"
#include "tb_lib/net_socket/ipv4_address.h"
#include "tb_lib/util.h"
#include <vector>
#include <fstream>
#include <pbc/pbc.h>
#include "util.h"


pairing_t pairing;

/* g */
element_t g;

/* secret key of cloud server */
element_t skcs;

/* public key of cloud server */
element_t pkcs;

void init() {
    char param[1024];
    size_t count = 0;

    /* Load pairing */
    FILE *fp = std::fopen("../temp/param", "r");
    if(!fp) {
        std::cout<<"Failed to open param file";
        exit(1);
    } else {
        count = fread(param, 1, 1024, fp);
        if (!count) {
            std::cout << "Failed to load";
            exit(1);
        }
    }

    pairing_init_set_buf(pairing, param, count);

    /* initialize parameters */

    // load or generate g
    element_init_G2(g, pairing);
    std::ifstream  ifs_g("../temp/g");
    if(!ifs_g.is_open()) {
        element_random(g);
        std::ofstream ofs_g("../temp/g", std::ios::binary);
        serialize_element_t(g, ofs_g);
    } else {
        deserialize_element_t(g, ifs_g);
    }
    element_printf("g: %B\n", g);

    // load or generate skcs
    element_init_Zr(skcs, pairing);
    std::ifstream ifs_skcs("../temp/skcs");
    if(!ifs_skcs.is_open()) {
        element_random(skcs);
        std::ofstream ofs_skcs("../temp/skcs", std::ios::binary);
        serialize_element_t(skcs, ofs_skcs);
    } else {
        deserialize_element_t(skcs, ifs_skcs);
    }
//    element_printf("skcs: %B\n", skcs);

    // load or generate pkcs
    std::ifstream ifs_pkcs("../temp/pkcs");
    element_init_G2(pkcs, pairing);
    if(!ifs_pkcs.is_open()) {
        element_pow_zn(pkcs, g, skcs);
        std::ofstream ofs_pkcs("../temp/pkcs", std::ios::binary);
        serialize_element_t(pkcs, ofs_pkcs);
    } else {
        deserialize_element_t(pkcs, ifs_pkcs);
    }
    element_printf("pkcs: %B\n", pkcs);

}

bool test(element_t &t1,
          element_t &t2,
          element_t &t3,
          element_t &tw,
          element_t &c1,
          element_t &c2,
          std::string &cw) {

    /* compute phi */
    element_t phi, num, denom;
    element_init_GT(phi, pairing);
    element_init_GT(num, pairing);
    element_init_GT(denom, pairing);
    element_pairing(num, tw, c1);
//    element_printf("num: %B\n", num);
    element_pairing(denom, t1, c1);
//    element_printf("denom: %B\n", denom);
    element_div(phi, num, denom);
//    element_printf("phi: %B\n", phi);

    /* compute LHS */
    element_t lhs, h3_res_t, pairing_res_lhs, exp_res_lhs;
    element_init_Zr(h3_res_t, pairing);
    element_init_GT(pairing_res_lhs, pairing);
    element_init_GT(exp_res_lhs, pairing);
    element_init_GT(lhs, pairing);

    std::string h3_res = h3(phi);
    element_from_hash(h3_res_t, (void *) h3_res.c_str(), h3_res.length());
//    element_printf("h3_res_t: %B\n", h3_res_t);

    element_pairing(pairing_res_lhs,t3, c1);
//    element_printf("pairing_res_lhs: %B\n", pairing_res_lhs);
    element_pow_zn(exp_res_lhs, pairing_res_lhs, skcs);
//    element_printf("exp_res_lhs: %B\n", exp_res_lhs);


    element_mul_zn(lhs, exp_res_lhs, h3_res_t);
//    element_printf("lhs: %B\n", lhs);

    /* compute RHS */
    element_t rhs, cw_t, pairing_res_rhs;
    element_init_GT(rhs, pairing);
    element_init_Zr(cw_t, pairing);
    element_init_GT(pairing_res_rhs, pairing);

    element_pairing(pairing_res_rhs, c2, t2);
//    element_printf("pairing_res_rhs: %B\n", pairing_res_rhs);

    element_from_hash(cw_t, (void *) cw.c_str(), cw.length());
//    element_printf("h3_res_t: %B\n", h3_res_t);

    element_mul_zn(rhs, pairing_res_rhs, cw_t);

    /* check if lhs = rhs */
    return element_cmp(lhs, rhs) == 0;
}

http::response handle_data_owner(std::vector<std::string> path_components, std::string body) {
    /* store the bytes in a file */
    std::cout<<"------Storing c in local database----\n";
    std::ofstream store("../temp/store");
    uint32_t len = body.length();
    store.write((char*)&len, 4);
    store.write(body.c_str(), body.length());

//    std::vector<tb_util::bytes> data = tb_util::deserialize_string_vec(body);
    return http::response(http::status::OK, "OK");
}

http::response handle_data_consumer(std::vector<std::string> path_components, std::string body) {
    /* deserialize vector received from data consumer */
    std::vector<std::string> data_vec = tb_util::deserialize_string_vec(body);

    /* deserialize data stored in store */
    std::ifstream ifs("../temp/store");
    if(!ifs.is_open()) {
        return  http::response(http::status::NOT_FOUND, "Data owner has not uploaded data");
    }
    uint32_t len;
    ifs.read((char*)&len, 4);
    char buf[len];
    ifs.read(buf, len);
    std::string stored_data(buf, len);
    std::vector<std::string> stored_data_vec = tb_util::deserialize_string_vec(stored_data);

    element_t t1, t2, t3, tw, c1, c2;
    element_init_G1(t1, pairing);
    element_init_G2(t2, pairing);
    element_init_G2(t3, pairing);
    element_init_G1(tw, pairing);
    element_init_G2(c1, pairing);
    element_init_G2(c2, pairing);
    string_to_element(t1, data_vec[0]);
    string_to_element(t2, data_vec[1]);
    string_to_element(t3, data_vec[2]);
    string_to_element(tw, data_vec[3]);
    string_to_element(c1, stored_data_vec[0]);
    string_to_element(c2, stored_data_vec[1]);
//    element_printf("t1: %B\n", t1);
//    element_printf("t2: %B\n", t2);
//    element_printf("t3: %B\n", t3);
//    element_printf("tw: %B\n", tw);
//    element_printf("c1: %B\n", c1);
//    element_printf("c2: %B\n", c2);

    /* test */
    std::string res = "NO";
    for(int i=2; i<stored_data_vec.size(); ++i) {
        std::string cw = stored_data_vec[i];
        if(test(t1, t2, t3, tw, c1, c2, cw)) {
//            std::cout<<cw<<" "<<"YES"<<std::endl;
            res = "YES";
            break;
        }
    }
    http::response resp(http::status::OK, "OK");
    resp.set_body(res);
    return resp;
}

http::response handler(http::request req, net_socket::sock_address addr) {
    std::cout << "\nRequest received from " << addr.ip.to_string() << " " << addr.port << "\n";
    auto path_components = tb_util::tokenize(req.get_resource(), "/");
//    std::cout<<"component 1 "<<path_components[1]<<"\n";
    if(path_components.size() < 2)
        return {http::status::NOT_FOUND, "Resource not found"};
    if(path_components[1] == "data_owner") {
        return handle_data_owner(path_components, req.get_body());
    } else if (path_components[1] == "data_consumer") {
        return handle_data_consumer(path_components, req.get_body());
    } else {
        return {http::status::NOT_FOUND, "Resource not found"};
    }
}


int main() {
    /* initialize params */
    init();

    /* load server ip and port */
    std::ifstream ifs("../temp/server_config");
    std::string ip;
    uint16_t port;
    ifs>>ip>>port;
    std::cout<<"Server IP: "<<ip<<std::endl;
    std::cout<<"Server Port: "<<port<<std::endl;

    auto addr = net_socket::ipv4_address(ip);

    auto server = http::http_server(addr, port);
    server.serve(handler);
    return 0;
}

