/*-------------------------------------------------------------------------*\
|  Module "inout.c"
|
|  1. Handle input from file format to internal DFA represenation
|  2. Handle output of internally stored DFA to human readable form
|
|  --- DFA Input file format ---
|  Input consist of integers to represent states and characters
|  to represent alphabet-symbols.
|
|  The expected format is:
|               +----------------+
|               |  NSTATES  NAB  |
|               |  L1 L2 ... Ln  |
|               |  Si Sj ... Sk  |
|               |		 |
|               |     .		 |
|               |     .		 |
|               |     .		 |
|               |		 |
|               |  Sw Sz ... Sy	 |
|               |  A1 A2 ... Am  |
|               +----------------+
|  Where:
|
|  NSTATES =  Number of states (nonnegative integer)
|  NAB     =  Number of alphabet symbols [Alphabet size]    (ditto)
|
|  Lx      =  A letter (symbol) of the alphabet
|             (A readable, nonwhite, ASCII character. e.g. a letter)
|  Sx      =  A state (nonnegative integers for valid states or -1 for
|             dead or illegal states)
|  Ax      =  An accept state (nonnegative integer)
|
|  The first state, S0, is assumed to be the initial state.
|
|  Letters and states may be separated by any amount of white space
|  (blanks, tabs, newlines, or formfeeds)
|
|  The number of states in the input (not including the last line of
|  accept states) must equal the product NSTATES * NAB
|  and the states should represent the state-transition matrix of
|  the DFA (i.e. the state Sij in line i, column j is the state to
|  which a transition from Si occurs on input symbol j, where symbol j
|  signifies the alphabet symbol (letter) which appears in column j
|  above the matrix of state transitions.
\*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "auto.h"

/*
 |  Attribute codes are:
 |	'A' - Accept state
 |	'D' - Dead state
 */
#define IS_ACCEPT(S) (dfa->state_attrib[S] == 'A')
#define IS_DEAD(S) (dfa->state_attrib[S] == 'D')

#define ATTRIB(S) (dfa->state_attrib[S] == '\0' ? 's' : dfa->state_attrib[S])

static char ab_map[AB_SIZE + 1]; /* Serial number mapping to Alphabet symbols */

/*-------------------------------------------------------------------------
|  void input_dfa (dfa)
|  automaton_t *dfa;
|
|  Inputs a DFA into an internal structure 'dfa'
|  Input is assumed to be correct and meaningful
|  (Only partial checks are performed).
`------------------------------------------------------------------------*/
void  input_dfa (dfa)
automaton_t  *dfa;
{
    int         nstates, nab, j;
    state_t     i, s;
    char        c[2];

    if (scanf("%d %d", &nstates, &nab) != 2)
	Abort(("Input must begin with no_of_states alphabet_size\n"));

    if (nstates < 1)
	Abort(("Nonsensible number of states (%d)\n", nstates));

    if (nstates > MAX_STATES)
	Abort(("Number of states (%d) too large, recompile with \"-DMAX_STATES=%d\"\n",
	       nstates, nstates));

    if (nab < 1)
	Abort(("Nonsensible number of alphabet symbols (%d)\n", nab));

    if (nab > AB_SIZE)
	Abort(("Number of states (%d) too large, recompile with \"-DAB_SIZE=%d\"\n",
	       nab, nab));

    dfa->nstates = nstates;
    dfa->nab = nab;
    dfa->init_state = 1;	/* internal representation of state 0 */

    /* read-in alphabet symbols */
    for (j = 1; j <= nab; j++) {
	if (scanf("%1s",c) == 1) {
	    ab_map[j] = c[0];
	} else
	    Abort(("Bad input while reading alphabet\n"));
    }

    /* read-in state-transition matrix + clear attributes */
    for (i = 1; i <= nstates; i++) {
	dfa->state_attrib[i] = '\0';	/* initialize attributes */
	for (j = 1; j <= nab; j++) {
	    if (scanf("%d",&s) != 1)
		Abort(("Bad input while reading states\n"));
	    else {
		if (s >= nstates)
		    Abort(("State (%d) - out of range\n",s));
		else
		    dfa->mat[i][j] = (s >= 0) ? s + 1 : 0 ;
	    }
	}
    }
    /* Read in list of accept-states */
    i = 0;
    while (scanf("%d",&s) != EOF) {
	if (s < 0 || nstates <= s)
	    Abort(("Accept state (%d) - out of range\n", s));
	else {
	    dfa->state_attrib[s + 1] = 'A';
	    dfa->accept[i++] = s + 1;
	}
    }
    dfa->accept[i] = 0;	  /* mark end of accept states */
}

/*-------------------------------------------------------------------------
|  void  output_dfa (dfa)
|  automaton_t  *dfa;
|
|  Print out the DFA 'dfa' in human readable form.
|  Regular states are marked as "sN".
|  Accept states  are marked by "AN".
`------------------------------------------------------------------------*/

void  output_dfa (dfa)
automaton_t  *dfa;
{
    int       j, empty = TRUE;  /* initially assume the automaton is empty */
    state_t   i, s;

    if (dfa->nstates == 0) {
        printf("Empty DFA\n");
	return;
    }

    printf("%9s","");

    for (j = 1; j <= dfa->nab; j++)
	printf("%-5c",ab_map[j]);

    putchar('\n');

    for (i = 1; i <= dfa->nstates; i++) {

	/* skip dead states */

	if (IS_DEAD(i))
	    continue;

	empty = FALSE;   /* At least one 'real' state is not dead */

	printf("\n%c%-8d", ATTRIB(i), i - 1);
	for (j = 1; j <= dfa->nab; j++) {
	    s = dfa->mat[i][j];
	    if (s <= 0 || IS_DEAD(s)) {
		/* No transition from state i on symbol j */
		printf("%-5c", '-');
	    } else {
		printf("%c%-4d", ATTRIB(s), s - 1);
	    }
	}
    }
    if (empty)
	printf("DFA minimized to EMPTY DFA...\n");
    else
	printf("\n\nInitial state: %c%d\n", ATTRIB(dfa->init_state), dfa->init_state - 1);
    
}

