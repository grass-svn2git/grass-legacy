
/**
 * @file labels.c
 * This file contains functions for label and label candidate manipulation
*/
#include "labels.h"
static int label_skyline(FT_Face face, const char *charset, label_t * label);
static struct line_pnts *box_trans_rot(BOUND_BOX * bb, label_point_t * p,
				       double angle);
static void label_point_candidates(label_t * label);
static void label_line_candidates(label_t * label);
static int candidate_compare(const void *a, const void *b);
static struct line_pnts *skyline_trans_rot(struct line_pnts *skyline,
					   label_point_t * p, double angle);
static double label_avedist(label_t * label, label_candidate_t * candidate);
static double label_flatness(label_t * label, label_candidate_t * candidate);
static double label_pointover(label_t * label, label_candidate_t * candidate);
static double label_lineover(label_t * label, label_candidate_t * candidate,
			     int linetype);
static label_point_t *lineover_intersection(struct bound_box *bb,
					    struct line_pnts *line);
static double min_dist_2_lines(struct line_pnts *skyline,
			       struct line_pnts *swathline, label_point_t * p);
static int box_overlap(BOUND_BOX * a, BOUND_BOX * b);
static int box_overlap2(struct line_pnts *a, struct line_pnts *b);

/**
 * The font size in map units. A global variable because I'l lazy :P
 */
static double font_size = 0.0;

/**
 * The ideal distance, that is the distance between the base line of a label and a line feature.
 */
static double ideal_distance;

/**The vector Map structure*/
static struct Map_info Map;

/**
 * The size of the buffer around features where labels should never be
 */
static double buffer = 0.0;

label_t *labels_init(struct params *p, int *n_labels)
{
    label_t *labels;
    char *mapset;
    int legal_types, layer, i = 0, error, sql_len;
    size_t label_sz;
    struct field_info *fi;
    dbDriver *driver;
    FT_Library library;
    FT_Face face;

    fprintf(stderr, "Initialising labels ...");
    legal_types = Vect_option_to_types(p->type);

    /* open vector */
    mapset = G_find_vector2(p->map->answer, NULL);
    if (mapset == NULL)
	G_fatal_error(_("Vector map [%s] not available"), p->map->answer);

    /* open vector for read only */
    Vect_open_old(&Map, p->map->answer, mapset);

    label_sz = Vect_get_num_primitives(&Map, legal_types);

    G_debug(1, "Need to allocate %ld bytes of memory",
	    sizeof(label_t) * label_sz);
    labels = malloc(sizeof(label_t) * label_sz);
    G_debug(1, "labels=%p", labels);

    if (labels == NULL)
	G_fatal_error(_("Cannot allocate %d bytes of memory"),
		      sizeof(label_t) * label_sz);

    /* open database */
    layer = atoi(p->layer->answer);
    fi = Vect_get_field(&Map, layer);
    if (fi == NULL)
	G_fatal_error(_("Cannot get layer info for vector map"));
    driver = db_start_driver_open_database(fi->driver, fi->database);
    if (driver == NULL)
	G_fatal_error(_("Cannot open database %s by driver %s"),
		      fi->database, fi->driver);

    sql_len = strlen(p->column->answer) + strlen(fi->table) +
	strlen(fi->key) + 30;

    /* initialize FT 2 library */
    if (FT_Init_FreeType(&library))
	G_fatal_error(_("Unable to initialise FreeType"));
    error = FT_New_Face(library, p->font->answer, 0, &face);
    if (error == FT_Err_Unknown_File_Format)
	G_fatal_error(_("Font file format is not supported by FreeType"));
    else if (error) {
	G_fatal_error(_("Font file can not be loaded"));
    }

    font_size = atof(p->size->answer);
    buffer = atof(p->isize->answer);

    /* use 1 point = 1 map unit */
    if (FT_Set_Char_Size(face, (int)(font_size * 64.0), 0, 100, 100))
	G_fatal_error(_("Unable to set font size"));

    /* start reading the map */
    while (1) {
	struct line_pnts *Points;
	struct line_cats *Cats;

	dbCursor cursor;
	dbTable *table;
	dbColumn *column;
	dbString query, value;

	int type, cat, more, nrows;
	char *sql;

	if (i == label_sz) {	/* we need more memory */
	    label_sz += 100;
	    G_debug(1, "Need to resize %p to %ld bytes of memory",
		    (void *)labels, sizeof(label_t) * label_sz);
	    labels = realloc(labels, sizeof(label_t) * label_sz);
	    if (labels == NULL) {
		G_fatal_error(_("Cannot allocate more memory"));
	    }
	}

	G_percent(i, label_sz, 10);

	memset(&labels[i], 0, sizeof(label_t));

	Points = Vect_new_line_struct();
	Cats = Vect_new_cats_struct();

	type = Vect_read_next_line(&Map, Points, Cats);
	if (type == -1)
	    G_fatal_error(_("Cannot read vector"));
	if (type == -2)
	    break;		/* EOF */
	if (!(legal_types & type))
	    continue;

	Vect_cat_get(Cats, layer, &cat);
	if (cat < 0)
	    continue;		/* no cat for this field */

	sql = G_malloc(sql_len);
	/* Read label from database */
	sprintf(sql, "select %s from %s where %s = %d", p->column->answer,
		fi->table, fi->key, cat);
	G_debug(3, "SQL: %s", sql);
	db_init_string(&query);
	db_set_string(&query, sql);
	free(sql);

	if (db_open_select_cursor(driver, &query, &cursor, DB_SEQUENTIAL) !=
	    DB_OK)
	    G_fatal_error(_("Cannot select attribute."));
	db_free_string(&query);
	nrows = db_get_num_rows(&cursor);
	if (nrows < 1) {
	    G_warning(_("No database record for category %d"), cat);
	    continue;
	}

	if (db_fetch(&cursor, DB_NEXT, &more) != DB_OK || !more)
	    continue;

	table = db_get_cursor_table(&cursor);
	column = db_get_table_column(table, 0);	/* first column */

	db_init_string(&value);
	db_convert_column_value_to_string(column, &value);
	db_close_cursor(&cursor);

	G_debug(3, "Label: %s", db_get_string(&value));

	/* ignor empty strings */
	if (strlen(db_get_string(&value)) == 0)
	    continue;

	labels[i].text = G_strdup(db_get_string(&value));
	labels[i].cat = cat;
	labels[i].type = type;
	labels[i].shape = Points;
	G_debug(3, "Label [%d]: %s, cat=%d, type=0x%02x", i, labels[i].text,
		labels[i].cat, labels[i].type);

	/* make a skyline for the text */
	label_skyline(face, p->charset->answer, &labels[i]);

	i++;

	db_free_string(&value);
	/*              Vect_destroy_line_struct(Points); */
	Vect_destroy_cats_struct(Cats);
    }

    {
	FT_UInt glyph_index;
	glyph_index = FT_Get_Char_Index(face, 'X');
	if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT))
	    G_fatal_error("Cannot determine ideal height");
	ideal_distance = 0.3 * face->glyph->metrics.height / 64.0;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    db_close_database_shutdown_driver(driver);
    /*      Vect_close(&Map); */
    G_percent(label_sz, label_sz, 10);

    *n_labels = i;
    return labels;

}

