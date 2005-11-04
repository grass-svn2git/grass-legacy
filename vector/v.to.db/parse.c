#include <string.h>
#include "global.h"
#include "Vect.h"
#include "glocale.h"

int parse_units();
int parse_option();
int match(); 

int 
parse_command_line (int argc, char *argv[])
{
    int ncols;

    struct {
	    struct Option *vect;
	    struct Option *option;
	    struct Option *type;
	    struct Option *field;
	    struct Option *qfield;
	    struct Option *col;
	    struct Option *units;		
	    struct Option *qcol;		
    } parms;
    struct {
	    struct Flag *p, *s, *t;
    } flags;

    parms.vect = G_define_standard_option(G_OPT_V_INPUT);
    parms.vect->key    = "map";

    parms.type = G_define_standard_option(G_OPT_V_TYPE) ;
    parms.type->options      = "point,line,boundary,centroid";
    parms.type->answer       = "point,line,boundary,centroid";
    parms.type->description  = _("Type of elements (for coor valid point/centroid, "
			       "for length valid line/boundary)");	
    
    parms.field = G_define_standard_option(G_OPT_V_FIELD);

    parms.qfield = G_define_standard_option(G_OPT_V_FIELD);
    parms.qfield->key = "qlayer";
    parms.qfield->description = _("Query layer. Used by 'query' option.");

    parms.option = G_define_option();
    parms.option->key          = "option";
    parms.option->type         = TYPE_STRING;
    parms.option->required     = YES;
    parms.option->multiple     = NO;
    parms.option->options      = "cat,area,compact,perimeter,length,count,coor,sides,query";
    parms.option->description  = _("Value to upload");
    parms.option->descriptions  = 
		 "cat;insert new row for each category if doesn't exist yet;"
		 "area;area size;"
    		 "compact;compactness of an area, calculated as \n"
		 "              compactness = perimeter / 2 * sqrt(PI * area);"
		 "perimeter;perimeter length of an area;"
		 "length;line length;"
		 "count;number of features for each category;"
		 "coor;point coordinates, X,Y or X,Y,Z;"
		 "sides;categories of areas on the left and right side of the boundary, "
		       "'qlayer' is used for area category;"
		 "query;result of a database query for all records of the geometry"
		       "(or geometries) from table specified by 'qlayer' option";	

    parms.units = G_define_option();
    parms.units->key   = "units";
    parms.units->type   = TYPE_STRING ;
    parms.units->required = NO ;
    parms.units->multiple = NO ;
    parms.units->options      = "mi,miles,f,feet,me,meters,k,kilometers,a,acres,h,hectares";
    parms.units->description = _("mi(les),f(eet),me(ters),k(ilometers),a(cres),h(ectares)");

    parms.col = G_define_option();
    parms.col->key    = "column";
    parms.col->type   = TYPE_STRING ;
    parms.col->required = NO ;
    parms.col->multiple = YES ;
    parms.col->gisprompt  = "column(s)" ;
    parms.col->description = _("column(s)");

    parms.qcol = G_define_option();
    parms.qcol->key    = "qcolumn";
    parms.qcol->type   = TYPE_STRING ;
    parms.qcol->required = NO ;
    parms.qcol->multiple = NO ;
    parms.qcol->gisprompt  = "query column";
    parms.qcol->description = _("Query column used for 'query' option. E.g. 'cat', 'count(*)', 'sum(val)'");

    flags.p = G_define_flag();
    flags.p->key = 'p';
    flags.p->description = _("print only");
    
    flags.s = G_define_flag();
    flags.s->key = 's';
    flags.s->description = _("only print sql statements");	
    
    flags.t = G_define_flag();
    flags.t->key = 'c';
    flags.t->description = _("In print mode prints totals for options: length,area,count");	

    if (G_parser(argc,argv)) exit(-1);

    options.print = flags.p->answer;
    options.sql = flags.s->answer;
    options.total = flags.t->answer;

    options.name = parms.vect->answer;
    options.mapset = G_find_vector2 (options.name, NULL);

    if (options.mapset == NULL) 
	G_fatal_error ( _("%s: <%s> vector map not found"), G_program_name(), options.name);

    options.type = Vect_option_to_types ( parms.type ); 
    options.field = atoi( parms.field->answer );
    options.qfield = atoi( parms.qfield->answer );

    options.option = parse_option (parms.option->answer);
    options.units = parse_units (parms.units->answer);

    /* Check number of columns */
    ncols = 0;
    options.col[0] = NULL;
    options.col[1] = NULL;
    options.col[2] = NULL;
    while (  parms.col->answers && parms.col->answers[ncols] ) {
	options.col[ncols] = G_store ( parms.col->answers[ncols] );
	ncols++;
    }
    
    if ( options.option == O_AREA || options.option == O_LENGTH || options.option == O_COUNT 
	 || options.option == O_QUERY || options.option == O_COMPACT || options.option == O_PERIMETER) /* one column required */
    {
	if ( ncols != 1 ) {
	    G_fatal_error ( _("This option requires one column") );
	}
    }  else if ( options.option == O_SIDES ) {
	if ( ncols != 2 ) {
	    G_fatal_error ( _("This option requires 2 columns") );
	}
    }  else if ( options.option == O_COOR ) {
	if ( ncols < 2 ) {
	    G_fatal_error ( _("This option requires at least 2 columns") );
	}
    }

    options.qcol = parms.qcol->answer;

    if ( options.option == O_SIDES && !(options.type | GV_BOUNDARY) )
	G_fatal_error ( _("The 'sides' option makes sense only for boundaries"));

    return 0;
}

int parse_units (char *s)
{
    int x=0;

    if (match (s, "miles",2)) x = U_MILES;
    else if (match (s, "feet",1)) x = U_FEET;
    else if (match (s, "meters",2)) x = U_METERS;
    else if (match (s, "kilometers",1)) x = U_KILOMETERS;
    else if (match (s, "acres",1)) x = U_ACRES;
    else if (match (s, "hectares",1)) x = U_HECTARES;

    return x;
}

int parse_option (char *s)
{
    int x=0;

    if (strcmp (s, "cat") == 0) x = O_CAT;
    else if (strcmp (s, "area") == 0) x = O_AREA;
    else if (strcmp (s, "length") == 0) x = O_LENGTH;
    else if (strcmp (s, "count") == 0) x = O_COUNT;
    else if (strcmp (s, "coor") == 0) x = O_COOR;
    else if (strcmp (s, "sides") == 0) x = O_SIDES;
    else if (strcmp (s, "query") == 0) x = O_QUERY;
    else if (strcmp (s, "compact") == 0) x = O_COMPACT;
    else if (strcmp (s, "perimeter") == 0) x = O_PERIMETER;

    return x;
}

int 
match (char *s, char *key, int min)
{
    size_t len;

    if (!s) return 0;
    len = strlen (s);
    if (len < min) return 0;
    return strncmp (s, key, len) == 0;
}
