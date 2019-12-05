#ifndef AUTO_H
#define AUTO_H

#ifndef MAX_STATES
#   define MAX_STATES 50
#endif

#ifndef AB_SIZE
#   define AB_SIZE 128	/* ASCII size */
#endif

typedef  int  state_t;

/*
 |  State attributes are one of:
 |
 |	'A'	Accept state
 |	'D'	Dead state
 |	NULL	Other
 |
 |	State 0 (1 in the internal representation)
 |	is always assumed to be the initial state.
 */
typedef struct {
	int	nstates;			/* number of states         */
	int	nab;				/* alphabet size            */
	state_t	mat[MAX_STATES+1][AB_SIZE+1];	/* state transition matrix  */
	state_t init_state;                 	/* initial state            */
	state_t	accept[MAX_STATES + 1];		/* accept states            */
	char	state_attrib[MAX_STATES + 1]; 	/* state attributes         */
} automaton_t;


#define TRUE 1
#define FALSE 0

#ifdef MSDOS
#  include <stdlib.h>
#endif
#define Abort(ARGS) ( printf ARGS , exit (1) )

#include <stdio.h>

#endif