/**
 * This function calculates the skyline of a label and stores it in the label structure.
 * @param face The openned FT library face to use.
 * @param The charset to use
 * @param The label to which we want to create a skyline
 */
static int label_skyline(FT_Face face, const char *charset, label_t * label)
{
    int i, len;
    double advance = 0.0;

    len = strlen(label->text);
    label->skyline = Vect_new_line_struct();
    G_debug(3, "Creating skyline for '%s'", label->text);

    for (i = 0; i < len; i++) {
	FT_UInt glyph_index;

	glyph_index = FT_Get_Char_Index(face, label->text[i]);
	if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT))
	    G_warning(_("Cannot load glyph for '%c'"), label->text[i]);

	/* insert the 4 corners of the bounding box */
	{
	    label_point_t top_left, top_right, bottom_right, bottom_left;

	    G_debug(5,
		    "horiBearingX=%ld horiBearingY=%ld width=%ld height=%ld advance=%ld",
		    face->glyph->metrics.horiBearingX,
		    face->glyph->metrics.horiBearingY,
		    face->glyph->metrics.width, face->glyph->metrics.height,
		    face->glyph->metrics.horiAdvance);

	    top_left.x = advance;
	    top_left.y = face->glyph->metrics.horiBearingY / 64.0;

	    top_right.x = advance + face->glyph->metrics.horiAdvance / 64.0;
	    top_right.y = face->glyph->metrics.horiBearingY / 64.0;

	    bottom_right.x = advance + face->glyph->metrics.horiAdvance / 64.0;
	    bottom_right.y = (face->glyph->metrics.horiBearingY -
			      face->glyph->metrics.height) / 64.0;

	    bottom_left.x = advance;
	    bottom_left.y = (face->glyph->metrics.horiBearingY -
			     face->glyph->metrics.height) / 64.0;

	    if (i == 0) {
		G_debug(5, "Character(%d) '%c': Adding UL point (%lf,%lf)",
			i, label->text[i], top_left.x, top_left.y);
		Vect_append_point(label->skyline, top_left.x, top_left.y, 0.0);
		G_debug(5, "Character(%d) '%c': Adding UR point (%lf,%lf)",
			i, label->text[i], top_right.x, top_right.y);
		Vect_append_point(label->skyline,
				  top_right.x, top_right.y, 0.0);

		G_debug(5, "Character(%d) '%c': Adding LR point (%lf,%lf)",
			i, label->text[i], bottom_right.x, bottom_right.y);
		Vect_append_point(label->skyline,
				  bottom_right.x, bottom_right.y, 0.0);

		G_debug(5, "Character(%d) '%c': Adding LL point (%lf,%lf)",
			i, label->text[i], bottom_left.x, bottom_left.y);
		Vect_append_point(label->skyline,
				  bottom_left.x, bottom_left.y, 0.0);
		Vect_append_point(label->skyline, top_left.x, top_left.y, 0.0);
	    }
	    else {
		G_debug(5, "Character(%d) '%c': Adding UL point (%lf,%lf)",
			i, label->text[i], top_left.x, top_left.y);
		Vect_line_insert_point(label->skyline, i * 2,
				       top_left.x, top_left.y, 0.0);
		G_debug(5, "Character(%d) '%c': Adding UR point (%lf,%lf)",
			i, label->text[i], top_right.x, top_right.y);
		Vect_line_insert_point(label->skyline, i * 2 + 1,
				       top_right.x, top_right.y, 0.0);

		G_debug(5, "Character(%d) '%c': Adding LR point (%lf,%lf)",
			i, label->text[i], bottom_right.x, bottom_right.y);
		Vect_line_insert_point(label->skyline, i * 2 + 2,
				       bottom_right.x, bottom_right.y, 0.0);

		G_debug(5, "Character(%d) '%c': Adding LL point (%lf,%lf)",
			i, label->text[i], bottom_left.x, bottom_left.y);
		Vect_line_insert_point(label->skyline, i * 2 + 3,
				       bottom_left.x, bottom_left.y, 0.0);
	    }

	    advance += face->glyph->metrics.horiAdvance / 64.0;
	    G_debug(5, "Total advance  %lf", advance);
	}
    }
    /* remove duplicate points */
    Vect_line_prune(label->skyline);
    /* get the boundingbox */
    Vect_line_box(label->skyline, &label->bb);
    return 1;
}

