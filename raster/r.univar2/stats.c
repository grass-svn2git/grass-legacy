/*
 *  Calculates univariate statistics from the non-null cells
 *
 *   Copyright (C) 2004-2007 by the GRASS Development Team
 *   Author(s): Hamish Bowman, University of Otago, New Zealand
 *              Martin Landa and Soeren Gebbert
 *
 *      This program is free software under the GNU General Public
 *      License (>=v2). Read the file COPYING that comes with GRASS
 *      for details.
 *
 */

#include "globals.h"

/* *************************************************************** */
/* **** univar_stat constructor ********************************** */
/* *************************************************************** */
univar_stat *create_univar_stat_struct(int map_type, int size)
{
    univar_stat *stats;

    stats = (univar_stat *) G_calloc(1, sizeof(univar_stat));

    stats->sum = 0.0;
    stats->sumsq = 0.0;
    stats->min = 0.0 / 0.0;	/*set to nan as default */
    stats->max = 0.0 / 0.0;	/*set to nan as default */
    stats->perc = 0;
    stats->sum_abs = 0.0;
    stats->n = 0;
    stats->size = size;
    stats->dcell_array = NULL;
    stats->fcell_array = NULL;
    stats->cell_array = NULL;
    stats->map_type = map_type;

    /* alloacte memory for extended computation */
    if (param.extended->answer) {
	if (map_type == DCELL_TYPE)
	    stats->dcell_array = (DCELL *) G_calloc(stats->size, sizeof(DCELL));
	if (map_type == FCELL_TYPE)
	    stats->fcell_array = (FCELL *) G_calloc(stats->size, sizeof(FCELL));
	if (map_type == CELL_TYPE)
	    stats->cell_array = (CELL *) G_calloc(stats->size, sizeof(CELL));
    }

    return stats;
}


/* *************************************************************** */
/* **** univar_stat destructor *********************************** */
/* *************************************************************** */
void free_univar_stat_struct(univar_stat * stats)
{
    if (stats->dcell_array)
	G_free(stats->dcell_array);
    if (stats->fcell_array)
	G_free(stats->fcell_array);
    if (stats->cell_array)
	G_free(stats->cell_array);

    G_free(stats);

    return;
}


