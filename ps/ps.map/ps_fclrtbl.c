/* Function: ps_fcolortable
**
** Author: Radim Blazek, leto 02
*/

#include <string.h>
#include "ps_info.h"
#include "colortable.h"
#include "local_proto.h"

#define NSTEPS 3
#define NNSTEP 4 /* number of nice steps */

extern int verbose;

int PS_fcolortable (void)
{
    char buf[512], *ch;
    int i, k;
    int R, G, B;
    DCELL dmin, dmax, val;
    double t, l, r; /* legend top, left, right */
    double x1, x2, y, dy, fontsize;
    double col_width;
    double width;  /* width of legend in map units */
    double height; /* width of legend in map units */
    double cwidth; /* width of one color line */
    double lwidth; /* line width - frame, ... */
    double step;   /* step between two values */
    int    ncols, cur_step, ddig;
    double nice_steps[NNSTEP] = { 1.0, 2.0, 2.5, 5.0 }; /* nice steps */
    struct FPRange range;
    double ex, cur_d, cur_ex;

    /* let user know what's happenning */
    if (verbose > 1)
    {
        fprintf (stdout,"PS-PAINT: creating color table for <%s in %s> ...",
	    PS.cell_name, PS.cell_mapset);
        fflush(stdout);
    }

    /* Get color range */
    if (G_read_fp_range(PS.cell_name, PS.cell_mapset, &range) == -1) {
         G_warning( "Range information not available (run r.support).");
	 return 1;
    }
    G_get_fp_range_min_max(&range, &dmin, &dmax);
    
    /* set font */
    fontsize = (double)ct.fontsize;
    fprintf(PS.fp, "(%s) FN %.1f SF\n", ct.font, fontsize);

    /* set colortable location,  */
    G_debug(3, "pwidth = %f pheight = %f", PS.page_width, PS.page_height);
    G_debug(3, "ct.width = %f ct.height = %f", ct.width, ct.height);
    
    dy = 1.5 * fontsize;
    /* reset position to get at least something in BBox */
    if (ct.y < PS.top_marg ) { /* heigher than top margin */
       	ct.y = PS.top_marg; 
    } else if (ct.y > PS.page_height - PS.bot_marg ) {
        /* lower than bottom margin - simply move one inch up from bottom margin */
	ct.y = PS.page_height - PS.bot_marg - 1 ; 
    };
    t = 72.0 * ( PS.page_height - ct.y); 

    if ( ct.x < PS.left_marg ) {
	ct.x = PS.left_marg;
    } else if ( ct.x > PS.page_width - PS.right_marg ) { /* move 1 inch to the left from right marg */
	ct.x = PS.page_width - PS.right_marg - 1;
    }
    l = 72.0 * ct.x;

    G_debug(3, "corrected ct.x = %f ct.y = %f", ct.x, ct.y);

    r  = l + 72.0 * ct.width;
    col_width = ct.width / (double)ct.cols;
    
    /* Calc number of colors to print */
    width =  72.0 * ct.width;
    height =  72.0 * ct.height;
    cwidth = 0.1;
    ncols = (int) height / cwidth;
    step = (dmax - dmin) / (ncols - 1);
    lwidth = 0.02 * width;

    /* Print color band */
    y = t;
    x1 = l; 
    x2 = x1 + width;
    fprintf(PS.fp, "%.8f W\n", cwidth);
    for ( i = 0; i < ncols; i++ ) {
	val = dmin + i * step; 
	G_get_d_raster_color(&val, &R, &G, &B, &PS.colors);
	fprintf(PS.fp, "%.2f %.2f %.2f C\n", (double)R/255., (double)G/255., (double)B/255.);
	fprintf(PS.fp, "NP\n");
        fprintf(PS.fp, "%f %f M\n", x1, y); 
        fprintf(PS.fp, "%f %f LN\n", x2, y); 
        fprintf(PS.fp, "D\n"); 
	y -= cwidth;
    }

    /* Frame around */
    fprintf(PS.fp, "NP\n");
    set_rgb_color ( ct.color ); 
    fprintf(PS.fp, "%.8f W\n", lwidth);
    fprintf(PS.fp, "%f %f %f %f B\n", x1, t-(ncols-1)*cwidth-(cwidth+lwidth)/2 , x2, t+(cwidth+lwidth)/2 );
    fprintf(PS.fp, "D\n"); 

    /* Print labels */
    /* maximum number of parts we can divide into */
    k = (ncols - 1) * cwidth / dy;
    /* step in values for labels */
    step = (dmax - dmin ) / k; /* raw step - usually decimal number with many places, not nice */
    
    /* find nice step and first nice value for label: nice steps are 1, 2, 2.5 or 5 * 10^n,
    *  we need nice step which is first >= raw step, we take each nice step and find 'n'
    *  and then compare differences */
    for ( i = 0; i < NNSTEP; i++ ) {
	/* smalest n for which nice step >= raw step */
        if ( nice_steps[i] <= step ) {
	    ex = 1;
	    while (  nice_steps[i] * ex < step ) ex *= 10;
	} else {
	    ex = 0.1;
	    while (  nice_steps[i] * ex > step ) ex *= 0.1;
	    ex *= 10;
	}
	if ( i == 0 || (nice_steps[i] * ex - step) < cur_d ) {
	    cur_step = i;
	    cur_d = nice_steps[i] * ex - step;
	    cur_ex = ex;
	}
    }
    step = nice_steps[cur_step] * cur_ex; 

    /* Nice first value: first multiple of step >= dmin */
    k = dmin / step ;
    val = k * step;
    if ( val < dmin ) val += step;
    
    x1 = l + width; 
    x2 = x1 + 0.5 * width; 
    
    /* do nice label: we need so many decimal places to hold all step decimal digits */	
    if ( step > 100 ) { /* nice steps do not have > 2 digits, important separate, otherwise */
	ddig = 0;       /* we can get something like 1000000.00000000765239 */
    } else { 
	sprintf (buf, "%.10f", step);
	k = strlen (buf) - 1;
	while ( buf[k] == '0' ) k--;
	k = k - (int) ( strchr( buf, '.') - (unsigned) buf );
	ddig = k;
    }
	
    fprintf(PS.fp, "%.8f W\n", lwidth);
    while ( val <= dmax ) {
        y = t - (val - dmin) * height / (dmax - dmin) ;
	fprintf(PS.fp, "NP\n");
        fprintf(PS.fp, "%f %f M\n", x1, y); 
        fprintf(PS.fp, "%f %f LN\n", x2, y); 
        fprintf(PS.fp, "D\n"); 
            
	sprintf (buf, "%f", val);
	ch = (char *) strchr (buf, '.');
	ch += ddig;
	if ( ddig > 0 ) ch++;
	*ch = '\0';

	fprintf(PS.fp, "(%s) %f %f MS\n", buf, x2 + 0.2 * fontsize , y - 0.35 * fontsize);
	
	val += step;
    }

    G_free_colors(&PS.colors);
    
    if (verbose > 1) fprintf (stdout,"\n");

    return 0;
}