void label_candidates(label_t * labels, int n_labels)
{
    int i;
    /* generate candidate location for each label based on feture type
     * see chapter 5 of MERL-TR-96-04 */
    fprintf(stderr, "Generating label candidates: ...");
    for (i = 0; i < n_labels; i++) {
	G_percent(i, n_labels - 1, 1);
	switch (labels[i].type) {
	case GV_POINT:
	    G_debug(1, "Line (%d): %s", i, labels[i].text);
	    label_point_candidates(&labels[i]);
	    break;
	case GV_LINE:
	    G_debug(3, "Line (%d): %s", i, labels[i].text);
	    label_line_candidates(&labels[i]);
	    break;
	    /*                case GV_AREA:
	     * label_area_candidates(labels[i]);
	     * break; */
	default:
	    /* this should never be reached */
	    break;
	}
    }
    Vect_close(&Map);
    return;
}

/**
 * This function generates candidates for point features.
 * @param label. The point label to which we want to create label candidates.
 */
static void label_point_candidates(label_t * label)
{
    double height, width;
    int i;
    label_candidate_t *candidates;
    candidates = calloc(19, sizeof(label_candidate_t));
    if (candidates == NULL) {
	G_fatal_error("Cannot allocate memory.");
    }

    height = label->bb.N - label->bb.S;
    width = label->bb.E - label->bb.W;
    /* 2 upper left-hand side labels are placed so that they are 
     * 1/3 and 2/3 of label height above point, and right aligned */
    candidates[0].point.x = label->shape->x[0] - width - buffer * 0.75;
    candidates[0].point.y = label->shape->y[0] + (5.0 / 9.0) * height;
    candidates[0].score = 0.63;

    candidates[1].point.x = label->shape->x[0] - width - buffer * 0.85;
    candidates[1].point.y = label->shape->y[0] + (1.0 / 3.0) * height;
    candidates[1].score = 0.44;
    /* same height as label point */
    candidates[2].point.x = label->shape->x[0] - width - buffer * 0.95;
    candidates[2].point.y = label->shape->y[0];
    candidates[2].score = 0.07;
    /* 3 lower left-hand side labels are placed so that they are 
     * 1/3, 2/3 and 3/3 of label height below point, and right aligned */
    candidates[3].point.x = label->shape->x[0] - width - buffer * 0.95;
    candidates[3].point.y = label->shape->y[0] - (1.0 / 3.0) * height;
    candidates[3].score = 0.10;

    candidates[4].point.x = label->shape->x[0] - width - buffer * 0.95;
    candidates[4].point.y = label->shape->y[0] - (5.0 / 9.0) * height;
    candidates[4].score = 0.02;

    candidates[5].point.x = label->shape->x[0] - width - buffer * 0.95;
    candidates[5].point.y = label->shape->y[0] - height;
    candidates[5].score = 0.37;

    /* 2 upper right-hand side labels are placed so that they are 
     * 1/3 and 2/3 of label height above point*/
    candidates[6].point.x = label->shape->x[0] + buffer * 0.85;
    candidates[6].point.y = label->shape->y[0] + (5.0 / 9.0) * height;
    candidates[6].score = 0.41;

    candidates[7].point.x = label->shape->x[0] + buffer * 0.95;
    candidates[7].point.y = label->shape->y[0] + (1.0 / 3.0) * height;
    candidates[7].score = 0.33;
    /* same height as label point */
    candidates[8].point.x = label->shape->x[0] + buffer;
    candidates[8].point.y = label->shape->y[0];
    candidates[8].score = 0.00;
    /* 4 lower left-hand side labels are placed so that they are 
     * 1/4, 2/4 3/4 and 4/4 of label height below point*/
    candidates[9].point.x = label->shape->x[0] + buffer;
    candidates[9].point.y = label->shape->y[0] - 0.25 * height;
    candidates[9].score = 0.04;

    candidates[10].point.x = label->shape->x[0] + buffer;
    candidates[10].point.y = label->shape->y[0] - 0.5 * height;
    candidates[10].score = 0.3;

    candidates[11].point.x = label->shape->x[0] + buffer;
    candidates[11].point.y = label->shape->y[0] - 0.75 * height;
    candidates[11].score = 0.12;

    candidates[12].point.x = label->shape->x[0] + buffer;
    candidates[12].point.y = label->shape->y[0] - height;
    candidates[12].score = 0.59;

    /* 3 labels above, centered, centered on left 1/3 and centered 
     * on right 1/3 of the label */
    candidates[13].point.x = label->shape->x[0] - (1.0 / 3.0) * width;
    candidates[13].point.y = label->shape->y[0] + fabs(label->bb.S) + buffer;
    candidates[13].score = 0.70;

    candidates[14].point.x = label->shape->x[0] - 0.5 * width;
    candidates[14].point.y = label->shape->y[0] + fabs(label->bb.S) + buffer;
    candidates[14].score = 0.89;

    candidates[15].point.x = label->shape->x[0] - (2.0 / 3.0) * width;
    candidates[15].point.y = label->shape->y[0] + fabs(label->bb.S) + buffer;
    candidates[15].score = 0.74;

    /* 3 labels below, centered, centered on left 1/3 and centered 
     * on right 1/3 of the label */
    candidates[16].point.x = label->shape->x[0] - (1.0 / 3.0) * width;
    candidates[16].point.y = label->shape->y[0] - height - buffer;
    candidates[16].score = 0.74;

    candidates[17].point.x = label->shape->x[0] - 0.5 * width;
    candidates[17].point.y = label->shape->y[0] - height - buffer;
    candidates[17].score = 0.89;

    candidates[18].point.x = label->shape->x[0] - (2.0 / 3.0) * width;
    candidates[18].point.y = label->shape->y[0] - height - buffer;
    candidates[18].score = 1.0;

    for (i = 0; i < 19; i++) {
	candidates[i].score += 10.0 * label_pointover(label, &candidates[i]);
	candidates[i].score += 15.0 * label_lineover(label, &candidates[i],
						     GV_LINE);
	G_debug(1, "calling label_lineover('%s', %d)", label->text, i);
	candidates[i].score += 10.0 * label_lineover(label, &candidates[i],
						     GV_BOUNDARY);
	candidates[i].rotation = 0;
    }

    /* randomly choose one candidate to be the current */
    label->current_candidate = (int)(19.0 * (rand() / (RAND_MAX + 1.0)));
    label->candidates = candidates;
    label->n_candidates = 19;
}

