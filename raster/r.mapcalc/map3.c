
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "gis.h"
#include "G3d.h"
#include "btree.h"

#include "mapcalc.h"
#include "globals.h"
#include "globals3.h"

/****************************************************************************/

G3D_Region current_region3;

void setup_region(void)
{
    	G3d_initDefaults();
	G3d_getWindow(&current_region3);

	rows = current_region3.rows;
	columns = current_region3.cols;
	depths = current_region3.depths;
}

/****************************************************************************/

typedef struct map
{
	const char *name;
	const char *mapset;
	int have_cats;
	int have_colors;
	int min_row, max_row;
	void *handle;
	int fd;
	struct Categories cats;
	struct Colors colors;
	BTREE btree;
} map;

/****************************************************************************/

static map *maps;
static int num_maps;
static int max_maps;

static unsigned char *red, *grn, *blu;
static unsigned char *set;

static int min_row = INT_MAX;
static int max_row = -INT_MAX;
static int min_col = INT_MAX;
static int max_col = -INT_MAX;

/****************************************************************************/

static int handle_to_fd(void *handle)
{
	return (int) handle;
}

static void *fd_to_handle(int fd)
{
	return (void *) fd;
}

static void read_row(void *handle, char *buf, int type, int depth, int row)
{
	int i;

	switch (type)
	{
	case CELL_TYPE:
		for (i = 0; i < columns; i++)
		{
			double x;

			G3d_getValueRegion(
				handle, i, row, depth, (char *) &x, G3D_DOUBLE);
			if (G3d_isNullValueNum(&x, G3D_DOUBLE))
				SET_NULL_C(&((CELL*)buf)[i]);
			else
				((CELL*)buf)[i] = (CELL) x;
		}
		break;
	case FCELL_TYPE:
		for (i = 0; i < columns; i++)
		{
			float x;

			G3d_getValueRegion(
				handle, i, row, depth, (char *) &x, G3D_FLOAT);
			if (G3d_isNullValueNum(&x, G3D_FLOAT))
				SET_NULL_F(&((FCELL*)buf)[i]);
			else
				((FCELL*)buf)[i] = x;
		}
		break;
	case DCELL_TYPE:
		for (i = 0; i < columns; i++)
		{
			double x;

			G3d_getValueRegion(
				handle, i, row, depth, (char *) &x, G3D_DOUBLE);
			if (G3d_isNullValueNum(&x, G3D_DOUBLE))
				SET_NULL_D(&((DCELL*)buf)[i]);
			else
				((DCELL*)buf)[i] = x;
		}
		break;
	}
}

static void write_row(void *handle, const char *buf, int type, int depth, int row)
{
	int i;

	switch (type)
	{
	case CELL_TYPE:
		for (i = 0; i < columns; i++)
		{
			double x;

			if (IS_NULL_C(&((CELL*)buf)[i]))
				G3d_setNullValue(&x, 1, G3D_DOUBLE);
			else
				x = ((CELL*)buf)[i];

			if (G3d_putValue(handle, i, row, depth, (char *) &x, G3D_DOUBLE) < 0)
				G_fatal_error("error writing data");
		}
		break;
	case FCELL_TYPE:
		for (i = 0; i < columns; i++)
		{
			float x;

			if (IS_NULL_F(&((FCELL*)buf)[i]))
				G3d_setNullValue(&x, 1, G3D_FLOAT);
			else
				x = ((FCELL*)buf)[i];

			if (G3d_putValue(handle, i, row, depth, (char *) &x, G3D_FLOAT) < 0)
				G_fatal_error("error writing data");
		}
		break;
	case DCELL_TYPE:
		for (i = 0; i < columns; i++)
		{
			double x;

			if (IS_NULL_D(&((DCELL*)buf)[i]))
				G3d_setNullValue(&x, 1, G3D_DOUBLE);
			else
				x = ((DCELL*)buf)[i];

			if (G3d_putValue(handle, i, row, depth, (char *) &x, G3D_DOUBLE) < 0)
				G_fatal_error("error writing data");
		}
		break;
	}
}

/****************************************************************************/

static int compare_ints(int *a, int *b)
{
	return *a - *b;
}

static void init_colors(map *m)
{
	if (!red) red = G_malloc(columns);
	if (!grn) grn = G_malloc(columns);
	if (!blu) blu = G_malloc(columns);
	if (!set) set = G_malloc(columns);

	if (G3d_readColors((char *) m->name, (char *) m->mapset, &m->colors) < 0)
		G_fatal_error("error reading color file for [%s in %s]\n",
			      m->name, m->mapset);

	m->have_colors = 1;
}

