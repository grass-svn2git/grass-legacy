#include "Gwater.h"

no_stream (row,col,basin_num,stream_length,old_elev)
double stream_length;
int row, col;
CELL basin_num,old_elev;
{
    int 	r, rr, c, cc, uprow, upcol;
    double 	slope;
    CELL 	downdir, max_drain, value, asp_value, hih_ele, new_ele, aspect;
    SHORT 	updir, riteflag, leftflag, thisdir;

    while (1) {
        max_drain = -1;
        for (r=row-1, rr=0; r<=row+1; r++, rr++) {
            for (c=col-1, cc=0; c<=col+1; c++, cc++) {
                if (r>=0 && c>=0 && r<nrows && c<ncols) { 
		    
		    cseg_get (&asp, &aspect, r, c);
                    if (aspect == drain[rr][cc]) {
			cseg_get (&wat, &value, r, c);
		        if (value < 0)
			    value = -value;
                        if (value > max_drain) {
                            uprow = r;
                            upcol = c;
                            max_drain = value;
                        }
                    }
                }
            }
        }
        if (max_drain > -1) {
            updir = drain[row-uprow+1][col-upcol+1];
	    cseg_get (&asp, &downdir, row, col);
            if (downdir < 0)
		downdir = -downdir;
	    if (sides == 8) {
	        if (uprow != row && upcol != col)
		    stream_length += diag;
	        else if (uprow != row)
		    stream_length += window.ns_res;
	        else
		    stream_length += window.ew_res;
	    } else /* sides == 4 */ {
	    	cseg_get (&asp, &asp_value, uprow, upcol);
		if (downdir == 2 || downdir == 6) {
			if (asp_value == 2 || asp_value == 6)
				stream_length += window.ns_res;
			else
				stream_length += diag;
		} else /* downdir == 4,8 */ {
			if (asp_value == 4 || asp_value == 8)
				stream_length += window.ew_res;
			else
				stream_length += diag;
		}
	    }
            riteflag = leftflag = 0;
            for (r=row-1, rr=0; rr<3; r++, rr++) {
                for (c=col-1, cc=0; cc<3; c++, cc++) {
                    if (r >= 0 && c >= 0 && r < nrows && c < ncols) {
		      cseg_get (&asp, &aspect, r, c);
		      if (aspect == drain[rr][cc]) {
			thisdir = updrain[rr][cc];
		        if (haf_basin_side (updir, 
				(SHORT) downdir, thisdir) == RITE)
		        {
		            overland_cells (r, c, basin_num, basin_num, 
				&new_ele);
		            riteflag++;
		        } else {
		            overland_cells (r, c, basin_num, basin_num-1, 
				&new_ele);
		            leftflag++;
	                }
		      }
                    }
                }
            }
            if (leftflag >= riteflag) {
		value = basin_num - 1;
	        cseg_put (&haf, &value, row, col);
            }
            else cseg_put (&haf, &basin_num, row, col);
	    row = uprow;
	    col = upcol;
        } else {
            if (arm_flag) {
		cseg_get (&alt, &hih_ele, row, col);
		slope = (hih_ele - old_elev) / stream_length;
                if (slope < MIN_SLOPE)
                    slope = MIN_SLOPE;
                fprintf (fp," %lf %lf\n",slope,stream_length);
            }
	    return;
        }
    }
}