/**
 * This function generates label candidates for a line feature.
 * @param label The label for which to create the candidates.
 */
static void label_line_candidates(label_t * label)
{
    double height, width, inc, length, pos;
    label_candidate_t *above_candidates, *below_candidates, *candidates;
    int i, n, n_c;
    FILE *sl;
    sl = fopen("skylines.txt", "a");

    height = label->bb.N - label->bb.S;
    width = label->bb.E - label->bb.W;
    inc = width / 8.0;
    length = Vect_line_length(label->shape);

    n = (int)(length / inc);
    if (n == 0) {
	/* treat the line as a point feature */
	struct line_pnts *tmp, *tmp_shape;
	double x, y;

	tmp = Vect_new_line_struct();
	Vect_point_on_line(label->shape, length / 2.0, &x, &y,
			   NULL, NULL, NULL);
	Vect_append_point(tmp, x, y, 0);
	tmp_shape = label->shape;
	label->shape = tmp;

	label_point_candidates(label);
	label->shape = tmp_shape;
	Vect_destroy_line_struct(tmp);

	return;
    }
    above_candidates = calloc(n, sizeof(label_candidate_t));
    below_candidates = calloc(n, sizeof(label_candidate_t));
    if ((above_candidates == NULL) || (below_candidates == NULL)) {
	G_fatal_error("Cannot allocate memory.");
    }

    /* find all candidate labels */
    for (pos = width / 2.0, i = 0; pos < (length - 1.5 * width); pos += inc) {
	label_point_t p1, p2, minimum_above_distance_p,
	    minimum_below_distance_p;
	int seg1, seg2, j;
	struct line_pnts *above_skyline, *below_skyline, *baseline;
	double above_distance = 0.0, below_distance = 0.0,
	    minimum_above_distance = 0.0, minimum_below_distance = 0.0, angle;

	seg1 =
	    Vect_point_on_line(label->shape, pos, &p1.x, &p1.y, NULL, NULL,
			       NULL);
	seg2 =
	    Vect_point_on_line(label->shape, pos + width, &p2.x, &p2.y, NULL,
			       NULL, NULL);

	G_debug(1, "pos=%lf i=%d p1 at (%lf,%lf), p2 at (%lf,%lf)",
		pos, i, p1.x, p1.y, p2.x, p2.y);

	angle = atan2((p2.y - p1.y), (p2.x - p1.x));
	if ((angle > M_PI / 2) || (angle < -M_PI / 2)) {
	    /* turn label around 180 degrees if it would be upside down */
	    double tmp;
	    tmp = p1.x;
	    p1.x = p2.x;
	    p2.x = tmp;

	    tmp = p1.y;
	    p1.y = p2.y;
	    p2.y = tmp;

	    /*                              p1.x = p1.x + length * cos(angle);
	     * p1.x = p1.x + length * sin(angle); */
	    if (angle < 0)
		angle += M_PI;
	    else
		angle -= M_PI;
	}

	/* find the maximum above_distance and below_distance from the swath
	 * "diagonal" to determine maximum deviation from a straight line 
	 * create the swath lines at the same time
	 */
	above_candidates[i].swathline = Vect_new_line_struct();
	below_candidates[i].swathline = Vect_new_line_struct();
	if ((above_candidates[i].swathline == NULL) ||
	    (below_candidates[i].swathline == NULL))
	    G_fatal_error("Cannot allocate memory!");
	Vect_append_point(above_candidates[i].swathline, p1.x, p1.y, 0);
	Vect_append_point(below_candidates[i].swathline, p1.x, p1.y, 0);

	baseline = Vect_new_line_struct();
	Vect_append_point(baseline, p1.x, p1.y, 0);
	Vect_append_point(baseline, p2.x, p2.y, 0);

	Vect_append_point(above_candidates[i].swathline, p1.x, p1.y, 0);
	Vect_append_point(below_candidates[i].swathline, p1.x, p1.y, 0);

	for (j = seg1 + 1; j < seg2; j++) {
	    double x, y, d;
	    Vect_line_distance(baseline, label->shape->x[j],
			       label->shape->y[j], 0, 0,
			       &x, &y, NULL, &d, NULL, NULL);
	    if (label->shape->y[j] < y) {
		/* swathline is beneath the "diagonal" */
		if (d > below_distance) {
		    below_distance = d;
		}
	    }
	    else {
		/* swatline is above or on the "diagonal" */
		if (d > above_distance) {
		    above_distance = d;
		}
	    }
	    Vect_append_point(above_candidates[i].swathline,
			      label->shape->x[j], label->shape->y[j], 0);
	    Vect_append_point(below_candidates[i].swathline,
			      label->shape->x[j], label->shape->y[j], 0);
	}

	Vect_append_point(above_candidates[i].swathline, p2.x, p2.y, 0);
	Vect_append_point(below_candidates[i].swathline, p2.x, p2.y, 0);
	Vect_destroy_line_struct(baseline);

	if (above_distance == 0.0) {
	    above_distance = height - label->bb.N;
	}
	if (below_distance == 0.0) {
	    below_distance = height - label->bb.S;
	}

	/* place a skyline at 1.1 * above_distance above line, and
	 * 1.1 * below_distance + height below line */
	{
	    label_point_t tp;

	    tp.x = p1.x - 1.1 * above_distance * sin(angle);
	    tp.y = p1.y + 1.1 * above_distance * cos(angle);
	    above_skyline = skyline_trans_rot(label->skyline, &tp, angle);
	    tp.x = p1.x + (1.1 * below_distance + height) * sin(angle);
	    tp.y = p1.y - (1.1 * below_distance + height) * cos(angle);
	    below_skyline = skyline_trans_rot(label->skyline, &tp, angle);
	}
	/* find minimum distance between swath line and skylines */
	minimum_above_distance = min_dist_2_lines(above_skyline,
						  above_candidates[i].swathline,
						  &minimum_above_distance_p);
	minimum_below_distance = min_dist_2_lines(below_skyline,
						  below_candidates[i].swathline,
						  &minimum_below_distance_p);

	/* adjust skylines so that the minimum distance is equal to the ideal
	 * distance (= 0.3 * glyph height of capital X) */

	above_distance += ideal_distance - minimum_above_distance;
	below_distance += ideal_distance - minimum_below_distance;

	Vect_destroy_line_struct(above_skyline);
	Vect_destroy_line_struct(below_skyline);

	above_candidates[i].point.x = p1.x - above_distance * sin(angle);
	above_candidates[i].point.y = p1.y + above_distance * cos(angle);

	below_candidates[i].point.x =
	    p1.x + (below_distance + height) * sin(angle);
	below_candidates[i].point.y =
	    p1.y - (below_distance + height) * cos(angle);

	G_debug(1, "above at (%lf,%lf) below at (%lf,%lf)",
		above_candidates[i].point.x, above_candidates[i].point.y,
		below_candidates[i].point.x, below_candidates[i].point.y);

	above_candidates[i].above = 1;
	below_candidates[i].above = 0;
	above_candidates[i].rotation = angle;
	below_candidates[i].rotation = angle;

	above_candidates[i].score += label_avedist(label, &above_candidates[i]);
	below_candidates[i].score += label_avedist(label, &below_candidates[i]);

	above_candidates[i].score +=
	    label_flatness(label, &above_candidates[i]);
	below_candidates[i].score +=
	    label_flatness(label, &below_candidates[i]);

	/* centerdness */
	above_candidates[i].score += 3.0 * fabs(pos / length - 1);
	below_candidates[i].score += 3.0 * fabs(pos / length - 1);

	above_candidates[i].score += 10.0 *
	    label_pointover(label, &above_candidates[i]);
	below_candidates[i].score += 10.0 *
	    label_pointover(label, &below_candidates[i]);
	above_candidates[i].score += 15.0 *
	    label_lineover(label, &above_candidates[i], GV_LINE);
	below_candidates[i].score += 15.0 *
	    label_lineover(label, &below_candidates[i], GV_LINE);
	above_candidates[i].score += 10.0 *
	    label_lineover(label, &above_candidates[i], GV_BOUNDARY);
	below_candidates[i].score += 10.0 *
	    label_lineover(label, &below_candidates[i], GV_BOUNDARY);

	below_candidates[i].score += 0.25;

	i++;
    }
    n = i;
    if (n == 0) {
	/* treat the line as a point feature */
	struct line_pnts *tmp, *tmp_shape;
	double x, y;

	tmp = Vect_new_line_struct();
	Vect_point_on_line(label->shape, length / 2.0, &x, &y,
			   NULL, NULL, NULL);
	Vect_append_point(tmp, x, y, 0);
	tmp_shape = label->shape;
	label->shape = tmp;

	label_point_candidates(label);
	label->shape = tmp_shape;
	Vect_destroy_line_struct(tmp);
	return;
    }

    /* pick the 32 best candidates */
    qsort(above_candidates, n, sizeof(label_candidate_t), candidate_compare);
    qsort(below_candidates, n, sizeof(label_candidate_t), candidate_compare);

    if (n > 32)
	n_c = 32;
    else
	n_c = n;

    candidates = calloc(n_c, sizeof(label_candidate_t));
    for (i = 0; i < n_c; i++) {
	if (candidate_compare(&below_candidates[i], &above_candidates[i]) < 0) {
	    memcpy(&candidates[i], &below_candidates[i],
		   sizeof(label_candidate_t));
	    memset(&below_candidates[i], 0, sizeof(label_candidate_t));
	}
	else {
	    memcpy(&candidates[i], &above_candidates[i],
		   sizeof(label_candidate_t));
	    memset(&above_candidates[i], 0, sizeof(label_candidate_t));
	}

	if (candidates[i].above) {
	    Vect_destroy_line_struct(below_candidates[i].baseline);
	    Vect_destroy_line_struct(below_candidates[i].swathline);
	}
	else {
	    Vect_destroy_line_struct(above_candidates[i].baseline);
	    Vect_destroy_line_struct(above_candidates[i].swathline);
	}
    }
    fclose(sl);
    for (; i < n; i++) {
	Vect_destroy_line_struct(above_candidates[i].baseline);
	Vect_destroy_line_struct(above_candidates[i].swathline);
	Vect_destroy_line_struct(below_candidates[i].baseline);
	Vect_destroy_line_struct(below_candidates[i].swathline);
    }
    free(above_candidates);
    free(below_candidates);

    label->current_candidate =
	(int)((double)(n_c) * (rand() / (RAND_MAX + 1.0)));
    label->candidates = candidates;
    label->n_candidates = n_c;
}

