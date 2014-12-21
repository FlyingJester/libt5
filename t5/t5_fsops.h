#pragma once
#include <string>

namespace t5 {
    bool IsDir(const char *aPath);
    bool IsFile(const char *aPath);

    inline bool IsDir(const std::string &aPath){
        return IsDir(aPath.c_str());
    }
    inline bool IsFile(const std::string &aPath){
        return IsFile(aPath.c_str());
    }
}

#ifndef T5_DEPRECATED
#define T5_DEPRECATED
#endif

inline bool T5_IsDir(const char *aPath) T5_DEPRECATED;
inline bool T5_IsFile(const char *aPath) T5_DEPRECATED;

inline bool T5_IsDir(const char *aPath){return t5::IsDir(aPath);}
inline bool T5_IsFile(const char *aPath){return t5::IsFile(aPath);}

//FS iterators

