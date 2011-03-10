#include "muscle.h"
#include "textfile.h"
#include "msa.h"
#include "objscore.h"
#include "tree.h"
#include "profile.h"
#include "muscle_context.h"

void DoSP()
	{
    MuscleContext *ctx = getMuscleContext();

	TextFile f(ctx->params.g_pstrSPFileName);

	MSA a;
	a.FromFile(f);

	ALPHA Alpha = ALPHA_Undefined;
	switch (ctx->params.g_SeqType)
		{
	case SEQTYPE_Auto:
		Alpha = a.GuessAlpha();
		break;

	case SEQTYPE_Protein:
		Alpha = ALPHA_Amino;
		break;

	case SEQTYPE_DNA:
		Alpha = ALPHA_DNA;
		break;

	case SEQTYPE_RNA:
		Alpha = ALPHA_RNA;
		break;

	default:
		Quit("Invalid SeqType");
		}
	SetAlpha(Alpha);
	a.FixAlpha();

	SetPPScore();

	const unsigned uSeqCount = a.GetSeqCount();
	if (0 == uSeqCount)
		Quit("No sequences in input file %s", ctx->params.g_pstrSPFileName);

	MSA::SetIdCount(uSeqCount);
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		a.SetSeqId(uSeqIndex, uSeqIndex);

	SetSeqWeightMethod(ctx->params.g_SeqWeight1);
	Tree tree;
	TreeFromMSA(a, tree, ctx->params.g_Cluster2, ctx->params.g_Distance2, ctx->params.g_Root2);
	SetMuscleTree(tree);
	SetMSAWeightsMuscle((MSA &) a);

	SCORE SP = ObjScoreSP(a);

	Log("File=%s;SP=%.4g\n", ctx->params.g_pstrSPFileName, SP);
	fprintf(stderr, "File=%s;SP=%.4g\n", ctx->params.g_pstrSPFileName, SP);
	}
