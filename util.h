//
// Created by abhishek on 24/9/22.
//
#include <pbc/pbc.h>
#include <string>
#include <sstream>
#include "tb_lib/util.h"

//typedef std::basic_string<unsigned char> bytes;
typedef std::basic_istringstream<unsigned char> bytes_istream;
typedef std::basic_ostringstream<unsigned char> bytes_ostream;
typedef unsigned char uchar;

void h1(element_t&, tb_util::bytes);
void h2(element_t&, tb_util::bytes);
tb_util::bytes h3(element_t&);
