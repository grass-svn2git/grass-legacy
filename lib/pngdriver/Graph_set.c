/*
 * Start up graphics processing.  Anything that needs to be assigned, set up,
 * started-up, or otherwise initialized happens here.  This is called only at
 * the startup of the graphics driver.
 *
 * The external variables define the pixle limits of the graphics surface.  The
 * coordinate system used by the applications programs has the (0,0) origin
 * in the upper left-hand corner.  Hence,
 *    screen_left < screen_right
 *    screen_top  < screen_bottom 
 */

#include <string.h>
#include <stdlib.h>
#ifndef __MINGW32__
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include <grass/gis.h>
#include "pngdriver.h"

char *file_name;
int currentColor;
int true_color;
int auto_write;
int has_alpha;
int mapped;

int clip_top, clip_bot, clip_left, clip_rite;
int width, height;
void *image;
unsigned int *grid;
unsigned char palette[256][4];
unsigned int background;
int modified;

static void map_file(void)
{
#ifndef __MINGW32__
	size_t size = HEADER_SIZE + width * height * sizeof(unsigned int);
	void *ptr;
	int fd;

	if (!mapped)
		return;

	mapped = 0;
	write_image();

	fd = open(file_name, O_RDWR);
	if (fd < 0)
		return;

	ptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t ) 0);
	if (ptr == MAP_FAILED)
		return;

	G_free(grid);
	grid = (int *)((char *) ptr + HEADER_SIZE);

	close(fd);

	mapped = 1;
#endif
}

int PNG_Graph_set(int argc, char **argv)
{
	unsigned int red, grn, blu;
	char *p;

	G_gisinit("PNG driver") ;

	p = getenv("GRASS_PNGFILE");
	if (!p || strlen(p) == 0)
		p = FILE_NAME;

	file_name = p;

	p = getenv("GRASS_TRUECOLOR");
	true_color = p && strcmp(p, "TRUE") == 0;

	G_message("PNG: GRASS_TRUECOLOR status: %s",
		true_color ? "TRUE" : "FALSE");

	p = getenv("GRASS_PNG_AUTO_WRITE");
	auto_write = p && strcmp(p, "TRUE") == 0;

	p = getenv("GRASS_PNG_MAPPED");
	mapped = p && strcmp(p, "TRUE") == 0;

	width = screen_right - screen_left;
	height = screen_bottom - screen_top;

	clip_top = screen_top;
	clip_bot = screen_bottom;
	clip_left = screen_left;
	clip_rite = screen_right;

	grid = G_malloc(width * height * sizeof(unsigned int));

	p = getenv("GRASS_TRANSPARENT");
	if (p && strcmp(p, "TRUE") == 0)
		has_alpha = 1;

	init_color_table();

	p = getenv("GRASS_BACKGROUNDCOLOR");
	if (p && *p && sscanf(p, "%02x%02x%02x", &red, &grn, &blu) == 3)
		background = get_color(red, grn, blu, has_alpha ? 255 : 0);
	else
	{
		/* 0xffffff = white, 0x000000 = black */
		if(strcmp(DEFAULT_FG_COLOR, "white") == 0)
			/* foreground: white, background: black */
			background = get_color(0, 0, 0, has_alpha ? 255 : 0);
		else
			/* foreground: black, background: white */
			background = get_color(255, 255, 255, has_alpha ? 255 : 0);
	}

	PNG_Erase();

	G_message("PNG: collecting to file: %s,\n     GRASS_WIDTH=%d, GRASS_HEIGHT=%d",
		file_name, width, height);

	modified = 1;

	p = getenv("GRASS_PNG_READ");
	if (p && strcmp(p, "TRUE") == 0)
		read_image();

	if (mapped)
		map_file();

	return 0;
}

