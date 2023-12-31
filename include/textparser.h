#pragma once

#include <string>
#include <sstream>
#include <vector>

void obj_split(const std::string& in, std::vector<std::string>& out, std::string token);
const std::string obj_tail(const std::string& in);
const std::string obj_firstToken(const std::string& in);


