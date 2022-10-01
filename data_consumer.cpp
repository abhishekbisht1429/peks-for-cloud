//
// Created by abhishek on 23/9/22.
//
#include <pbc/pbc.h>
#include <iostream>
#include <fstream>
#include "util.h"
#include "tb_lib/http/http_client.h"

pairing_t pairing;

/* g */
element_t g;

/* secret key data consumer */
element_t skc;

/* public key data consumer */
element_t pkc;

/* public key of owner */
element_t pko;

/* public key of cloud server */
element_t pkcs;

void init_dc() {
    /* Load pairing */
    FILE *fp = std::fopen("../temp/param", "r");
    if(!fp) {
        std::cout<<"Failed to open param file";
        exit(1);
    }
    char param[1024];
    size_t count = fread(param, 1, 1024, fp);
    if(!count) {
        std::cout << "Failed to load";
        exit(1);
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
    element_printf("%B\n", g);

    // load or generate skc
    element_init_Zr(skc, pairing);
    std::ifstream ifs_skc("../temp/skc");
    if(!ifs_skc.is_open()) {
        element_random(skc);
        std::ofstream ofs_skc("../temp/skc", std::ios::binary);
        serialize_element_t(skc, ofs_skc);
    } else {
        deserialize_element_t(skc, ifs_skc);
    }

    // load or generate pkc
    std::ifstream ifs_pkc("../temp/pkc");
    element_init_G2(pkc, pairing);
    if(!ifs_pkc.is_open()) {
        element_pow_zn(pkc, g, skc);
        std::ofstream ofs_pkc("../temp/pkc", std::ios::binary);
        serialize_element_t(pkc, ofs_pkc);
    } else {
        deserialize_element_t(pkc, ifs_pkc);
    }

}

int main(int argc, char **argv) {
//    if (argc < 2) {
//        std::cout<<"required - <keyword>";
//        exit(1);
//    }
    //load param
    init_dc();

    // load pko - It must be created before this function is called
    std::ifstream ifs_pko("../temp/pko");
    if(!ifs_pko.is_open()) {
        std::cout<<"Required PKO - run data_owner";
        exit(0);
    }
    element_init_G2(pko, pairing);
    deserialize_element_t(pko, ifs_pko);

    // load pkcs
    std::ifstream ifs_pkcs("../temp/pkcs");
    element_init_G2(pkcs, pairing);
    deserialize_element_t(pkcs, ifs_pkcs);

    /* select r in Z_p* */
    element_t r;
    element_init_Zr(r, pairing);
    element_random(r);

    /* t1 calculation */
    element_t e1, prod;
    element_init_G2(e1, pairing);
    element_init_Zr(prod, pairing);
    element_mul(prod, skc, r);
    element_pow_zn(e1, pko, prod);

    element_t t1;
    element_init_G1(t1, pairing);
    h1(t1, element_to_string(e1));
    element_printf("t1: %B\n", t1);

    /* t2 calculation */
    element_t t2;
    element_init_G2(t2, pairing);
    element_pow_zn(t2, pkcs, prod);
    element_printf("t2: %B\n", t2);

    /* t3 calculation */
    element_t t3;
    element_init_G2(t3, pairing);
    element_pow_zn(t3, g, r);
    element_printf("t3: %B\n", t3);

    /* tw calculation */
//    std::string kw = argv[1];
    std::string kw = "Spheres,";
    element_t tw, h2_res;
    element_init_G1(tw, pairing);
    element_init_G1(h2_res, pairing);
    h2(h2_res, kw);
    element_mul(tw, h2_res, t1);
    element_printf("tw: %B\n", tw);

    /* send t = (t1, t2, t3, tw) to cloud server */
    std::vector<std::string> t;
    t.push_back(element_to_string(t1));
    t.push_back(element_to_string(t2));
    t.push_back(element_to_string(t3));
    t.push_back(element_to_string(tw));

    /* Load server server_ip and server_port */
    std::ifstream ifs("../temp/server_config");
    std::string server_ip;
    uint16_t server_port;
    ifs >> server_ip >> server_port;

    // create http client and connect to cloud server
    http::http_client client;
    auto server_addr = net_socket::ipv4_address(server_ip);
    client.connect(server_addr, server_port);

    // create and send request
    auto request = http::request(http::method::GET, "/data_consumer");
    auto body = tb_util::serialize_string_vec(t);
    request.set_body(body);
    auto resp = client.send_request(request);
    if(resp.get_status() == http::status::OK) {
        std::cout<<"Reply: "<<resp.get_body();
    } else {
        std::cout<<http::to_string(resp.get_status());
    }
}