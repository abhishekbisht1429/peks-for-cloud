//
// Created by abhishek on 23/9/22.
//
#include <iostream>
#include <pbc/pbc.h>
#include <vector>
#include <fstream>
#include <openssl/sha.h>
#include "util.h"
#include "tb_lib/http/http_client.h"
#include "tb_lib/util.h"

pairing_t pairing;

/* g */
element_t g;

/* secret key data owner */
element_t sko;

/* public key data owner */
element_t pko;

/* public key of consumer */
element_t pkc;

void init_do() {
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
    element_printf("g: %B\n", g);

    // load or generate sko
    element_init_Zr(sko, pairing);
    std::ifstream ifs_sko("../temp/sko");
    if(!ifs_sko.is_open()) {
        element_random(sko);
        std::ofstream ofs_sko("../temp/sko", std::ios::binary);
        serialize_element_t(sko, ofs_sko);
    } else {
        deserialize_element_t(sko, ifs_sko);
    }
//    element_printf("sko: %B\n", sko);

    // load of generate pko
    std::ifstream ifs_pko("../temp/pko");
    element_init_G2(pko, pairing);
    if(!ifs_pko.is_open()) {
        element_pow_zn(pko, g, sko);
        std::ofstream ofs_pko("../temp/pko", std::ios::binary);
        serialize_element_t(pko, ofs_pko);
    } else {
        deserialize_element_t(pko, ifs_pko);
    }
    element_printf("pko: %B\n", pko);
}

std::vector<std::string> load_keywords(int n) {
    std::ifstream ifs("../temp/tesham_mutna_ruins.txt");
    std::vector<std::string> keywords;
    for(int i=0; i<n; ++i) {
        std::string keyword;
        ifs>>keyword;
        if(ifs.fail())
            break;
        keywords.push_back(keyword);
    }
    return keywords;
}

int main() {
    // Load param
    init_do();

    // load pkc - It must be created before this function is called
    std::ifstream ifs_pkc("../temp/pkc");
    if(! ifs_pkc.is_open()) {
        std::cout<<"Required PKC - run data consumer";
        exit(0);
    }
    element_init_G2(pkc, pairing);
    deserialize_element_t(pkc, ifs_pkc);

    /* select r in Z_p* */
    element_t r;
    element_init_Zr(r, pairing);
    element_random(r);
//    element_printf("r: %B\n", r);

    /* calculate c1 */
    element_t c1, prod;
    element_init_G2(c1, pairing);
    element_init_Zr(prod, pairing);
    element_mul(prod, sko, r);
    element_pow_zn(c1, pkc, prod);
    element_printf("c1: %B\n", c1);

    /* calculate c2 */
    element_t c2;
    element_init_G2(c2, pairing);
    element_pow_zn(c2, pko, r);
    element_printf("c2: %B\n", c2);

    //select private key
    std::cout<<"\n------------------Loading Keywords...-------------------\n";
    auto keywords = load_keywords(200);
    std::cout<<keywords.size()<<" keywords loaded"<<std::endl;
    std::vector<std::string> c;
    c.push_back(element_to_string(c1));
    c.push_back(element_to_string(c2));
    for(auto w : keywords) {
//        std::cout<<w<<"\n";
        element_t h2_res, pairing_res;
        element_init_G1(h2_res, pairing);
        element_init_GT(pairing_res, pairing);

        h2(h2_res, w);
//        element_printf("h2_res: %B\n", h2_res);
        element_pairing(pairing_res, h2_res, c1);
//        element_printf("pairing_res: %B\n", pairing_res);
        std::string cw = h3(pairing_res);
        c.push_back(cw);
    }

    /* Load server server_ip and server_port */
    std::ifstream ifs("../temp/server_config");
    std::string server_ip;
    uint16_t server_port;
    ifs >> server_ip >> server_port;

    /* Send c1, c2 and c to cloud server */
    std::cout<<"\n---------Sending c1, c2 and c to cloud server------------\n"<<std::endl;
    auto client = http::http_client();
    auto server_addr = net_socket::ipv4_address(server_ip);
    client.connect(server_addr, server_port);
    auto request = http::request(http::method::GET, "/data_owner");
    auto body = tb_util::serialize_string_vec(c);
    request.set_body(body);
    client.send_request(request);
}