/**
 * This function compares two label candidates scores and determines which is better.
 * @param a Candidate A
 * @param b Candidate B
 * @return -1 if candidate a has a lower score then candidate b. = if the
 * scores are equal, and 1 if candidate a has a higher score then candidate b.
 */
static int candidate_compare(const void *a, const void *b)
{
    const label_candidate_t *ca = a, *cb = b;

    if (ca->score < cb->score) {
	return -1;
    }
    else if (ca->score == cb->score) {
	return 0;
    }
    else {
	return 1;
    }
}

/**
 * This function rotates the label skyline and then translates it to the
 * given point.
 * @param skyline The skyline to translate
 * @param p The point to translate the skyline to
 * @param angle The angle (in radians) to rotate the label counter-clockwise
 * @return A lint_pnts structure containing the rotated and translated
 * skyline.
 */
static struct line_pnts *skyline_trans_rot(struct line_pnts *skyline,
					   label_point_t * p, double angle)
{
    int i;
    struct line_pnts *Points;

    Points = Vect_new_line_struct();

    for (i = 0; i < skyline->n_points; i++) {
	double x, y;
	x = skyline->x[i] * cos(angle) - skyline->y[i] * sin(angle);
	y = skyline->x[i] * sin(angle) + skyline->y[i] * cos(angle);
	Vect_append_point(Points, x + p->x, y + p->y, 0);
    }

    return Points;
}

