//@@TODO reconcile /muscle with /muscle3.6

#include "muscle.h"
#include <stdio.h>
#include "muscle_context.h"
#ifdef	WIN32
#include <windows.h>	// for SetPriorityClass()
#include <io.h>			// for isatty()
#else
#include <unistd.h>		// for isatty()
#endif

int xmain(int argc, char **argv)
	{
    MuscleContext *ctx = getMuscleContext();
#if	WIN32
// Multi-tasking does not work well in CPU-bound
// console apps running under Win32.
// Reducing the process priority allows GUI apps
// to run responsively in parallel.
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif
	ctx->muscle.g_argc = argc;
	ctx->muscle.g_argv = argv;

	SetNewHandler();
	SetStartTime();
	ProcessArgVect(argc - 1, argv + 1);
	SetParams();
	SetLogFile();

	//extern void TestSubFams(const char *);
	//TestSubFams(g_pstrInFileName);
	//return 0;

	if (ctx->params.g_bVersion)
		{
		printf(MUSCLE_LONG_VERSION "\n");
		exit(EXIT_SUCCESS);
		}

	if (!ctx->params.g_bQuiet)
		//Credits();

	if (MissingCommand() && isatty(0))
		{
		Usage();
		exit(EXIT_SUCCESS);
		}

	if (ctx->params.g_bCatchExceptions)
		{
		try
			{
			Run();
			}
		catch (...)
			{
			OnException();
			exit(EXIT_Except);
			}
		}
	else
		Run();

#ifdef _CRTDBG_MAP_ALLOC
    delete ctx;
    _CrtDumpMemoryLeaks(); //to Output window of MSVC
#endif

	exit(EXIT_Success);
	}
