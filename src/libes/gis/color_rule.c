#include "gis.h"

#define LIMIT(x) if (x < 0) x = 0; else if (x > 255) x = 255;

static int add_color_rule (void *,int,int,int,
           void *,int,int,int,
           struct _Color_Info_ *,int,
           DCELL *,DCELL *,RASTER_MAP_TYPE);


/*!
 * \brief 
 *
 * Adds the floating-point rule that the range [<em>v1,v2</em>] gets a
 * linear ramp of colors from [<em>r1,g1,b1</em>] to
 * [<em>r2,g2,b2</em>].
 * If either <em>v1</em> or <em>v2</em> is the NULL-value, this call is converted into
 * <tt>G_set_null_value_color (r1, g1, b1, colors)</tt>
 *
 *  \param v1
 *  \param r1
 *  \param g1
 *  \param b1
 *  \param v2
 *  \param r2
 *  \param g2
 *  \param b2
 *  \param colors
 *  \return int
 */

int 
G_add_d_raster_color_rule (DCELL *val1, int r1, int g1, int b1, DCELL *val2, int r2, int g2, int b2, struct Colors *colors)
{
    add_color_rule ( (void *) val1, r1,g1,b1,  (void *) val2, r2,g2,b2, &colors->fixed, colors->version,
	&colors->cmin, &colors->cmax, DCELL_TYPE);
    return 1;
}


/*!
 * \brief 
 *
 * Adds the floating-point rule that the range [<em>v1,v2</em>] gets a
 * linear ramp of colors from [<em>r1,g1,b1</em>] to
 * [<em>r2,g2,b2</em>].
 * If either <em>v1</em> or <em>v2</em> is the NULL-value, this call is converted into
 * <tt>G_set_null_value_color (r1, g1, b1, colors)</tt>
 *
 *  \param v1
 *  \param r1
 *  \param g1
 *  \param b1
 *  \param v2
 *  \param r2
 *  \param g2
 *  \param b2
 *  \param colors
 *  \return int
 */

int 
G_add_f_raster_color_rule (FCELL *cat1, int r1, int g1, int b1, FCELL *cat2, int r2, int g2, int b2, struct Colors *colors)
{
    add_color_rule ((void *) cat1, r1,g1,b1, (void *) cat2, r2,g2,b2, &colors->fixed, colors->version,
	&colors->cmin, &colors->cmax, FCELL_TYPE);
    return 1;
}


/*!
 * \brief 
 *
 * Calls G_add_color_rule(*v1, r1, g1, b1, *v2, r2, g2, b2, colors).
 *
 *  \param v1
 *  \param r1
 *  \param g1
 *  \param b1
 *  \param v2
 *  \param r2
 *  \param g2
 *  \param b2
 *  \param colors
 *  \return int
 */

int 
G_add_c_raster_color_rule (CELL *cat1, int r1, int g1, int b1, CELL *cat2, int r2, int g2, int b2, struct Colors *colors)
{
    add_color_rule ((void *) cat1, r1,g1,b1, (void *) cat2, r2,g2,b2, &colors->fixed, colors->version,
	&colors->cmin, &colors->cmax, CELL_TYPE);
    return 1;
}


/*!
 * \brief 
 *
 * If <em>map_type</em> is CELL_TYPE, calls G_add_c_raster_color_rule ((CELL
 * *) v1, r1, g1, b1, (CELL *) v2, r2, g2, b2, colors);
 * If <em>map_type</em> is FCELL_TYPE, calls G_add_f_raster_color_rule
 * ((FCELL *) v1, r1, g1, b1, (FCELL *) v2, r2, g2, b2, colors);
 * If <em>map_type</em> is DCELL_TYPE, calls G_add_d_raster_color_rule
 * ((DCELL *) v1, r1, g1, b1, (DCELL *) v2, r2, g2, b2, colors);
 *
 *  \param v1
 *  \param r1
 *  \param g1
 *  \param b1
 *  \param v2
 *  \param r2
 *  \param g2
 *  \param b2
 *  \param colors
 *  \param map_type
 *  \return int
 */

int 
G_add_raster_color_rule (void *val1, int r1, int g1, int b1, void *val2, int r2, int g2, int b2, struct Colors *colors, RASTER_MAP_TYPE data_type)
{
    add_color_rule (val1, r1,g1,b1, val2, r2,g2,b2, &colors->fixed, colors->version,
	&colors->cmin, &colors->cmax, data_type);
    return 1;
}


