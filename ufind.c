/*-------------------------------------------------------------------------*\
|  Fastest known Union-find algorithm
|
|  Source: Robert Sedgewick - Algorithms, chapter 30 (my variant)
|
|  According to R. E. Tarjan. This algorithm performs as good as
|  the lower bound for the Union-Find problem. This result is achieved
|  by using both path-compression (during Find operations) and tree
|  weight-balancing (during Union operations).
|
|  The amortized complexity for building-up a Union-Find structure is
|               O(E * a(E))
|  where E is the number of edges and a() is the inverse of Ackerman's
|  function.
|
|  The 'rep[]' (short for 'representative') Data Structure:
|
|  The 'rep[]' array which is passed as a parameter to both 'find()' and
|  'Union()' represents a partition of the elements into equivalence
|  classes.  The elements are numbered 1 through N (0 is unused). rep[i]
|  (i = 1..N) contains one of the following:
|
|     1. 0 (zero) - meaning that the element i is a singleton (no other
|        members in its class. This is how we start.
|
|     2. An id j of another element (j is the parent of i) belonging
|        to the same equivalence-class as as the element i.
|
|     3. A negative number -M meaning that i is a `root' i.e. the
|        `representative' of all the members of his equivalence-class
|        and that the number of descendant-elements for this root is M.
\*-------------------------------------------------------------------------*/

#include "auto.h"


/*-------------------------------------------------------------------------
|  state_t  find (elem, rep)
|  state_t  elem;
|  state_t  rep[];
|
|  Return the equivalence class (representative member of class)
|  of the element 'elem'
`------------------------------------------------------------------------*/

state_t  find (elem, rep)
state_t  elem;
state_t  rep[];
{
    state_t	    i, temp;

    /*
     * (a) Find root of class (class-representative) of 'elem'
     *     At the end of this process, 'i' points to the root
     */
    for (i = elem; rep[i] > 0; i = rep[i])
	;

    /*
     * (b) Perform "path compression": point all members along
     *	   the just-found path directly to the root so future
     *	   find operations are faster
     */
    while (rep[elem] > 0) {
	temp = elem;
	elem = rep[elem];
	rep[temp] = i;
    }
    /* (c) return the class representative found in (a) */
    return i;
}


/*-------------------------------------------------------------------------
|  void  Union (elem1, elem2, rep)
|  state_t  elem1, elem2;
|  state_t  rep[];
|
|  Unify the elements 'elem1' and 'elem2' to belong to the same
|  equivalence class.  Side efects: weight balancing union and
|  update of the root with the (negated) value of its equiv-class
`------------------------------------------------------------------------*/

void  Union (elem1, elem2, rep)
state_t	 elem1, elem2;
state_t  rep[];
{
    state_t   i, j;

    i = find(elem1, rep);
    j = find(elem2, rep);

    if (i != j) {   /* elem1 and elem2 are in different equivalence classes */
	/*
	 | Perform a weight balancing union of the two paths:
	 | i.e. make the shallower tree a subtree of the bigger one
	 | Note: rep[root] values are negative so smaller values
	 | represent deeper trees.  -1 is needed because we want
	 | to add (in the negative) the root itself, as well
	 */
	if (rep[j] > rep[i]) {		    /* j tree is shallower than i */
	    rep[i] += (rep[j] - 1);         /* add j to the sons of i */
	    rep[j] = i;
	    // fprintf(stderr, "union: %u weight %u\n", i, -rep[i]);
	} else {
	    rep[j] += (rep[i] - 1);         /* add i to the sons of j */
	    rep[i] = j;
	    // fprintf(stderr, "union: %u weight %u\n", j, -rep[j]);
	}
    }
}