/**
 * This function rotates and translates the label bounding box to the
 * given point, and returns it as a polygon.
 * @param bb The bounding box to translate and rotate.
 * @param p The point to translate the skyline to
 * @param angle The angle (in radians) to rotate the label counter-clockwise
 * @return A lint_pnts structure containing the rotated and translated
 * bounding box as a polygon.
 */
static struct line_pnts *box_trans_rot(BOUND_BOX * bb, label_point_t * p,
				       double angle)
{
    struct line_pnts *Points;
    double x0, y0, x, y;

    Points = Vect_new_line_struct();

    x0 = bb->W * cos(angle) - bb->N * sin(angle);
    y0 = bb->W * sin(angle) + bb->E * cos(angle);
    Vect_append_point(Points, x0 + p->x, y0 + p->y, 0);

    x = bb->E * cos(angle) - bb->N * sin(angle);
    y = bb->E * sin(angle) + bb->N * cos(angle);
    Vect_append_point(Points, x + p->x, y + p->y, 0);

    x = bb->E * cos(angle) - bb->S * sin(angle);
    y = bb->E * sin(angle) + bb->S * cos(angle);
    Vect_append_point(Points, x + p->x, y + p->y, 0);

    x = bb->W * cos(angle) - bb->S * sin(angle);
    y = bb->W * sin(angle) + bb->S * cos(angle);
    Vect_append_point(Points, x + p->x, y + p->y, 0);
    // close line;
    Vect_append_point(Points, x0 + p->x, y0 + p->y, 0);

    return Points;
}

/** 
 * This function calculates the AveDist metric for line label candidates
 * @param label The label to which candidate belongs to.
 * @candidate The candidate of which we are to calculate the metric.
 * @return The metric;
 */
static double label_avedist(label_t * label, label_candidate_t * candidate)
{
    struct line_pnts *trsk;
    double avedist = 0.0;
    int i;

    G_debug(3, "Candidate point is: (%lf,%lf)",
	    candidate->point.x, candidate->point.y);
    trsk = skyline_trans_rot(label->skyline, &candidate->point,
			     candidate->rotation);

    for (i = 0; i < trsk->n_points; i++) {
	double d;
	Vect_line_distance(candidate->swathline, trsk->x[i], trsk->y[i],
			   0, 0, NULL, NULL, NULL, &d, NULL, NULL);
	avedist += d;
    }

    for (i = 0; i < candidate->swathline->n_points; i++) {
	double d;
	Vect_line_distance(trsk, candidate->swathline->x[i],
			   candidate->swathline->y[i], 0, 0,
			   NULL, NULL, NULL, &d, NULL, NULL);
	avedist += d;
    }

    avedist /= (candidate->swathline->n_points + trsk->n_points);
    Vect_destroy_line_struct(trsk);

    return ((avedist - ideal_distance) * (avedist - ideal_distance)) /
	(ideal_distance * ideal_distance);
}

/** 
 * This function calculates the Flatness metric for line label candidates
 * @param label The label to which candidate belongs to.
 * @candidate The candidate of which we are to calculate the metric.
 * @return The metric;
 */
static double label_flatness(label_t * label, label_candidate_t * candidate)
{
    struct line_pnts *line;
    double flatness = 0.0, x, y;
    int i;

    line = Vect_new_line_struct();
    Vect_append_point(line, candidate->point.x, candidate->point.y, 0);
    x = candidate->point.x +
	(label->bb.E - label->bb.W) * cos(candidate->rotation);
    y = candidate->point.y +
	(label->bb.E - label->bb.W) * sin(candidate->rotation);
    Vect_append_point(line, x, y, 0);

    for (i = 0; i < candidate->swathline->n_points; i++) {
	double d;
	Vect_line_distance(line, candidate->swathline->x[i],
			   candidate->swathline->y[i], 0, 0,
			   NULL, NULL, NULL, &d, NULL, NULL);
	flatness += d;
    }

    flatness /= (candidate->swathline->n_points);
    Vect_destroy_line_struct(line);

    return (flatness * flatness) / (ideal_distance * ideal_distance);
}