/*!
 * \brief set colors
 *
 * This is the heart
 * and soul of the new color logic. It adds a color rule to the <b>colors</b>
 * structure. The colors defined by the red, green, and blue values
 * <b>r1,g1,b1</b> and <b>r2,g2,b2</b> are assigned to <b>cat1</b> and
 * <b>cat2</b> respectively. Colors for data values between <b>cat1</b> and
 * <b>cat2</b> are not stored in the structure but are interpolated when
 * queried by <i>G_lookup_colors</i> and<i>G_get_color.</i> The color
 * components <b>r1,g1,b1</b> and <b>r2,g2,b2</b> must be in the range
 * 0 -- 255.
 * For example, to create a linear grey scale for the range 200 -- 1000:
 \code
  struct Colors colr;
  G_init_colors (&colr);
  G_add_color_rule ((CELL)200, 0,0,0, (CELL)1000, 255,255,255);
 \endcode
 * The programmer is encouraged to review Raster_Color_Table_Format how
 * this routine fits into the 5.x raster color logic.
 * <b>Note.</b> The <b>colors</b> structure must have been initialized by
 * <i>G_init_colors.</i> See Predefined_Color_Tables for routines to
 * build some predefined color tables. 
 *
 *  \param cat1
 *  \param r1
 *  \param g1
 *  \param b1
 *  \param cat2
 *  \param r2
 *  \param g2
 *  \param b2
 *  \param colors
 *  \return int
 */

int 
G_add_color_rule (CELL cat1, int r1, int g1, int b1, CELL cat2, int r2, int g2, int b2, struct Colors *colors)
{
    add_color_rule ((void *) &cat1, r1,g1,b1, (void *) &cat2, r2,g2,b2, &colors->fixed, colors->version,
	&colors->cmin, &colors->cmax, CELL_TYPE);
    return 1;
}

int 
G_add_modular_d_raster_color_rule (DCELL *val1, int r1, int g1, int b1, DCELL *val2, int r2, int g2, int b2, struct Colors *colors)
{
    DCELL min, max;
    if (colors->version < 0)
	return -1; /* can;t use this on 3.0 colors */
    min = colors->cmin;
    max = colors->cmax;
    add_color_rule ((void *) val1, r1,g1,b1, (void *) val2, r2,g2,b2, &colors->modular, 0,
	&colors->cmin, &colors->cmax, DCELL_TYPE);
    colors->cmin = min; /* don't reset these */
    colors->cmax = max;
	
    return 1;
}

int 
G_add_modular_f_raster_color_rule (FCELL *val1, int r1, int g1, int b1, FCELL *val2, int r2, int g2, int b2, struct Colors *colors)
{
    DCELL min, max;
    if (colors->version < 0)
	return -1; /* can;t use this on 3.0 colors */
    min = colors->cmin;
    max = colors->cmax;
    add_color_rule ((void *) val1, r1,g1,b1, (void *) val2, r2,g2,b2, &colors->modular, 0,
	&colors->cmin, &colors->cmax, FCELL_TYPE);
    colors->cmin = min; /* don't reset these */
    colors->cmax = max;
	
    return 1;
}

int 
G_add_modular_c_raster_color_rule (CELL *val1, int r1, int g1, int b1, CELL *val2, int r2, int g2, int b2, struct Colors *colors)
{
    return G_add_modular_color_rule (*val1, r1,g1,b1, *val2, r2,g2,b2, colors);
}

int 
G_add_modular_raster_color_rule (void *val1, int r1, int g1, int b1, void *val2, int r2, int g2, int b2, struct Colors *colors, RASTER_MAP_TYPE data_type)
{
    CELL min, max;
    if (colors->version < 0)
	return -1; /* can't use this on 3.0 colors */
    min = colors->cmin;
    max = colors->cmax;
    add_color_rule (val1, r1,g1,b1, val2, r2,g2,b2, &colors->modular, 0,
	&colors->cmin, &colors->cmax, data_type);
    colors->cmin = min; /* don't reset these */
    colors->cmax = max;
	
    return 1;
}

int 
G_add_modular_color_rule (CELL cat1, int r1, int g1, int b1, CELL cat2, int r2, int g2, int b2, struct Colors *colors)
{
    CELL min, max;
    if (colors->version < 0)
	return -1; /* can;t use this on 3.0 colors */
    min = colors->cmin;
    max = colors->cmax;
    add_color_rule ((void *) &cat1, r1,g1,b1, (void *) &cat2, r2,g2,b2, &colors->modular, 0,
	&colors->cmin, &colors->cmax, CELL_TYPE);
    colors->cmin = min; /* don't reset these */
    colors->cmax = max;
	
    return 1;
}

