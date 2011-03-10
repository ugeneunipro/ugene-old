#ifndef _KALIGN_UTILS_H_
#define _KALIGN_UTILS_H_

struct kalign_context;

namespace U2 {

void setTaskProgress(struct kalign_context* ctx, int percent);

void setTaskDesc(struct kalign_context* ctx, const char *str);

bool isCanceled(struct kalign_context* ctx);
} // namespace U2

#endif // _KALIGN_UTILS_H_

