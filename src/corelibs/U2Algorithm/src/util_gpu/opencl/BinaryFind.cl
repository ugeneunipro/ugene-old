
typedef long NumberType;

__kernel void
        binarySearch_classic( __const __global NumberType  * sortedArray,
                                      __global NumberType  * findMeArray,
                             __const int  findMeArraySize,
                             __const   unsigned int globalLowerBound,
                             __const   unsigned int globalUpperBound,
                             __const   unsigned int numberOfIteration,
                             __const __global NumberType* preSaveBounds,
                             __const __global NumberType* c_preSaveValues,
                             __const int preSaveDepth,
                             __local NumberType* preSaveValues,
                             __const NumberType filter)
{
    const unsigned int tid = get_global_id(0);

    event_t event = 0;
    async_work_group_copy(preSaveValues, c_preSaveValues, preSaveDepth, event);

    unsigned int iterPos = tid * numberOfIteration;
    if (iterPos >= findMeArraySize) {
        return;
    }

    NumberType lowBound = 0, highBound = 0, findMe = 0,
        preLowBound = 0, preUpperBound = 0, preMid = 0,
        preCurValue = 0, mid = 0, curValue = 0;

    const NumberType firstPreMid = preSaveDepth / 2;
    const NumberType firstPreCurValue = preSaveValues[firstPreMid];
    NumberType preAns = 0;

    for (int i = iterPos; i < iterPos + numberOfIteration; i++)
    {
        findMe = findMeArray[i];
        //TODO: optimize
        if ((findMe & filter) > (preSaveValues[preSaveDepth - 1] & filter)) {
            findMeArray[i] = -1;
            continue;
        }

        preLowBound = 0;
        preUpperBound = preSaveDepth;
        preMid = firstPreMid;
        preCurValue = firstPreCurValue;

//*****search in const memory
        for (int j = 0; preLowBound <= preUpperBound && (preCurValue & filter) != (findMe & filter); j++){

            if ((findMe & filter) > (preCurValue & filter)) {
                preLowBound = preMid + 1;
            }
            else {
                preUpperBound = preMid - 1;
            }
            preMid = preLowBound + (preUpperBound - preLowBound) / 2;
            preCurValue = preSaveValues[preMid];
        }

        if (((preCurValue & filter) == (findMe & filter))) {
            //findMeArray[i] = preSaveBounds[preMid];
            preAns = preSaveBounds[preMid];
            for(;preAns>=0 && (sortedArray[preAns] & filter)==(findMe & filter); preAns--){};
            findMeArray[i] = preAns + 1;
            continue;
        } else {
            lowBound = preSaveBounds[preUpperBound];
            highBound = preSaveBounds[preLowBound];
        }
//*******

        mid = lowBound + (highBound - lowBound) / 2;
        curValue = sortedArray[mid];

        for (int j = 0; lowBound <= highBound && (curValue & filter) != (findMe & filter); j++){

            if ((findMe & filter) > (curValue & filter)) {
                lowBound = mid + 1;
            }
            else {
                highBound = mid - 1;
            }
            mid = lowBound + (highBound - lowBound) / 2;
            curValue = sortedArray[mid];
        }
        if (((curValue & filter) == (findMe & filter))) {
            //findMeArray[i] = mid;
            preAns = mid;
            for(;preAns>=0 && (sortedArray[preAns] & filter)==(findMe & filter); preAns--){};
            findMeArray[i] = preAns + 1;
        } else {
            findMeArray[i] = -1;
        }
    }
}