static int add_color_rule (void *pt1, int r1, int g1, int b1, void *pt2, int r2, int g2, int b2, struct _Color_Info_ *cp, int version, DCELL *cmin, DCELL *cmax, RASTER_MAP_TYPE data_type)
{
    struct _Color_Rule_ *rule, *next;
    unsigned char red, grn, blu;
    DCELL min,max, val1, val2;
    CELL cat;

    val1 = G_get_raster_value_d(pt1, data_type);
    val2 = G_get_raster_value_d(pt2, data_type);
/* allocate a low:high rule */
    rule = (struct _Color_Rule_ *) G_malloc (sizeof(*rule));
    rule->next = rule->prev = NULL;

/* make sure colors are in the range [0,255] */
    LIMIT(r1);
    LIMIT(g1);
    LIMIT(b1);
    LIMIT(r2);
    LIMIT(g2);
    LIMIT(b2);

/* val1==val2, use average color */
/* otherwise make sure low < high */
    if (val1 == val2)
    {
	rule->low.value = rule->high.value = val1;
	rule->low.red = rule->high.red = (r1+r2)/2;
	rule->low.grn = rule->high.grn = (g1+g2)/2;
	rule->low.blu = rule->high.blu = (b1+b2)/2;
    }
    else if (val1 < val2)
    {
	rule->low.value = val1;
	rule->low.red = r1;
	rule->low.grn = g1;
	rule->low.blu = b1;

	rule->high.value = val2;
	rule->high.red = r2;
	rule->high.grn = g2;
	rule->high.blu = b2;
    }
    else
    {
	rule->low.value = val2;
	rule->low.red = r2;
	rule->low.grn = g2;
	rule->low.blu = b2;

	rule->high.value = val1;
	rule->high.red = r1;
	rule->high.grn = g1;
	rule->high.blu = b1;
    }

/* keep track of the overall min and max, excluding null */
    if (G_is_d_null_value(&(rule->low.value)))
        return 0;
    if (G_is_d_null_value(&(rule->high.value)))
        return 0;
    min = rule->low.value;
    max = rule->high.value;
    if (min <= max)
    {
	if (cp->min > cp->max)
	{
	    cp->min = min;
	    cp->max = max;
	}
	else
	{
	    if(cp->min > min)
		cp->min = min;
	    if(cp->max < max)
		cp->max = max;
	}
    }
    if (*cmin > *cmax)
    {
	*cmin = cp->min;
	*cmax = cp->max;
    }
    else
    {
	if(*cmin > cp->min)
	    *cmin = cp->min;
	if(*cmax < cp->max)
	    *cmax = cp->max;
    }

/* If version is old style (i.e., pre 4.0),
 *     interpolate this rule from min to max
 *     and insert each cat into the lookup table.
 *     Then free the rule.
 * Otherwise, free the lookup table, if active.
 *     G_organize_colors() will regenerate it
 *     Link this rule into the list of rules
 */

    if (version < 0)
    {
	for (cat = (CELL) min; cat <= (CELL) max; cat++)
	{
	    G__interpolate_color_rule ((DCELL) cat, &red, &grn, &blu, rule);
	    G__insert_color_into_lookup (cat, (int)red, (int)grn, (int)blu, cp);
	}
	G_free (rule);
    }
    else
    {
	if (cp->rules)
	    cp->rules->prev = rule;
	rule->next = cp->rules;
	cp->rules = rule;

    /* prune the rules:
     * remove all rules that are contained by this rule 
     */
	min = rule->low.value;  /* mod 4.1 */
	max = rule->high.value; /* mod 4.1 */
        cp->n_rules++;
	for (rule = rule->next; rule; rule = next)
	{
	    next = rule->next; /* has to be done here, not in for stmt */
	    if (min <= rule->low.value && max >= rule->high.value)
	    {
		if (rule->prev->next = next) /* remove from the list */
		    next->prev = rule->prev;
		G_free(rule);
                cp->n_rules--;
	    }
	}

    /* free lookup array, if allocated */
	G__color_free_lookup(cp);
	G__color_free_fp_lookup(cp);
    }

    return 0;
}
