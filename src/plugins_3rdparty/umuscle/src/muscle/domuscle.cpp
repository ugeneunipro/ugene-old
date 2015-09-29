#include "muscle.h"
#include "textfile.h"
#include "seqvect.h"
#include "distfunc.h"
#include "msa.h"
#include "tree.h"
#include "profile.h"
#include "timing.h"
#include "muscle_context.h"

static char g_strUseTreeWarning[] =
"\n******** WARNING ****************\n"
"\nYou specified the -usetree option.\n"
"Note that a good evolutionary tree may NOT be a good\n"
"guide tree for multiple alignment. For more details,\n"
"please refer to the user guide. To disable this\n"
"warning, use -usetree_nowarn <treefilename>.\n\n";


void DoMuscle()
	{
    MuscleContext *ctx = getMuscleContext();

	SetOutputFileName(ctx->params.g_pstrOutFileName);
	SetInputFileName(ctx->params.g_pstrInFileName);

	SetMaxIters(ctx->params.g_uMaxIters);
	SetSeqWeightMethod(ctx->params.g_SeqWeight1);

	TextFile fileIn(ctx->params.g_pstrInFileName);
	SeqVect v;
	v.FromFASTAFile(fileIn);
	const unsigned uSeqCount = v.Length();

	if (0 == uSeqCount)
		Quit("No sequences in input file");

	ALPHA Alpha = ALPHA_Undefined;
	switch (ctx->params.g_SeqType)
		{
	case SEQTYPE_Auto:
		Alpha = v.GuessAlpha();
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
		Quit("Invalid seq type");
		}
	SetAlpha(Alpha);
	v.FixAlpha();

	PTR_SCOREMATRIX UserMatrix = 0;
	if (0 != ctx->params.g_pstrMatrixFileName)
		{
		const char *FileName = ctx->params.g_pstrMatrixFileName;
		const char *Path = getenv("MUSCLE_MXPATH");
		if (Path != 0)
			{
			size_t n = strlen(Path) + 1 + strlen(FileName) + 1;
			char *NewFileName = new char[n];
			sprintf(NewFileName, "%s/%s", Path, FileName);
			FileName = NewFileName;
			}
		TextFile File(FileName);
		UserMatrix = ReadMx(File);
		ctx->alpha.g_Alpha = ALPHA_Amino;
		ctx->params.g_PPScore = PPSCORE_SP;
		}

	SetPPScore();

	if (0 != UserMatrix)
		ctx->params.g_ptrScoreMatrix = UserMatrix;

    unsigned uMinL = 0;
	unsigned uMaxL = 0;
	unsigned uTotL = 0;
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		{
		unsigned L = v.GetSeq(uSeqIndex).Length();
		uTotL += L;
        if (uMinL == 0 || L < uMinL)
            uMinL = L;
		if (L > uMaxL)
			uMaxL = L;
		}

	SetIter(1);
	ctx->params.g_bDiags = ctx->params.g_bDiags1;
    SetSeqStats(uSeqCount, uMinL, uMaxL, uTotL/uSeqCount);

	SetMuscleSeqVect(v);

	MSA::SetIdCount(uSeqCount);

// Initialize sequence ids.
// From this point on, ids must somehow propogate from here.
	for (unsigned uSeqIndex = 0; uSeqIndex < uSeqCount; ++uSeqIndex)
		v.SetSeqId(uSeqIndex, uSeqIndex);

	if (0 == uSeqCount)
		Quit("Input file '%s' has no sequences", ctx->params.g_pstrInFileName);
	if (1 == uSeqCount)
		{
		TextFile fileOut(ctx->params.g_pstrOutFileName, true);
		v.ToFile(fileOut);
		return;
		}

	if (uSeqCount > 1)
		MHackStart(v);

// First iteration
	Tree GuideTree;
	if (0 != ctx->params.g_pstrUseTreeFileName)
		{
	// Discourage users...
		if (!ctx->params.g_bUseTreeNoWarn)
			fprintf(stderr,"%s", g_strUseTreeWarning);

	// Read tree from file
		TextFile TreeFile(ctx->params.g_pstrUseTreeFileName);
		GuideTree.FromFile(TreeFile);

	// Make sure tree is rooted
		if (!GuideTree.IsRooted())
			Quit("User tree must be rooted");

		if (GuideTree.GetLeafCount() != uSeqCount)
			Quit("User tree does not match input sequences");

		const unsigned uNodeCount = GuideTree.GetNodeCount();
		for (unsigned uNodeIndex = 0; uNodeIndex < uNodeCount; ++uNodeIndex)
			{
			if (!GuideTree.IsLeaf(uNodeIndex))
				continue;
			const char *LeafName = GuideTree.GetLeafName(uNodeIndex);
			unsigned uSeqIndex;
			bool SeqFound = v.FindName(LeafName, &uSeqIndex);
			if (!SeqFound)
				Quit("Label %s in tree does not match sequences", LeafName);
			unsigned uId = v.GetSeqIdFromName(LeafName);
			GuideTree.SetLeafId(uNodeIndex, uId);
			}
		}
	else
		TreeFromSeqVect(v, GuideTree, ctx->params.g_Cluster1, ctx->params.g_Distance1, ctx->params.g_Root1,
		  ctx->params.g_pstrDistMxFileName1);

	const char *Tree1 = ValueOpt("Tree1");
	if (0 != Tree1)
		{
		TextFile f(Tree1, true);
		GuideTree.ToFile(f);
		if (ctx->params.g_bClusterOnly)
			return;
		}

	SetMuscleTree(GuideTree);
	ValidateMuscleIds(GuideTree);

	MSA msa;
	ProgNode *ProgNodes = 0;
	if (ctx->params.g_bLow)
		ProgNodes = ProgressiveAlignE(v, GuideTree, msa);
	else
		ProgressiveAlign(v, GuideTree, msa);
	SetCurrentAlignment(msa);

	if (0 != ctx->params.g_pstrComputeWeightsFileName)
		{
		extern void OutWeights(const char *FileName, const MSA &msa);
		SetMSAWeightsMuscle(msa);
		OutWeights(ctx->params.g_pstrComputeWeightsFileName, msa);
        delete[] ProgNodes;
		return;
		}

	ValidateMuscleIds(msa);

	if (1 == ctx->params.g_uMaxIters || 2 == uSeqCount)
		{
		//TextFile fileOut(g_pstrOutFileName, true);
		//MHackEnd(msa);
		//msa.ToFile(fileOut);
		MuscleOutput(msa);
        delete[] ProgNodes;
		return;
		}

	if (0 == ctx->params.g_pstrUseTreeFileName)
		{
		ctx->params.g_bDiags = ctx->params.g_bDiags2;
		SetIter(2);

		if (ctx->params.g_bLow)
			{
			if (0 != ctx->params.g_uMaxTreeRefineIters)
				RefineTreeE(msa, v, GuideTree, ProgNodes);
			}
		else
			RefineTree(msa, GuideTree);

		const char *Tree2 = ValueOpt("Tree2");
		if (0 != Tree2)
			{
			TextFile f(Tree2, true);
			GuideTree.ToFile(f);
			}
		}
    delete[] ProgNodes;
	SetSeqWeightMethod(ctx->params.g_SeqWeight2);
	SetMuscleTree(GuideTree);

	if (ctx->params.g_bAnchors)
		RefineVert(msa, GuideTree, ctx->params.g_uMaxIters - 2);
	else
		RefineHoriz(msa, GuideTree, ctx->params.g_uMaxIters - 2, false, false);

#if	0
// Refining by subfamilies is disabled as it didn't give better
// results. I tried doing this before and after RefineHoriz.
// Should get back to this as it seems like this should work.
	RefineSubfams(msa, GuideTree, g_uMaxIters - 2);
#endif

	ValidateMuscleIds(msa);
	ValidateMuscleIds(GuideTree);

	//TextFile fileOut(g_pstrOutFileName, true);
	//MHackEnd(msa);
	//msa.ToFile(fileOut);
	MuscleOutput(msa);
	}

