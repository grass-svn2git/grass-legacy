/* Program: ps.map
**
** This is an enhanced PostScript version of the p.map program.
**
** Author: Paul W. Carlson	1992
*/

#define MAIN
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "map_info.h"
#include "vector.h"
#include "labels.h"
#include "header.h"
#include "comment.h"
#include "colortable.h"
#include "decorate.h"
#include "ps_info.h"
#include "group.h"
#include "local_proto.h"

FILE *tracefd;
FILE *inputfd;
int do_mapinfo;
int do_vlegend;
char *ps_mask_file;

#define KEY(x) (strcmp(key,x)==0)

static char *help[]=
{
    "rast       rastermap             setcolor   val_range(s) color",
    "vpoints    vector points map     scalebar   [f|s]",
    "vlines     vector lines map      paper      [a4|a3|us-letter|...]",
    "vareas     vector areas map      maploc     x y [width height]",
    "labels     labelfile             text       east north text",
    "region     regionfile            line       east north east north",
    "grid       spacing               point      east north",
    "geogrid    spacing               header",
    "outline                          mapinfo",
    "colortable [y|n]                 vlegend",
    "comments   [unix-file]           psfile     PostScript include file",
    "read       unix-file             eps        Encapsulated PostScript file",
    "verbose    [0|1|2]               rectangle  east north east north",
    "scale      1:#|# inches|# panels|1 inch = # miles",
    ""
};
int verbose;
int rotate_plot;
int eps_output;
int ps_copies = 1;

