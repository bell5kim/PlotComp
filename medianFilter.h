#ifndef _MEDIANFILTER_H
#define _MEDIANFILTER_H
//==============================================================================
//
//  Date:    15-Jun-2006
//
//  File:    medianFilter.h
//
//  Purpose: Header file for a median filter function template.
//
//==============================================================================

using namespace std;

#include <algorithm>
#include <cassert>
#include <vector>


// Function template definition:
//========================================

// NOTE: A function based on this template acts as a constructor; the user is
// responsible for deleting the vector that it constructs.

template <class T>
vector<T>* medianFilter(vector<T>& raTOriginal, const int iNumberOfFilterPoints)
{
    assert(iNumberOfFilterPoints >= 1);
    assert(iNumberOfFilterPoints % 2 == 1);

    // Create a new vector that extends the original at the endpoints:
    int iOriginalSize  = static_cast<int>(raTOriginal.size());
    int iExtension = iNumberOfFilterPoints / 2;  // integer arithmetic!
    int iExtendedSize  = iOriginalSize + 2 * iExtension; 
    vector<T> aTExtended(iExtendedSize);

    // Fill the leading edge of the extended array with the first value
    // of the original array:
    for (int iPoint = 0; iPoint < iExtension; iPoint++)
    {
        aTExtended[iPoint] = raTOriginal[0];
    }
   
    // Fill the central part of the extended array with all the data from
    // the original array:
    for (int iPoint = iExtension; iPoint < iExtension + iOriginalSize; iPoint++)
    {
        aTExtended[iPoint] = raTOriginal[iPoint-iExtension];
    }

    // Fill the trailing edge of the extended array with the last value
    // of the original array:
    for (int iPoint = iExtension + iOriginalSize; iPoint < iExtendedSize;
        iPoint++)
    {
        aTExtended[iPoint] = raTOriginal[iOriginalSize-1];
    }

    // Create and size the new array:
    vector<T>* paTFiltered = new vector<T>;
    vector<T>& raTFiltered = *paTFiltered;  // simplifies the ensuing syntax
    raTFiltered.resize(iOriginalSize);

    // Loop through the array and filter it:
    int& riCentralPoint = iExtension;  // e.g., '2' is the middle index of 5 pts
    for (int iPoint = 0; iPoint < iOriginalSize; iPoint++)
    {
        int iExtended = iPoint + iExtension;  // index in the extended array

        // Load up the filtering array:
        vector<T> aTFilter(iNumberOfFilterPoints);
        for (int iOffset = -iExtension; iOffset <= iExtension; iOffset++)
        {
            aTFilter[iExtension+iOffset] = aTExtended[iExtended+iOffset];
        }

        // Sort the filtering array:
        sort(aTFilter.begin(), aTFilter.end());

        // Extract the median value:
        raTFiltered[iPoint] = aTFilter[riCentralPoint]; 
    }

    return paTFiltered;
}

//==============================================================================
#endif // _MEDIANFILTER_H
