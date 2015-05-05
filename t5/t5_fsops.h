#pragma once

#include "t5_stub.h"

#include <string>

namespace t5 {
    bool T5_EXPORT IsDir(const char *aPath);
    bool T5_EXPORT IsFile(const char *aPath);

    inline bool IsDir(const std::string &aPath){
        return IsDir(aPath.c_str());
    }
    inline bool IsFile(const std::string &aPath){
        return IsFile(aPath.c_str());
    }
}
