/***************************************************************************
                 atcorr - atmospheric correction for Grass GIS

                                    was
        6s - Second Simulation of Satellite Signal in the Solar Spectrum.
                             -------------------
    begin                : Fri Jan 10 2003
    copyright            : (C) 2003 by Christo Zietsman
    email                : 13422863@sun.ac.za

    This program has been rewriten in C/C++ from the fortran source found
    on http://www.ltid.inpe.br/dsr/mauro/6s/index.html. This code is
    provided as is, with no implied warranty and under the conditions
    and restraint as placed on it by previous authors.

    atcorr is an atmospheric correction module for Grass GIS.  Limited testing
    has been done and therefore it should not be assumed that it will work
    for all possible inputs.

    Care has been taken to test new functionality brought to the module by
    Grass such as the command-line options and flags. The extra feature of
    supplying an elevation map has not been run to completion, because it
    takes to long and no sensible data for the test data was at hand.
    Testing would be welcomed. :)  
 ***************************************************************************/

extern "C" {
#include <grass/gis.h>
#include <grass/glocale.h>
}

#include "Transform.h"
#include "6s.h"
#include <stdlib.h>
#include <math.h>
#include <map>

/* Input options and flags */
struct Options
{
    /* options */
	struct Option *iimg;    /* input satelite image */
	struct Option *iscl;    /* input data is scaled to this range */
	struct Option *ialt;    /* an input elevation map in km used to increase */
                            /* atmospheric correction accuracy, including this */
                            /* will make computations take much, much longer */
    struct Option *ivis;    /* an input visibility map in km (same purpose and effect as ialt) */
	struct Option *icnd;    /* the input conditions file */
	struct Option *oimg;    /* output image name */
	struct Option *oscl;    /* scale the output data (reflectance values) to this range */

    /* flags */
	struct Flag *oflt;      /* output data as floating point and do not round */
	struct Flag *irad;      /* treat input values as reflectance instead of radiance values */
    struct Flag *etmafter;  /* treat input data as a satelite image of type etm+ taken after July 1, 2000 */
    struct Flag *etmbefore; /* treat input data as a satelite image of type etm+ taken before July 1, 2000 */
    struct Flag *optimize;
};

struct ScaleRange
{
    int min;
    int max;
};

/* 
 Adjust the region to that of the input raster.
 Atmospheric corrections should be done on the whole
 satelite image, not just portions.
 */
void adjust_region(char *name)
{
	struct Cell_head iimg_head;	/* the input image header file */

	if(G_get_cellhd(name, G_mapset(), &iimg_head) < 0) 
		G_fatal_error("Unable to retreive header dat for input image");
	if(G_set_window(&iimg_head) < 0) 
		G_fatal_error("Invalid graphics region coordinates");
}
/* Rounds a floating point cell value */
CELL round_c (FCELL x)
{
	if (x >= 0.0) return (CELL)(x + .5);
	else return (CELL)(-(-x + .5));
}

/* Converts the buffer to cell and write it to disk */
void write_fp_to_cell(int ofd, FCELL* buf)
{
	CELL* cbuf;
	int col;
	cbuf = (CELL*)G_allocate_raster_buf(CELL_TYPE);

	for(col = 0; col < G_window_cols(); col++) cbuf[col] = round_c(buf[col]);
	G_put_raster_row(ofd, cbuf, CELL_TYPE);
}

int hit = 0;
int mis = 0;

class TICache
{
    enum TICacheSize
    {
        MAX_TIs = 128 /* this value is a guess, increase it if in general more categories are used */
    };
    TransformInput tis[MAX_TIs];
    float alts[MAX_TIs];
    int p;

public:
    TICache() { p = 0; for(int i = 0; i < MAX_TIs; i++) alts[i] = -1; }
    int search(float alt) { 
	    for(int i = 0; i < MAX_TIs; i++) 
		    if(alt == alts[i]) 
		    {
			    hit++;
			    return i;
		    } 
	    mis++;
	    return -1; 
    }
    TransformInput get(int i) { return tis[i]; }
    void add(TransformInput ti, float alt) { 
	    tis[p] = ti; 
	    alts[p] = alt; 
	    p++; 
	    if(p >= MAX_TIs) p = 0; 
    }
};

