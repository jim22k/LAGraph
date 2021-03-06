//------------------------------------------------------------------------------
// LAGraph_Calloc:  wrapper for calloc
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

// The 3rd parameter has been added, since some memory managers (PMR in C++,
// and the Rapids Memory Manager) require it passed back to the deallocate
// method.  For now, size_allocated is the same as nitems*size_of_item, but
// another memory manager could allocate more space than that, for better
// performance.

#include "LG_internal.h"

void *LAGraph_Calloc
(
    size_t nitems,          // number of items
    size_t size_of_item,    // size of each item
    // output:
    size_t *size_allocated  // # of bytes actually allocated
)
{

    // make sure at least one item is allocated
    nitems = LAGraph_MAX (1, nitems) ;

    // make sure at least one byte is allocated
    size_of_item = LAGraph_MAX (1, size_of_item) ;

    // compute the size and check for integer overflow
    size_t size ;
    bool ok = LG_Multiply_size_t (&size, nitems, size_of_item) ;
    if (!ok || nitems > GxB_INDEX_MAX || size_of_item > GxB_INDEX_MAX)
    {
        // overflow
        (*size_allocated) = 0 ;
        return (NULL) ;
    }

    // calloc the space
    void *p = NULL ;
    if (LAGraph_Calloc_function != NULL)
    {
        // use the calloc function
        p = LAGraph_Calloc_function (nitems, size_of_item) ;
        (*size_allocated) = (p == NULL) ? 0 : size ;
    }
    else
    {
        // calloc function not available; use malloc and memset
        void *p = LAGraph_Malloc (nitems, size_of_item, size_allocated) ;
        if (p != NULL)
        {
            memset (p, 0, size) ;
        }
    }
    return (p) ;
}

