#pragma once

#include "zetton_common/log/log.h"
#include "zetton_stream/util/v4l/cv4l-helpers.h"
#include "zetton_stream/util/v4l/v4l-helpers.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace zetton {
namespace stream {

#if 0
static void errno_exit(const char* s) {
  AERROR_F("{} error: code {}, string [{}]", s, errno, strerror(errno));
  exit(EXIT_FAILURE);
}
#endif

static int xioctl(int fd, int request, void* arg) {
  int r = 0;

  do r = ioctl(fd, request, arg);
  while (-1 == r && EINTR == errno);

  return r;
}

}  // namespace stream
}  // namespace zetton