/* the transform input map, is a array of ticaches.
 The first key is the visibility which matches to a TICache for the altitudes.
 This code is horrible, i just spent 20min writing and 5min debugging it. */
class TIMap
{
	enum TIMapSize
	{
		MAX_TICs = 128 /* this value is a guess. It means that 1024 TI's will be the max combinations of vis/alt pairs */
	};

	TICache tic[MAX_TICs]; /* array of TICaches */
	float visi[MAX_TICs];
	int p;

public:
	struct Position
	{
		int i, j;
		Position() : i(-1), j(-1) {}
		Position(int x, int y) : i(x), j(y) {}
		bool valid() { return i != -1 && j != -1; }
	};	
	
	TIMap() { p = 0; for(int i = 0; i < MAX_TICs; i++) visi[i] = -1; }
	Position search(float vis, float alt) { 
		for(int i = 0; i < MAX_TICs; i++)
			if(vis == visi[i]) {
				Position pos;
				pos.i = i;
				pos.j = tic[i].search(alt);
				return pos;
			} 
		return Position();
	}
	TransformInput get(Position pos) { return tic[pos.i].get(pos.j); }
	void add(TransformInput ti, float vis, float alt) {
		tic[p].add(ti, alt);
		visi[p] = vis;
		p++;
		if(p >= MAX_TICs) p = 0;
	}
};

struct IntPair
{
	FCELL x;
	FCELL y;
	
	IntPair(FCELL i, FCELL j) : x(i), y(j) {}
	
	bool operator<(const IntPair& b) const
	{
		if(x < b.x) return true;
		else if(x > b.x) return false;
		else if(y < b.y) return true;
		return false;
	}	
};

typedef std::map<IntPair, TransformInput> CacheMap;

const TransformInput& optimize_va(const FCELL& vis, const FCELL& alt)
{
	static CacheMap timap;
	static TransformInput ti;

	IntPair key(vis, alt);
	CacheMap::iterator it = timap.find(key);

	if(it != timap.end()) /* search found key */
	{
		ti = (*it).second;
	}
	else
	{
		pre_compute_hv(alt, vis);
		ti = compute();
		timap.insert(std::make_pair(key, ti));
	}
	
	return ti;
}	

