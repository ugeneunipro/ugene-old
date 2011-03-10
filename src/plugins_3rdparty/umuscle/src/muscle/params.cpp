#include "muscle.h"
#include "objscore.h"
#include "profile.h"
#include "enumopts.h"
#include "params.h"
#include "muscle_context.h"


const double DEFAULT_MAX_MB_FRACT = 0.8;

extern SCOREMATRIX VTML_LA;
extern SCOREMATRIX PAM200;
extern SCOREMATRIX PAM200NoCenter;
extern SCOREMATRIX VTML_SP;
extern SCOREMATRIX VTML_SPNoCenter;
extern SCOREMATRIX NUC_SP;

static unsigned atou(const char *s)
	{
	return (unsigned) atoi(s);
	}

//const char *MaxSecsToStr()
//	{
//    unsigned long &g_ulMaxSecs = getMuscleContext()->params.g_ulMaxSecs;
//
//	if (0 == g_ulMaxSecs)
//		return "(No limit)";
//	return SecsToStr(g_ulMaxSecs);
//	}

void ListParams()
	{
    /*MuscleContext *ctx = getMuscleContext(); 
	Log("\n");
	Log("%s\n", MUSCLE_LONG_VERSION);
	Log("http://www.drive5.com/muscle\n");
	Log("\n");
	Log("Profile-profile score    %s\n", PPSCOREToStr(ctx->params.g_PPScore));
	Log("Max iterations           %u\n", ctx->params.g_uMaxIters);
	Log("Max trees                %u\n", ctx->params.g_uMaxTreeRefineIters);
	Log("Max time                 %s\n", MaxSecsToStr());
	Log("Max MB                   %u\n", ctx->params.g_uMaxMB);
	Log("Gap open                 %g\n", ctx->params.g_scoreGapOpen);
	Log("Gap extend (dimer)       %g\n", ctx->params.g_scoreGapExtend);
	Log("Gap ambig factor         %g\n", ctx->params.g_scoreAmbigFactor);
	Log("Gap ambig penalty        %g\n", ctx->params.g_scoreGapAmbig);
	Log("Center (LE)              %g\n", ctx->params.g_scoreCenter);
	Log("Term gaps                %s\n", TERMGAPSToStr(ctx->params.g_TermGaps));

	Log("Smooth window length     %u\n", ctx->params.g_uSmoothWindowLength);
	Log("Refine window length     %u\n", ctx->params.g_uRefineWindow);
	Log("Min anchor spacing       %u\n", ctx->params.g_uAnchorSpacing);
	Log("Min diag length (lambda) %u\n", ctx->params.g_uMinDiagLength);
	Log("Diag margin (mu)         %u\n", ctx->params.g_uDiagMargin);
	Log("Min diag break           %u\n", ctx->params.g_uMaxDiagBreak);
	Log("Hydrophobic window       %u\n", ctx->params.g_uHydrophobicRunLength);

	Log("Hydrophobic gap factor   %g\n", ctx->params.g_dHydroFactor);
	Log("Smooth score ceiling     %g\n", ctx->params.g_dSmoothScoreCeil);
	Log("Min best col score       %g\n", ctx->params.g_dMinBestColScore);
	Log("Min anchor score         %g\n", ctx->params.g_dMinSmoothScore);
	Log("SUEFF                    %g\n", ctx->params.g_dSUEFF);

	Log("Brenner root MSA         %s\n", BoolToStr(ctx->params.g_bBrenner));
	Log("Normalize counts         %s\n", BoolToStr(ctx->params.g_bNormalizeCounts));
	Log("Diagonals (1)            %s\n", BoolToStr(ctx->params.g_bDiags1));
	Log("Diagonals (2)            %s\n", BoolToStr(ctx->params.g_bDiags2));
	Log("Anchors                  %s\n", BoolToStr(ctx->params.g_bAnchors));
	Log("MSF output format        %s\n", BoolToStr(ctx->params.g_bMSF));
	Log("Phylip interleaved       %s\n", BoolToStr(ctx->params.g_bPHYI));
	Log("Phylip sequential        %s\n", BoolToStr(ctx->params.g_bPHYS));
	Log("ClustalW output format   %s\n", BoolToStr(ctx->params.g_bAln));
	Log("Catch exceptions         %s\n", BoolToStr(ctx->params.g_bCatchExceptions));
	Log("Quiet                    %s\n", BoolToStr(ctx->params.g_bQuiet));
	Log("Refine                   %s\n", BoolToStr(ctx->params.g_bRefine));
	Log("ProdfDB                  %s\n", BoolToStr(ctx->params.g_bProfDB));
	Log("Low complexity profiles  %s\n", BoolToStr(ctx->params.g_bLow));

	Log("Objective score          %s\n", OBJSCOREToStr(ctx->params.g_ObjScore));

	Log("Distance method (1)      %s\n", DISTANCEToStr(ctx->params.g_Distance1));
	Log("Clustering method (1)    %s\n", CLUSTERToStr(ctx->params.g_Cluster1));
	Log("Root method (1)          %s\n", ROOTToStr(ctx->params.g_Root1));
	Log("Sequence weighting (1)   %s\n", SEQWEIGHTToStr(ctx->params.g_SeqWeight1));

	Log("Distance method (2)      %s\n", DISTANCEToStr(ctx->params.g_Distance2));
	Log("Clustering method (2)    %s\n", CLUSTERToStr(ctx->params.g_Cluster2));
	Log("Root method (2)          %s\n", ROOTToStr(ctx->params.g_Root2));
	Log("Sequence weighting (2)   %s\n", SEQWEIGHTToStr(ctx->params.g_SeqWeight2));

	Log("\n");*/
	}

