#include <stdio.h>

typedef unsigned long long NumberType;

__global__ void binarySearch(NumberType array[], int arraySize, 
			NumberType query[], int querySize) {
	
	unsigned int threadId = blockIdx.x * blockDim.x + threadIdx.x;
	
	int left = 0;
	int right = arraySize - 1;
	const NumberType target = query[threadId];
	
	while (left <= right)
	{
		int mid = (left + right) / 2;
		NumberType midValue = array[mid];
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

extern "C" void cudaBinarySearch(NumberType* array, int arraySize, NumberType* query, int querySize )
{
	printf("Starting binary search...\n");
	binarySearch <<<querySize / 256 + 1, 256>>> (array, arraySize, query, querySize);	
	cudaThreadSynchronize();
	printf("Binary search is finished...\n");	
}