/* Process the raster and do atmospheric corrections.
Params:
 * INPUT FILE
 ifd: input file descriptor
 iref: input file has radiance values (default is reflectance) ?
 iscale: input file's range (default is min = 0, max = 255)
 ialt_fd: height map file descriptor, negative if global value is used
 ivis_fd: visibility map file descriptor, negative if global value is used

 * OUTPUT FILE
 ofd: output file descriptor
 oflt: if true use FCELL_TYPE for output
 oscale: output file's range (default is min = 0, max = 255)
*/
void process_raster(int ifd, InputMask imask, ScaleRange iscale, int ialt_fd, int ivis_fd,
                    int ofd, bool oflt, ScaleRange oscale, bool optimize)
{
	FCELL* buf;         /* buffer for the input values */
    FCELL* alt = NULL;         /* buffer for the elevation values */
    FCELL* vis = NULL;         /* buffer for the visibility values */
    FCELL  prev_alt = -1.f;
    FCELL  prev_vis = -1.f;
	int row, col;

    /* do initial computation with global elevation and visibility values */
    TransformInput ti;
    ti = compute();

    TICache ticache;    /* use this to increase computation speed when an elevation map with categories are given */
	
    /* allocate memory for buffers */
	buf = (FCELL*)G_allocate_raster_buf(FCELL_TYPE);
    if(ialt_fd >= 0) alt = (FCELL*)G_allocate_raster_buf(FCELL_TYPE);
    if(ivis_fd >= 0) vis = (FCELL*)G_allocate_raster_buf(FCELL_TYPE);
    fprintf(stderr, "Percent complete: ");
    
	for(row = 0; row < G_window_rows(); row++)
	{
        	G_percent(row, G_window_rows(), 1);     /* keep the user informed of our progress */
		
        /* read the next row */
		if(G_get_raster_row(ifd, buf, row, FCELL_TYPE) < 0)
			G_fatal_error("Unable to read from input file");

        /* read the next row of elevation values */
        if(ialt_fd >= 0)
    		if(G_get_raster_row(ialt_fd, alt, row, FCELL_TYPE) < 0)
	    		G_fatal_error("Unable to read from elevation raster");

        /* read the next row of elevation values */
        if(ivis_fd >= 0)
    		if(G_get_raster_row(ivis_fd, vis, row, FCELL_TYPE) < 0)
	    		G_fatal_error("Unable to read from visibility raster");
		

        /* loop over all the values in the row */
		for(col = 0; col < G_window_cols(); col++)
		{
			if(isnan(vis[col]) || isnan(alt[col]) || isnan(buf[col])) {buf[col] = FP_NAN; continue;}
      alt[col] /= 1000.0f; /* converting to km from input which should be in meter */

            /* check if both maps are active and if whether any value has changed */
            if((ialt_fd >= 0) && (ivis_fd >= 0) && ((prev_vis != vis[col]) || (prev_alt != alt[col])))
            {
               	prev_alt = alt[col]; /* update new values */
               	prev_vis = vis[col];
 		if(optimize) ti = optimize_va(vis[col], alt[col]); /* try to optimize? */
		else { /* no optimizations */
		   pre_compute_hv(alt[col], vis[col]);
               	   ti = compute();
		}	
            }
            else    /* only one of the maps is being used */
            {
                if((ivis_fd >= 0) && (prev_vis != vis[col]))
                {
                    prev_vis = vis[col];        /* keep track of previous visibility */
                    
                    if(optimize)
                    {
                        int p = ticache.search(vis[col]);
                        if(p >= 0) ti = ticache.get(p);
                        else
                        {
                            pre_compute_v(vis[col]);    /* re-compute transformation inputs */
                            ti = compute();             /* ... */

                            ticache.add(ti, vis[col]);                        
                        }
                    }
                    else
                    {
                        pre_compute_v(vis[col]);    /* re-compute transformation inputs */
                        ti = compute();             /* ... */
                    }
                }

                if((ialt_fd >= 0) && (prev_alt != alt[col]))
                {
                    prev_alt = alt[col];        /* keep track of previous altitude */

                    if(optimize)
                    {
                        int p = ticache.search(alt[col]);
                        if(p >= 0) ti = ticache.get(p);
                        else
                        {
                            pre_compute_h(alt[col]);    /* re-compute transformation inputs */
                            ti = compute();             /* ... */

                            ticache.add(ti, alt[col]);
                        }
                    }
                    else
                    {
                        pre_compute_h(alt[col]);    /* re-compute transformation inputs */
                        ti = compute();             /* ... */
                    }
                }
            }

            /* transform from iscale.[min,max] to [0,1] */
            buf[col] = (buf[col] - iscale.min) / ((float)iscale.max - (float)iscale.min);
            buf[col] = transform(ti, imask, buf[col]);
            /* transform from [0,1] to oscale.[min,max] */
            buf[col] = buf[col] * ((float)oscale.max - (float)oscale.min) + oscale.min;
            if(~oflt && (buf[col] > (float)oscale.max))
              fprintf(stderr,"The output data will overflow. Reflectance > 100%\n");
		}

        /* write output */
		if(oflt) G_put_raster_row(ofd, buf, FCELL_TYPE);
		else write_fp_to_cell(ofd, buf);
	}

    /* free allocated memory */
	G_free(buf);
    if(ialt_fd >= 0) G_free(alt);
    if(ivis_fd >= 0) G_free(vis);
}



/* Copy the colors from map named iname to the map named oname */
void copy_colors(char *iname, char *oname)
{
	struct Colors colors;
	G_read_colors(iname, G_mapset(), &colors);
	G_write_colors(oname, G_mapset(), &colors);
}

