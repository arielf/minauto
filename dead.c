/*-------------------------------------------------------------------------*\
|  Module "dead.c"
|
|  Find out and mark as dead all the DFA states which are either
|       unreachable from the initial state
|  or
|       cannot reach any accept-state
|
|  The method used is to calculate the full transitive-closure of each
|  state using Warshall algorithm.
\*-------------------------------------------------------------------------*/

#include "auto.h"

/*
 |  The full transitive-closure matrix:
 |	connected[i][j] == TRUE   iff   j is reachable from i
 |  Initially all entries are FALSE
 */
static char   connected[MAX_STATES+1][MAX_STATES+1];

static void   init_connections ();
static void   t_closure ();

/*-------------------------------------------------------------------------
|  void  find_dead_states (dfa)
|  automaton_t  *dfa;
|
|  Mark the following DFA states as dead states:
|	1. States not reachable from the initial state.
|	2. States not reaching an accept state.
`------------------------------------------------------------------------*/

void find_dead_states (dfa)
automaton_t  *dfa;
{
    state_t	i, j, accept_st;
    char	attrib;

    init_connections(dfa->mat, dfa->nstates, dfa->nab);
    t_closure(dfa->nstates);

    /* Mark all the states not reachable from s0 (initial state) as dead */
    for (i = 1; i <= dfa->nstates; i++)
	if (! connected[dfa->init_state][i]) {
	    dfa->state_attrib[i] = 'D';
	}

    /* Mark all states not reaching an accept state as dead */
    for (i = 1; i <= dfa->nstates; i++) {
	attrib = dfa->state_attrib[i];
	if (attrib == 'D' || attrib == 'A')
	    continue;

	/* Loop over accept-states */
	for (j = 0; (accept_st = dfa->accept[j]) != 0; j++)
	    if (connected[i][accept_st]) /* i reaches an accept-state */
		break;                   /* no more checking needed   */

	if (accept_st == 0) {            /* All accept states scanned     */
	    dfa->state_attrib[i] = 'D';  /* and none was reachable from i */
	}
    }
}

/*-------------------------------------------------------------------------
|  static void init_connections (transitions_mat, nstates, nab)
|  state_t  transitions_mat[][AB_SIZE + 1];
|  int  nstates;
|  int  nab;
|
|  Initialize the 'connected[][]' matrix according to 'transitions_mat[][]'.
`------------------------------------------------------------------------*/

static void init_connections (transitions_mat, nstates, nab)
state_t  transitions_mat[MAX_STATES + 1][AB_SIZE + 1];
int  nstates;
int  nab;
{
    state_t	src, dest, i;

    for (src = 1; src <= nstates; src++) {

	for (dest = 1; dest <= nstates; dest++)
	    connected[src][dest] = FALSE;       /* clear 'src' connections */

	connected[src][src] = TRUE;
	for (i = 1; i <= nab; i++) {
	    if ((dest = transitions_mat[src][i]) > 0)
		/* 'src' goes to 'dest' on alphabet symbol 'i' */
		connected[src][dest] = TRUE;
	}
    }
}


/*-------------------------------------------------------------------------
|  static void t_closure (nstates)
|  int  nstates;
|
|  Compute the transitive closure of the 'connected[][]' matrix.
|  Method: S. Warshall algorithm (See Sedgewick, Algorithms chap. 32)
`------------------------------------------------------------------------*/

static void t_closure (nstates)
int  nstates;
{
    state_t	i, j, k;

    for (i = 1; i <= nstates; i++) {
	for (j = 1; j <= nstates; j++) {
	    if (connected[j][i]) {
		for (k = 1; k <= nstates; k++) {
		    if (connected[i][k]) {
			connected[j][k] = TRUE;
		    }
		}
	    }
	}
    }
}

