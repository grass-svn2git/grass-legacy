/****** path_finder.c ***********************************************
	
	This recursive function traces the least cost path backwards
	to cells from which the cumulative cost was determined 

*********************************************************************/
#include "segment.h"

path_finder (row, col, backrow, backcol)

int row, col, backrow, backcol;

{
	int data, new_backrow, new_backcol;
	char buf[400];
	extern char *value;
	extern int nrows, ncols;
	extern SEGMENT in_row_seg, in_col_seg, out_seg;

	if (row < 0 || row >= nrows || col < 0 || col >= ncols)
		return;			/* outside the window*/

	/* if the pt has already been traversed, return	*/
	value = (char *) &data;
	segment_get(&out_seg, value, row, col);
	if(data == 1) return;		/* already traversed */

	/* otherwise, draw a line on output*/
	drawline(row, col, backrow, backcol);
/*DEBUG
printf("\nrow=%d, col=%d, backrow=%d, backcol=%d", row, col, backrow, backcol);
*/
	/* update path position */
	if (row == backrow && col == backcol) {printf("\n");return;}/* reach an origin */

	value = (char *) &new_backrow;
	segment_get(&in_row_seg, value, backrow, backcol);
	value = (char *) &new_backcol;
	segment_get(&in_col_seg, value, backrow, backcol);

	path_finder (backrow, backcol, new_backrow, new_backcol);
	return;
}