/* Define our module so that Grass can print it if the user wants to know more. */
void define_module()
{
	struct GModule *module;
	module = G_define_module();
	module->description =
	 " 6s - Second Simulation of Satellite Signal in the Solar Spectrum.\n";/*
	 
	 " Incorporated into Grass by Christo A. Zietsman, January 2003.\n"
	 " Converted from Fortran to C by Christo A. Zietsman, November 2002.\n\n"
	 " Adapted by Mauro A. Homem Antunes for atmopheric corrections of\n"
	 " remotely sensed images in raw format (.RAW) of 8 bits.\n"
	 " April 4, 2001.\n\n"
	 " Please refer to the following paper and acknowledge the authors of\n"
	 " the model:\n"
	 " Vermote, E.F., Tanre, D., Deuze, J.L., Herman, M., and Morcrette,\n"
	 "    J.J., (1997), Second simulation of the satellite signal in\n"
	 "    the solar spectrum, 6S: An overview., IEEE Trans. Geosc.\n"
	 "    and Remote Sens. 35(3):675-686.\n"
	 " The code is provided as is and is not to be sold. See notes on\n"
	 " http://loasys.univ-lille1.fr/informatique/sixs_gb.html\n"
	 " http://www.ltid.inpe.br/dsr/mauro/6s/index.html\n"
	 " and on http://www.cs.sun.ac.za/~caz/index.html\n";*/
}

/* Define the options and flags */
struct Options define_options()
{
	struct Options opts;

	opts.iimg = G_define_option();
	opts.iimg->key		= "iimg";
	opts.iimg->type		= TYPE_STRING;
	opts.iimg->required	= YES;
	opts.iimg->gisprompt	= "old,cell,raster";
	opts.iimg->description	= "Input raster map to be corrected";
	opts.iimg->answer	= "ETM4_400x400.raw";

	opts.iscl = G_define_option();
	opts.iscl->key          = "iscl";
	opts.iscl->type         = TYPE_INTEGER;
	opts.iscl->key_desc     = "input scale range";
	opts.iscl->required     = NO;
/*	opts.iscl->answer       = "0,255"; */
	opts.iscl->description  = "Input raster's range [0,255]";

	opts.ialt = G_define_option();
	opts.ialt->key		= "ialt";
	opts.ialt->type		= TYPE_STRING;
	opts.ialt->required	= NO;
	opts.ialt->gisprompt	= "old,cell,raster";
	opts.ialt->answer	= "dem_float";
	opts.ialt->description	= "Input altitude map in m (optional)";

	opts.ivis = G_define_option();
	opts.ivis->key		= "ivis";
	opts.ivis->type		= TYPE_STRING;
	opts.ivis->required	= NO;
	opts.ivis->gisprompt	= "old,cell,raster";
	opts.ivis->answer	= "visibility";
	opts.ivis->description	= "Input visibility map in km (optional)";
    
	opts.icnd = G_define_option();
	opts.icnd->key		= "icnd";
	opts.icnd->type		= TYPE_STRING;
	opts.icnd->required	= YES;
	opts.icnd->gisprompt	= "old_file,file,file";
	opts.icnd->answer	= "ETM4_atmospheric_input_GRASS.txt";
	opts.icnd->description	= "6S input text file";

	opts.oimg = G_define_option();
	opts.oimg->key		= "oimg";
	opts.oimg->type		= TYPE_STRING;
	opts.oimg->required	= NO;
	opts.oimg->gisprompt	= "new,cell,raster";
	opts.oimg->answer	= "6s_output_file";
	opts.oimg->description	= "Output raster map";

	opts.oscl = G_define_option();
	opts.oscl->key          = "oscl";
	opts.oscl->type         = TYPE_INTEGER;
	opts.oscl->key_desc     = "Output scale range";
	opts.oscl->required     = NO;
/*	opts.oscl->answer       = "0,255"; */
	opts.oscl->description  = "Rescale output raster map [0,255]";

	opts.oflt = G_define_flag();
	opts.oflt->key = 'f';
	opts.oflt->description = "Output is floating point";

	opts.irad = G_define_flag();
	opts.irad->key = 'r';
	opts.irad->description = "Input map converted to reflectance (default is radiance)";

	opts.etmafter = G_define_flag();
	opts.etmafter->key = 'a';
	opts.etmafter->description = "Input from ETM+ image taken after July 1, 2000";

	opts.etmbefore = G_define_flag();
	opts.etmbefore->key = 'b';
	opts.etmbefore->description = "Input from ETM+ image taken before July 1, 2000";

