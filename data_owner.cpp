//
// Created by abhishek on 23/9/22.
//
#include <iostream>
#include <pbc/pbc.h>
#include <vector>
#include <fstream>
#include<openssl/sha.h>
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

template<typename charT>
void serialize_element_t(element_t &e, std::basic_ostream<charT> &of) {
    uint32_t len = element_length_in_bytes(e);
    unsigned char buf[len];
    element_to_bytes(buf, e);
    of.write((charT*)&len, 4);
    of.write((charT*)buf, len);
}

template<typename charT>
void deserialize_element_t(element_t &e, std::basic_ifstream<charT> &is) {
    uint32_t len;
    is.read((charT*)&len, 4);
    unsigned char buf[1024];
    is.read((charT*)buf, len);
    element_from_bytes(e, buf);
}

void init() {
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
        serialize_element_t<char>(g, ofs_g);
    } else {
        deserialize_element_t<char>(g, ifs_g);
    }

    // load or generate sko
    element_init_Zr(sko, pairing);
    std::ifstream ifs_sko("../temp/sko");
    if(!ifs_sko.is_open()) {
        element_random(sko);
        std::ofstream ofs_sko("../temp/sko", std::ios::binary);
        serialize_element_t<char>(sko, ofs_sko);
    } else {
        deserialize_element_t<char>(sko, ifs_sko);
    }

    // load of generate pko
    std::ifstream ifs_pko("../temp/pko");
    element_init_G2(pko, pairing);
    if(!ifs_sko.is_open()) {
        element_pow_zn(pko, g, sko);
        std::ofstream ofs_pko("../temp/pko", std::ios::binary);
        serialize_element_t<char>(pko, ofs_pko);
    } else {
        deserialize_element_t<char>(pko, ifs_pko);
    }
}

std::vector<std::string> load_keywords(int n) {
    std::ifstream ifs("../temp/keywords.txt");
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
    init();

    // load pkc - It must be created before this function is called
    std::ifstream ifs_pkc("../temp/pko");
    element_init_G2(pkc, pairing);
    deserialize_element_t<char>(pkc, ifs_pkc);

    // select r in Z_p*
    element_t r;
    element_init_Zr(r, pairing);

    // calculate c1
    element_t c1, prod;
    element_init_G2(c1, pairing);
    element_init_Zr(prod, pairing);
    element_mul(prod, sko, r);
    element_pow_zn(c1, pkc, prod);

    // calculate c2
    element_t c2;
    element_init_G2(c2, pairing);
    element_pow_zn(c2, pko, r);

    //select private key
    auto keywords = load_keywords(5);
    std::vector<tb_util::bytes> c;
    for(auto w : keywords) {
        std::cout<<w<<"\n";
        element_t h2_res, pairing_res;
        element_init_G1(h2_res, pairing);
        element_init_GT(pairing_res, pairing);

        h2(h2_res, tb_util::s2b(w));

//        std::cout<<"h2_res: ";
//        serialize_element_t(h2_res, std::cout);
//        std::cout<<"\n";
        element_pairing(pairing_res, h2_res, c1);
//        std::cout<<"pairing_res: ";
//        serialize_element_t(pairing_res, std::cout);
//        std::cout<<"\n";
        tb_util::bytes cw = h3(pairing_res);
        tb_util::output_bytes(std::cout, cw);
        c.push_back(cw);
    }

    /* Send c1, c2 and c to cloud server */
    tb_util::bytes c1_bytes, c2_bytes;
    bytes_ostream bos1(c1_bytes), bos2(c2_bytes);
    serialize_element_t<unsigned char>(c1, bos1);
    serialize_element_t<unsigned char>(c2, bos2);

    c.push_back(c1_bytes);
    c.push_back(c2_bytes);

    auto client = http::http_client();
    auto server_addr = net_socket::ipv4_address("127.0.0.1");
    auto server_port = 8500;
    client.connect(server_addr, server_port);
    auto request = http::request(http::method::GET, "/data_owner");
    auto body = tb_util::serialize_bytes_vec(c);
    tb_util::output_bytes(std::cout, body);
    request.set_body(body);
    client.send_request(request);
}