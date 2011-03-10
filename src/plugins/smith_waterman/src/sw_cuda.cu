//#ifdef SW2_BUILD_WITH_CUDA

typedef int ScoreType;

// CUDA global constants
__constant__ int partSeqSize, partsNumber, overlapLength, seqLibLength, queryLength;
__constant__ int gapOpen, gapExtension, maxScore, queryPartLength;


//global function
__global__ void calculateMatrix(const char * seqLib, ScoreType* queryProfile,
								ScoreType* g_HdataUp, ScoreType* g_HdataRec, ScoreType* g_HdataMax,
								ScoreType* g_FdataUp,
								ScoreType* g_directionsUp, ScoreType* g_directionsRec, ScoreType* g_directionsMax,
								int queryStartPos) 
{

	//registers
	int patternPos = threadIdx.x;	
	int globalPatternPos =  queryStartPos + patternPos;

	int seqStartPos = blockIdx.x * (partSeqSize - overlapLength);
	int globalStartPos = blockIdx.x * (partSeqSize + 1);
	
	int seqPos = 0, globalPos = 0, diagNum = 0;
	ScoreType substScore = 0;	
	ScoreType E = 0, E_left = 0, F = 0, F_up = 0, H = 0,
		H_left = 0, H_up = 0, H_upleft = 0, E_left_init = 0, 
		H_left_init = 0, directionLeft = 0, directionUp = 0, 
		directionUpLeft = 0, direction = 0, directionInit = 0,
		maxScore = 0;



	//dynamic allocation shared memory
	extern __shared__ ScoreType shared_H[];
	ScoreType* shared_E = (ScoreType*)&shared_H[queryPartLength + 1];
	ScoreType* shared_direction = (ScoreType*)&shared_E[queryPartLength + 1];

	shared_H[patternPos] = 0;
	shared_E[patternPos] = 0;
	shared_direction[patternPos] = 0;

	__syncthreads();	

	seqPos = seqStartPos - patternPos;
	globalPos = globalStartPos - patternPos;

	diagNum = (partSeqSize + queryPartLength - 1);
	for (int iteration = 0; iteration < diagNum; iteration++) {

		//check boundaries
		bool isActual = seqPos < seqLibLength && seqPos >= seqStartPos && seqPos < seqStartPos + partSeqSize && globalPatternPos < queryLength;
		if (isActual) {
			substScore = queryProfile[seqLib[seqPos] * queryLength + globalPatternPos];

			H_left = shared_H[patternPos];
			E_left = shared_E[patternPos];
			directionLeft = shared_direction[patternPos];

			if (patternPos == 0) {

				H_left = g_HdataUp[globalPos];
				E_left = g_FdataUp[globalPos];
				directionLeft = g_directionsUp[globalPos];

				if (globalPos > 0) {
					H_upleft = g_HdataUp[globalPos - 1];
					directionUpLeft = g_directionsUp[globalPos - 1];

				} 
				if (queryStartPos == 0 || iteration == 0) {
					directionUpLeft = seqPos;				
				}	
			}
		}
		__syncthreads();

		if (isActual) {
			E = max(E_left - gapExtension, H_left - gapOpen);
			F = max(F_up - gapExtension, H_up - gapOpen);

			// Compute H
			H = max(0, E);
			H = max(H, F);
			H = max(H, H_upleft + substScore);	

			//chose direction
			if (H == 0) {		 
				direction = seqPos + 1;				
			}
			else if (H == E) {
				direction = directionLeft;
			}	
			else if (H == F) {
				direction = directionUp;
			}	
			//(H == H_upleft + substScore)
			else {
				direction = directionUpLeft;
			}		
				
			shared_E[patternPos + 1] = E;
			shared_H[patternPos + 1] = H;
			shared_direction[patternPos + 1] = direction;

			H_upleft = H_left;
			H_up = H;
			F_up = F;

			directionUp = direction;			
			directionUpLeft = directionLeft;

			//collect best result
			maxScore = max(H, g_HdataMax[globalPos]);
			if (maxScore == H) {
				g_HdataMax[globalPos] = maxScore;
				g_directionsMax[globalPos] = direction;
			}

			//if this last iteration then start prepare next
			if (patternPos == (queryPartLength - 1)) {
				g_HdataRec[globalPos] = H;
				g_FdataUp[globalPos] = E;			
				g_directionsRec[globalPos]  = direction;
			}
		}
		__syncthreads();

		seqPos++;// = seqStartPos + iteration - patternPos;
		globalPos++;// = globalStartPos + iteration - patternPos;

	}

}


void calculateMatrix_wrap(int blockSize, int threadNum, const char * seqLib, ScoreType* queryProfile,                           
						  ScoreType* g_HdataUp, ScoreType* g_HdataRec, ScoreType* g_HdataMax,
						  ScoreType* g_FdataUp,
						  ScoreType* g_directionsUp, ScoreType* g_directionsRec, ScoreType* g_directionsMax,
						  int iteration) 
{
	size_t sh_mem_size = sizeof(ScoreType) * (threadNum + 1) * 3;
	calculateMatrix<<<blockSize, threadNum, sh_mem_size>>>(seqLib,
		queryProfile, g_HdataUp, 
		g_HdataRec, g_HdataMax, g_FdataUp,                                   
		g_directionsUp, g_directionsRec, g_directionsMax, iteration);
}

void setConstants(int partSeqSize, int partsNumber, int overlapLength, int seqLibLength, int queryLength, int gapOpen, int gapExtension, int maxScore, int queryPartLength) {
	cudaMemcpyToSymbol("partSeqSize",    &partSeqSize,    sizeof(partSeqSize));
	cudaMemcpyToSymbol("partsNumber",    &partsNumber,    sizeof(partsNumber));
	cudaMemcpyToSymbol("overlapLength",    &overlapLength,    sizeof(overlapLength));
	cudaMemcpyToSymbol("seqLibLength",    &seqLibLength,    sizeof(seqLibLength));
	cudaMemcpyToSymbol("queryLength",    &queryLength,    sizeof(queryLength));
	cudaMemcpyToSymbol("gapOpen",    &gapOpen,    sizeof(gapOpen));
	cudaMemcpyToSymbol("gapExtension",    &gapExtension,    sizeof(gapExtension));
	cudaMemcpyToSymbol("maxScore",    &maxScore,    sizeof(maxScore));
	cudaMemcpyToSymbol("queryPartLength",    &queryPartLength,    sizeof(queryPartLength));
}

//#endif //SW2_BUILD_WITH_CUDA
