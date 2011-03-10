#include "muscle.h"
#include <ctype.h>

#include "params.h"
#include "muscle_context.h"

/***
From Bioperl docs:
Extended DNA / RNA alphabet
------------------------------------------
Symbol       Meaning      Nucleic Acid
------------------------------------------
    A            A           Adenine
    C            C           Cytosine
    G            G           Guanine
    T            T           Thymine
    U            U           Uracil
    M          A or C
    R          A or G
    W          A or T
    S          C or G
    Y          C or T
    K          G or T
    V        A or C or G
    H        A or C or T
    D        A or G or T
    B        C or G or T
    X      G or A or T or C
    N      G or A or T or C

IUPAC-IUB SYMBOLS FOR NUCLEOTIDE NOMENCLATURE:
         Cornish-Bowden (1985) Nucl. Acids Res. 13: 3021-3030.
***/

#define Res(c, Letter)												\
	{																\
    MuscleContext *ctx = getMuscleContext();              \
	const unsigned char Upper = (unsigned char) toupper(c);			\
	const unsigned char Lower = (unsigned char) tolower(c);			\
	ctx->alpha.g_CharToLetter[Upper] = Letter;								\
	ctx->alpha.g_CharToLetter[Lower] = Letter;								\
	ctx->alpha.g_CharToLetterEx[Upper] = Letter;							\
	ctx->alpha.g_CharToLetterEx[Lower] = Letter;							\
	ctx->alpha.g_LetterToChar[Letter] = Upper;								\
	ctx->alpha.g_LetterExToChar[Letter] = Upper;							\
	ctx->alpha.g_IsResidueChar[Upper] = true;								\
	ctx->alpha.g_IsResidueChar[Lower] = true;								\
	ctx->alpha.g_AlignChar[Upper] = Upper;									\
	ctx->alpha.g_AlignChar[Lower] = Upper;									\
	ctx->alpha.g_UnalignChar[Upper] = Lower;								\
	ctx->alpha.g_UnalignChar[Lower] = Lower;								\
	}

#define Wild(c, Letter)												\
    {																\
    MuscleContext *ctx = getMuscleContext();              \
    const unsigned char Upper = (unsigned char) toupper(c);			\
    const unsigned char Lower = (unsigned char) tolower(c);			\
    ctx->alpha.g_CharToLetterEx[Upper] = Letter;							\
    ctx->alpha.g_CharToLetterEx[Lower] = Letter;							\
    ctx->alpha.g_LetterExToChar[Letter] = Upper;							\
    ctx->alpha.g_IsResidueChar[Upper] = true;								\
    ctx->alpha.g_IsResidueChar[Lower] = true;								\
    ctx->alpha.g_AlignChar[Upper] = Upper;									\
    ctx->alpha.g_AlignChar[Lower] = Upper;									\
    ctx->alpha.g_UnalignChar[Upper] = Lower;								\
    ctx->alpha.g_UnalignChar[Lower] = Lower;								\
    ctx->alpha.g_IsWildcardChar[Lower] = true;								\
    ctx->alpha.g_IsWildcardChar[Upper] = true;								\
    }


static unsigned GetAlphaSize(ALPHA Alpha)
	{
	switch (Alpha)
		{
	case ALPHA_Amino:
		return 20;

	case ALPHA_RNA:
	case ALPHA_DNA:
		return 4;
    default: break;
		}
	Quit("Invalid Alpha=%d", Alpha);
	return 0;
	}

static void InitArrays()
	{
    MuscleContext *ctx = getMuscleContext();
	memset(ctx->alpha.g_CharToLetter, 0xff, sizeof(ctx->alpha.g_CharToLetter));
	memset(ctx->alpha.g_CharToLetterEx, 0xff, sizeof(ctx->alpha.g_CharToLetterEx));

	memset(ctx->alpha.g_LetterToChar, '?', sizeof(ctx->alpha.g_LetterToChar));
	memset(ctx->alpha.g_LetterExToChar, '?', sizeof(ctx->alpha.g_LetterExToChar));

	memset(ctx->alpha.g_AlignChar, '?', sizeof(ctx->alpha.g_UnalignChar));
	memset(ctx->alpha.g_UnalignChar, '?', sizeof(ctx->alpha.g_UnalignChar));

	memset(ctx->alpha.g_IsWildcardChar, 0, sizeof(ctx->alpha.g_IsWildcardChar));
	}

