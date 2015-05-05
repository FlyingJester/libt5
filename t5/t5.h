#ifndef LIBT5_HEAD
#define LIBT5_HEAD

#ifdef __GNUC__

#define T5_DEPRECATED __attribute__ ((deprecated))

#elif defined(_MSC_VER)

#define T5_DEPRECATED

#else

#define T5_DEPRECATED

#endif

#include "t5_stub.h"
#include "t5_datasource.h"
#include "t5_genericmap.h"
#include "t5_fs.h"
#include "t5_fsops.h"

namespace t5{


}


#endif