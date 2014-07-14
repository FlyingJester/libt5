#include "t5_fsops.h"
#include <string>

bool t5::IsDir(const char *aPath){
    if(std::string(aPath).empty())
        return false;
    
    return true;

}