static void init_cats(map *m)
{
	if (G3d_readCats((char *) m->name, (char *) m->mapset, &m->cats) < 0)
		G_fatal_error("error reading category file for [%s in %s]\n",
			      m->name, m->mapset);

	if (!btree_create(&m->btree, compare_ints, 1))
		G_fatal_error("unable to create btree for [%s in %s]\n",
			      m->name, m->mapset);

	m->have_cats = 1;
}

static void translate_from_colors(map *m, DCELL *rast, CELL *cell, int ncols, int mod)
{
	int i;

	G_lookup_d_raster_colors(rast, red, grn, blu, set, ncols, &m->colors);

	switch (mod)
	{
	case 'r':
		for (i = 0; i < ncols; i++)
			cell[i] = red[i];
		break;
	case 'g':
		for (i = 0; i < ncols; i++)
			cell[i] = grn[i];
		break;
	case 'b':
		for (i = 0; i < ncols; i++)
			cell[i] = blu[i];
		break;
	case '#': /* grey (backwards compatible) */
		/* old weightings: R=0.177, G=0.813, B=0.011 */
		for (i = 0; i < ncols; i++)
			cell[i] = (181*red[i] + 833*grn[i] + 11*blu[i] + 512) / 1024;
		break;
	case 'y': /* grey (NTSC) */
		/* NTSC weightings: R=0.299, G=0.587, B=0.114 */
		for (i = 0; i < ncols; i++)
			cell[i] = (306*red[i] + 601*grn[i] + 117*blu[i] + 512) / 1024;
		break;
	case 'i': /* grey (equal weight) */
		for (i = 0; i < ncols; i++)
			cell[i] = (red[i] + grn[i] + blu[i]) / 3;
		break;
	case 'M':
	case '@':
	default:
		G_fatal_error("invalid map modifier: '%c'\n", mod);
		break;
	}
}

/* convert cell values to double based on the values in the
 * category file.
 *
 * This requires performing sscanf() of the category label
 * and only do it it for new categories. Must maintain
 * some kind of maps of already scaned values.
 *
 * This maps is a hybrid tree, where the data in each node
 * of the tree is an array of, for example, 64 values, and
 * the key of the tree is the category represented by the
 * first index of the data
 *
 * To speed things up a little, use shifts instead of divide or multiply
 * to compute the key and the index
 *
 * This uses the BTREE library to manage the tree itself
 * btree structure must already be intialized
 * pcats structure must already contain category labels
 */

#define SHIFT 6
#define NCATS (1<<SHIFT)

static void translate_from_cats(map *m, CELL *cell, DCELL *xcell, int ncols)
{
	struct Categories *pcats;
	BTREE *btree;
	int i, idx;
	CELL cat, key;
	double vbuf[1<<SHIFT];
	double *values;
	char *label;

	btree = &m->btree;
	pcats = &m->cats;

	for (; ncols-- > 0; cell++, xcell++)
	{
		cat = *cell;
		if (IS_NULL_C(cell))
		{
			SET_NULL_D(xcell);
			continue;
		}

/* compute key as cat/NCATS * NCATS, adjusting down for negatives
 * and idx so that key+idx == cat
 */
		if (cat < 0)
			key = - (((-cat-1) >> SHIFT) << SHIFT) - NCATS;
		else
			key = (cat >> SHIFT) << SHIFT;
		idx = cat - key;

/* If key not already in the tree, sscanf() all cats for this key
 * and put them into the tree
 */
		if (!btree_find(btree, (char *)&key, (char **)&values))
		{
			values = vbuf;
			for (i = 0; i < NCATS; i++)
			{
				if ((label = G_get_cat((CELL)(i+key), pcats)) == NULL
				    || sscanf(label, "%lf", values) != 1)
					SET_NULL_D(values);
				values++;
			}

			values = vbuf;
			btree_update(btree, (char *)&key, sizeof(key),
				      (char *)values, sizeof(vbuf));
		}

/* and finally lookup the translated value */
		if (IS_NULL_D(&values[idx]))
			SET_NULL_D(xcell);
		else
			*xcell = values[idx];
	}
}