static void SetDefaultsLE()
	{
    MuscleContext *ctx = getMuscleContext();
	ctx->params.g_ptrScoreMatrix = &VTML_LA;
    
	//g_scoreGapOpen = (SCORE) -3.00;
	//g_scoreCenter = (SCORE) -0.55;
	ctx->params.g_scoreGapOpen = (SCORE) -2.9;
	ctx->params.g_scoreCenter = (SCORE) -0.52;

	ctx->params.g_bNormalizeCounts = true;

	//g_dSmoothScoreCeil = 5.0;
	//g_dMinBestColScore = 4.0;
	//g_dMinSmoothScore = 2.0;
	ctx->params.g_dSmoothScoreCeil = 3.0;
	ctx->params.g_dMinBestColScore = 2.0;
	ctx->params.g_dMinSmoothScore = 1.0;

	ctx->params.g_Distance1 = DISTANCE_Kmer6_6;
	ctx->params.g_Distance2 = DISTANCE_PctIdKimura;
	}

static void SetDefaultsSP()
	{
    MuscleContext *ctx = getMuscleContext();
	ctx->params.g_ptrScoreMatrix = &PAM200;

	ctx->params.g_scoreGapOpen = -1439;
	ctx->params.g_scoreCenter = 0.0;	// center pre-added into score mx

	ctx->params.g_bNormalizeCounts = false;

	ctx->params.g_dSmoothScoreCeil = 200.0;
	ctx->params.g_dMinBestColScore = 300.0;
	ctx->params.g_dMinSmoothScore = 125.0;

	ctx->params.g_Distance1 = DISTANCE_Kmer6_6;
	ctx->params.g_Distance2 = DISTANCE_PctIdKimura;
	}

static void SetDefaultsSV()
	{
    MuscleContext *ctx = getMuscleContext();
	ctx->params.g_ptrScoreMatrix = &VTML_SP;

	ctx->params.g_scoreGapOpen = -300;
	ctx->params.g_scoreCenter = 0.0;	// center pre-added into score mx

	ctx->params.g_bNormalizeCounts = false;

	ctx->params.g_dSmoothScoreCeil = 90.0;
	ctx->params.g_dMinBestColScore = 130.0;
	ctx->params.g_dMinSmoothScore = 40.0;

	ctx->params.g_Distance1 = DISTANCE_Kmer6_6;
	ctx->params.g_Distance2 = DISTANCE_PctIdKimura;
	}

