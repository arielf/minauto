/*--------------------------------------------------------------------------*\
|  DFA Minimization
|
|  Synopsis:
|
|             minauto   [ dfa_1 ... dfa_N ]
|
|    Where each 'dfa_i' is a filename containing a DFA description.
|    When no arguments are given - standard input is assumed.
|
|  Input:
|
|    Any file with a DFA in transition table representation
|    (for a detailed input description refer to the module "inout.c")
|
|  Output:
|
|    An equivalent minimal DFA (i.e. a DFA which accepts the same
|    language but with a minimal number of states).
|
|  Algorithm:
|
|    Generally - as outlined in Aho & Ullman "Principles of Compiler design"
|
|    Partition is done using Robert Tarjan's fast Union-Find algorithm.
|
|    Additional optimization to Aho & Ullman's algorithm is achieved by
|    allowing partial partitions (partitions not necessarily of the whole
|    group of states) to take effect immediately as they are discovered
|    and not only after an entire partition iteration is completed on all
|    the groups of the previous iteration.
|
|    Dead states are discovered using Warshall's transitive-closure
|    algorithm.
|
|  Notes:
|    Due to implementation convenience the states are assumed to be
|    numbered from 0 to N-1 with the internal representation of this
|    range being mapped to the range 1 to N.
|
|  Design outline:
|
|    Module "main.c"    -   Main program.
|    Module "ufind.c"   -   Union-Find functions.
|    Module "partit.c"  -   Initialize partitions and partition iteration.
|    Module "dead.c"    -   Find dead-states (transitive closure) functions.
|    Module "inout.c"   -   DFA-input and DFA-output functions.
\*--------------------------------------------------------------------------*/


#include  <stdio.h>
#include  "auto.h"

static void     process_file ();
static void     minimize_dfa ();
static void     compress_dfa ();

void            input_dfa ();
void            output_dfa ();
void            find_dead_states ();

#if DEBUG > 0
  void dump_state ();
#endif

extern state_t   find ();


static automaton_t   in_dfa;	/* Input DFA  */
static automaton_t   out_dfa;	/* Output DFA */

/*
 |  The partition into equivalence-classes or groups (Union-Find) array
 |  (see module "ufind.c" for details)
 */
static state_t   groups[MAX_STATES + 1];

/*-------------------------------------------------------------------------
|  main (argc, argv)
|  int   argc;
|  char  *argv[];
|
|  Main program - each argument is a DFA description file.
|  when no arguments standard input is processed
`------------------------------------------------------------------------*/

main (argc, argv)
int  argc;
char *argv[];
{
    int    i;

    if (argc > 1) {            /* Handle arguments one by one */
	for (i = 1; i < argc; i++) {
	    process_file(argv[i]);
	}
    } else                     /* no arguments */
	process_file(NULL);   /* process standard input */

    return 0;
}

/*-------------------------------------------------------------------------
|  static void process_file (filename)
|  char   *filename;
|
|  Process an argument file. a NULL argument means standard input is
|  to be processed, otherwise - standard input is substituted by the
|  argument file
`------------------------------------------------------------------------*/

static  void process_file (filename)
char    *filename;
{
    if (filename != NULL)  /* if there's need to open a file */
	if (freopen(filename, "r", stdin) != stdin) {
	    perror(filename);
	    return;
	}
    input_dfa(&in_dfa);
    printf("\n------- Original  DFA -------\n\n");
    output_dfa(&in_dfa);

    minimize_dfa(&in_dfa, &out_dfa, groups);
    printf("\n\n------- Minimized DFA -------\n\n");
    output_dfa(&out_dfa);
}

/*-------------------------------------------------------------------------
|  void  minimize_dfa (old_dfa, new_dfa, groups)
|  automaton_t  *old_dfa, *new_dfa;
|  state_t      groups[];
|
|  Minimize the DFA 'old_dfa' into 'new_dfa'
|  using the partition array 'groups[]'.
|  Algorithm according to:
|  Al Aho & Jeffrey D. Ullman - Principles of Compiler Design.
`------------------------------------------------------------------------*/

static  void  minimize_dfa (old_dfa, new_dfa, groups)
automaton_t  *old_dfa, *new_dfa;
state_t      groups[];
{
    extern   void     init_partitions ();
    extern   int      partition ();

    init_partitions(old_dfa->nstates, old_dfa->state_attrib, groups);

    /*
     |  Partition equivalence-classes of states
     |  until no further partition can be done.
     */
    while (partition(old_dfa, groups) == TRUE)
	;

    compress_dfa(old_dfa, new_dfa, groups);

    find_dead_states(new_dfa);
}

