//
// Created by abhishek on 24/9/22.
//
#include <pbc/pbc.h>
#include <string>
#include <sstream>
#include <functional>
#include <map>
#include <string_view>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <exception>
#include <vector>
#include <iostream>
#include <thread>
#include <regex>
#include <cstring>
#include "tb_lib/util.h"

void serialize_element_t(element_t &e, std::ostream &os);
void deserialize_element_t(element_t &e, std::istream &is);

std::string element_to_string(element_t &e);
void string_to_element(element_t &e, std::string);

void h1(element_t&, std::string);
void h2(element_t&, std::string);
std::string h3(element_t&);