/**
 * This function checks if the label candidate overlaps with a pointfeature.
 * And calculates the PointOver metric.
 * @param label The label whose candidate we are investigating.
 * @param candidate The label candidate we are investigating.
 * @return The unweighted raw score of the PointOver metric for this label.
 */
static double label_pointover(label_t * label, label_candidate_t * candidate)
{
    double pointover;
    struct ilist *il;
    struct line_pnts *trsk;
    BOUND_BOX bb;

    il = Vect_new_list();

    trsk = skyline_trans_rot(label->skyline, &candidate->point,
			     candidate->rotation);

    bb.N = label->bb.N + candidate->point.y + buffer;
    bb.E = label->bb.E + candidate->point.x + buffer;
    bb.W = label->bb.W + candidate->point.x - buffer;
    bb.S = label->bb.S + candidate->point.y - buffer;
    bb.T = label->bb.T;
    bb.B = label->bb.B;
    Vect_select_lines_by_box(&Map, &bb, GV_POINT, il);

    pointover = (double)il->n_values;
    Vect_destroy_list(il);

    return pointover;
}

/**
 * This function calculates the LineOver metric for a label candidate.
 * @param label The label whose candidate we are investigating.
 * @param candidate The label candidate we are investigating.
 * @return The unweighted raw score of the LineOver metric for this label.
 */
static double label_lineover(label_t * label, label_candidate_t * candidate,
			     int linetype)
{
    double lineover = 0.0;
    struct ilist *il;
    struct line_pnts *trsk;
    label_point_t b;
    int i;
    BOUND_BOX bb;

    il = Vect_new_list();
    G_debug(3, "Candidate point is: (%lf,%lf)",
	    candidate->point.x, candidate->point.y);
    trsk = skyline_trans_rot(label->skyline, &candidate->point,
			     candidate->rotation);
    b.x = (label->bb.E - label->bb.W) * cos(candidate->rotation);
    b.y = (label->bb.E - label->bb.W) * sin(candidate->rotation);

    Vect_select_lines_by_polygon(&Map, trsk, 0, NULL, linetype, il);

    bb.N = label->bb.N + candidate->point.y;
    bb.E = label->bb.E + candidate->point.x;
    bb.W = label->bb.W + candidate->point.x;
    bb.S = label->bb.S + candidate->point.y;
    bb.T = label->bb.T;
    bb.B = label->bb.B;

    for (i = 0; i < il->n_values; i++) {
	struct line_pnts *line;
	label_point_t *v;

	line = Vect_new_line_struct();
	Vect_read_line(&Map, line, NULL, il->value[i]);
	v = lineover_intersection(&bb, line);
	if (v == NULL) {
	    continue;		/* no overlap */
	}
	else {
	    lineover += 1.0 + 9.0 * fabs((v->x * b.x + v->y * b.y))
		/ (sqrt(v->x * v->x + v->y * v->y) *
		   sqrt(b.x * b.x + b.y * b.y));
	}
	free(v);
    }

    Vect_destroy_list(il);

    return lineover;
}

/**
 * This function checks a line and a box intersect.
 * @param bb The box to check
 * @param line The polyline to check.
 * @return The vector of the overlap. NULL if there is no overlap.
 */
static label_point_t *lineover_intersection(BOUND_BOX * bb,
					    struct line_pnts *line)
{
    int i, found = 0;
    label_point_t *ret;
    double dx1[4], dy1[4], dx2[4], dy2[4];

    ret = malloc(sizeof(label_point_t));
    ret->x = 0.0;
    ret->y = 0.0;

    dy1[0] = bb->N;
    dx1[0] = bb->W;
    dy2[0] = bb->N;
    dx2[0] = bb->E;

    dy1[1] = bb->S;
    dx1[1] = bb->E;
    dy2[1] = bb->N;
    dx2[1] = bb->E;

    dy1[2] = bb->S;
    dx1[2] = bb->W;
    dy2[2] = bb->S;
    dx2[2] = bb->E;

    dy1[3] = bb->S;
    dx1[3] = bb->W;
    dy2[3] = bb->N;
    dx2[3] = bb->W;

    /* simply check each segment for intersection with each edge of the bb */
    for (i = 1; i < line->n_points; i++) {
	double x1, x2, y1, y2, z1, z2;
	int r, j;
	for (j = 0; j < 4; j++) {
	    r = Vect_segment_intersection(dx1[j], dy1[j], 0, dx2[j], dy2[j], 0,
					  line->x[i - 1], line->y[i - 1], 0,
					  line->x[i], line->y[i], 0,
					  &x1, &y1, &z1, &x2, &y2, &z2, 0);
	    if (r == 1) {	/* intersection at one point */
		if (!found) {
		    found++;
		    ret->x = x1;
		    ret->y = y1;
		}
		else {
		    found++;
		    ret->x -= x1;
		    ret->y -= y1;
		    break;
		}
	    }
	}
	if (found)
	    break;
    }
    if (!found) {
	free(ret);
	return NULL;
    }
    if (found == 1) {
	ret->x = fabs(ret->x - (line->x[line->n_points - 1]));
	ret->y = fabs(ret->y - (line->y[line->n_points - 1]));
    }
    else {
	ret->x = fabs(ret->x);
	ret->y = fabs(ret->y);
    }
    return ret;
}

/**
 * This function calculates the minimum distance between a skyline and a swath line.
 * @param skyline The skyline to investigate.
 * @param swathline The swath line to investigate.
 * @param p The point on the skyline which is neares to the swath line is stored in this structure.
 * @return The distance in map units.
 */
