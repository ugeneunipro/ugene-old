#ifndef _MUSCLE_UTILS_H_
#define _MUSCLE_UTILS_H_
#include <stdio.h>

#include "muscle/muscle.h"
#include "muscle/seqvect.h"
#include "muscle/distfunc.h"
#include "muscle/msa.h"
#include "muscle/tree.h"
#include "muscle/profile.h"
#include "muscle/muscle_context.h"
#include "muscle/estring.h"

#include <U2Core/Task.h>

#include <U2Core/MAlignment.h>

class MuscleContext;

namespace U2 {

class DNAAlphabet;

struct FILEStub : public FILE {
public:
    FILEStub(TaskStateInfo& _tsi) : tsi(_tsi){}
    TaskStateInfo& tsi;
};

class MuscleParamsHelper{
public:
    MuscleParamsHelper(TaskStateInfo& ti, MuscleContext *ctx);
    ~MuscleParamsHelper();
private:
    MuscleContext *ctx;
    FILEStub ugeneFileStub;
};

int ugene_printf(FILE *f, const char *format, ...);
ALPHA convertAlpha(DNAAlphabet* al);
void setupAlphaAndScore(DNAAlphabet* al, TaskStateInfo& ti);
void convertMAlignment2MSA(MSA& muscleMSA, const MAlignment& ma, bool fixAlpha);
void convertMAlignment2SecVect(SeqVect& sv, const MAlignment& ma, bool fixAlpha);
void convertMSA2MAlignment(MSA& msa, DNAAlphabet* al, MAlignment& res);
void prepareAlignResults(MSA& msa, DNAAlphabet* al, MAlignment& ma, bool mhack);

} //namespace

#endif //_MUSCLE_ACCESSORIES_H_