/*-------------------------------------------------------------------------
|  static  void  compress_dfa (old_dfa, new_dfa, groups)
|  automaton_t   *old_dfa, *new_dfa;
|  state_t       groups[];
|
|  Receives an old DFA and a new one. Compresses the old into the new such
|  that the new contains representatives only.
|  Since the compression process may map state names into lower-numbered
|  states - the process may not preserve the original state names.
|  'groups[]' holds the partition of the old DFA states into
|  equivalence-classes.
`------------------------------------------------------------------------*/

static  void  compress_dfa (old_dfa, new_dfa, groups)
automaton_t   *old_dfa, *new_dfa;
state_t       groups[];
{
    state_t  map[MAX_STATES + 1];   /* old->new (compressed) state mapping */
    state_t  pam[MAX_STATES + 1];   /* new->old (inverse) state mapping    */
    state_t  rep[MAX_STATES + 1];   /* Representative-states array         */
    state_t  rep_count = 0;         /* Representative-states counter       */
    state_t  a_count = 0;           /* Accept-states counter               */
    state_t  i;
    int      j;

#if DEBUG > 1
    printf("------- State Compressions -------\n");
#endif

    map[0] = pam[0] = rep[0] = 0;

    for (i = 1; i <= old_dfa->nstates; i++) {

	rep[i] = find(i, groups);   /* fill representatives array  */

	if (i == rep[i]) {          /* i is a representative state */
	    rep_count++;
	    map[i] = rep_count;     /* compressed mapping: i -> rep_count */
	    pam[rep_count] = i;     /* inverse mapping:    rep_count -> i */
#if DEBUG > 1
	    /* Compression mapping (debug printout) */
	    printf("\t%d -->> %d\n", i-1, rep_count-1);
#endif
	}
    }

    /* Fill transition matrix for compressed DFA */
    for (i = 1; i <= rep_count; i++) {

	for (j = 1; j <= old_dfa->nab; j++) {
	    new_dfa->mat[i][j] = map[ rep[ old_dfa->mat[pam[i]][j] ] ];
	}

	/* Set state attributes in new_dfa */
	new_dfa->state_attrib[i] = old_dfa->state_attrib[pam[i]];

	/* Fill list of accept-states for compressed DFA */
	if (new_dfa->state_attrib[i] == 'A') {
	    new_dfa->accept[a_count++] = i;
	}
    }

    new_dfa->nstates = rep_count;                       /* Number of states */
    new_dfa->nab = old_dfa->nab;                         /* Alphabet size   */
    new_dfa->init_state = map[rep[old_dfa->init_state]]; /* Initial state   */
}

#if DEBUG > 0
/*-------------------------------------------------------------------------
|  void  dump_state (dfa, groups)
|  automaton_t  *dfa;
|  state_t      groups[];
|
|  Dump the current transitions in 'dfa' of each state in 'dfa'.
|  according to the equivalence classes in 'groups[]'.
|  Serves for debugging purposes only.
`------------------------------------------------------------------------*/
void  dump_state (dfa, groups)
automaton_t  *dfa;
state_t      groups[];
{
    state_t  i, j, rep;    /* loop indices & representative state */
    void     dump_transitions();

    printf("------- Current partition [AB transitions] -------\n");
    for (i = 1; i <= dfa->nstates; i++) {
	if ((rep = find(i, groups)) == i) { /* a representative */
	    /* print it followed by its group members */
	    /* each one followed by its transitions   */
	    printf("%d", rep - 1);
#if DEBUG > 2
	    dump_transitions(dfa, groups, rep);
#endif
	    for (j = 1; j <= dfa->nstates; j++) {
		if (rep != j && rep == find(j, groups)) {
		    printf(" %d", j - 1);
#if DEBUG > 2
		    dump_transitions(dfa, groups, j);
#endif
		}
	    }
	    putchar('\n');
	}
    }
}

#if DEBUG > 2
/*-------------------------------------------------------------------------
|  void dump_transitions(dfa, groups, s)
|  automaton_t   *dfa;
|  state_t       groups[];
|  state_t       s;
|
|  Dump the current transitions in 'dfa' of a single state 's' (i.e. the
|  list of states to which 's' goes on each alphabet symbol)
|  each state is represented by its equivalence-class representative
|  according to 'groups[]'.
|  Serves for debugging purposes only.
`------------------------------------------------------------------------*/
void dump_transitions(dfa, groups, s)
automaton_t   *dfa;
state_t       groups[];
state_t       s;
{
    int	let;              /* letter index */
    int	nab = dfa->nab;   /* alphabet size */

    putchar('[');
    for (let = 1; let <= nab; let++)
	printf("%d ", find(dfa->mat[s][let], groups) - 1);
    printf("\b]");
}
#endif

#endif