//static void SetDefaultsSPN()
//	{
//	g_ptrScoreMatrix = &NUC_SP;
//
//	g_scoreGapOpen = -400;
//	g_scoreCenter = 0.0;	// center pre-added into score mx
//
//	g_bNormalizeCounts = false;
//
//	g_dSmoothScoreCeil = 999.0;		// disable
//	g_dMinBestColScore = 90;
//	g_dMinSmoothScore = 90;
//
//	g_Distance1 = DISTANCE_Kmer4_6;
//	g_Distance2 = DISTANCE_PctIdKimura;
//	}

static void SetDefaultsSPN_DNA()
	{

    MuscleContext *ctx = getMuscleContext();
	ctx->params.g_ptrScoreMatrix = &NUC_SP;

	ctx->params.g_scoreGapOpen = -400;
	ctx->params.g_scoreCenter = 0.0;	// center pre-added into score mx
	ctx->params.g_scoreGapExtend = 0.0;

	ctx->params.g_bNormalizeCounts = false;

	ctx->params.g_dSmoothScoreCeil = 999.0;		// disable
	ctx->params.g_dMinBestColScore = 90;
	ctx->params.g_dMinSmoothScore = 90;

	ctx->params.g_Distance1 = DISTANCE_Kmer4_6;
	ctx->params.g_Distance2 = DISTANCE_PctIdKimura;
	}

static void SetDefaultsSPN_RNA()
	{

    MuscleContext *ctx = getMuscleContext();
	ctx->params.g_ptrScoreMatrix = &NUC_SP;

	ctx->params.g_scoreGapOpen = -420;
	ctx->params.g_scoreCenter = -300;	// total center = NUC_EXTEND - 300 
	ctx->params.g_scoreGapExtend = 0.0;

	ctx->params.g_bNormalizeCounts = false;

	ctx->params.g_dSmoothScoreCeil = 999.0;		// disable
	ctx->params.g_dMinBestColScore = 90;
	ctx->params.g_dMinSmoothScore = 90;

	ctx->params.g_Distance1 = DISTANCE_Kmer4_6;
	ctx->params.g_Distance2 = DISTANCE_PctIdKimura;
	}

static void FlagParam(const char *OptName, bool *ptrParam, bool bValueIfFlagSet)
	{
	bool bIsSet = FlagOpt(OptName);
	if (bIsSet)
		*ptrParam = bValueIfFlagSet;
	}

static void StrParam(const char *OptName, const char **ptrptrParam)
	{
	const char *opt = ValueOpt(OptName);
	if (0 != opt)
		*ptrptrParam = opt;
	}

static void FloatParam(const char *OptName, float *ptrParam)
	{
	const char *opt = ValueOpt(OptName);
	if (0 != opt)
		*ptrParam = (float) atof(opt);
	}

static void UintParam(const char *OptName, unsigned *ptrParam)
	{
	const char *opt = ValueOpt(OptName);
	if (0 != opt)
		*ptrParam = atou(opt);
	}

static void EnumParam(const char *OptName, EnumOpt *Opts, int *Param)
	{
	const char *Value = ValueOpt(OptName);
	if (0 == Value)
		return;

	for (;;)
		{
		if (0 == Opts->pstrOpt)
			Quit("Invalid parameter -%s %s", OptName, Value);
		if (0 == stricmp(Value, Opts->pstrOpt))
			{
			*Param = Opts->iValue;
			return;
			}
		++Opts;
		}
	}