void Run()
	{
    MuscleContext *ctx = getMuscleContext();
	SetStartTime();
        Log("Started %s\n", GetTimeAsStr());
    int &g_argc = ctx->muscle.g_argc;
    char **g_argv = ctx->muscle.g_argv;
	for (int i = 0; i < g_argc; ++i)
		Log("%s ", g_argv[i]);
	Log("\n");

#if	TIMING
	TICKS t1 = GetClockTicks();
#endif
	if (ctx->params.g_bRefine)
		Refine();
	else if (ctx->params.g_bRefineW)
		{
		extern void DoRefineW();
		DoRefineW();
		}
	else if (ctx->params.g_bProfDB)
		ProfDB();
	else if (ctx->params.g_bSW)
		Local();
	else if (0 != ctx->params.g_pstrSPFileName)
		DoSP();
	else if (ctx->params.g_bProfile)
		Profile();
	else if (ctx->params.g_bPPScore)
		PPScore();
	else if (ctx->params.g_bPAS)
		ProgAlignSubFams();
	else if (ctx->params.g_bMakeTree)
		{
		extern void DoMakeTree();
		DoMakeTree();
		}
	else
		DoMuscle();

#if	TIMING
	extern TICKS g_ticksDP;
	extern TICKS g_ticksObjScore;
	TICKS t2 = GetClockTicks();
	TICKS TotalTicks = t2 - t1;
	TICKS ticksOther = TotalTicks - g_ticksDP - g_ticksObjScore;
	double dSecs = TicksToSecs(TotalTicks);
	double PctDP = (double) g_ticksDP*100.0/(double) TotalTicks;
	double PctOS = (double) g_ticksObjScore*100.0/(double) TotalTicks;
	double PctOther = (double) ticksOther*100.0/(double) TotalTicks;
	Log("                 Ticks     Secs    Pct\n");
	Log("          ============  =======  =====\n");
	Log("DP        %12ld  %7.2f  %5.1f%%\n",
	  (long) g_ticksDP, TicksToSecs(g_ticksDP), PctDP);
	Log("OS        %12ld  %7.2f  %5.1f%%\n",
	  (long) g_ticksObjScore, TicksToSecs(g_ticksObjScore), PctOS);
	Log("Other     %12ld  %7.2f  %5.1f%%\n",
	  (long) ticksOther, TicksToSecs(ticksOther), PctOther);
	Log("Total     %12ld  %7.2f  100.0%%\n", (long) TotalTicks, dSecs);
#endif

	ListDiagSavings();
        Log("Finished %s\n", GetTimeAsStr());
	}
