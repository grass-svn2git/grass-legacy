
/**
 * \file percent.c
 *
 * \brief GIS Library - percentage progress functions.
 *
 * (C) 2001-2008 by the GRASS Development Team
 *
 * This program is free software under the GNU General Public License
 * (>=v2). Read the file COPYING that comes with GRASS for details.
 *
 * \author GRASS GIS Development Team
 *
 * \date 1999-2008
 */

#include <stdio.h>
#include <grass/gis.h>


static int prev = -1;
static int first = 1;


/**
 * \brief Print percent complete messages.
 *
 * This routine prints a percentage complete message to stderr. The
 * percentage complete is <i>(<b>n</b>/<b>d</b>)*100</i>, and these are 
 * printed only for each <b>s</b> percentage. This is perhaps best 
 * explained by example:
\code
  #include <stdio.h>
  #include <grass/gis.h>
  int row;
  int nrows;
  nrows = 1352; // 1352 is not a special value - example only

  G_message(_("Percent complete..."));
  for (row = 0; row < nrows; row++)
  {
      G_percent (row+1, nrows, 10);
      do_calculation(row);
  }
\endcode
 *
 * This example code will print completion messages at 10% increments;
 * i.e., 10%, 20%, 30%, etc., up to 100%. Each message does not appear
 * on a new line, but rather erases the previous message.
 * 
 * Note that to prevent the illusion of the module stalling, the G_percent()
 * call is placed before the time consuming part of the for loop, and an
 * additional call is added after the loop to "finish it off" at 100%.
 *
 * \param n current element
 * \param d total number of elements
 * \param s increment size
 *
 * \return always returns 0
 */

int G_percent(long n, long d, int s)
{
    return (G_percent2(n, d, s, stderr));
}


/**
 * \brief Print percent complete messages.
 *
 * This routine prints a percentage complete message to the file given
 * by the out parameter. Otherwise usage is the same as G_percent().
 *
 * \param n current element
 * \param d total number of elements
 * \param s increment size
 * \param[out] out file to print to
 *
 * \return always returns 0
 */

int G_percent2(long n, long d, int s, FILE *out)
{
    int x, format;

    format = G_info_format();

    x = (d <= 0 || s <= 0)
	? 100 : (int)(100 * n / d);

    /* be verbose only 1> */
    if (format == G_INFO_FORMAT_SILENT || G_verbose() < 1)
	return 0;

    if (n <= 0 || n >= d || x > prev + s) {
	prev = x;

	if (format == G_INFO_FORMAT_STANDARD) {
	    if (out != NULL) {
		fprintf(out, "%4d%%\b\b\b\b\b", x);
	    }
	}
	else {
	    if (format == G_INFO_FORMAT_PLAIN) {
		if (out != NULL) {
		    if (x == 100)
			fprintf(out, "%d\n", x);
		    else
			fprintf(out, "%d..", x);
		}
	    }
	    else {		/* GUI */
		if (out != NULL) {
		    if (first) {
			fprintf(out, "\n");
		    }
		    fprintf(out, "GRASS_INFO_PERCENT: %d\n", x);
		    fflush(out);
		}
		first = 0;
	    }
	}
    }

    if (x >= 100) {
	if (format == G_INFO_FORMAT_STANDARD) {
	    if (out != NULL) {
		fprintf(out, "\n");
	    }
	}
	prev = -1;
	first = 1;
    }

    return 0;
}


/**
 * \brief Reset G_percent() to 0%; do not add newline.
 *
 * \return always returns 0
 */

int G_percent_reset(void)
{
    prev = -1;
    first = 1;

    return 0;
}
