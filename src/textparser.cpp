#include "textparser.h"

void obj_split(const std::string& in, std::vector<std::string>& out, std::string token) {
    out.clear();
    std::string temp;
    for (int i = 0; i < int(in.size()); i++) {
        std::string test = in.substr(i, token.size());
        if (test == token) {
            if (!temp.empty()) {
                out.push_back(temp);
                temp.clear();
                i += (int)token.size() - 1;
            }
            else {
                out.push_back("");
            }
        }
        else if (i + token.size() >= in.size()) {
            temp += in.substr(i, token.size());
            out.push_back(temp);
            break;
        }
        else {
            temp += in[i];
        }
    }
}

const std::string obj_tail(const std::string& in) {
    size_t token_start = in.find_first_not_of(" \t");
    size_t space_start = in.find_first_of(" \t", token_start);
    size_t tail_start = in.find_first_not_of(" \t", space_start);
    size_t tail_end = in.find_last_not_of(" \t");
    if (tail_start != std::string::npos && tail_end != std::string::npos) {
        return in.substr(tail_start, tail_end - tail_start + 1);
    }
    else if (tail_start != std::string::npos) {
        return in.substr(tail_start);
    }
    return "";
}

// Get first token of string
const std::string obj_firstToken(const std::string& in) {
    if (!in.empty()) {
        size_t token_start = in.find_first_not_of(" \t");
        size_t token_end = in.find_first_of(" \t", token_start);
        if (token_start != std::string::npos && token_end != std::string::npos) {
            return in.substr(token_start, token_end - token_start);
        }
        else if (token_start != std::string::npos) {
            return in.substr(token_start);
        }
    }
    return "";
}

