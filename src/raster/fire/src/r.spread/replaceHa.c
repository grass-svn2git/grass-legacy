/***********************************************************
 *
 *                 replaceHa.c (for spread)
 *  This routine is to delete a cell in a heap. 
 *  It 1) searches the cell backward and sequentially from 
 *        the heap (if not found, returns a error message), 
 *     2) repalce that cell with the new min_cost and
 *        restore a heap order.
 *
 ************************************************************/

#include "costHa.h"

replaceHa(new_min_cost, angle, row, col, heap, heap_len)
	struct costHa  *heap;
	float           new_min_cost, angle;
	int             row, col;
	long           *heap_len;
{
	long            i, smaller_child;

	if (*heap_len < 1) {
		printf("programming ERROR: can't delete a cell from an ampty list");
		exit(1);
	}
	
	/* search the cell with row and col from the heap */
	for (i = *heap_len; i >=0; i--) {
		    if (heap[i].row == row && heap[i].col == col)
			break;
	}
	if (i == 0) {
		printf("programming ERROR: can't find the old_cell from the list");
		exit(1);
	}

	/* replace this cell, fix the heap */
	/*take care upward*/
        while (i > 1 && new_min_cost < heap[i / 2].min_cost) {
                heap[i].min_cost = heap[i / 2].min_cost;
                heap[i].angle = heap[i / 2].angle;
                heap[i].row = heap[i / 2].row;
                heap[i].col = heap[i / 2].col;
                i = i / 2;
        }

	/*take care downward*/ 
	if (2*i <= *heap_len)
		smaller_child = 2*i;
	if ((2*i < *heap_len) && (heap[2*i].min_cost > heap[2*i + 1].min_cost))
		smaller_child++;
        while ((smaller_child <= *heap_len) && (new_min_cost > heap[smaller_child].min_cost)) {
                heap[i].min_cost = heap[smaller_child].min_cost;
                heap[i].angle = heap[smaller_child].angle;
                heap[i].row = heap[smaller_child].row;
	        heap[i].col = heap[smaller_child].col;
                i = smaller_child;
                smaller_child = 2*i;
        	if ((2*i < *heap_len) && (heap[2*i].min_cost > heap[2*i + 1].min_cost))                 smaller_child++;
	}

	/*now i is the right position*/	
	heap[i].min_cost = new_min_cost;
	heap[i].angle = angle;
 	heap[i].row = row;
        heap[i].col = col;

        return;
}
