//
// Created by abhishek on 24/9/22.
//

#include "util.h"
#include<openssl/sha.h>
#include <fstream>
#include <iostream>
#include <openssl/evp.h>
#include <pbc/pbc.h>
#include "tb_lib/util.h"

void serialize_element_t(element_t &e, std::ostream &os) {
    uint32_t len = element_length_in_bytes(e);
    char buf[len];
    element_to_bytes((unsigned char*)buf, e);
    uint32_t temp = len;
    for(int i=0; i<4; ++i) {
        char val = (char)(temp & 255u);
        os.write(&val, 1);
        temp >>= 8;
    }
    for(int i=0; i<len; ++i) {
        os.write(buf + i, 1);
    }
    os.flush();
}

//template<typename charT>
void deserialize_element_t(element_t &e, std::istream &is) {
    uint32_t len = 0;
    for(int i=0; i<4; ++i) {
        char byte;
        is.read(&byte, 1);
        uint32_t temp = (unsigned char)byte;
        len |= temp << (i*8);
    }
    char buf[len];
    for(int i=0; i<len; ++i)
        is.read(buf+i, 1);
    element_from_bytes(e, (unsigned char*)buf);
}

std::string element_to_string(element_t &e) {
    std::ostringstream bos(std::ios_base::app);
    serialize_element_t(e, bos);
    return bos.str();
}

void string_to_element(element_t &e, std::string s) {
    std::istringstream bis(s);
    deserialize_element_t(e, bis);
}


std::string hash(std::string inp) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname("SHA256");
    if(!md) {
        printf("Unknown message digest\n");
        exit(1);
    }
    unsigned int md_len;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, inp.c_str(), inp.length());
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_destroy(mdctx);
    EVP_cleanup();

//    element_from_hash(e, md_value, md_len);
    std::string res;
    for(int i=0; i<md_len; ++i)
        res += md_value[i];
    return res;
}

void h1(element_t &e, std::string inp) {
    std::string val = hash(inp);
    element_from_hash(e, (void *) val.c_str(), val.length());
}

void h2(element_t &e, std::string inp) {
    std::string val = hash(inp);
    element_from_hash(e, (void *) val.c_str(), val.length());
}

std::string h3(element_t &e) {
    uint32_t len = element_length_in_bytes(e);
    unsigned char buf[len];
    element_to_bytes(buf, e);

    std::string data;
    for(int i=0; i<len; ++i)
        data += buf[i];
    return hash(data);
}