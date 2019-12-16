//------------------------------------------------------------------------------
// p3test: read in (or create) a matrix and test PageRank3
//------------------------------------------------------------------------------

/*
LAGraph:  graph algorithms based on GraphBLAS

Copyright 2019 LAGraph Contributors.

(see Contributors.txt for a full list of Contributors; see
ContributionInstructions.txt for information on how you can Contribute to
this project).

All Rights Reserved.

NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

Released under a BSD license, please see the LICENSE file distributed with
this Software or contact permission@sei.cmu.edu for full terms.

Created, in part, with funding and support from the United States
Government.  (see Acknowledgments.txt file).

This program includes and/or can make use of certain third party source
code, object code, documentation and other files ("Third Party Software").
See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// Contributed by Tim Davis, Texas A&M and Gabor Szarnyas, BME

// usage:
// p3test < in > out

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL                        \
{                                               \
    if (P != NULL) { free (P) ; P = NULL ; }    \
    GrB_free (&A) ;                             \
    GrB_free (&PR) ;                            \
    GrB_free (&A_fp32) ;                        \
}

int main ( )
{

    GrB_Info info ;
    GrB_Matrix A = NULL ;
    GrB_Matrix A_fp32 = NULL ;
    LAGraph_PageRank *P = NULL ;
    GrB_Vector PR = NULL;

    LAGRAPH_OK (LAGraph_init ( )) ;

    int nthreads_max = LAGraph_get_nthreads ( ) ;
    LAGraph_set_nthreads (1) ;

    //--------------------------------------------------------------------------
    // read in a matrix from a file and convert to boolean
    //--------------------------------------------------------------------------

    double tic [2] ;
    LAGraph_tic (tic) ;

    // read in the file in Matrix Market format
    LAGRAPH_OK (LAGraph_mmread (&A, stdin)) ;
    // GxB_fprint (A, GxB_SHORT, stdout) ;
    // LAGraph_mmwrite (A, stdout) ;

    // convert to FP32
    LAGRAPH_OK (LAGraph_pattern (&A_fp32, A, GrB_FP32)) ;
    // LAGraph_mmwrite (A_fp32, stdout) ;
    GrB_free (&A) ;
    A = A_fp32 ;
    A_fp32 = NULL ;
    LAGRAPH_OK(GxB_set (A, GxB_FORMAT, GxB_BY_COL));
    // GxB_fprint (A, GxB_COMPLETE, stdout) ;

    // finish any pending computations
    GrB_Index nvals ;
    GrB_Matrix_nvals (&nvals, A) ;

    //--------------------------------------------------------------------------
    // get the size of the problem.
    //--------------------------------------------------------------------------

    GrB_Index nrows, ncols ;
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows, A)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols, A)) ;
    GrB_Index n = nrows ;

    // LAGRAPH_OK (GrB_Matrix_setElement (A, 0, 0, n-1)) ;     // hack

    printf ("\n=========="
            "input graph: nodes: %"PRIu64" edges: %"PRIu64"\n", n, nvals) ;

    double tread = LAGraph_toc (tic) ;
    printf ("read time: %g sec\n", tread) ;

    GxB_fprint (A, GxB_SHORT, stdout) ;

    //--------------------------------------------------------------------------
    // compute the pagerank
    //--------------------------------------------------------------------------

    int ntrials = 1 ;       // increase this to 10, 100, whatever, for more
    // accurate timing

    float tol = 1e-4 ;
    int iters, itermax = 100 ;

    //    #define NTHRLIST 7
    //    int nthread_list [NTHRLIST] = {1, 2, 4, 8, 16, 20, 40} ;

#define NTHRLIST 2
    int nthread_list [NTHRLIST] = {1, 40} ;    

    //#define NTHRLIST 1
    //    int nthread_list [NTHRLIST] = {40} ;    

    //uncomment the one that you want to run
    printf ("Testing pagerank3a (slower than 3b)\n") ;
//  printf ("Testing pagerank3b (fast version)\n") ;

    for (int kk = 0 ; kk < NTHRLIST; kk++)
    {
        int nthreads = nthread_list [kk] ;
        LAGraph_set_nthreads (nthreads) ;

        // start the timer
        LAGraph_tic (tic) ;

        for (int trial = 0 ; trial < ntrials ; trial++)
        {
            GrB_free (&PR) ;
            //uncomment the one that you want to run
            LAGRAPH_OK (LAGraph_pagerank3a (&PR, A, 0.85, itermax, &iters)) ;
//          LAGRAPH_OK (LAGraph_pagerank3b (&PR, A, 0.85, itermax, &iters)) ;
        }

        // stop the timer
        double t1 = LAGraph_toc (tic) / ntrials ;
        printf ("pagerank  time: %12.6e (sec), "
                "rate: %g (1e6 edges/sec) iters: %d threads: %d\n",
                t1, 1e-6*((double) nvals) / t1, iters, nthreads) ;
    }

    //--------------------------------------------------------------------------
    // print results
    //--------------------------------------------------------------------------

    /*
       for (int64_t k = 0 ; k < n ; k++)
       {
       printf ("%" PRIu64 " %g\n", P [k].page, P [k].pagerank) ;
       }
       */
    GxB_Vector_fprint(PR, "---- PR ------", GxB_SHORT, stdout);

    //--------------------------------------------------------------------------
    // free all workspace and finish
    //--------------------------------------------------------------------------

    LAGRAPH_FREE_ALL ;
    LAGRAPH_OK (LAGraph_finalize ( )) ;
    return (GrB_SUCCESS) ;
}