static double min_dist_2_lines(struct line_pnts *skyline,
			       struct line_pnts *swathline, label_point_t * p)
{
    int i;
    double dist = 10000000000000000.0;

    for (i = 0; i < skyline->n_points; i++) {
	double x, y, d;
	Vect_line_distance(swathline, skyline->x[i], skyline->y[i], 0, 0,
			   &x, &y, NULL, &d, NULL, NULL);
	if (d < dist) {
	    dist = d;
	    p->x = skyline->x[i];
	    p->y = skyline->y[i];
	}
    }

    for (i = 0; i < swathline->n_points; i++) {
	double x, y, d;
	Vect_line_distance(skyline, swathline->x[i], swathline->y[i], 0, 0,
			   &x, &y, NULL, &d, NULL, NULL);
	if (d < dist) {
	    dist = d;
	    p->x = x;
	    p->y = y;
	}
    }

    return dist;
}

void label_candidate_overlap(label_t * labels, int n_labels)
{
    int i;
    fprintf(stderr, "Finding label overlap: ...");
    for (i = 0; i < n_labels; i++) {
	int j;
	for (j = 0; j < labels[i].n_candidates; j++) {
	    int k;
	    for (k = i + 1; k < n_labels; k++) {
		int l;
		for (l = 0; l < labels[k].n_candidates; l++) {
		    int overlap = 0;
		    struct line_pnts *a = NULL, *b = NULL;

		    if ((labels[i].candidates[j].rotation == 0) &&
			(labels[k].candidates[l].rotation == 0)) {
			BOUND_BOX a, b;
			a.N = labels[i].bb.N + labels[i].candidates[j].point.y;
			a.E = labels[i].bb.E + labels[i].candidates[j].point.x;
			a.W = labels[i].bb.W + labels[i].candidates[j].point.x;
			a.S = labels[i].bb.S + labels[i].candidates[j].point.y;

			b.N = labels[k].bb.N + labels[k].candidates[l].point.y;
			b.E = labels[k].bb.E + labels[k].candidates[l].point.x;
			b.W = labels[k].bb.W + labels[k].candidates[l].point.x;
			b.S = labels[k].bb.S + labels[k].candidates[l].point.y;
			overlap = box_overlap(&a, &b);
		    }
		    else {
			a = box_trans_rot(&labels[i].bb,
					  &labels[i].candidates[j].point,
					  labels[i].candidates[j].rotation);
			b = box_trans_rot(&labels[k].bb,
					  &labels[k].candidates[l].point,
					  labels[k].candidates[l].rotation);
			overlap = box_overlap2(a, b);
			Vect_destroy_line_struct(a);
			Vect_destroy_line_struct(b);
		    }
		    if (overlap) {
			int n;
			label_intersection_t *li;
			n = ++(labels[i].candidates[j].n_intersections);
			li = realloc(labels[i].candidates[j].intersections,
				     n * sizeof(label_intersection_t));
			if (li == NULL)
			    G_fatal_error("\nUnable to allocate memory\n");
			li[n - 1].label = &labels[k];
			li[n - 1].candidate = l;
			if ((labels[k].current_candidate == l) &&
			    (labels[i].current_candidate == j)) {
			    labels[i].current_score += 80.0;
			    labels[k].current_score += 80.0;
			}
			labels[i].candidates[j].intersections = li;
			n = ++(labels[k].candidates[l].n_intersections);
			li = realloc(labels[k].candidates[l].intersections,
				     n * sizeof(label_intersection_t));
			if (li == NULL)
			    G_fatal_error("\nUnable to allocate memory\n");
			li[n - 1].label = &labels[i];
			li[n - 1].candidate = j;

			labels[k].candidates[l].intersections = li;
		    }
		}
	    }
	}
	G_percent(i, n_labels, 1);
    }
    G_percent(n_labels, n_labels, 1);
}

/**
 * This function checks if the two given boxes overlap.
 * @param a Bounding box A
 * @param b Bounding box B
 * @return REtruns 1 if the two boxes overlap 0 if not.
 */
static int box_overlap(BOUND_BOX * a, BOUND_BOX * b)
{
    int vert = 0, hori = 0;
    if (((a->W < b->W) && (b->W < a->E)) || ((a->W < b->E) && (b->E < a->E)))
	vert = 1;
    if (((b->W < a->W) && (a->W < b->E)) || ((b->W < a->E) && (a->E < b->E)))
	vert = 1;

    if (((a->S < b->S) && (b->S < a->N)) || ((a->S < b->N) && (b->N < a->N)))
	hori = 1;
    if (((b->S < a->S) && (a->S < b->N)) || ((b->S < a->N) && (a->N < b->N)))
	hori = 1;

    return (hori && vert);
}

/**
 * This function checks if two rotated boxes overlap. The boxes are stored
 * as polygons and the function assumes that each box has exactly 4 sides. 
 * @param a Bounding box A
 * @param b Bounding box B
 * @return returns 1 if the given boxes overlap. 0 if not.
 */
static int box_overlap2(struct line_pnts *a, struct line_pnts *b)
{
    int i;
    for (i = 0; i < (a->n_points - 1); i++) {
	int j;
	for (j = 0; j < (b->n_points - 1); j++) {
	    double d[6];
	    return Vect_segment_intersection(a->x[i], a->y[i], 0,
					     a->x[i + 1], a->y[i + 1], 0,
					     b->x[j], b->y[j], 0,
					     b->x[j + 1], a->y[j + 1], 0,
					     &d[0], &d[1], &d[2],
					     &d[3], &d[4], &d[5], 0);
	}
    }
    return 0;
}
