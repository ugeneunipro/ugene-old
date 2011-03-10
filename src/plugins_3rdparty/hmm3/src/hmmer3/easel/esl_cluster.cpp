/* Generalized single linkage clustering.
* 
* Table of contents:
*     1. Single linkage clustering, generalized
*     5. Copyright and license
*     
* SRE, Mon Jan  7 09:21:56 2008 [Janelia] [HHGTTG]
* SVN $Id: esl_cluster.c 269 2008-06-19 13:47:41Z eddys $
*/

#include <hmmer3/easel/esl_config.h>

#include <stdlib.h>

#include <hmmer3/easel/easel.h>
#include "esl_cluster.h"


/*****************************************************************
* 1. Single linkage clustering, generalized
*****************************************************************/

/* Function:  esl_cluster_SingleLinkage()
* Synopsis:  Generalized single linkage clustering.
* Incept:    SRE, Mon Jan  7 08:35:10 2008 [Janelia]
*
* Purpose:   Given a set of vertices, cluster them by single-linkage
*            clustering.
*            
*            The data describing each vertex is provided in an array
*            starting at <base>, consisting of <n> vertices. Each
*            vertex can be of any type (structure, scalar, pointer)
*            so long as each vertex element is of fixed size <n>
*            bytes.
*            
*            A pointer to the clustering function is provided in
*            <(*linkfunc)()>, and a pointer to any necessary
*            parameters for that function (for example, any
*            thresholds) is provided in <param>. 
*            
*            The <int (*linkfunc)()> must be written by the
*            caller. It takes arguments <(void *v1, void *v2, void
*            *param, int *ret_link)>: pointers to two vertices to
*            test for linkage and a pointer to any necessary
*            parameters, and it passes the answer <TRUE> (1) or
*            <FALSE> (0) back in <*ret_link>. The <(*linkfunc)()>
*            returns <eslOK> (0) on success, and a nonzero error code
*            on failure (see <easel.h> for a list of Easel's error
*            codes).
*            
*            The caller provides an allocated <workspace> with space
*            for at least <2n> integers. (Allocation in the caller
*            allows the caller to reuse memory and save
*            allocation/free cycles, if it has many rounds of
*            clustering to do.)
*            
*            The caller also provides allocated space in
*            <assignments> for <n> integers which, upon successful
*            return, contains assignments of the <0..n-1> vertices to
*            <0..C-1> clusters. That is, if <assignments[42] = 1>,
*            that means vertex 42 is assigned to cluster 1.  The
*            total number of clusters is returned in <ret_C>.
*            
*            The algorithm runs in $O(N)$ memory; importantly, it
*            does not require a $O(N^2)$ adjacency matrix. Worst case
*            time complexity is $O(N^2)$ (multiplied by any
*            additional complexity in the <(*linkfunc()> itself), but
*            the worst case (no links at all; <C=n> clusters) should
*            be unusual. More typically, time scales as about $N \log
*            N$. Best case is $N$, for a completely connected graph
*            in which all vertices group into one cluster. (More
*            precisely, best case complexity arises when vertex 0 is
*            connected to all other <n-1> vertices.)
*            
* Notes:    I don't know if this algorithm is published. I 
*           haven't seen it in graph theory books, but that might
*           be because it's so obvious that nobody's bothered.
*           
*           In brief, we're going to do a breadth-first search of the
*           graph, and we're going to calculate links on the fly
*           rather than precalculating them into a standard adjacency
*           matrix.
*           
*           While working, we keep two stacks of maximum length N:
*                a : list of vertices that are still unconnected.
*                b : list of vertices that we've connected to 
*                    in our current breadth level, but we haven't
*                    yet tested for other connections to a.
*           The current length (number of elements in) a and b are
*           kept in na, nb.
*                    
*           We store our results in an array of length N:
*                c : assigns each vertex to a component. for example
*                    c[4] = 1 means that vertex 4 is in component 1.
*                    nc is the number of components. Components
*                    are numbered from 0 to nc-1. We return c and nc
*                    to our caller.
*                    
*           The algorithm is:
*           
*           Initialisation: 
*                a  <-- all the vertices
*                na <-- N
*                b  <-- empty set
*                nb <-- 0
*                nc <-- 0
*                
*           Then:
*                while (a is not empty)
*                  pop a vertex off a, push onto b
*                  while (b is not empty)
*                    pop vertex v off b
*                    assign c[v] = nc
*                    for each vertex w in a:
*                       compare v,w. If w is linked to v, remove w
*                       from a, push onto b.
*                  nc++     
*           q.e.d. 
*
* Args:      base        - pointer to array of n fixed-size vertices to be clustered.
*            n           - number of vertices
*            size        - size of each vertex element
*            linkfunc    - pointer to caller's function for defining linked pairs
*            param       - pointer to any data that needs to be provided to <(*linkfunc)>
*            workspace   - caller provides at least 2n*sizeof(int) of workspace
*            assignments - RETURN: assignments to clusters (caller provides n*sizeof(int) space)
*            ret_C       - RETURN: number of clusters
*
* Returns:   <eslOK> on success; <assignments[0..n-1]> contains cluster assigments 
*            <0..C-1> for each vertex, and <*ret_C> contains the number of clusters
*            <C>
*
* Throws:    status codes from the caller's <(*linkfunc)> on failure; in this case, 
*            the contents of <*assignments> is undefined, and <*ret_C> is 0.
*/
int
esl_cluster_SingleLinkage(void *base, size_t n, size_t size, 
                          int (*linkfunc)(const void *, const void *, const void *, int *), void *param,
                          int *workspace, int *assignments, int *ret_C)
{
    int na, *a = NULL;		/* stack of available vertices (still unconnected)       */
    int nb, *b = NULL; 		/* stack of connected but unextended vertices            */
    int nc, *c = NULL;		/* array of results: # clusters, assignments to clusters */
    int v,w;			/* indices of vertices                                   */
    int i;			/* counter over the available list                       */
    int do_link;
    int status;

    a = workspace;
    b = workspace + n;
    c = assignments;

    for (v = 0; v < (int)n; v++) a[v] = n-v-1; /* initialize by pushing all vertices onto available list (backwards) */
    na = n;
    nb = 0;
    nc = 0;

    while (na > 0)		/* while vertices remain unexamined or unclustered: */
    {
        v = a[na-1]; na--;	/* pop a vertex off a, */
        b[nb] = v;   nb++;	/* and push it onto b  */

        while (nb > 0) 		/* while vertices remain unextended: */
        {
            v = b[nb-1]; nb--;	/* pop vertex off b        */
            c[v] = nc;		/* assign it to cluster nc */
            for (i = na-1; i >= 0; i--) /* backwards, because of deletion/swapping we do*/
            {
                if ((status = (*linkfunc)( (char *) base + v*size, (char *) base + a[i]*size, param, &do_link)) != eslOK) goto ERROR;
                if (do_link)
                {
                    w = a[i]; a[i] = a[na-1]; na--; /* delete w from a    */
                    b[nb] = w; nb++;                /* and push it onto b */
                }
            }
        }
        nc++;
    }

    *ret_C = nc;
    return eslOK;

ERROR:
    *ret_C = 0;
    return status;
}
/*------------------ end, single linkage clustering -------------*/

/*****************************************************************
* Easel - a library of C functions for biological sequence analysis
* Version h3.0; March 2010
* Copyright (C) 2010 Howard Hughes Medical Institute.
* Other copyrights also apply. See the COPYRIGHT file for a full list.
* 
* Easel is distributed under the Janelia Farm Software License, a BSD
* license. See the LICENSE file for more details.
*****************************************************************/
