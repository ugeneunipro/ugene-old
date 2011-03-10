#ifndef _KALIGN_CONTEXT_
#define _KALIGN_CONTEXT_

typedef struct kalign_context {
	
	int stride;
	int dim;
	int gpo_pos;
	int gpe_pos;
	int tgpe_pos;
	unsigned int numseq;
	unsigned int numprofiles;
	float gpo;
	float gpe;
	float tgpe;
	float secret;
	void* ptask_state;
} kalign_context;

struct kalign_context* init_context(kalign_context *ctx, void* ptsi);

struct kalign_context* get_kalign_context();

char* k_printf(const char *format, ...);

void set_task_progress(int percents);

int check_task_canceled(struct kalign_context* ctx);

#endif //_KALIGN_CONTEXT_