#include <signal.h>
int main(int argc,char *argv[])
{
    char buf[1024];
    char name[100], mapset[50];
    int i;
    int iflag;
    int can_reset_scale ;
    int copies_set;
    struct Option *map_scale;
    struct Option *input_file;
    struct Option *output_file;
    struct Option *copies;
    struct Flag *rflag, *pflag, *eflag;
    struct GModule *module;
    static char *def_font = "Helvetica";

    /**************** begin ******************************/
    verbose = 2;

    signal (SIGINT, exit);
    signal (SIGTERM, exit);

    setbuf (stderr, NULL);

    G_gisinit(argv[0]) ;
    
    /* Set description */
    module              = G_define_module();
    module->description = ""\
    "Hardcopy PostScript map output utility.";

    rflag = G_define_flag();
    rflag->key = 'r';
    rflag->description = "rotate plot";

    pflag = G_define_flag();
    pflag->key = 'p';
    pflag->description = "print paper formats ( name width height left right tob bottom(margin) )";

    eflag = G_define_flag();
    eflag->key = 'e';
    eflag->description = "create EPS (Encapsulated PostScript) instead of PostScript file";

    input_file = G_define_option();
    input_file->key = "input";
    input_file->type = TYPE_STRING;
    input_file->description = "file containing mapping instructions (or use input=- to enter from keyboard)";
    input_file->gisprompt = "file,file,file";
    input_file->required = NO;

    map_scale = G_define_option();
    map_scale->key = "scale";
    map_scale->key_desc = "mapscale";
    map_scale->type = TYPE_STRING;
    map_scale->description = "scale of the output map, e.g. 1:25000 (default: 1panel)";

    copies = G_define_option();
    copies->key = "copies";
    copies->type = TYPE_STRING;
    copies->description = "number of copies to print";
    copies->required = NO;

    output_file = G_define_option();
    output_file->key = "output";
    output_file->type = TYPE_STRING;
    output_file->description = "PostScript output file";
    output_file->required = NO;

    if (!isatty(0)) G_disable_interactive();
    if (G_parser(argc, argv)) usage(0);

    /* Print papers */
    if ( pflag->answer ) {
	print_papers();
	exit(0);
    }
    
    rotate_plot = rflag->answer;
    eps_output = eflag->answer;
    /* set default paper */
    set_paper ("a4" );

    strcpy(buf,"black");
    BLACK = get_color_number(buf);
    strcpy(buf, "white");
    WHITE = get_color_number(buf);
    strcpy(buf, "grey"); 
    GREY  = get_color_number(buf);

    /* initialize */
    vector_init();
    
    copies_set = 0;
    m_info.x = m_info.y  = -1.0;
    vector.x = vector.y  = -1.0;
    ct.x     = ct.y      = -1.0;
    ct.width = -1.0;
    m_info.color    = BLACK;
    hdr.color       = BLACK;
    cmt.color       = BLACK;
    PS.grid_color   = BLACK;
    m_info.font = G_store(def_font);
    vector.font = G_store(def_font);
    hdr.font    = G_store(def_font);
    cmt.font    = G_store(def_font);
    ct.font     = G_store(def_font);
    m_info.fontsize = 10;
    vector.fontsize = 10;
    hdr.fontsize    = 10;
    cmt.fontsize    = 10;
    ct.fontsize     = 10;
    ct.cols     = 1;
    tracefd = NULL;
    inputfd = stdin;
    iflag = 0;
    labels.count = 0;
    labels.other = NULL;
    can_reset_scale = 1;
    hdr.fp = NULL;
    grp.do_group = 0;
    PS.grey = 0;
    PS.mask_needed = 0;
    PS.do_header = 0;
    PS.min_y = 72.0 * (PS.page_height - PS.top_marg);
    PS.set_y = 100.0 * PS.min_y;
    PS.startpanel = 0;
    PS.endpanel = 0;
    PS.cell_fd = -1;
    PS.do_outline = 0;
    PS.do_colortable = 0;
    PS.do_scalebar = 0;
    PS.grid = 0;
    PS.scaletext[0] = 0;
    PS.celltitle[0] = 0;
    PS.commentfile = NULL;
    PS.num_psfiles = 0;
    PS.mask_color = 0;

    /* PS.map_* variables are set to 0 (not defined) and then may be reset by 'maploc'.
     * When script is read, main() should call reset_map_location() to reset map size to fit to paper */
     
    PS.map_width = 0;
    PS.map_height = 0;
    PS.map_x_orig = 0;
    PS.map_y_orig = 0;
    PS.map_y_loc = 0;

    /* arguments */
    if (input_file->answer && strcmp (input_file->answer, "-"))
    {
	if (NULL == freopen(input_file->answer, "r", stdin))
	{
	    fprintf(stderr, "%s - ", G_program_name());
	    perror(input_file->answer);
	    exit(1);
	}
    }
    if (map_scale->answer)
    {
	can_reset_scale = isatty(0);
	if (check_scale(map_scale->answer))
	    strcpy(PS.scaletext, map_scale->answer);
	else error(map_scale->answer, "", "illegal scale request");
    }
    if (copies->answer)
    {
	if (sscanf(copies->answer, "%d", &ps_copies) != 1
		|| ps_copies < 1 || ps_copies > 20)
	{
	    ps_copies = 1;
	    error(copies->answer, "", "illegal copies request");
	}
	copies_set = 1;
    }
    if (output_file->answer)
    {
	if ((PS.fp = fopen(output_file->answer, "w")) == NULL)
	{
	    fprintf(stderr, "%s - ", G_program_name());
	    perror(output_file->answer);
	    exit(1);
	}
    }
    else usage(1);

    /* get current mapset */
    PS.cell_mapset = G_mapset();

    /* set current window */
    G_get_set_window(&PS.w);
    if (G_set_window(&PS.w) == -1)
	G_fatal_error("Current region not settable");

    while (1)
    {
	char *key;
	char *data;

	if (!input(1, buf, help))
	{
	    if (!iflag) break;
	    iflag = 0;
	    continue;
	}
	if (!key_data(buf, &key, &data))
	    continue ;

	if (KEY("read"))
	{
	    if (inputfd != stdin) fclose(inputfd);

	    if (sscanf(data, "%s", name) != 1)
	    {
		error(key, data, "no file specified");
		inputfd = stdin;
	    }
	    else if ((inputfd = fopen(name, "r")) == NULL)
	    {
		error(key, data, "unable to open");
		inputfd = stdin;
	    }
	    else iflag = 1;
	    continue;
	}

	if (KEY("verbose"))
	{
	    if (sscanf(data, "%d", &verbose) != 1) verbose = 2;
	    continue;
	}

	if (KEY("paper"))
	{
	    if ( strlen (data) > 0 ) {
                set_paper ( data );
	    }
	    read_paper ();
	      	
	    continue;
	}

	if (KEY("maploc"))
	{
	    int n;
	    double x, y, w, h;

	    n = sscanf(data, "%lf %lf %lf %lf", &x, &y, &w, &h);
	    if ( n == 2 || n ==4 ) {
	        PS.map_x_orig = x;
                PS.map_y_loc = y;
	        if ( n == 4) {
    		    PS.map_width = w;
    		    PS.map_height = h;
		}
	    }
	    else
	    {
		error(key, data, "illegal maploc request");
		gobble_input();
	    }
	    continue;
	}

	if (KEY("copies"))
	{
	    int n, copies;

	    if (copies_set) continue;
	    n = sscanf(data, "%d", &copies);
	    if (n != 1 || copies < 1 || copies > 20)
	    {
		ps_copies = 1;
		error(key, data, "illegal copies request");
	    }
	    ps_copies = copies;
	    continue;
	}

	if (KEY("setcolor"))
	{
	    float R,G,B;
	    int r,g,b;
	    int color;
	    int count;
	    DCELL *val_list;
	    DCELL dmin,dmax;
	    char colorbuf[100];
	    char catsbuf[100];

	    if (PS.cell_fd < 0)
	    {
		error (key,data,"no raster file selected yet");
		continue;
	    }
	    if (sscanf (data, "%s %[^\n]", catsbuf, colorbuf) == 2)
	    {
	        color = get_color_number(colorbuf);
	        if (color < 0)
	        {
	    	    error(key, data, "illegal color");
	            continue;
                }
		get_color_rgb(color, &R, &G, &B);
		r= 255.0 * R;
		g = 255.0 * G;
		b = 255.0 * B;

		if(strncmp(catsbuf, "null", 4)== 0)
		{
		    G_set_null_value_color(r,g,b,&PS.colors);
		    continue;
                }
		if(strncmp(catsbuf, "default", 7)== 0)
		{
		    G_set_default_color(r,g,b,&PS.colors);
		    continue;
                }
		if ((count = parse_val_list (catsbuf, &val_list)) < 0)
		{
		    error (key,data,"illegal value list");
		    continue;
		}
		for (i = 0; i < count; i += 2)
		{
		    dmin = val_list[i];
		    dmax = val_list[i+1];
		    G_add_d_raster_color_rule (&dmin, r,g,b, &dmax,r,g,b, &PS.colors);
		}
		free (val_list);
	    }
	    continue;
	}

	if (KEY("colortable"))
	{
	    PS.do_colortable = 0;
	    /*
	    if (PS.cell_fd < 0)
		error(key, data, "no raster file selected yet");
	    else
	    */
		PS.do_colortable = yesno(key, data);
	    if (PS.do_colortable) read_colortable();
	    continue;
	}
	
	if (KEY("scalebar"))
	{
	   PS.do_scalebar = 1;
	   if (sscanf(data, "%s", sb.type) != 1)
	   	strcpy (sb.type, "f"); /* default to fancy scalebar */
	   read_scalebar();
	   if (sb.length <= 0.) {
		   error(key, data, "Bad scalebar length");
		   gobble_input();
	   }
	   continue;
	}
	
	if (KEY("text"))
	{
	    double e, n;
	    char east[50], north[50];
	    char text[200];

	    if (sscanf(data, "%s %s %[^\n]", east, north, text) == 3
	        && (scan_easting(east, &e) && scan_northing(north, &n)))
		    read_text(east, north, text);
	    else
	    {
		gobble_input();
		error(key, data, "illegal text request");
	    }
	    continue;
	}

	if (KEY("point"))
	{
	    double e, n;
	    char east[50], north[50];

	    if (sscanf(data, "%s %s", east, north) == 2
	        && (scan_easting(east, &e) && scan_northing(north, &n)))
		    read_point(e, n);
	    else
	    {
		gobble_input();
		error(key, data, "illegal point request");
	    }
	    continue;
	}

	if (KEY("eps"))
	{
	    double e, n;
	    char east[50], north[50];

	    if (sscanf(data, "%s %s", east, north) == 2
	        && (scan_easting(east, &e) && scan_northing(north, &n)))
		    read_eps(e, n);
	    else
	    {
		gobble_input();
		error(key, data, "illegal eps request");
	    }
	    continue;
	}

	if (KEY("line"))
	{
	    char east1[50], north1[50];
	    char east2[50], north2[50];
	    double e1, n1, e2, n2;

	    if (sscanf(data, "%s %s %s %s", east1, north1, east2, north2) == 4
	        && (scan_easting(east1, &e1)  && scan_easting(east2, &e2)
	        && scan_northing(north1, &n1) && scan_northing(north2, &n2)))
		    read_line(e1, n1, e2, n2);
	    else
	    {
		gobble_input();
		error(key, data, "illegal line request");
	    }
	    continue;
	}

	if (KEY("rectangle"))
	{
	    char east1[50], north1[50];
	    char east2[50], north2[50];
	    double e1, n1, e2, n2;

	    if (sscanf(data, "%s %s %s %s", east1, north1, east2, north2) == 4
	        && (scan_easting(east1, &e1)  && scan_easting(east2, &e2)
	        && scan_northing(north1, &n1) && scan_northing(north2, &n2)))
		    read_rectangle(e1, n1, e2, n2);
	    else
	    {
		gobble_input();
		error(key, data, "illegal rectangle request");
	    }
	    continue;
	}

	if (KEY("comments"))
	{
	    switch (sscanf(data, "%s %s", name, mapset))
	    {
	        case 1: read_comment(name); break;
	        case 2: error(key, data, "illegal comments request"); break;
	        default: read_comment(""); break;
	    }
	    continue;
	}

	if (KEY("scale"))
	{
	    if (!can_reset_scale) continue;
	    if (check_scale(data)) strcpy(PS.scaletext, data);
	    else
	    {
		PS.scaletext[0] = 0;
		error(key, data, "illegal scale request");
	    }
	    continue;
	}

	if (KEY("labels"))
	{
	    if (scan_gis("paint/labels", "label", key, data, name, mapset, 1))
		read_labels(name, mapset);
	    continue;
	}

	if (KEY("header"))
	{
	    read_header();
	    PS.do_header = 1;
	    continue;
	}

	if (KEY("mapinfo"))
	{
	    read_info();
	    do_mapinfo = 1;
	    continue;
	}

	if (KEY("vlegend"))
	{
	    read_vlegend();
	    do_vlegend = 1;
	    continue;
	}

	if (KEY("outline"))
	{
	    if (PS.cell_fd < 0)
	    {
		error(key, data, "no raster file selected yet");
		gobble_input();
	    }
	    else read_outline();
	    continue;
	}

	if (KEY("cell") || KEY("rast") || KEY("raster"))
	{
	    if (scan_gis("cell", "raster", key, data, name, mapset, 0))
		read_cell(name, mapset);
	    continue;
	}

	if (KEY("greyrast") || KEY("grayrast"))
	{
	    if (scan_gis("cell", "raster", key, data, name, mapset, 0))
		read_cell(name, mapset);
	    PS.grey = 1;
	    continue;
	}

	if (KEY("group"))
	{
	    G_strip(data);
	    if (I_find_group(data)) 
	    {
		grp.group_name = G_store(data);
		grp.do_group = 1;
	 	read_group();
	    }
	    else error(key, data, "group not found");
	    continue;
	}

	if (KEY("rgb"))
	{
	    G_strip(data);
	    grp.do_group = 1;
	    read_rgb(key, data);
	    continue;
	}

	if (KEY("vpoints"))
	{
	    if (scan_gis("vector", "vector", key, data, name, mapset, 1))
		read_vpoints(name, mapset);
	    continue;
	}

	if (KEY("vlines") )
	{
	    if (scan_gis("vector", "vector", key, data, name, mapset, 1))
		read_vlines(name, mapset);
	    continue;
	}
 
	if (KEY("vareas") )
	{
	    if (scan_gis("vector", "vector", key, data, name, mapset, 1))
	    	read_vareas(name, mapset);
	    continue;
	}

	if (KEY ("window") || KEY("region"))
	{
	    if (scan_gis("windows", "region definition", key, data, name,
		mapset, 1)) read_wind(name, mapset);
	    continue;
	}

	if (KEY("grid"))
	{
	    PS.grid = -1;
	    PS.grid_numbers = 0;
	    sscanf(data, "%d", &PS.grid);
	    if (PS.grid < 0)
	    {
		PS.grid = 0;
		error(key, data, "illegal grid spacing");
		gobble_input();
	    }
	    else getgrid();
	    continue;
	}

        if (KEY("geogrid"))
        {
	if (G_projection() == PROJECTION_LL || G_projection() == PROJECTION_XY) {
		error(key, data, "geogrid is not available for this projection");
		gobble_input();
	}
            PS.geogrid = -1.;
            PS.geogrid_numbers = 0;
            sscanf(data, "%d %s", &PS.geogrid, PS.geogridunit);
            if (PS.geogrid < 0)
            {
                PS.geogrid = 0;
                error(key, data, "illegal geo-grid spacing");
                gobble_input();
            }
            else getgeogrid();
            continue;
        }

	if (KEY("psfile"))
	{
    	    if (PS.num_psfiles >= MAX_PSFILES) continue;
	    G_strip(data);
	    PS.psfiles[PS.num_psfiles] = G_store(data);
	    PS.num_psfiles++;
	    continue;
	}

	if (KEY("maskcolor"))
	{
	    int ret, r, g, b;

	    ret = G_str_to_color( data, &r,  &g,  &b);
	    if ( ret == 1 ) {
		PS.mask_r = r/255;
		PS.mask_g = g/255;
		PS.mask_b = b/255;
		PS.mask_color = 1;
		continue;
	    } else if ( ret == 2 ) { /* none */
		continue; 
	    } else {
		error (key,data,"illegal color request");
	    }
	}

	if (*key)
	    error(key,"","illegal request");
    }

    /* reset map location base on 'paper' on 'location' */
    reset_map_location ( );

    /* write the PostScript output file */
    ps_mask_file = G_tempfile();
    ps_map();
    if (verbose > 1)
    {
        fprintf (stdout,"PS-PAINT: PostScript file \"%s\" successfully written.\n",
    		output_file->answer);
        fflush(stdout);
    }
    unlink (ps_mask_file);
    exit(0);
}


int 
usage (int full)
{
    if (full) G_usage();
    exit(1);
}
