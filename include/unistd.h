#pragma once
#ifdef _WIN32
  #include <BaseTsd.h>
  typedef SSIZE_T ssize_t;

  #include <io.h>        // _access, _close, _read, _write
  #include <process.h>   // _getpid
  #include <direct.h>    // _getcwd
  #include <cstdint>
  #include <cstdlib>
  #include <cstdio>
  #include <chrono>
  #include <thread>

  // Safe inline wrappers (avoid macros like access/getcwd that break Boost)
  static inline int access(const char* path, int mode)            { return _access(path, mode); }
  static inline int getpid(void)                                  { return _getpid(); }
  static inline char* getcwd(char* buffer, int maxlen)            { return _getcwd(buffer, maxlen); }

  // Sleep helpers without including <windows.h>
  static inline void usleep(unsigned int usec)                    { std::this_thread::sleep_for(std::chrono::microseconds(usec)); }
  static inline unsigned int sleep(unsigned int sec)              { std::this_thread::sleep_for(std::chrono::seconds(sec)); return 0; }
#endif
