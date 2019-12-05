/*-------------------------------------------------------------------------*\
|  Module "partit.c"
|
|  Handles partitions of equivalence-classes into finer-grain (smaller)
|  classes according to the same-transition criterion: two states should
|  stay in the same equivalence-class if and only if they both have
|  exactly the same transitions (i.e. they both go to the same equivalence-
|  class on every possible alphabet symbol). For further details see
|  Aho & Ullman's "Principles of Compiler design" - DFA Minimization.
|
|  Partitioning a group of states into equivalence-classes is done
|  much more efficiently than the straight-forward way by two stages:
|
|      1. First split the group into singleton subgroups (each subgroup
|         consists of a single member) and then
|
|      2. Use R.E. Tarjan fast Union-Find algorithm to merge singleton
|         groups into equivalence-classes.
\*-------------------------------------------------------------------------*/
#include	"auto.h"

extern state_t   find ();
extern void      Union ();

#ifdef DEBUG
  extern void dump_state ();
#endif

/*-------------------------------------------------------------------------
|  static int update_partitions (group_size, member, old_groups, new_groups)
|  int		group_size;
|  state_t	member[], old_groups[], new_groups[];
|
|  1. scans all members of 'member[]' group and checks if the Union-Find
|     arrays 'old_groups[]' & 'new_groups[]' are partitioned equivalently
|     with respect to the (partial) group 'member[]'.
|  2. If partitions are not equivalent - updates 'old_groups[]' according
|     to 'new_groups[]' (only in the states of 'member[]') and returns TRUE.
|     Otherwise - no updating is done and FALSE is returned.
`------------------------------------------------------------------------*/

static  int  update_partitions (group_size, member, old_groups, new_groups)
int group_size;
state_t member[], old_groups[], new_groups[];
{
    state_t	i, state;
    state_t	old_rep, new_rep;
    int		update = FALSE;

    for (i = 0; i < group_size; i++) {
	state = member[i];
	old_rep = find(state, old_groups);
	new_rep = find(state, new_groups);
	if (old_rep != new_rep) {
	    update = TRUE;  /* at least one transition is different */
	    break;
	}
    }
    if (update) {
	for (i = 0; i < group_size; i++) {
	    state = member[i];
	    old_groups[state] = new_groups[state];
	}
	return TRUE;	    /* partitions updated */
    } else
	return FALSE;	    /* no updating of partitions */
}


/*-------------------------------------------------------------------------
|  void  init_partitions (nstates, attribs, groups)
|  int      nstates;
|  char     attribs[];
|  state_t  groups[];
|
|  Initialize the Union-Find array 'groups[]' with two disjoint equivalence
|  groups according to the state attributes 'attribs[]' of a DFA:
|	a. The accept states
|	b. All states that are not accept states
`------------------------------------------------------------------------*/

void  init_partitions (nstates, attribs, groups)
int      nstates;
char     attribs[];
state_t  groups[];
{
    state_t	   accept_rep = 0; /* representative state of accept-states */
    state_t	   others_rep = 0; /* representative state of other states  */
    state_t	   i;

    for (i = 1; i <= nstates; i++) /* clear 'groups[]' array         */
	groups[i] = 0;             /* i.e. make its members disjoint */

    for (i = 1; i <= nstates; i++) {
	if (attribs[i] == 'A') {
	    if (accept_rep == 0) { /* no representative yet */
		accept_rep = i;
	    } else
		Union(accept_rep, i, groups);
	} else {
	    if (others_rep == 0) { /* no representative yet */
		others_rep = i;
	    } else
		Union(others_rep, i, groups);
	}
    }
}


/*-------------------------------------------------------------------------
|  static  int  same_transitions (s1, s2, mat, nab, groups)
|  state_t   s1, s2;
|  state_t   mat[MAX_STATES + 1][AB_SIZE + 1];
|  int       nab;
|  state_t   groups[];
|
|  Return TRUE if the two states 's1' and 's2' have equivalent transitions
|  according to the transition-matrix 'mat[][]', FALSE otherwise.
|  'nab' is the number of symbols in the alphabet.
|  'groups[]' is the current partition of the states into equivalence-
|  classes.
`------------------------------------------------------------------------*/