static void column_shift(void *buf, int res_type, int col)
{
	CELL *ibuf = buf;
	FCELL *fbuf = buf;
	DCELL *dbuf = buf;
	int i;

	/* if column offset, copy cell to itself shifting by col */
	if (col > 0)
	{
		switch (res_type)
		{
		case CELL_TYPE:
			for (i = 0; i < columns - col; i++)
			{
				if (IS_NULL_C(&ibuf[i + col]))
					SET_NULL_C(&ibuf[i]);
				else
					ibuf[i] = ibuf[i + col];
			}
			for ( ; i < columns; i++)
				SET_NULL_C(&ibuf[i]);
			break;

		case FCELL_TYPE:
			for (i = 0; i < columns - col; i++)
			{
				if (IS_NULL_F(&fbuf[i + col]))
					SET_NULL_F(&fbuf[i]);
				else
					fbuf[i] = fbuf[i + col];
			}
			for ( ; i < columns; i++)
				SET_NULL_F(&fbuf[i]);
			break;

		case DCELL_TYPE:
			for (i = 0; i < columns - col; i++)
			{
				if (IS_NULL_D(&dbuf[i + col]))
					SET_NULL_D(&dbuf[i]);
				else
					dbuf[i] = dbuf[i + col];
			}
			for ( ; i < columns; i++)
				SET_NULL_D(&dbuf[i]);
			break;
		}
	}
	else if (col < 0)
	{
		col = -col;
		switch (res_type)
		{
		case CELL_TYPE:
			for (i = columns - 1; i >= col; i--)
			{
				if (IS_NULL_C(&ibuf[i - col]))
					SET_NULL_C(&ibuf[i]);
				else
					ibuf[i] = ibuf[i - col];
			}
			for ( ; i >= 0; i--)
				SET_NULL_C(&ibuf[i]);
			break;

		case FCELL_TYPE:
			for (i = columns - 1; i >= col; i--)
			{
				if (IS_NULL_F(&fbuf[i - col]))
					SET_NULL_F(&fbuf[i]);
				else
					fbuf[i] = fbuf[i - col];
			}
			for ( ; i >= 0; i--)
				SET_NULL_F(&fbuf[i]);
			break;

		case DCELL_TYPE:
			for (i = columns - 1; i >= col; i--)
			{
				if (IS_NULL_D(&dbuf[i - col]))
					SET_NULL_D(&dbuf[i]);
				else
					dbuf[i] = dbuf[i - col];
			}
			for ( ; i >= 0; i--)
				SET_NULL_D(&dbuf[i]);
			break;
		}
	}
}

static void setup_map(map *m)
{
}

static void read_map(
	map *m, void *buf, int res_type, int depth, int row, int col)
{
	CELL *ibuf = buf;
	FCELL *fbuf = buf;
	DCELL *dbuf = buf;

	if (row < 0 || row >= rows || depth < 0 || depth >= depths)
	{
		int i;

		switch (res_type)
		{
		case CELL_TYPE:
			for (i = 0; i < columns; i++)
				SET_NULL_C(&ibuf[i]);
			break;
		case FCELL_TYPE:
			for (i = 0; i < columns; i++)
				SET_NULL_F(&fbuf[i]);
			break;
		case DCELL_TYPE:
			for (i = 0; i < columns; i++)
				SET_NULL_D(&dbuf[i]);
			break;
		default:
			G_fatal_error("read_map: unknown type: %d", res_type);
			break;
		}

		return;
	}

	read_row(m->handle, buf, res_type, depth, row);

	if (col)
		column_shift(buf, res_type, col);
}

static void close_map(map *m)
{
	if (!m->handle)
		return;

	if (!G3d_closeCell(m->handle))
		G_fatal_error("unable to close map [%s in %s]",
			      m->name, m->mapset);

	if (m->have_cats)
	{
		btree_free(&m->btree);
		G_free_cats(&m->cats);
		m->have_cats = 0;
	}

	if (m->have_colors)
	{
		G_free_colors(&m->colors);
		m->have_colors = 0;
	}
}

/****************************************************************************/

int map_type(const char *name, int mod)
{
	char *mapset, *tmpname;
	int result;

	switch (mod)
	{
	case 'M':
		tmpname = G_store((char *) name);
		mapset = G_find_grid3(tmpname, "");
		if (mapset)
		{
			void *handle = G3d_openCellOld(
				tmpname, mapset, &current_region3,
				G3D_TILE_SAME_AS_FILE, G3D_NO_CACHE);
			result = (G3d_fileTypeMap == G3D_FLOAT)
				? FCELL_TYPE
				: DCELL_TYPE;
			G3d_closeCell(handle);
		}
		else
			result = -1;
		G_free(tmpname);
		return result;
	case '@':
		return DCELL_TYPE;
	case 'r':
	case 'g':
	case 'b':
	case '#':
	case 'y':
	case 'i':
		return CELL_TYPE;
	default:
		G_fatal_error("invalid map modifier: '%c'", mod);
		return -1;
	}
}

