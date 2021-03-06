/* Functions: do_map_header, read_header_file
**
** Author: Paul W. Carlson	April 1992
*/

#include "ps_info.h"
#include "header.h"

static double x, y, dy, fontsize;

do_map_header(date)
char *date;
{
    char buf[100];
    char temp[100];

    /* set color and font */
    set_rgb_color(hdr.color);
    fontsize = (double)hdr.fontsize;
    fprintf(PS.fp, "(%s) FN %.1lf SF\n", hdr.font, fontsize);

    /* set start of first line */
    dy = 1.2 * fontsize;
    y = 72.0 * (PS.page_height - PS.top_marg) - fontsize - 1.5;

    if (hdr.fp == NULL)
    {

    	if (PS.celltitle[0])
    	{
	    fprintf(PS.fp, "/t (TITLE:  %s) def\n", PS.celltitle);
	    fprintf(PS.fp, "t SW pop %.1lf XS D2 t exch %.1lf MS\n", 
		72 * PS.page_width, y);
	    y -= dy;
	}
    	strcpy(temp, G_myname());
    	G_strip(temp);
    	if (*temp == 0) strcpy(temp, G_location());
	fprintf(PS.fp, "/t (LOCATION:  %s) def\n", temp);
	fprintf(PS.fp, "t SW pop %.1lf XS D2 t exch %.1lf MS\n", 
	    72 * PS.page_width, y);
	y -= 0.25 * dy;
    	if (PS.min_y > y) PS.min_y = y;
	return;
    }
	    
    x = 72.0 * PS.left_marg + 1.5;
    read_header_file(date);
    y -= 0.25 * dy;
    if (PS.min_y > y) PS.min_y = y;
}

    
read_header_file(date)
char *date;
{
    char buf[1024];

    while (G_getl(buf, sizeof buf, hdr.fp)) output(buf, date);
    fclose (hdr.fp);
}


static output(line, date)
char *line, *date;
{
    char text[1024];
    char fmt[30];
    char *get_format();
    char *buf;

    buf = line;
    *text = 0;
    while (*buf)
    {
	if (*buf == '%')
	{
	    buf++;
	    if (*buf == '%') strcat (text, "%");
	    else if (*buf == 'n')
	    {
		if (*text) show_text(x, y, text);
		y -= dy;
		return;
	    }
	    else if (*buf == '_')
	    {
		fprintf(PS.fp, "BW ");
		draw_line(x, y + 0.2 * fontsize,
			  72.0 * (PS.page_width - PS.right_marg), 
			  y + 0.2 * fontsize);
		y -= dy;
		return;
	    }
	    else
	    {
		buf = get_format(buf, fmt);
		append('s', fmt);
		switch(*buf)
		{
		    case 'd': apply(date,          fmt, text); break;
		    case 'l': apply(G_location(),  fmt, text); break;
		    case 'L': apply(G_myname(),    fmt, text); break;
		    case 'c': 
			    if (PS.cell_fd >= 0)
			    {
			        char name[100];

			        sprintf(name, "<%s> in mapset <%s>",
				    PS.cell_name, PS.cell_mapset);
			        apply(name,        fmt, text);
			    }
			    else apply("none",     fmt, text);
			    break;
		    case 'm': apply(G_mapset(),    fmt, text); break;
		    case 'u': apply(G_whoami(),    fmt, text); break;
		    case 'x': apply(G_mask_info(), fmt, text); break;
		    case 0: continue;
		}
	    }
	}
	else append(*buf, text);
	buf++;
    }
    if (*text) show_text(x, y, text);
    y -= dy;
}


static char *get_format(buf, fmt)
char *buf, *fmt;
{
    strcpy(fmt, "%");
    if (*buf == '-')
    {
	append(*buf++, fmt);
	if (*buf < '1' || *buf > '9') return(buf - 1);
    }
    while (*buf >= '0' && *buf <= '9') append(*buf++, fmt);
    return buf;
}


static append(c, buf)
char c, *buf;
{
    while (*buf) buf++;
    *buf++ = c;
    *buf = 0;
}


static apply(buf, fmt, text)
char *buf, *fmt, *text;
{
    char temp[300];

    sprintf(temp, fmt, buf);
    strcat(text, temp);
}
