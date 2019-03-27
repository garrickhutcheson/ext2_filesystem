#pragma once

//// DEBUG ////

// uncommment for debug mode
#define DEBUG_MODE 1

// debug messages
#if defined DEBUG_MODE
#define DEBUG_PRINT(fmt, args...)                                              \
  fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__,     \
          ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* Don't do anything in release builds */
#endif