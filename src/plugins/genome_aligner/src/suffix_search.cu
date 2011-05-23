#include <stdio.h>
//#include "cuPrintf.cu"

typedef unsigned long long BMType;
typedef unsigned int SAType;

__global__ void binarySearch(BMType array[], int arraySize, 
            BMType query[], int querySize) {

    unsigned int threadId = blockIdx.x * blockDim.x + threadIdx.x;

    int left = 0;
    int right = arraySize - 1;
    const BMType target = query[threadId];

    while (left <= right)
    {
        int mid = (left + right) >> 1;
        BMType midValue = array[mid];
        if(midValue > target) {
            right = mid - 1;
        } else if(midValue < target) {
            left = mid + 1;
        } else {
            for (mid = mid - 1; mid >= 0; --mid ) {
                if (array[mid] != target) {
                    break;
                }
            }
            query[threadId] = mid + 1;
            return;
        }
    }

    query[threadId] = -1;

}

extern "C" void cudaBinarySearch(BMType* array, int arraySize, BMType* query, int querySize )
{
    printf("Starting binary search...\n");
    binarySearch <<<querySize / 256 + 1, 256>>> (array, arraySize, query, querySize);
    cudaThreadSynchronize();
    printf("Binary search is finished...\n");
}


struct ShortReads 
{
    char* data;
    int* offsets;
    int* sizes;
    int count;
};

struct BMArray
{
    BMType* data;
    int size;
};

struct SArray
{
    SAType* data;
    int size;
};

struct AlgSettings
{
    BMType bitFilter;
    int refStartPos;
    int loadedPartSize;
    int CMAX;
    int w;
};


__device__ int lowerBound(BMType* bmArray, int bmArraySize, BMType target, BMType bitFilter) {
    int left = 0;
    int right = bmArraySize - 1;
    int result = -1;

    while (left <= right)
    {
        int mid = (left + right) >> 1;
        long long rc = (bmArray[mid]&bitFilter) - (target&bitFilter);
        if(rc > 0) {
            right = mid - 1;
        } else if(rc < 0) {
            left = mid + 1;
        } else {
            for (mid = mid - 1; mid >= 0; --mid ) {
                if ((bmArray[mid]&bitFilter) != (target&bitFilter)) {
                    break;
                }
            }
            result = mid + 1;
            break;
        }
    }

    return result;

}

__device__ BMType calcBitValue(char* readsData, int readOffset, int length, int charsInMask, SAType* bitTable)
{
    const int bitCharLen = 2;
    BMType bitValue = 0;
    //cuPrintf("L = %d, charsInMask = %d\n",length,charsInMask);
    //cuPrintf("bw=%ullx\n", bitValue);
    for (int i = 0; i < length; i++) {
        char c = readsData[readOffset+i];
        //cuPrintf("C=%c\n",c);
        //cuPrintf("bw=%llx\n", bitValue);
        bitValue = (bitValue << bitCharLen) | bitTable[c  - 65];
    }

    bitValue <<= 2*charsInMask - bitCharLen*length;

    return bitValue;    
}

__device__ void initBitTable(SAType* bitTable)
{
    bitTable['A' - 65] = 0x0; //00
    bitTable['C' - 65] = 0x1; //01
    bitTable['T' - 65] = 0x2; //10
    bitTable['G' - 65] = 0x3; //11
}


__device__ bool compare(const char *sourceSeq, const char *querySeq, int startPos, int w, int& c, int CMAX, int length) 
{
    // forward collect
    for (int i=startPos+w; i<length && c <= CMAX; i++) {
        char c1 = querySeq[i];
        char c2 = sourceSeq[i];
        //cuPrintf("F: %c %c pos=%d\n",c1,c2,i);
        c += (c1 == c2) ? 0 : 1;
    }
    if (c>CMAX) {
        return false;
    }
    // backward collect
    for (int i=startPos-1; i>=0 && c <= CMAX; i--) {
        char c1 = querySeq[i];
        char c2 = sourceSeq[i];
        c += (c1 == c2) ? 0 : 1;
        //cuPrintf("B: %c %c pos=%d\n",c1,c2,i);
    }
    if (c <= CMAX) {
        return true;
    }
    return false;
}


