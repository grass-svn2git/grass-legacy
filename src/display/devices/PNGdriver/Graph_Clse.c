/*
 * Close down the graphics processing.  This gets called only at driver
 * termination time.
 */

#include <stdio.h>
#include <png.h>

#include "gis.h"
#include "pngdriver.h"

int
Graph_Close(void)
{
	static jmp_buf jbuf;
	static png_struct *png_ptr;
	static png_info *info_ptr;
	FILE *output;
	int x, y;
	unsigned int *p;
	png_bytep line;

	png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, &jbuf, NULL, NULL);
	if (!png_ptr)
		G_fatal_error("PNG: couldn't allocate PNG structure");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		G_fatal_error("PNG: couldn't allocate PNG structure");

	if (setjmp(png_jmpbuf(png_ptr)))
		G_fatal_error("error writing PNG file");

	output = fopen(file_name, "wb");
	if (!output)
		G_fatal_error("PNG: couldn't open output file %s", file_name);

	png_init_io(png_ptr, output);

	png_set_IHDR(
		png_ptr, info_ptr,
		width, height, 8,
		true_color ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_PALETTE,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	if (!true_color)
	{
		png_color png_pal[256];
		png_byte trans = (png_byte) transparent;
		int i;

		for (i = 0; i < 256; i++)
		{
			png_pal[i].red   = palette[i][0];
			png_pal[i].green = palette[i][1];
			png_pal[i].blue  = palette[i][2];
		}

		png_set_PLTE(png_ptr, info_ptr, png_pal, 256);

		png_set_tRNS(png_ptr, info_ptr, &trans, 1, NULL);
	}

	png_set_invert_alpha(png_ptr);

	png_write_info(png_ptr, info_ptr);

	line = G_malloc(width * 4);

	for (y = 0, p = grid; y < height; y++)
	{
		png_bytep q = line;

		if (true_color)
			for (x = 0; x < width; x++, p++)
			{
				unsigned int c = *p;
				*q++ = (png_byte) (c >> 16);
				*q++ = (png_byte) (c >>  8);
				*q++ = (png_byte) (c >>  0);
				*q++ = (png_byte) (c >> 24);
			}
		else
			for (x = 0; x < width; x++, p++, q++)
				*q = (png_byte) *p;

		png_write_row(png_ptr, line);
	}

	G_free(line);

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(output);
	G_free(grid);

	return 0;
}