int open_map(const char *name, int mod, int row, int col)
{
	int i;
	char *mapset, *tmpname;
	int use_cats = 0;
	int use_colors = 0;
	map *m;

	if (row < min_row) min_row = row;
	if (row > max_row) max_row = row;
	if (col < min_col) min_col = col;
	if (col > max_col) max_col = col;

	tmpname = G_store((char *) name);
	mapset = G_find_grid3(tmpname, "");
	G_free(tmpname);

	if (!mapset)
		G_fatal_error("open_map: map [%s] not found", name);

	switch (mod)
	{
	case 'M':
		break;
	case '@':
		use_cats = 1;
		break;
	case 'r':
	case 'g':
	case 'b':
	case '#':
	case 'y':
	case 'i':
		use_colors = 1;
		break;
	default:
		G_fatal_error("internal error: open_map: invalid map modifier: '%c'", mod);
		break;
	}

	for (i = 0; i < num_maps; i++)
	{
		m = &maps[i];

		if (strcmp(m->name, name) != 0 ||
		    strcmp(m->mapset, mapset) != 0)
			continue;

		if (row < m->min_row) m->min_row = row;
		if (row > m->max_row) m->max_row = row;

		if (use_cats && !m->have_cats)
			init_cats(m);

		if (use_colors && !m->have_colors)
			init_colors(m);

		return i;
	}


	if (num_maps >= max_maps)
	{
		max_maps += 10;
		maps = G_realloc(maps, max_maps * sizeof(map));
	}

	m = &maps[num_maps];

	m->name = name;
	m->mapset = mapset;
	m->have_cats = 0;
	m->have_colors = 0;
	m->min_row = row;
	m->max_row = row;

	if (use_cats)
		init_cats(m);
	if (use_colors)
		init_colors(m);

	m->handle = G3d_openCellOld(
		(char *) name, (char *) mapset, &current_region3,
		G3D_DOUBLE, G3D_USE_CACHE_DEFAULT);

	if (!m->handle)
		G_fatal_error("unable to open map [%s in %s]", name, mapset);

	return num_maps++;
}

void setup_maps(void)
{
	int i;

	for (i = 0; i < num_maps; i++)
		setup_map(&maps[i]);
}

void get_map_row(int idx, int mod, int depth, int row, int col, void *buf, int res_type)
{
	static CELL *ibuf;
	static DCELL *fbuf;
	map *m = &maps[idx];

	switch (mod)
	{
	case 'M':
		read_map(m, buf, res_type, depth, row, col);
		break;
	case '@':
		if (!ibuf)
			ibuf = G_malloc(columns * sizeof(CELL));
		read_map(m, ibuf, CELL_TYPE, depth, row, col);
		translate_from_cats(m, ibuf, buf, columns);
		break;
	case 'r':
	case 'g':
	case 'b':
	case '#':
	case 'y':
	case 'i':
		if (!fbuf)
			fbuf = G_malloc(columns * sizeof(DCELL));
		read_map(m, fbuf, DCELL_TYPE, depth, row, col);
		translate_from_colors(m, fbuf, buf, columns, mod);
		break;
	default:
		G_fatal_error("internal error: get_map_row: invalid map modifier: '%c'", mod);
		break;
	}
}

void close_maps(void)
{
	int i;

	for (i = 0; i < num_maps; i++)
		close_map(&maps[i]);

	num_maps = 0;
}

/****************************************************************************/

int open_output_map(const char *name, int res_type)
{
	void *handle;
	int fd;

	handle = G3d_openCellNew(
		(char *) name,
		res_type == FCELL_TYPE ? G3D_FLOAT : G3D_DOUBLE,
		G3D_USE_CACHE_DEFAULT,
		&current_region3);

	if (!handle)
		G_fatal_error("cannot create output map [%s]", name);

	fd = handle_to_fd(handle);

	return fd;
}

void put_map_row(int fd, void *buf, int res_type)
{
	void *handle = fd_to_handle(fd);

	write_row(handle, buf, res_type, current_depth, current_row);
}

void close_output_map(int fd)
{
	void *handle = fd_to_handle(fd);

	if (!G3d_closeCell(handle))
		G_fatal_error("unable to close output map");
}

void unopen_output_map(int fd)
{
	close_output_map(fd);
}

/****************************************************************************/

void copy_cats(const char *dst, int idx)
{
	const map *m = &maps[idx];
	struct Categories cats;

	if (G3d_readCats((char *) m->name, (char *) m->mapset, &cats) < 0)
		return;

	G3d_writeCats((char *) dst, &cats);
	G_free_cats(&cats);
}

void copy_colors(const char *dst, int idx)
{
	const map *m = &maps[idx];
	struct Colors colr;

	if (G3d_readColors((char *) m->name, (char *) m->mapset, &colr) <= 0)
		return;

	G3d_writeColors((char *) dst, G_mapset(), &colr);
	G_free_colors(&colr);
}

void copy_history(const char *dst, int idx)
{
}

void create_history(const char *dst, expression *e)
{
}

/****************************************************************************/

