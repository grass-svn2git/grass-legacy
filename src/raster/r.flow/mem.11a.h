/************************** MEMORY MANAGEMENT ***************************/

#include "gis.h"
#define KB 1024
#define MB (KB * KB)
#define SEGSINMEM 9
#define SEGCOLS ((int) (region.cols/3) + 1)
#define SEGROWS ((int)(MB/region.cols/3) <= 1 ? 1 : (int)(MB/region.cols/3))

CELL v;			/* address for segment retrieval macros */

/*
 * initialize_globals: self-explanatory
 * globals w: parm, region, el, as, ds, asp, put, get
 */

void
initialize_globals(/* argc, argv */);

/*
 * allocate_heap: allocate and initialize matrices, cell buffers, headers
 * globals r: parm, region
 * globals w: fl, density, ew_dist, el, as, ds
 */

void
allocate_heap();

/* 
 * deallocate_heap: frees space for other processes, closes cts output files
 * globals r: parm, bitbar, lgfd, el, as, ew_dist
 */

void
deallocate_heap();

void
put_row_seg(/* l, row */);

#define get_row(l, row) \
    ((parm.seg && (segment_flush(l.seg) < 1 || \
		   segment_get_row(l.seg, l.buf[row] - l.col_offset, \
				          row + l.row_offset) < 1)) ? \
	(sprintf(string, "r.flow: cannot write segment file for %s", l.name),\
	 G_fatal_error(string), (CELL *) NULL) : \
	l.buf[row])

#define aspect(row, col) \
    (parm.seg ? \
	(segment_get(as.seg, &v, \
			row + as.row_offset, col + as.col_offset) < 1 ? \
	  (sprintf(string,"r.flow: cannot read segment file for %s",as.name), \
	   G_fatal_error(string)) : \
	  v) : \
	(parm.mem ? \
	   aspect_fly(el.buf[row - 1] + col, \
			  el.buf[row] + col, \
			  el.buf[row + 1] + col, \
			  ew_dist[row]) : as.buf[row][col]))

#define get(l, row, col) \
    (parm.seg ? \
	(segment_get(l.seg, &v, row + l.row_offset, col + l.col_offset) < 1 ? \
	  (sprintf(string,"r.flow: cannot read segment file for %s",l.name),\
	   G_fatal_error(string)) : \
	 v) : \
	l.buf[row][col])

#define put(l, row, col, w) \
    (parm.seg ? \
	(v = w, \
	 segment_put(l.seg, &v, row + l.row_offset, col + l.col_offset) < 1 ? \
	  (sprintf(string,"r.flow: cannot write segment file for %s",l.name), \
	   G_fatal_error(string)) : \
	 0) : \
	(l.buf[row][col] = w))

