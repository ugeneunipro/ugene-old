#include "muscle.h"
#include "msa.h"
#include "params.h"
#include "textfile.h"
#include "muscle_context.h"

static void DoOutput(MSA &msa)
	{
    MuscleContext *ctx = getMuscleContext();
	bool AnyOutput = false;

// Value options
	if (ctx->params.g_pstrFASTAOutFileName)
		{
		TextFile File(ctx->params.g_pstrFASTAOutFileName, true);
		msa.ToFASTAFile(File);
		AnyOutput = true;
		}

	if (ctx->params.g_pstrMSFOutFileName)
		{
		TextFile File(ctx->params.g_pstrMSFOutFileName, true);
		msa.ToMSFFile(File);
		AnyOutput = true;
		}

	if (ctx->params.g_pstrClwOutFileName)
		{
		TextFile File(ctx->params.g_pstrClwOutFileName, true);
		msa.ToAlnFile(File);
		AnyOutput = true;
		}

	if (ctx->params.g_pstrClwStrictOutFileName)
		{
		ctx->params.g_bClwStrict = true;
		TextFile File(ctx->params.g_pstrClwStrictOutFileName, true);
		msa.ToAlnFile(File);
		AnyOutput = true;
		}

	if (ctx->params.g_pstrHTMLOutFileName)
		{
		TextFile File(ctx->params.g_pstrHTMLOutFileName, true);
		msa.ToHTMLFile(File);
		AnyOutput = true;
		}

	if (ctx->params.g_pstrPHYIOutFileName)
		{
		TextFile File(ctx->params.g_pstrPHYIOutFileName, true);
		msa.ToPhyInterleavedFile(File);
		AnyOutput = true;
		}

	if (ctx->params.g_pstrPHYSOutFileName)
		{
		TextFile File(ctx->params.g_pstrPHYSOutFileName, true);
		msa.ToPhySequentialFile(File);
		AnyOutput = true;
		}

// Flag options, at most one used (because only one -out filename)
	TextFile fileOut(ctx->params.g_pstrOutFileName, true);
	if (ctx->params.g_bFASTA)
		{
		msa.ToFASTAFile(fileOut);
		AnyOutput = true;
		}
	else if (ctx->params.g_bMSF)
		{
		msa.ToMSFFile(fileOut);
		AnyOutput = true;
		}
	else if (ctx->params.g_bAln)
		{
		msa.ToAlnFile(fileOut);
		AnyOutput = true;
		}
	else if (ctx->params.g_bHTML)
		{
		msa.ToHTMLFile(fileOut);
		AnyOutput = true;
		}
	else if (ctx->params.g_bPHYI)
		{
		msa.ToPhyInterleavedFile(fileOut);
		AnyOutput = true;
		}
	else if (ctx->params.g_bPHYS)
		{
		msa.ToPhySequentialFile(fileOut);
		AnyOutput = true;
		}

// If -out option was given but no flags, output as FASTA
	if (!AnyOutput)
		msa.ToFASTAFile(fileOut);
	
	fileOut.Close();

	if (0 != ctx->params.g_pstrScoreFileName)
		WriteScoreFile(msa);
	}

void MuscleOutput(MSA &msa)
	{
	MHackEnd(msa);
	if (getMuscleContext()->params.g_bStable)
		{
		MSA msaStable;
		Stabilize(msa, msaStable);
		msa.Clear();	// save memory
		DoOutput(msaStable);
		}
	else
		DoOutput(msa);
	}
