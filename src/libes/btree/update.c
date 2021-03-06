#include <stdio.h>
#include "btree.h"

char *malloc();
char *realloc();

btree_update (B, key, keylen, data, datalen)
    BTREE *B;
    char *key;
    char *data;
{
    register int p,q;
    int N;
    int (*cmp)();
    int dir;
    char *store();

/* first node is special case */
    N = B->N;
    if (N == 0)
    {
	N = B->N = 1;
	B->node[N].key = store (key, keylen);
	B->node[N].data = store (data, datalen);
	B->node[N].left = 0;
	B->node[N].right = 0;
	if (B->node[N].key && B->node[N].data)
	    return 1;
	else
	    return 0;
    }

    cmp = B->cmp;

    q = 1;
    while (q > 0)
    {
	p = q;
	dir = (*cmp)(B->node[q].key, key) ;
	if (dir == 0)
	{
	    free (B->node[q].data);
	    if(B->node[q].data = store (data, datalen))
		return 1;
	    else
		return 0;
	}
	if (dir > 0)
	    q = B->node[q].left;             /* go left */
	else
	    q = B->node[q].right;            /* go right */
    }

/* new node */
    B->N++;
    N = B->N;

/* grow the tree? */
    if (N >= B->tlen)
    {
	B->node = (BTREE_NODE *) realloc (B->node, sizeof(BTREE_NODE) * (B->tlen += B->incr));
	if (B->node == NULL)
	    return 0;
    }

/* add node to tree */
    B->node[N].key = store (key, keylen);
    B->node[N].data = store (data, datalen);
    B->node[N].left = 0;

    if (dir > 0)
    {
	B->node[N].right = -p;            /* create thread */
	B->node[p].left  = N;             /* insert left */
    }
    else
    {
	B->node[N].right = B->node[p].right; /* copy right link/thread */
	B->node[p].right = N;                /* add right */
    }
    return 1;
}

static
char *
store (s, n)
    char *s;
{
    char *b,*c;

    if (n <= 0)
	return (b = NULL);
    c = b = malloc (n);
    if (b == NULL)
	return b;
    while (n-- > 0)
	*b++ = *s++;
    return c;
}
