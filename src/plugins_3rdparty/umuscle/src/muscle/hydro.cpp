#include "muscle.h"
#include "profile.h"
#include "muscle_context.h"

extern void TomHydro(ProfPos *Prof, unsigned Length);

// Apply hydrophobicity heuristic to a profile
void Hydro(ProfPos *Prof, unsigned uLength)
	{
    MuscleContext *ctx = getMuscleContext();
    unsigned &g_uHydrophobicRunLength = ctx->params.g_uHydrophobicRunLength;
    float &g_dHydroFactor = ctx->params.g_dHydroFactor;
	if (ALPHA_Amino != ctx->alpha.g_Alpha)
		return;

	if (ctx->params.g_bTomHydro)
		{
		TomHydro(Prof, uLength);
		return;
		}

	if (0 == ctx->params.g_uHydrophobicRunLength)
		return;

	if (uLength <= ctx->params.g_uHydrophobicRunLength)
		return;

	unsigned uRunLength = 0;
	unsigned L2 = g_uHydrophobicRunLength/2;
	for (unsigned uColIndex = L2; uColIndex < uLength - L2; ++uColIndex)
		{
		ProfPos &PP = Prof[uColIndex];
		bool bHydro = IsHydrophobic(PP.m_fcCounts);
		if (bHydro)
			{
			++uRunLength;
			if (uRunLength >= g_uHydrophobicRunLength)
				{
				Prof[uColIndex-L2].m_scoreGapOpen *= (SCORE) g_dHydroFactor;
				Prof[uColIndex-L2].m_scoreGapClose *= (SCORE) g_dHydroFactor;
				}
			}
		else
			uRunLength = 0;
		}
	}
