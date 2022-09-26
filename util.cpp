//
// Created by abhishek on 24/9/22.
//
#include <openssl/evp.h>
#include <pbc/pbc.h>
#include <string>
#include "tb_lib/util.h"

tb_util::bytes hash(tb_util::bytes inp) {
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
    tb_util::bytes res;
    for(int i=0; i<md_len; ++i)
        res += md_value[i];
    return res;
}

void h1(element_t &e, tb_util::bytes inp) {
    tb_util::bytes val = hash(inp);
    element_from_hash(e, (void *) val.c_str(), val.length());
}

void h2(element_t &e, tb_util::bytes inp) {
    tb_util::bytes val = hash(inp);
    element_from_hash(e, (void *) val.c_str(), val.length());
}

tb_util::bytes h3(element_t &e) {
    uint32_t len = element_length_in_bytes(e);
    unsigned char buf[len];
    element_to_bytes(buf, e);

    tb_util::bytes data;
    for(int i=0; i<len; ++i)
        data += buf[i];
    return hash(data);
}