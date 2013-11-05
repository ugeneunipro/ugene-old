#include "kalign2_context.h"

kalign_context *init_context(kalign_context *ctx, void *ptsi) {
	ctx->numseq = 0;
	ctx->numprofiles = 0;
	ctx->gpo = -1;
	ctx->gpe = -1;
	ctx->tgpe = -1;
	ctx->secret = -1;
	ctx->ptask_state = ptsi;
}

extern kalign_context* getKalignContext();

kalign_context* get_kalign_context() {
	return getKalignContext();
}
