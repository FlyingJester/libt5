#pragma once
namespace t5 {
    bool IsDir(const char *aPath);
    bool IsFile(const char *aPath);
}

#ifndef T5_DEPRECATED
#pragma warning T5_DEPRECATED was undefined. Only include this header through t5.h.

#define T5_DEPRECATED
#endif

inline bool T5_IsDir(const char *aPath) T5_DEPRECATED;
inline bool T5_IsFile(const char *aPath) T5_DEPRECATED;

inline bool T5_IsDir(const char *aPath){return t5::IsDir(aPath);}
inline bool T5_IsFile(const char *aPath){return t5::IsFile(aPath);}
