#include "muscle.h"
#include <stdio.h>
#include <new>
#include "muscle_context.h"

const int ONE_MB = 1024*1024;
const size_t RESERVE_BYTES = 8*ONE_MB;

void OnOutOfMemory()
	{
    MuscleContext *ctx = getMuscleContext();
    //ctx->setnewhandler.EmergencyReserve;
	free(ctx->setnewhandler.EmergencyReserve);
	fprintf(stderr, "\n*** OUT OF MEMORY ***\n");
//s	fprintf(stderr, "Memory allocated so far %g MB\n", GetMemUseMB());
	MSA* &ptrBestMSA = ctx->savebest.ptrBestMSA;
	if (ptrBestMSA == 0)
		fprintf(stderr, "No alignment generated\n");
	else
		SaveCurrentAlignment();
	exit(EXIT_FatalError);
	}

void SetNewHandler()
	{
	getMuscleContext()->setnewhandler.EmergencyReserve = malloc(RESERVE_BYTES);
	std::set_new_handler(OnOutOfMemory);
	}