/* *************************************************************** */
/* **** compute and print univar statistics to stdout ************ */
/* *************************************************************** */
int print_stats(univar_stat * stats)
{
    char sum_str[100];
    double mean, variance, stdev, var_coef;
    /*for extendet stats */
    double quartile_25 = 0.0, quartile_75 = 0.0, quartile_perc = 0.0;
    double median = 0.0;
    int qpos_25, qpos_75, qpos_perc;


    /* all these calculations get promoted to doubles, so any DIV0 becomes nan */
    mean = stats->sum / stats->n;
    variance = (stats->sumsq - stats->sum * stats->sum / stats->n) / stats->n;
    if (variance < GRASS_EPSILON)
	variance = 0.0;
    stdev = sqrt(variance);
    var_coef = (stdev / mean) * 100.;	/* perhaps stdev/fabs(mean) ? */

    sprintf(sum_str, "%.10f", stats->sum);
    G_trim_decimal(sum_str);

    if (!param.shell_style->answer) {
	fprintf(stdout, "total null and non-null cells: %d\n", stats->size);
	fprintf(stdout, "total null cells: %d\n\n", stats->size - stats->n);
	fprintf(stdout, "Of the non-null cells:\n----------------------\n");
    }

    if (param.shell_style->answer) {
	fprintf(stdout, "n=%d\n", stats->n);
	fprintf(stdout, "min=%g\n", stats->min);
	fprintf(stdout, "max=%g\n", stats->max);
	fprintf(stdout, "range=%g\n", stats->max - stats->min);
	fprintf(stdout, "mean=%g\n", mean);
	fprintf(stdout, "mean_of_abs=%g\n", stats->sum_abs / stats->n);
	fprintf(stdout, "stddev=%g\n", stdev);
	fprintf(stdout, "variance=%g\n", variance);
	fprintf(stdout, "coeff_var=%g\n", var_coef);
	fprintf(stdout, "sum=%s\n", sum_str);
    }
    else {
	fprintf(stdout, "n: %d\n", stats->n);
	fprintf(stdout, "minimum: %g\n", stats->min);
	fprintf(stdout, "maximum: %g\n", stats->max);
	fprintf(stdout, "range: %g\n", stats->max - stats->min);
	fprintf(stdout, "mean: %g\n", mean);
	fprintf(stdout, "mean of absolute values: %g\n",
		stats->sum_abs / stats->n);
	fprintf(stdout, "standard deviation: %g\n", stdev);
	fprintf(stdout, "variance: %g\n", variance);
	fprintf(stdout, "variation coefficient: %g %%\n", var_coef);
	fprintf(stdout, "sum: %s\n", sum_str);
    }


    /* TODO: mode, skewness, kurtosis */
    if (param.extended->answer) {
	qpos_25 = (int)(stats->n * 0.25 - 0.5);
	qpos_75 = (int)(stats->n * 0.75 - 0.5);
	qpos_perc = (int)(stats->n * stats->perc / 100. - 0.5);

	switch (stats->map_type) {
	case CELL_TYPE:
	    heapsort_int(stats->cell_array, stats->n);

	    quartile_25 = (double)stats->cell_array[qpos_25];
	    if (stats->n % 2)	/* odd */
		median = (double)stats->cell_array[(int)(stats->n / 2)];
	    else		/* even */
		median =
		    (double)(stats->cell_array[stats->n / 2 - 1] +
			     stats->cell_array[stats->n / 2]) / 2.0;
	    quartile_75 = (double)stats->cell_array[qpos_75];
	    quartile_perc = (double)stats->cell_array[qpos_perc];
	    break;

	case FCELL_TYPE:
	    heapsort_float(stats->fcell_array, stats->n);

	    quartile_25 = (double)stats->fcell_array[qpos_25];
	    if (stats->n % 2)	/* odd */
		median = (double)stats->fcell_array[(int)(stats->n / 2)];
	    else		/* even */
		median =
		    (double)(stats->fcell_array[stats->n / 2 - 1] +
			     stats->fcell_array[stats->n / 2]) / 2.0;
	    quartile_75 = (double)stats->fcell_array[qpos_75];
	    quartile_perc = (double)stats->fcell_array[qpos_perc];
	    break;

	case DCELL_TYPE:
	    heapsort_double(stats->dcell_array, stats->n);

	    quartile_25 = stats->dcell_array[qpos_25];
	    if (stats->n % 2)	/* odd */
		median = stats->dcell_array[(int)(stats->n / 2)];
	    else		/* even */
		median =
		    (stats->dcell_array[stats->n / 2 - 1] +
		     stats->dcell_array[stats->n / 2]) / 2.0;
	    quartile_75 = stats->dcell_array[qpos_75];
	    quartile_perc = stats->dcell_array[qpos_perc];
	    break;

	default:
	    break;
	}

	if (param.shell_style->answer) {
	    fprintf(stdout, "first_quartile=%g\n", quartile_25);
	    fprintf(stdout, "median=%g\n", median);
	    fprintf(stdout, "third_quartile=%g\n", quartile_75);
	    fprintf(stdout, "percentile_%d=%g\n", stats->perc, quartile_perc);
	}
	else {
	    fprintf(stdout, "1st quartile: %g\n", quartile_25);
	    if (stats->n % 2)
		fprintf(stdout, "median (odd number of cells): %g\n", median);
	    else
		fprintf(stdout, "median (even number of cells): %g\n", median);
	    fprintf(stdout, "3rd quartile: %g\n", quartile_75);

	    if (stats->perc % 10 == 1 && stats->perc != 11)
		fprintf(stdout, "%dst percentile: %g\n", stats->perc,
			quartile_perc);
	    else if (stats->perc % 10 == 2 && stats->perc != 12)
		fprintf(stdout, "%dnd percentile: %g\n", stats->perc,
			quartile_perc);
	    else if (stats->perc % 10 == 3 && stats->perc != 13)
		fprintf(stdout, "%drd percentile: %g\n", stats->perc,
			quartile_perc);
	    else
		fprintf(stdout, "%dth percentile: %g\n", stats->perc,
			quartile_perc);
	}
    }


    if (!(param.shell_style->answer))
	G_message("\n");

    return 1;
}
