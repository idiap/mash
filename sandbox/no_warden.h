#ifndef _MASH_SANDBOX_NOWARDEN_H_
#define _MASH_SANDBOX_NOWARDEN_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* tWardenContext;

#define setWardenListener(listener)
#define setWardenContext(context)
#define getWardenContext() 0
#define wardenEnableUnsafeFree()
#define wardenDisableUnsafeFree()

#ifdef __cplusplus
}
#endif

#endif
