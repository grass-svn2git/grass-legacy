#include "Gwater.h"

sg_factor()
{
    int		r, c;
    CELL	low_elev, hih_elev;
    double	height, length, S, sin_theta;

    printf ("\nSECTION 4: Length Slope determination.      Percent complete: ");
    for (r = 0; r < nrows; r++) {
	G_percent (r, nrows, 1);
	if (ril_flag) {
		G_get_map_row (ril_fd, ril_buf, r);
	}
	for (c = 0; c < ncols; c++) {
	    	low_elev = alt[SEG_INDEX(alt_seg,r,c)];
	    	hih_elev = r_h[SEG_INDEX(r_h_seg,r,c)];
	    	length = s_l[SEG_INDEX(s_l_seg,r,c)];
	    	height = hih_elev - low_elev;
		if (length > max_length) {
			height *= max_length / length;
			length = max_length;
		}
		sin_theta = height / sqrt ( height * height + length * length);
		if (height / length < .09) S = 10.8 * sin_theta + .03;
		else 	S = 16.8 * sin_theta - .50;
	    	if (sg_flag) s_g[SEG_INDEX(s_g_seg,r,c)] = S;
	    	if (ls_flag) {
	    		length *= METER_TO_FOOT;
			len_slp_equ(length, sin_theta, S, r, c);
		}
	}
    }
    if (ril_flag) {
	free (ril_buf);
    	G_close_cell (ril_fd);
    }
    printf ("\n");
}

len_slp_equ(slope_length, sin_theta, S, r, c)
double slope_length, sin_theta, S;
int r, c;
{
	double 	ril, s_l_exp, 	/* m				*/
		rill_ratio, 	/* Beta				*/
		L;

	rill_ratio = (sin_theta / 0.0896) / (3.0 * pow (sin_theta, 0.8) + 0.56);
	if (ril_flag)	{
		ril = ril_buf[c];
	} else if (ril_value >= 0.0) {
		ril = ril_value;
	} else	ril = 0.0;
	/* rill_ratio equation from Steve Warren */
	rill_ratio *= .5 + .005 * ril + .0001 * ril * ril; 
	s_l_exp = rill_ratio / (1 + rill_ratio);
	L = 100 * pow ((slope_length / 72.6), s_l_exp);
	l_s[SEG_INDEX(l_s_seg,r,c)] = L * S;
}
