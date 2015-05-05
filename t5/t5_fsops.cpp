#include "t5_fsops.h"
#include <string>

bool T5_IsFileNaive(const char *path);
bool T5_IsDirNaive(const char *path);

bool t5::IsDir(const char *aPath){
    if(T5_IsDirNaive(aPath))
      return true;

    return false;
}

bool t5::IsFile(const char *aPath){
    if(T5_IsFileNaive(aPath))
      return true;

    return false;
}

#ifdef _MSC_VER

#include <Windows.h>

#define DOESNTEXIST(attrs) attrs==INVALID_FILE_ATTRIBUTES

bool T5_IsFileNaive(const char *path){
DWORD attrs = GetFileAttributesA(path);
if(DOESNTEXIST(attrs)){
return false;
}
return (attrs&FILE_ATTRIBUTE_DIRECTORY)?false:true;
}

bool T5_IsDirNaive(const char *path){
DWORD attrs = GetFileAttributesA(path);
if(DOESNTEXIST(attrs)){
return false;
}
return (attrs&FILE_ATTRIBUTE_DIRECTORY)?true:false;
}

#else
#include <unistd.h>
#ifdef HAS_STAT_SYS
  #include <sys/stat.h>
#elif HAS_STAT
  #include <stat.h>
#endif

bool T5_IsFileNaive(const char *path){
    struct stat buf;
    stat(path, &buf);
    return S_ISREG(buf.st_mode);
}

bool T5_IsDirNaive(const char *path){
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

#endif
