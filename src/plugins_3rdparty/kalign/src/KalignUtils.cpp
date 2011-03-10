#include "KalignUtils.h"
#include <U2Core/Task.h>

extern "C" {

#include "kalign2/kalign2_context.h"
#include <stdarg.h>
#include <stdio.h>

char* k_printf(const char *format, ...) {
	if (format[0] <= 31 || strlen(format) == 1) {
		return 0;
	}
	static char str[1024];
	va_list ArgList;
	va_start(ArgList, format);
	int n = vsprintf(str, format, ArgList);
	assert(n>=0 && n < 1024);
	Q_UNUSED(n);
	U2::setTaskDesc(get_kalign_context(), str);
	return str;
}

void set_task_progress(int percent) {
	U2::setTaskProgress(get_kalign_context(),percent);
}

int check_task_canceled(kalign_context *ctx) {
	return U2::isCanceled(ctx);
}
};

namespace U2 {

void setTaskProgress(struct kalign_context* ctx, int percent) {
	((TaskStateInfo*)ctx->ptask_state)->progress = percent;
}

void setTaskDesc(struct kalign_context* ctx, const char *str ) {
	TaskStateInfo *tsi = (TaskStateInfo*)ctx->ptask_state;
	tsi->setStateDesc(QString::fromAscii(str));
}

bool isCanceled(struct kalign_context* ctx) {
	return ((TaskStateInfo*)ctx->ptask_state)->cancelFlag;
}

} //namespace U2

