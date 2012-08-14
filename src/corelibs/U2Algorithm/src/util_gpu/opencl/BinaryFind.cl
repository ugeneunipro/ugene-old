
typedef long NumberType;

__kernel void
        binarySearch_classic(__const __global NumberType* haystack,
                             __const int haystackSize,
                             __global NumberType* needlesArr,
                             __const int needlesArrSize,
							 __const __global int *windowSizes,
							 // the number of needles a particular kernel execution should search for
                             __const int workItemSize)
							 // indices into the big haystack where the values from the mini haystack can be found
//                              __const __global NumberType* miniHaystackOffsets,
							 // a shrunken down version of the haystack, containing more or less evenly spaced values from it
							 // first and last values are the same as in the big haystack
//                              __const __global NumberType* miniHaystack,
//                              __const int miniHaystackSize)
{
    int start = get_global_id(0) * workItemSize, 
		end = min(start + workItemSize, needlesArrSize);

    if (start >= needlesArrSize) {
        return;
    }

    NumberType needle = 0, // the number to search for, with the filter applied
		curValue = 0, // the current value from the haystack
		low = 0, hi = 0, mid = 0, // indices used when searching in both mini and full-sized arrays
		firstOccurrenceOffset; // used to search the first occurrence of a needle when a row of identical ones is found

    for (int i = start; i < end; i++)
    {
		long filter = ((long)0 - 1) << (62 - windowSizes[i] * 2);
        needle = needlesArr[i] & filter;

		// needle < min(haystack)
// 		if (needle < (miniHaystack[0] & filter)) {
        if (needle < (haystack[0] & filter)) {
			needlesArr[i] = -1;
			continue;
		}

		// needle > max(haystack)
//         if (needle > (miniHaystack[miniHaystackSize - 1] & filter)) {
        if (needle > (haystack[haystackSize - 1] & filter)) {
            needlesArr[i] = -1;
            continue;
        }

		// needle == haystack[0]
// 		if(needle == (miniHaystack[0] & filter)) {
		if(needle == (haystack[0] & filter)) {
			needlesArr[i] = 0;
			continue;
		}

//         low = 0;
//         hi = miniHaystackSize - 1;
//         mid = miniHaystackSize / 2;
//         curValue = miniHaystack[mid];
// 
// 		// search in the mini haystack first
//         for (int j = 0; low <= hi && (curValue & filter) != needle; j++){
// 
//             if (needle > (curValue & filter)) {
//                 low = mid + 1;
//             }
//             else {
//                 hi = mid - 1;
//             }
//             mid = low + (hi - low) / 2;
//             curValue = miniHaystack[mid];
//         }
// 
// 		// found the needle in the mini haystack, look for an actual first occurrence in case there's more then 1 identical needle
//         if (((curValue & filter) == needle)) {
//             for(firstOccurrenceOffset = miniHaystackOffsets[mid]; 
// 				(firstOccurrenceOffset >= 0) && ((haystack[firstOccurrenceOffset] & filter) == needle);
// 				firstOccurrenceOffset--) {};
// 
// 			needlesArr[i] = firstOccurrenceOffset + 1;
//             continue;
//         } else {
// 			// low > hi, the needle is between these two offsets
// 			mid = miniHaystackOffsets[hi];
//             hi = miniHaystackOffsets[low];
// 			low = mid;
//         }
//         mid = low + (hi - low) / 2;

		// search in the big haystack now
        low = 0;
        hi = haystackSize - 1;
        mid = haystackSize / 2;
        curValue = haystack[mid] & filter;

        for (int j = 0; low <= hi && curValue != needle; j++){
            if (needle > curValue) {
                low = mid + 1;
            }
            else {
                hi = mid - 1;
            }
            mid = low + (hi - low) / 2;
            curValue = haystack[mid] & filter;
        }
        
        if (curValue == needle) {
            for(firstOccurrenceOffset = mid; 
				firstOccurrenceOffset >= 0 && (haystack[firstOccurrenceOffset] & filter) == needle;
				firstOccurrenceOffset--) {};

			needlesArr[i] = firstOccurrenceOffset + 1;
        } else {
			needlesArr[i] = -1;
        }
    }
}
