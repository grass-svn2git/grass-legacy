#include "gis.h"

#define INCR 10
#define NCATS 100

typedef struct
{
    int idx;
    char cat[NCATS];
    int left;
    int right;
} NODE;

static NODE *tree = 0 ;  /* tree of values */
static int tlen ;               /* allocated tree size */
static int N;                   /* number of actual nodes in tree */

plant_tree()
{
    N = 0;
    if (!tree)
    {
	tlen = INCR;
	tree = (NODE *) G_malloc (tlen * sizeof (NODE));
    }
}

add_node_to_tree (cat)
    register CELL cat;
{
    register int p,q;
    int idx, offset;

    if (cat < 0)
    {
	idx = -(-cat/NCATS) - 1;
	offset = cat - idx*NCATS - 1;
    }
    else
    {
	idx = cat/NCATS;
	offset = cat - idx*NCATS;
    }
    if (offset < 0 || offset >= NCATS)
	printf ("OOPS: cat %ld got offset %d - shouldn't happen\n",(long)cat,offset);
/* first node is special case */
    if (N == 0)
    {
	N = 1;
	G_zero (tree[N].cat, sizeof (tree[N].cat));
	tree[N].idx = idx;
	tree[N].cat[offset] = 1;
	tree[N].left = 0;
	tree[N].right = 0;
	return;
    }

    q = 1;
    while (q > 0)
    {
	p = q;
	if (tree[q].idx == idx)
	{
	    tree[q].cat[offset] = 1;
	    return;                       /* found */
	}
	if (tree[q].idx > idx)
	    q = tree[q].left;             /* go left */
	else
	    q = tree[q].right;            /* go right */
    }

/* new node */
    N++;

/* grow the tree? */
    if (N >= tlen)
	tree = (NODE *) G_realloc (tree, sizeof(NODE) * (tlen += INCR));

/* add node to tree */
    G_zero (tree[N].cat, sizeof (tree[N].cat));
    tree[N].idx = idx;
    tree[N].cat[offset] = 1;
    tree[N].left = 0;

    if (tree[p].idx > idx)
    {
	tree[N].right = -p;            /* create thread */
	tree[p].left  = N;             /* insert left */
    }
    else
    {
	tree[N].right = tree[p].right; /* copy right link/thread */
	tree[p].right = N;             /* add right */
    }
}

static int curp;
static int curoffset;

first_node ()
{
    int q;

/* start at root and go all the way to the left */
    curp = 1;
    while (q = tree[curp].left)
	curp = q;
}

next_node ()
{
    int q;

/* go to the right */
    curp = tree[curp].right;

    if (curp == 0)          /* no more */
	return 0;

    if (curp < 0)           /* thread. stop here */
    {
	curp = -curp ;
	return 1;
    }

    while (q = tree[curp].left)   /* now go all the way left */
	curp = q;

    return 1;
}

#ifdef COMMENT_OUT
first_cat (cat)
    CELL *cat;
{
    first_node();
    curoffset = -1;

    return next_cat (cat);
}

next_cat (cat)
    CELL *cat;
{
    int idx;
    for(;;)
    {
	curoffset++;
	if (curoffset >= NCATS)
	{
	    if(!next_node())
		return 0;
	    curoffset = -1;
	    continue;
	}
	if (tree[curp].cat[curoffset])
	{
	    idx = tree[curp].idx;

	    if (idx < 0)
		*cat = idx*NCATS + curoffset+1;
	    else
		*cat = idx*NCATS + curoffset;

	    return 1;
	}
    }
}
#endif