	opts.optimize = G_define_flag();
	opts.optimize->key = 'o';
	opts.optimize->description = "Try to increase computation speed when categorized altitude or/and visibility map is used.";

	return opts;
}

/* Read the min and max values from the iscl and oscl options */
void read_scale(Option *scl, ScaleRange &range)
{
    /* set default values */
    range.min = 0;
    range.max = 255;
    
    if(scl->answer)
    {
        sscanf(scl->answers[0], "%d", &range.min);
        sscanf(scl->answers[1], "%d", &range.max);

        if(range.min==range.max)
        {
            fprintf(stderr, "Scale range length should be >0; Using default values: 0,255\n\n");
            range.min = 0;
            range.max = 255;
        }
    }

    /* swap values if max is smaller than min */
    if(range.max < range.min)
    {
        int temp;
        temp = range.max;
        range.max = range.min;
        range.min = temp;
    }
}

int main(int argc, char* argv[])
{
	struct Options opts;        
    struct ScaleRange iscale;   /* input file's data is scaled to this interval */
    struct ScaleRange oscale;   /* output file's scale */
	int iimg_fd;	        /* input image's file descriptor */
	int oimg_fd;	        /* output image's file descriptor */
	int ialt_fd;            /* input elevation map's file descriptor */
    int ivis_fd;                /* input visibility map's file descriptor */

    
	/* Define module */
	define_module();
  
    /* Define the different input options */
	opts = define_options();

	/**** Start ****/
	G_gisinit(argv[0]);
	if (G_parser(argc, argv) < 0) exit(-1);

    adjust_region(opts.iimg->answer);

	/* open input raster */
	if((iimg_fd = G_open_cell_old(opts.iimg->answer, G_mapset())) < 0)
		G_fatal_error("Can not open input raster.");
        
    ialt_fd = -1;   /* initialize and assume there is no elevation map */
    if(opts.ialt->answer)
        if((ialt_fd = G_open_cell_old(opts.ialt->answer, G_mapset())) < 0)
            G_fatal_error("Can not open elavation raster.");

    ivis_fd = -1;   /* initialize and assume there is no visiblility map */
    if(opts.ivis->answer)
        if((ivis_fd = G_open_cell_old(opts.ivis->answer, G_mapset())) < 0)
            G_fatal_error("Can not open visibility raster.");
                
	/* open a floating point raster or not? */
	if(opts.oflt->answer)
	{
		if((oimg_fd = G_open_fp_cell_new(opts.oimg->answer)) < 0)
			G_fatal_error("Can not create output raster.");
	}
	else
	{
		if((oimg_fd = G_open_raster_new(opts.oimg->answer, CELL_TYPE)) < 0)
			G_fatal_error("Can not create output raster.");
	}

    /* read the scale parameters */
    read_scale(opts.iscl, iscale);
    read_scale(opts.oscl, oscale);

    /* initialize this 6s computation and parse the input conditions file */
	init_6S(opts.icnd->answer);
	
    InputMask imask = RADIANCE;         /* the input mask tells us what transformations if any
                                         needs to be done to make our input values, reflectance
                                         values scaled between 0 and 1 */
    if(opts.irad->answer) imask = REFLECTANCE;
    if(opts.etmbefore->answer) imask = (InputMask)(imask | ETM_BEFORE);
    if(opts.etmafter->answer) imask = (InputMask)(imask | ETM_AFTER);
    /* process the input raster and produce our atmospheric corrected output raster. */
	process_raster(iimg_fd, imask, iscale, ialt_fd, ivis_fd,
                   oimg_fd, opts.oflt->answer, oscale, opts.optimize->answer);


    /* Close the input and output file descriptors */
	G_close_cell(iimg_fd);
    if(opts.ialt->answer) G_close_cell(ialt_fd);
    if(opts.ivis->answer) G_close_cell(ivis_fd);
	G_close_cell(oimg_fd);

    /* Copy the colors of the input raster to the output raster.
       Scaling is ignored and color ranges might not be correct. */
	copy_colors(opts.iimg->answer, opts.oimg->answer);

    /* we are now done, so notify the user */
	fprintf(stderr, "Done!\n"); fflush(stderr);
	exit(0);
}