static void SetPPDefaultParams()
	{
    MuscleContext *ctx = getMuscleContext();

	switch (ctx->params.g_PPScore)
		{
	case PPSCORE_SP:
		SetDefaultsSP();
		break;

	case PPSCORE_LE:
		SetDefaultsLE();
		break;

	case PPSCORE_SV:
		SetDefaultsSV();
		break;

	case PPSCORE_SPN:
		switch (ctx->alpha.g_Alpha)
			{
		case ALPHA_DNA:
			SetDefaultsSPN_DNA();
			break;
		case ALPHA_RNA:
			SetDefaultsSPN_RNA();
			break;
		default:
			Quit("Invalid alpha %d", ctx->alpha.g_Alpha);
			}
		break;

	default:
		Quit("Invalid g_PPScore");
		}
	}

static void SetPPCommandLineParams()
	{
    MuscleContext *ctx = getMuscleContext();

	FloatParam("GapOpen", &ctx->params.g_scoreGapOpen);
	FloatParam("GapOpen2", &ctx->params.g_scoreGapOpen2);
	FloatParam("GapExtend", &ctx->params.g_scoreGapExtend);
	FloatParam("GapExtend2", &ctx->params.g_scoreGapExtend2);
	FloatParam("GapAmbig", &ctx->params.g_scoreAmbigFactor);
	FloatParam("Center", &ctx->params.g_scoreCenter);
	FloatParam("SmoothScoreCeil", &ctx->params.g_dSmoothScoreCeil);
	FloatParam("MinBestColScore", &ctx->params.g_dMinBestColScore);
	FloatParam("MinSmoothScore", &ctx->params.g_dMinSmoothScore);

	EnumParam("Distance", DISTANCE_Opts, (int *) &ctx->params.g_Distance1);
	EnumParam("Distance", DISTANCE_Opts, (int *) &ctx->params.g_Distance2);

	EnumParam("Distance1", DISTANCE_Opts, (int *) &ctx->params.g_Distance1);
	EnumParam("Distance2", DISTANCE_Opts, (int *) &ctx->params.g_Distance2);
	}

void SetPPScore(bool bRespectFlagOpts)
	{
    MuscleContext *ctx = getMuscleContext();
	if (bRespectFlagOpts)
		{
		if (FlagOpt("SP"))
			ctx->params.g_PPScore = PPSCORE_SP;
		else if (FlagOpt("LE"))
			ctx->params.g_PPScore = PPSCORE_LE;
		else if (FlagOpt("SV"))
			ctx->params.g_PPScore = PPSCORE_SV;
		else if (FlagOpt("SPN"))
			ctx->params.g_PPScore = PPSCORE_SPN;
		}

	switch (ctx->params.g_PPScore)
		{
	case PPSCORE_LE:
	case PPSCORE_SP:
	case PPSCORE_SV:
		if (ALPHA_RNA == ctx->alpha.g_Alpha || ALPHA_DNA == ctx->alpha.g_Alpha)
			ctx->params.g_PPScore = PPSCORE_SPN;
		break;
	case PPSCORE_SPN:
		if (ALPHA_Amino == ctx->alpha.g_Alpha)
			ctx->params.g_PPScore = PPSCORE_LE;
		break;
        default: break;
		}

	SetPPDefaultParams();
	SetPPCommandLineParams();

	if (ctx->params.g_bVerbose)
		ListParams();
	}

void SetPPScore(PPSCORE p)
	{
	getMuscleContext()->params.g_PPScore = p;
	SetPPScore(true);
	}

static void SetMaxSecs()
	{
	float fMaxHours = 0.0;
	FloatParam("MaxHours", &fMaxHours);
	if (0.0 == fMaxHours)
		return;
	getMuscleContext()->params.g_ulMaxSecs = (unsigned long) (fMaxHours*60*60);
	}

static bool CanDoLowComplexity()
	{
    MuscleContext *ctx = getMuscleContext();
	if (ctx->params.g_SeqWeight1 != SEQWEIGHT_ClustalW)
		return false;
	if (1 == ctx->params.g_uMaxIters)
		return true;
	return ctx->params.g_SeqWeight2 == SEQWEIGHT_ClustalW;
	}

