#include <string.h>
#include <grass/dbmi.h>
#include "global.h"

static int get_field_cat(struct Map_info *, char *, int *, int *);

static char **field_names = NULL;
static int num_fields = 0, *field_cat = NULL;
static struct field_info **Fi = NULL;
static dbDriver *driver = NULL;
static dbString sql, str;
static char buf[1000];

void write_vect(struct Map_info *Map, char *layer, char *entity, char *label,
		int arr_size, int type)
{
    struct line_cats *Cats;
    int i, field, cat;

    for (i = 0; i < arr_size; i++)
	check_ext(xpnts[i], ypnts[i], zpnts[i]);

    /* copy xyzpnts to Points */
    Vect_copy_xyz_to_pnts(Points, xpnts, ypnts, zpnts, arr_size);

    /* set field and cat numbers */
    Cats = Vect_new_cats_struct();
    if (!flag_table) {
	i = get_field_cat(Map, layer, &field, &cat);
	sprintf(buf, "insert into %s (%s"
		", layer"
		", entity"
		", label" ") values (%d, '", Fi[i]->table, Fi[i]->key, cat);

	if (layer) {
	    db_set_string(&str, layer);
	    db_double_quote_string(&str);
	    strcat(buf, db_get_string(&str));
	}
	strcat(buf, "', '");

	if (entity) {
	    db_set_string(&str, entity);
	    db_double_quote_string(&str);
	    strcat(buf, db_get_string(&str));
	}
	strcat(buf, "', '");

	if (label) {
	    db_set_string(&str, label);
	    db_double_quote_string(&str);
	    strcat(buf, db_get_string(&str));
	}
	strcat(buf, "')");

	db_set_string(&sql, buf);
	if (db_execute_immediate(driver, &sql) != DB_OK)
	    G_fatal_error(_("Cannot execute: %s"), db_get_string(&sql));
	db_free_string(&sql);
    }
    else
	get_field_cat(Map, layer, &field, &cat);
    Vect_cat_set(Cats, field, cat);

    /* write */
    Vect_write_line(Map, type, Points, Cats);

    Vect_destroy_cats_struct(Cats);

    return;
}

void write_done(struct Map_info *Map)
{
    int i;

    if (!(found_layers = (num_fields > 0))) {
	fprintf(stderr, _("\nNo DXF layers found!\n"));
	return;
    }

    if (!flag_table) {
	db_commit_transaction(driver);
	db_close_database_shutdown_driver(driver);
    }

    fprintf(stderr, _("\nFollowing DXF layers found:\n"));
    for (i = 0; i < num_fields; i++) {
	fprintf(stderr, _("Layer %d: %s\n"), i + 1, field_names[i]);
	G_free(field_names[i]);
	if (!flag_table) {
	    if (flag_one_layer && i > 0)
		continue;
	    /* no function to do this? */
	    G_free(Fi[i]->name);
	    G_free(Fi[i]->table);
	    G_free(Fi[i]->key);
	    G_free(Fi[i]->database);
	    G_free(Fi[i]->driver);
	    G_free(Fi[i]);
	}
    }
    G_free(field_names);
    G_free(field_cat);

    num_fields = 0;
    field_names = NULL;
    field_cat = NULL;

    if (!flag_table) {
	G_free(Fi);
	Fi = NULL;
	driver = NULL;
    }

    return;
}

static int get_field_cat(struct Map_info *Map, char *field_name, int *field,
			 int *cat)
{
    int i, type;

    for (i = 0; i < num_fields; i++) {
	/* field name already exists */
	if (strcmp(field_name, field_names[i]) == 0) {
	    /* for -1 flag, *field should be always 1 */
	    if (flag_one_layer)
		i = 0;
	    *field = i + 1;
	    *cat = ++field_cat[i];
	    return i;
	}
    }

    num_fields++;

    /* create new field */
    field_names = (char **)G_realloc(field_names, (i + 1) * sizeof(char *));
    field_names[i] = G_store(field_name);

    /* for -1 flag, *field should be always 1 */
    if (flag_one_layer)
	i = 0;

    if (!flag_one_layer || !field_cat) {
	field_cat = (int *)G_realloc(field_cat, (i + 1) * sizeof(int));
	field_cat[i] = 0;
    }

    /* assign field and cat numbers */
    *field = i + 1;
    *cat = ++field_cat[i];

    /* do not create tables */
    if (flag_table)
	return i;

    /* create a table */

    /* only one table */
    if (flag_one_layer) {
	if (Fi)
	    return i;
	type = GV_1TABLE;
    }
    else
	type = GV_MTABLE;

    Fi = (struct field_info **)G_realloc(Fi,
					 (i + 1) * sizeof(struct field_info *));

    Fi[i] = Vect_default_field_info(Map, *field, field_name, type);
    {
	char *p;

	/* make table name SQL compliant */
	for (p = Fi[i]->table; *p; p++)
	    if (*p == '.' || *p == '-')
		*p = '_';
    }

    if (!driver) {
	driver =
	    db_start_driver_open_database(Fi[i]->driver,
					  Vect_subst_var(Fi[i]->database, Map));
	if (!driver)
	    G_fatal_error(_("Cannot open database %s by driver %s"),
			  Vect_subst_var(Fi[i]->database, Map), Fi[i]->driver);

	db_begin_transaction(driver);

	db_init_string(&sql);
	db_init_string(&str);
    }

    sprintf(buf, "create table %s (cat integer"
	    ", layer varchar(%d)"
	    ", entity varchar(%d)"
	    ", label varchar(%d)"
	    ")", Fi[i]->table, DXF_BUF_SIZE, DXF_BUF_SIZE, DXF_BUF_SIZE);
    db_set_string(&sql, buf);

    if (db_execute_immediate(driver, &sql) != DB_OK)
	G_fatal_error(_("Cannot create table: %s"), db_get_string(&sql));
    db_free_string(&sql);

    if (db_grant_on_table
	(driver, Fi[i]->table, DB_PRIV_SELECT, DB_GROUP | DB_PUBLIC) != DB_OK)
	G_fatal_error(_("Cannot grant privileges on table %s"), Fi[i]->table);
    if (db_create_index2(driver, Fi[i]->table, Fi[i]->key) != DB_OK)
	G_warning(_("Cannot create index"));

    if (Vect_map_add_dblink(Map, *field, field_name, Fi[i]->table, "cat",
			    Fi[i]->database, Fi[i]->driver))
	G_warning(_("Cannot link table: %s"), Fi[i]->table);

    return i;
}