static  int  same_transitions (s1, s2, mat, nab, groups)
state_t  s1, s2;
state_t  mat[MAX_STATES + 1][AB_SIZE + 1];
int      nab;
state_t  groups[];
{
    state_t	   i, transition1, transition2;

    for (i = 1; i <= nab; i++) { /* Loop over Alphabet symbols */

	transition1 = mat[s1][i];
	transition2 = mat[s2][i];

	if (transition1 > 0)   /* find the representative of transition1 */
	    transition1 = find(transition1, groups);

	if (transition2 > 0)   /* find the representative of transition2 */
	    transition2 = find(transition2, groups);

	if (transition1 != transition2)
	    return FALSE;
    }

    return TRUE;    /* All transitions of s1 & s2 were equivalent */
}


/*-------------------------------------------------------------------------
|  int  partition (dfa, old_groups)
|  automaton_t   *dfa;
|  state_t       old_groups[];
|
|  Partition the Union-Find array 'old_groups[]' to finer-grain partitions
|  according to the transitions of the initial partition.
|  (i.e. if members of a group in a partition go to different groups on
|   at least one input symbol - the group is partitioned such that these
|   members no longer lie in the same group).
|  'dfa' point to the DFA according to which partition is done.
|  Return TRUE iff the initial partition was further partitioned,
|  Otherwise - FALSE
`------------------------------------------------------------------------*/

int  partition (dfa, old_groups)
automaton_t   *dfa;
state_t       old_groups[];
{
    state_t	member[MAX_STATES + 1];      /* single-group members */
    state_t	new_groups[MAX_STATES + 1];  /* temporary Union-Find array */
    char	unified[MAX_STATES + 1];     /* flags to mark unified states */
    state_t	rep;                         /* group representative */
    int		updated = FALSE;
    int		group_size, nstates = dfa->nstates;
    int		i, j;

    for (rep = 1; rep <= nstates; rep++) {
	if (old_groups[rep] >= 0)     /* skip non-representatives */
	    continue;                 /* and groups with cardinality 1 */

	/*
	 | else - found a group representative:
	 |	1. Fill 'member[]' with all members of this group
	 |	2. Build a temporary disjoint group with these members
	 |	   in 'new_groups[]'
	 */
	group_size = 0;
	for (i = 1; i <= nstates; i++) {
	    if (find(i, old_groups) == rep) {   /* found a group member */
		member[group_size++] = i;
		new_groups[i] = 0;
		unified[i] = FALSE;
	    }
	}

	/*
	 |  For every pair of members member[i] & member[j] in the temporary
	 |  disjoint group check iff they have the same transitions according
	 |  To the main partition 'old_groups[]':
	 |	If so: Unify them (in the temporary group) into one group
	 |
	 |  Every already treated member[i] is marked by setting:
	 |
	 |	unified[member[i]] = TRUE
	 |
	 |  This is done for efficiency since all members that have the same
	 |  transitions as member[i] are unified with member[i] in the Ith
	 |  pass through the loop so there's no use in trying to check them
	 |  further. (only those members of the disjoint group who are still
	 |  "ununified" need to be checked among themselves).
	 */
	for (i = 0; i < group_size - 1; i++) {
	    if (unified[member[i]])
		continue;

	    unified[member[i]] = TRUE;
	    /*
	     |  Since the equivalence relation is symmetric it is
	     |  sufficient to check i & j pairs only when i < j
	     */
	    for (j = i + 1; j < group_size; j++) {
		if (unified[member[j]])
		    continue;
		if (same_transitions(member[i], member[j], dfa->mat, dfa->nab, old_groups)) {
		    Union(member[i], member[j], new_groups);
		    unified[member[j]] = TRUE;
		}
	    }
	}

#if DEBUG > 0
	dump_state (dfa, old_groups);
#endif
	if (update_partitions(group_size, member, old_groups, new_groups)) {
	    updated = TRUE;
	}
    }
    return (updated);
}