bool MissingCommand()
	{
    MuscleContext *ctx = getMuscleContext();
	if (strcmp(ctx->params.g_pstrInFileName, "-"))
		return false;
	if (0 != ctx->params.g_pstrFileName1)
		return false;
	if (0 != ctx->params.g_pstrSPFileName)
		return false;
	return true;
	}

void SetParams()
	{
    MuscleContext *ctx = getMuscleContext();
	SetMaxSecs();

	StrParam("in", &ctx->params.g_pstrInFileName);
	StrParam("out", &ctx->params.g_pstrOutFileName);

	StrParam("FASTAOut", &ctx->params.g_pstrFASTAOutFileName);
	StrParam("ClwOut", &ctx->params.g_pstrClwOutFileName);
	StrParam("ClwStrictOut", &ctx->params.g_pstrClwStrictOutFileName);
	StrParam("HTMLOut", &ctx->params.g_pstrHTMLOutFileName);
	StrParam("PHYIOut", &ctx->params.g_pstrPHYIOutFileName);
	StrParam("PHYSOut", &ctx->params.g_pstrPHYSOutFileName);
	StrParam("MSFOut", &ctx->params.g_pstrMSFOutFileName);

	StrParam("in1", &ctx->params.g_pstrFileName1);
	StrParam("in2", &ctx->params.g_pstrFileName2);

	StrParam("Matrix", &ctx->params.g_pstrMatrixFileName);
	StrParam("SPScore", &ctx->params.g_pstrSPFileName);

	StrParam("UseTree_NoWarn", &ctx->params.g_pstrUseTreeFileName);
	if (0 != ctx->params.g_pstrUseTreeFileName)
		ctx->params.g_bUseTreeNoWarn = true;

	StrParam("UseTree", &ctx->params.g_pstrUseTreeFileName);
	StrParam("ComputeWeights", &ctx->params.g_pstrComputeWeightsFileName);
	StrParam("ScoreFile", &ctx->params.g_pstrScoreFileName);
	StrParam("DistMx1", &ctx->params.g_pstrDistMxFileName1);
	StrParam("DistMx2", &ctx->params.g_pstrDistMxFileName2);

	FlagParam("Core", &ctx->params.g_bCatchExceptions, false);
	FlagParam("NoCore", &ctx->params.g_bCatchExceptions, true);

	FlagParam("Diags1", &ctx->params.g_bDiags1, true);
	FlagParam("Diags2", &ctx->params.g_bDiags2, true);

	bool Diags = false;
	FlagParam("Diags", &Diags, true);
	if (Diags)
		{
		ctx->params.g_bDiags1 = true;
		ctx->params.g_bDiags2 = true;
		}

	FlagParam("Anchors", &ctx->params.g_bAnchors, true);
	FlagParam("NoAnchors", &ctx->params.g_bAnchors, false);

	FlagParam("Quiet", &ctx->params.g_bQuiet, true);
	FlagParam("Verbose", &ctx->params.g_bVerbose, true);
	FlagParam("Version", &ctx->params.g_bVersion, true);
	FlagParam("Stable", &ctx->params.g_bStable, true);
	FlagParam("Group", &ctx->params.g_bStable, false);
	FlagParam("Refine", &ctx->params.g_bRefine, true);
	FlagParam("RefineW", &ctx->params.g_bRefineW, true);
	FlagParam("ProfDB", &ctx->params.g_bProfDB, true);
	FlagParam("SW", &ctx->params.g_bSW, true);
	FlagParam("ClusterOnly", &ctx->params.g_bClusterOnly, true);
	FlagParam("Profile", &ctx->params.g_bProfile, true);
	FlagParam("PPScore", &ctx->params.g_bPPScore, true);
	FlagParam("Brenner", &ctx->params.g_bBrenner, true);
	FlagParam("Dimer", &ctx->params.g_bDimer, true);

	FlagParam("MSF", &ctx->params.g_bMSF, true);
	FlagParam("PHYI", &ctx->params.g_bPHYI, true);
	FlagParam("PHYS", &ctx->params.g_bPHYS, true);
	FlagParam("clw", &ctx->params.g_bAln, true);
	FlagParam("HTML", &ctx->params.g_bHTML, true);
	FlagParam("FASTA", &ctx->params.g_bFASTA, true);
	FlagParam("PAS", &ctx->params.g_bPAS, true);
	FlagParam("MakeTree", &ctx->params.g_bMakeTree, true);

	bool b = false;
	FlagParam("clwstrict", &b, true);
	if (b)
		{
		ctx->params.g_bAln = true;
		ctx->params.g_bClwStrict = true;
		}

	UintParam("MaxIters", &ctx->params.g_uMaxIters);
	UintParam("MaxTrees", &ctx->params.g_uMaxTreeRefineIters);
	UintParam("SmoothWindow", &ctx->params.g_uSmoothWindowLength);
	UintParam("RefineWindow", &ctx->params.g_uRefineWindow);
	UintParam("FromWindow", &ctx->params.g_uWindowFrom);
	UintParam("ToWindow", &ctx->params.g_uWindowTo);
	UintParam("SaveWindow", &ctx->params.g_uSaveWindow);
	UintParam("WindowOffset", &ctx->params.g_uWindowOffset);
	UintParam("AnchorSpacing", &ctx->params.g_uAnchorSpacing);
	UintParam("DiagLength", &ctx->params.g_uMinDiagLength);
	UintParam("DiagMargin", &ctx->params.g_uDiagMargin);
	UintParam("DiagBreak", &ctx->params.g_uMaxDiagBreak);
	UintParam("MaxSubFam", &ctx->params.g_uMaxSubFamCount);

	UintParam("Hydro", &ctx->params.g_uHydrophobicRunLength);
	FlagParam("TomHydro", &ctx->params.g_bTomHydro, true);
	if (ctx->params.g_bTomHydro)
		ctx->params.g_uHydrophobicRunLength = 0;

	FloatParam("SUEFF", &ctx->params.g_dSUEFF);
	FloatParam("HydroFactor", &ctx->params.g_dHydroFactor);

	EnumParam("ObjScore", OBJSCORE_Opts, (int *) &ctx->params.g_ObjScore);
	EnumParam("TermGaps", TERMGAPS_Opts, (int *) &ctx->params.g_TermGaps);

	EnumParam("Weight", SEQWEIGHT_Opts, (int *) &ctx->params.g_SeqWeight1);
	EnumParam("Weight", SEQWEIGHT_Opts, (int *) &ctx->params.g_SeqWeight2);

	EnumParam("Weight1", SEQWEIGHT_Opts, (int *) &ctx->params.g_SeqWeight1);
	EnumParam("Weight2", SEQWEIGHT_Opts, (int *) &ctx->params.g_SeqWeight2);

	EnumParam("Cluster", CLUSTER_Opts, (int *) &ctx->params.g_Cluster1);
	EnumParam("Cluster", CLUSTER_Opts, (int *) &ctx->params.g_Cluster2);

	EnumParam("Cluster1", CLUSTER_Opts, (int *) &ctx->params.g_Cluster1);
	EnumParam("Cluster2", CLUSTER_Opts, (int *) &ctx->params.g_Cluster2);

	EnumParam("Root1", ROOT_Opts, (int *) &ctx->params.g_Root1);
	EnumParam("Root2", ROOT_Opts, (int *) &ctx->params.g_Root2);

	EnumParam("SeqType", SEQTYPE_Opts, (int *) &ctx->params.g_SeqType);

	ctx->params.g_scoreGapAmbig = ctx->params.g_scoreGapOpen*ctx->params.g_scoreAmbigFactor;
	ctx->params.g_bLow = CanDoLowComplexity();

	if (ctx->params.g_bDimer)
		ctx->params.g_bPrecompiledCenter = false;
    
	UintParam("MaxMB", &ctx->params.g_uMaxMB);
	if (0 == ValueOpt("MaxMB"))
		ctx->params.g_uMaxMB = (unsigned) (GetRAMSizeMB()*DEFAULT_MAX_MB_FRACT);
    
	}