static void SetGapChar(char c)
	{
    MuscleContext *ctx = getMuscleContext();

	unsigned char u = (unsigned char) c;

	ctx->alpha.g_CharToLetterEx[u] = AX_GAP;
	ctx->alpha.g_LetterExToChar[AX_GAP] = u;
	ctx->alpha.g_AlignChar[u] = u;
	ctx->alpha.g_UnalignChar[u] = u;
	}

static void SetAlphaDNA()
	{
	Res('A', NX_A)
	Res('C', NX_C)
	Res('G', NX_G)
	Res('T', NX_T)
	Wild('M', NX_M)
	Wild('R', NX_R)
	Wild('W', NX_W)
	Wild('S', NX_S)
	Wild('Y', NX_Y)
	Wild('K', NX_K)
	Wild('V', NX_V)
	Wild('H', NX_H)
	Wild('D', NX_D)
	Wild('B', NX_B)
	Wild('X', NX_X)
	Wild('N', NX_N)
	}

static void SetAlphaRNA()
	{
	Res('A', NX_A)
	Res('C', NX_C)
	Res('G', NX_G)
	Res('U', NX_U)
	Res('T', NX_T)
	Wild('M', NX_M)
	Wild('R', NX_R)
	Wild('W', NX_W)
	Wild('S', NX_S)
	Wild('Y', NX_Y)
	Wild('K', NX_K)
	Wild('V', NX_V)
	Wild('H', NX_H)
	Wild('D', NX_D)
	Wild('B', NX_B)
	Wild('X', NX_X)
	Wild('N', NX_N)
	}

static void SetAlphaAmino()
	{
	Res('A', AX_A)
	Res('C', AX_C)
	Res('D', AX_D)
	Res('E', AX_E)
	Res('F', AX_F)
	Res('G', AX_G)
	Res('H', AX_H)
	Res('I', AX_I)
	Res('K', AX_K)
	Res('L', AX_L)
	Res('M', AX_M)
	Res('N', AX_N)
	Res('P', AX_P)
	Res('Q', AX_Q)
	Res('R', AX_R)
	Res('S', AX_S)
	Res('T', AX_T)
	Res('V', AX_V)
	Res('W', AX_W)
	Res('Y', AX_Y)

	Wild('B', AX_B)
	Wild('X', AX_X)
	Wild('Z', AX_Z)
	}

void SetAlpha(ALPHA Alpha)
	{
    MuscleContext *ctx = getMuscleContext();
	InitArrays();

	SetGapChar('.');
	SetGapChar('-');

	switch (Alpha)
		{
	case ALPHA_Amino:
		SetAlphaAmino();
		break;

	case ALPHA_DNA:
		SetAlphaDNA();

	case ALPHA_RNA:
		SetAlphaRNA();
		break;

	default:
		Quit("Invalid Alpha=%d", Alpha);
		}

	ctx->alpha.g_AlphaSize = GetAlphaSize(Alpha);
	ctx->alpha.g_Alpha = Alpha;

	if (ctx->params.g_bVerbose)
		Log("Alphabet %s\n", ALPHAToStr(ctx->alpha.g_Alpha));
	}

char GetWildcardChar()
	{
    ALPHA &g_Alpha = getMuscleContext()->alpha.g_Alpha;
	switch (g_Alpha)
		{
	case ALPHA_Amino:
		return 'X';

	case ALPHA_DNA:
	case ALPHA_RNA:
		return 'N';

	default:
		Quit("Invalid Alpha=%d", g_Alpha);
		}
	return '?';
	}

bool IsNucleo(char c)
	{
	return strchr("ACGTURYNacgturyn", c) != 0;
	}

bool IsDNA(char c)
	{
	return strchr("AGCTNagctn", c) != 0;
	}

bool IsRNA(char c)
	{
	return strchr("AGCUNagcun", c) != 0;
	}

void ClearInvalidLetterWarning()
	{
	memset(getMuscleContext()->alpha.InvalidLetters, 0, 256);
	}

void InvalidLetterWarning(char c, char w)
	{
    MuscleContext *ctx = getMuscleContext();
	ctx->alpha.InvalidLetters[(unsigned char) c] = 1;
	++ctx->alpha.InvalidLetterCount;
	}

void ReportInvalidLetters()
	{
    MuscleContext *ctx = getMuscleContext();
	if (0 == ctx->alpha.InvalidLetterCount)
		return;

	char Str[257];
	memset(Str, 0, 257);

	int n = 0;
	for (int i = 0; i < 256; ++i)
		{
		if (ctx->alpha.InvalidLetters[i])
			Str[n++] = (char) i;
		}
	Warning("Assuming %s (see -seqtype option), invalid letters found: %s",
	  ALPHAToStr(ctx->alpha.g_Alpha), Str);
	}
