#include <stdio.h>

//#define DBG_PRINTBUFFER_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

#define ENABLE_USER_LOG 1
#define ENABLE_DEBUG_LOG 1
#define ENABLE_TRACE_LOG 0

#if ENABLE_USER_LOG
  #define USER_LOG(fmt, ...) printf(/*"[USER] " */fmt, ##__VA_ARGS__)
#else
  #define USER_LOG(fmt, ...)
#endif

#if ENABLE_DEBUG_LOG
  #define DEBUG_LOG(fmt, ...) printf(/*"[DEBUG] " */fmt, ##__VA_ARGS__)
#else
  #define DEBUG_LOG(fmt, ...)
#endif

#if ENABLE_TRACE_LOG
  #define TRACE_LOG(fmt, ...) printf(/*"[TRACE] " */fmt, ##__VA_ARGS__)
#else
  #define TRACE_LOG(fmt, ...)
#endif

//void DBG(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
