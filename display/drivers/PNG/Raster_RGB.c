
#include "pngdriver.h"

static int red[256], grn[256], blu[256];

void PNG_RGB_set_colors(
	const unsigned char *r, const unsigned char *g, const unsigned char *b)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		red[i] = r[i];
		grn[i] = g[i];
		blu[i] = b[i];
	}
}

void PNG_RGB_raster(
	int num, int nrows,
	const unsigned char *r, const unsigned char *g, const unsigned char *b,
	const unsigned char *nul)
{
	int x, y;

	for (x = 0; x < num; x++)
	{
		int our_x = cur_x + x;
		int c;

		if (nul && nul[x])
			continue;

		c = PNG_lookup_color(red[r[x]], grn[g[x]], blu[b[x]]);

		for (y = 0; y < nrows; y++)
		{
			int our_y = cur_y + y;

			grid[our_y * width + our_x] = c;
		}
	}

	modified = 1;
}

