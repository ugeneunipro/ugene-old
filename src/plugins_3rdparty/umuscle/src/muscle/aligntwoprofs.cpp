#include "muscle.h"
#include "msa.h"
#include "profile.h"
#include "pwpath.h"
#include "muscle_context.h"

SCORE GlobalAlign4(ProfPos *PA, unsigned uLengthA, ProfPos *PB,
  unsigned uLengthB, PWPath &Path);

SCORE AlignTwoProfs(
  const ProfPos *PA, unsigned uLengthA, WEIGHT wA,
  const ProfPos *PB, unsigned uLengthB, WEIGHT wB,
  PWPath &Path, ProfPos **ptrPout, unsigned *ptruLengthOut)
	{
	if(uLengthA >= 100000 || uLengthB >= 100000)
		throw MuscleException("Can't align sequence longer 100000");
	//assert(uLengthA < 100000);
	//assert(uLengthB < 100000);

	float r = (float) uLengthA/ (float) (uLengthB + 1); // +1 to prevent div 0
	if (r < 1)
		r = 1/r;

	SCORE Score = GlobalAlign(PA, uLengthA, PB, uLengthB, Path);
    
    AlignTwoProfsGivenPath(Path, PA, uLengthB, wA/(wA + wB), PB, uLengthB, wB/(wA + wB),
	  ptrPout, ptruLengthOut);

#if	HYDRO
	if (ALPHA_Amino == g_Alpha)
		Hydro(*ptrPout, *ptruLengthOut);
#endif
	return Score;
	}
