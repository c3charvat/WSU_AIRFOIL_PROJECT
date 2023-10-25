
#include "luapatch.h"

extern "C" {
  int _gettimeofday(struct timeval *tv, struct timezone *tz)
  {
    return 0;
  }
  
  int _open(const char* pathname, int flags, mode_t mode)
  {
      return 0;
  }
  
  clock_t _times(struct tms* tms)
  {
      errno = EINVAL;
      return (clock_t)-1;
  }
  // int _unlink(const char *pathname)
  // {
  //   errno = ENOTSUP;
  //   return 0;
  // }
  // int _link(const char *pathname)
  // {
  //   errno = ENOTSUP;
  //   return 0;
  // }
}