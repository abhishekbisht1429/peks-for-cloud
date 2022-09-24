//
// Created by abhishek on 23/9/22.
//
#include <iostream>
#include <pbc/pbc.h>
#include <vector>
#include <fstream>
#include<openssl/sha.h>
#include "util.cpp"

pairing_t pairing;

/* g */
element_t g;

/* secret key data owner */
element_t sko;

/* public key data owner */
element_t pko;

/* public key of consumer */
element_t pkc;

void serialize_element_t(element_t &e, std::ofstream &ofs) {
    uint32_t len = element_length_in_bytes(e);
    unsigned char buf[len];
    element_to_bytes(buf, e);
    ofs.write((char*)&len, 4);
    ofs.write((char*)buf, len);
}

void deserialize_element_t(element_t &e, std::ifstream &ifs) {
    uint32_t len;
    ifs.read((char*)&len, 4);
    unsigned char buf[1024];
    ifs.read((char*)buf, len);
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
        serialize_element_t(g, ofs_g);
    } else {
        deserialize_element_t(g, ifs_g);
    }

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

    // load of generate pko
    std::ifstream ifs_pko("../temp/pko");
    element_init_G2(pko, pairing);
    if(!ifs_sko.is_open()) {
        element_pow_zn(pko, g, sko);
        std::ofstream ofs_pko("../temp/pko", std::ios::binary);
        serialize_element_t(pko, ofs_pko);
    } else {
        deserialize_element_t(pko, ifs_pko);
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
    deserialize_element_t(pkc, ifs_pkc);

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

    //initialize a pairing
    pairing_pp_t pp;
    pairing_pp_init(pp, c1, pairing); // x is some element of G1

    //select private key
    auto keywords = load_keywords(5);
    std::vector<std::string> c;
    for(auto w : keywords) {
        element_t h2_res, pairing_res;
        element_init_G1(h2_res, pairing);
        element_init_GT(pairing_res, pairing);

        h2(h2_res, w);
        pairing_pp_apply(pairing_res, h2_res, pp);
        std::string cw = h3(pairing_res);
        std::cout<<cw<<"\n";
        c.push_back(cw);
    }

    pairing_pp_clear(pp); // don't need pp anymore

    /* TODO: Send c1, c2, c3, and c to Cloud server */
}