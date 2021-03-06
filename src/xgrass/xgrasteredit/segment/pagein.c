/* %W% %G% */

#include <stdio.h>
#include "segment.h"

segment_pagein (SEG,n)
    SEGMENT *SEG;
{
    int cur,ret;

/* JNK 07/17/91 -- changed to make the old first part a subroutine
                   named segment_inmem() which finds or frees a RAM
                   segment-slot.  If segment_inmem() returns a slot
                   whose file segment number is the one requested,
                   then no page-in is necessary.  Also the old select(),
                   which was local to this file was made external and
                   renamed segment_select().                                  */

    if((cur=segment_inmem(SEG,n))>=(SEG->nseg)) return -1;/* error paging-out */
    if(SEG->scb[cur].n == n) return cur;/* segment already in memory */
/* JNK 07/17/91 -- end of change */

/* inmem found or made a free slot (at index cur), so read in the segment */
    SEG->scb[cur].n = n;
    SEG->scb[cur].dirty = 0;
    segment_seek (SEG, SEG->scb[cur].n, 0);

    if (ret=read (SEG->fd, SEG->scb[cur].buf, SEG->size) != SEG->size)
    {
	fprintf (stderr,"pagein - read error fd=%d seg.n=%d size=%d ret=%d\n",
           SEG->fd,SEG->scb[cur].n,SEG->size,ret);
	return -1;
    }

/* JNK 07/17/91 -- changed to refer to the external routine */
    return segment_select (SEG,cur);
/* JNK 07/17/91 -- end of change */
}