__global__ void alignReadsKernel(ShortReads reads,
                               char* refSeq,
                               int refSeqSize,
                               SArray sArray,
                               BMArray bmArray,
                               AlgSettings s,
                               SAType* results)
{

    const int CHARS_IN_MASK = 31;
    unsigned int threadId = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (threadId >= reads.count) {
        return;
    }
    
    int readOffset = reads.offsets[threadId];
    int readSize = reads.sizes[threadId];

    unsigned int bitTable[32];
    initBitTable(bitTable);
    
    /*
    if (threadId == 0) {
        cuPrintf("Offset is %d\n", readOffset);
        cuPrintf("Read size is: %d\n", readSize);
        cuPrintf("Read first symbol is %c\n", reads.data[0]);
        cuPrintf("First symbol of ref seq is %c\n", refSeq[0]);
        cuPrintf("Read last symbol is %c\n", reads.data[readOffset + readSize - 1]);
    }*/

    int CMAX = s.CMAX;
    int q = readSize / (CMAX + 1);
    SAType result = -1;
    int bestC = 0;
    for (int startPosInRead = 0; startPosInRead < readSize - s.w + 1; startPosInRead += q) {
        int len = readSize - startPosInRead > CHARS_IN_MASK ? CHARS_IN_MASK : readSize - startPosInRead;
        BMType bitValue = calcBitValue(reads.data, readOffset + startPosInRead, len, CHARS_IN_MASK, bitTable);
        SAType bmResult = lowerBound(bmArray.data, bmArray.size, bitValue, s.bitFilter);
        //cuPrintf("BW result is %u\n", bmResult);
        if (bmResult == -1) {
            continue;
        }            
        
        for (SAType k = bmResult; (k < s.loadedPartSize) && (bitValue & s.bitFilter)==(bmArray.data[k] & s.bitFilter); k++) {
            SAType pos = sArray.data[k];
            SAType firstSymbolPos = pos - startPosInRead;
            /*cuPrintf("pos is %d\n", pos);
            cuPrintf("startPosInRead=%d\n", startPosInRead);
            cuPrintf("firstSymbolPos=%d\n", firstSymbolPos);*/
            if (firstSymbolPos == result) {
                continue;
            }
            
            //TODO: check if valid pos in sense of sequence ends...
            /*
            if (!isValidPos( pos + loadedSeqStart, readStartPos, readSize, fisrtSymbol, q)) {
            continue;
            }
            */
            char* refBuff = refSeq + firstSymbolPos;
            char* readBuff = reads.data + readOffset;
            int c = 0;
            if (compare(refBuff, readBuff, startPosInRead, s.w, c, CMAX, readSize)) {
                //cuPrintf("c=%d,CMAX=%d\n",c, CMAX);
                result = firstSymbolPos;
                if (0 == c) {
                    break;
                }
                bestC = c;
                CMAX = bestC - 1;
            }

        }
    }
    //cuPrintf("Result is %d, mismatch rate is %d\n", result,bestC);
    results[2*threadId] = result + s.refStartPos;
    results[2*threadId + 1] = bestC;

}

extern "C" void cudaAlignReads(char* readsData, 
                               int* readSizes,
                               int* readOffsets,
                               int readsNumber,
                               char* refSeq,
                               int refSeqSize,
                               SAType* sArrayData,
                               int sArraySize,
                               BMType* bmArrayData,
                               int bmArraySize,
                               BMType bitFilter,
                               int refStartPos,
                               int loadedPartSize,
                               int CMAX,
                               int w,
                               SAType* results)
{

    printf("Starting aligning reads with CUDA...\n");
    
    ShortReads reads;
    reads.data = readsData;
    reads.sizes = readSizes;
    reads.offsets = readOffsets;
    reads.count = readsNumber;

    BMArray bmArray;
    bmArray.data = bmArrayData;
    bmArray.size = bmArraySize;
    
    SArray sArray;
    sArray.data = sArrayData;
    sArray.size = sArraySize;
    
    AlgSettings settings;
    settings.bitFilter = bitFilter;
    settings.refStartPos = refStartPos;
    settings.loadedPartSize = loadedPartSize;
    
    settings.CMAX = CMAX;
    settings.w = w;
    //cudaPrintfInit(10*1024*1024);

    alignReadsKernel <<<readsNumber / 256 + 1, 256>>> (reads, refSeq, refSeqSize,
        sArray, bmArray, settings, results);
    //cudaPrintfDisplay(stdout, true);
    cudaThreadSynchronize();
    //cudaPrintfEnd();


    printf("Finished aligning reads with CUDA\n");


}
