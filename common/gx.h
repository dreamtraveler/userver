#ifndef __DEF_H__
#define __DEF_H__

#include "log.h"

#define return_if_err(status)												    \
    do {																		\
        int code = (int)(status);												\
        if(code < 0){															\
            log_error("uv error %s: %s", uv_err_name(code), uv_strerror(code)); \
            return;																\
        }																		\
    } while(0)

static void* safe_malloc(size_t n) {
	void* p = malloc(n);
	if (!p) {
		log_fatal("out of memory(%dz bytes)", n);
	}
	return p;
}

static void* safe_realloc(void* oldp, size_t n) {
	void* p = realloc(oldp, n);
	if (!p) {
		log_fatal("out of memory(%dz bytes)", n);
	}
	return p;
}

#define SAFE_MALLOC(n) safe_malloc(n)
#define SAFE_REALLOC(p, n) safe_realloc(p, n)

#endif
