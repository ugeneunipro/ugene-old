#include "muscle.h"
#include <stdio.h>
#include "muscle_context.h"
typedef MuscleContext::options_struct::VALUE_OPT VALUE_OPT;
typedef MuscleContext::options_struct::FLAG_OPT FLAG_OPT;

static bool TestSetFlagOpt(const char *Arg)
	{
    MuscleContext *ctx = getMuscleContext();
    int &FlagOptCount = ctx->options.FlagOptCount;
    FLAG_OPT *FlagOpts = ctx->options.FlagOpts;
	for (int i = 0; i < FlagOptCount; ++i)
		if (!stricmp(Arg, FlagOpts[i].m_pstrName))
			{
			FlagOpts[i].m_bSet = true;
			return true;
			}
	return false;
	}

static bool TestSetValueOpt(const char *Arg, const char *Value)
	{
    MuscleContext *ctx = getMuscleContext();
    VALUE_OPT *ValueOpts = ctx->options.ValueOpts;
    int &ValueOptCount = ctx->options.ValueOptCount;
	for (int i = 0; i < ValueOptCount; ++i)
		if (!stricmp(Arg, ValueOpts[i].m_pstrName))
			{
			if (0 == Value)
				{
				fprintf(stderr, "Option -%s must have value\n", Arg);
				exit(EXIT_NotStarted);
				}
			ValueOpts[i].m_pstrValue = strsave(Value);
			return true;
			}
	return false;
	}

bool FlagOpt(const char *Name)
	{
    MuscleContext *ctx = getMuscleContext();
    int &FlagOptCount = ctx->options.FlagOptCount;
    FLAG_OPT *FlagOpts = ctx->options.FlagOpts;
	for (int i = 0; i < FlagOptCount; ++i)
		if (!stricmp(Name, FlagOpts[i].m_pstrName))
			return FlagOpts[i].m_bSet;
	Quit("FlagOpt(%s) invalid", Name);
	return false;
	}

const char *ValueOpt(const char *Name)
	{
    MuscleContext *ctx = getMuscleContext();
    VALUE_OPT *ValueOpts = ctx->options.ValueOpts;
    int &ValueOptCount = ctx->options.ValueOptCount;
	for (int i = 0; i < ValueOptCount; ++i)
		if (!stricmp(Name, ValueOpts[i].m_pstrName))
			return ValueOpts[i].m_pstrValue;
	Quit("ValueOpt(%s) invalid", Name);
	return 0;
	}

void ProcessArgVect(int argc, char *argv[])
	{
	for (int iArgIndex = 0; iArgIndex < argc; )
		{
		const char *Arg = argv[iArgIndex];
		if (Arg[0] != '-')
			{
			fprintf(stderr, "Command-line option \"%s\" must start with '-'\n", Arg);
			exit(EXIT_NotStarted);
			}
		const char *ArgName = Arg + 1;
		if (TestSetFlagOpt(ArgName))
			{
			++iArgIndex;
			continue;
			}
		
		char *Value = 0;
		if (iArgIndex < argc - 1)
			Value = argv[iArgIndex+1];
		if (TestSetValueOpt(ArgName, Value))
			{
			iArgIndex += 2;
			continue;
			}
		fprintf(stderr, "Invalid command line option \"%s\"\n", ArgName);
		Usage();
		exit(EXIT_NotStarted);
		}
	}

void ProcessArgStr(const char *ArgStr)
	{
	const int MAX_ARGS = 64;
	char *argv[MAX_ARGS];

	if (0 == ArgStr)
		return;

// Modifiable copy
	char *StrCopy = strsave(ArgStr);

	int argc = 0;
	bool bInArg = false;
	char *Str = StrCopy;
	while (char c = *Str)
		{
		if (isspace(c))
			{
			*Str = 0;
			bInArg = false;
			}
		else if (!bInArg)
			{
			bInArg = true;
			if (argc >= MAX_ARGS)
				Quit("Too many args in MUSCLE_CMDLINE");
			argv[argc++] = Str;
			}
		Str++;
		}
	ProcessArgVect(argc, argv);
	free(StrCopy);
	}

void ListFlagOpts()
	{
    MuscleContext *ctx = getMuscleContext();
    int &FlagOptCount = ctx->options.FlagOptCount;
    FLAG_OPT *FlagOpts = ctx->options.FlagOpts;
	for (int i = 0; i < FlagOptCount; ++i)
		Log("%s %d\n", FlagOpts[i].m_pstrName, FlagOpts[i].m_bSet);
	}
