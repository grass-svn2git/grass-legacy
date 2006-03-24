/*
 * $Id$
 *
 * 2006-MAR-24 ported to GRASS 6 by Huidae Cho <grass4u@gmail.com>
 *
 * 2001-APR-19 change from Michel Wurtz <michel.wurtz@teledetection.fr>
 *             Cleaning and merge diffs from v.in.dxf2
 *
 * 1998-SEP-30 added -n flag to suppress text boxes
 *      Benjamin Horner-Johnson <ben@earth.nwu.edu>
 *
 * 1/28/98 change from Jacques Bouchard <bouchard@onera.fr>
 *
 *  Original written by Chuck Ehlshlaeger  6/89
 *  Revised by Dave Gerdes  12/89
 *  US Army Construction Engineering Research Lab
 */

#define MAIN
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "dxf2vect.h"

/* #define DEBUG */

/* gotta change this to be malloced */
#define MAX_ALLOC 100

static char *line_list[MAX_ALLOC][2], *label_list[MAX_ALLOC][2];
static int num_lines = 0, num_labels = 0;

/* command line args */
static char *out_name = NULL;

/* TODO */
#if 0
int all_lines = 1;		/* dump ALL lines unless user override */
int all_atts = 1;		/* dump ALL atts  unless user override */
#endif

int main(int argc, char *argv[])
{
    FILE *dxf_fp;
    char *p;
    int i;
    int count, list;
    struct Option *old_opt, *line_opt, *labl_opt, *prefix_opt;
    struct GModule *module;

    G_gisinit(argv[0]);

    module = G_define_module();
    module->description =
	"Converts files in DXF format to GRASS vector file format.";

    /* TODO */
#if 0
    txtbox_flag = G_define_flag();
    txtbox_flag->key = 'n';
    txtbox_flag->description = "Suppress drawing of text outlines";
#endif

    old_opt = G_define_option();
    old_opt->key = "dxf";
    old_opt->type = TYPE_STRING;
    old_opt->required = YES;
    old_opt->multiple = NO;
    /*old_opt->gisprompt                = "old,dxf,dxf"; */
    old_opt->description = "DXF input file";

    line_opt = G_define_option();
    line_opt->key = "lines";
    line_opt->type = TYPE_STRING;
    line_opt->required = NO;
    line_opt->multiple = YES;
    line_opt->description = "DXF layers with line data";

    /* TODO */
#if 0
    labl_opt = G_define_option();
    labl_opt->key = "labels";
    labl_opt->type = TYPE_STRING;
    labl_opt->required = NO;
    labl_opt->multiple = YES;
    labl_opt->description = "DXF layers with label data";
#endif

    prefix_opt = G_define_option();
    prefix_opt->key = "prefix";
    prefix_opt->type = TYPE_STRING;
    prefix_opt->required = NO;
    prefix_opt->multiple = NO;
    prefix_opt->description = "Prefix for output files";

    if (G_parser(argc, argv)) {
	/* do not print extra help when --... option is given */
	if (argv[1] && argv[1][1] != '-')
	    extra_help();
	exit(-1);
    }

    Points = Vect_new_line_struct();

    dxf_file = old_opt->answer;
    if (prefix_opt->answer != NULL)
	out_name = G_store(prefix_opt->answer);

    if (txtbox_flag->answer)
	fprintf(stderr, "text boxes will not be drawn\n");

    /*DEBUG
     * if (out_name != NULL)
     * fprintf (stderr, "out = '%s'\n", out_name);
     * else
     * fprintf (stderr, "out = NULL\n");
     * * */


    debuginit();

    /*process line and label arguments */
    if (line_opt->answers != NULL) {
	i = 0;
	while (line_opt->answers[i]) {
	    add_line_layer(line_opt->answers[i++]);
	    from_table = 1;
	}
    }

    if (labl_opt->answers != NULL) {
	i = 0;
	while (labl_opt->answers[i]) {
	    add_att_layer(labl_opt->answers[i++]);
	    from_table = 1;
	}
    }


    if (dxf_file == NULL) {
	fprintf(stderr, "%s: Command line error.\n\n", argv[0]);
	G_usage();
	exit(-1);
    }

    if ((dxf_fp = fopen(dxf_file, "r")) == NULL) {
	fprintf(stderr, "\ncannot open [%s] for dxf file\n", dxf_file);
	exit(-2);
    }

    /* check the number of lines in the file so big_percent can be used while
     * program is running
     */

    fseek(dxf_fp, 0L, 2);
    file_size = ftell(dxf_fp);
    rewind(dxf_fp);
    fprintf(stderr, "\nCONVERSION OF %s TO DIG FILE:  ", dxf_file);
    if (file_size < 500000)
	percent = 10;
    else if (file_size < 800000)
	percent = 5;
    else
	percent = 2;
    big_percent(0, file_size, percent);	/* initializing variables inside big_percent */

    /* make base_name from name of dxf file.  This will be used as
     * the prefix for all layers that are created to avoid layer name
     * conflicts between dxf files
     */
    if (out_name != NULL)
	strcpy(base_name, out_name);
    else {
	p = G_rindex(dxf_file, '/');
	if (p == NULL)
	    p = dxf_file;
	else
	    p++;
	strcpy(base_name, p);
	if (NULL != (p = G_rindex(base_name, '.')))
	    if (p != base_name)
		*p = '\0';	/* knock off any suffix */
    }

    dxf_init_chars();
    dxf_find_lines(dxf_fp);
    fclose(dxf_fp);
    /* NOTE:  examples of dxf files with inaccurate information
     * have led us not to use the EXTMIN and EXTMAX information
     * found in the HEAD SECTION of a dxf file
     */

    list = 1;	/* make a flag similar to v.in.shape after improving the code */
    if (list) {
	fprintf(stderr, "Following DXF layers found:\n");
	for (count = 0; count < num_open_layers; count++) {
	    fprintf(stderr, "Layer %d %s\n", count + 1, layers[count].name);
	}
    }

    /*
     * if (BOUNDARIES == 4)
     * dxf_add_boundaries ();
     */
    dxf_add_extents();		/*extents of map calculated as points were read in */

    exit(0);
}

int add_line_layer(char *str)
{
    return add_layer(str, line_list, &num_lines);
}

int add_att_layer(char *str)
{
    return add_layer(str, label_list, &num_labels);
}

int add_layer(char *str, char *list[][2], int *num)
{
    char buf[200], *buf_p, *p = NULL;

    strcpy(buf, str);
    G_squeeze(buf);
    p = G_index(buf, ':');
    if (*buf == '!') {		/* not to be written out */
	p = NULL;
	buf_p = buf + 1;
    }
    else {
	buf_p = buf;
	if (p != NULL) {	/* have an alias for output */
	    *p = 0;
	    p++;
	}
	else
	    p = buf_p;		/* output is same as original layer */
    }

    return _add_layer(list, num, buf_p, p);
}

int _add_layer(char *list[][2], int *num, char *from, char *to)
{
    list[*num][0] = G_store(from);
    if (to == NULL)
	list[*num][1] = NULL;
    else
	list[*num][1] = G_store(to);
    (*num)++;

    return *num;
}

char *remap(char *str, int type)
{
    /*
     * char *list[][2];
     */
    int num, i;

    if (!from_table)
	return (str);

    /* do lookups based on label remapping */
    if (type == DXF_LABEL_LINE)
	type = DXF_LABEL;

    if (type == DXF_ASCII) {
	/* list = line_list; */
	num = num_lines;
    }
    else {
	/* list = label_list; */
	num = num_labels;
    }


    if (type == DXF_ASCII) {
	for (i = 0; i < num; i++)
	    if (!strcmp(str, line_list[i][0]))
		return (line_list[i][1]);
	return (NULL);
    }
    else {
	for (i = 0; i < num; i++)
	    if (!strcmp(str, label_list[i][0]))
		return (label_list[i][1]);
	return (NULL);
    }
}



char *dxf_fgets(char *buf, int size, FILE *fp)
{
    char *p;
    static unsigned long current_size = 0;

    p = fgets(buf, size, fp);
    if (p != NULL) {
	current_size += strlen(p);
	big_percent(current_size, file_size, percent);	/* reporting status of job */
	G_squeeze(buf);
    }
    return (p);
}

int extra_help(void)
{
    fprintf(stderr, "\n\nWhere lines and labels are one or more of:\n\n");
    fprintf(stderr, "    layername1[,layername2,layername3,...]\n\n");
    fprintf(stderr, "      and/or   \n\n");
    fprintf(stderr,
    "    in_layername1:out_layername1[,inlayername2:outlayername2,.....]\n\n");

    return 0;
}

/***************************  big_percent  **********************************/
/* this is a modified version of G_percent created because of the
 * use of unsigned long ints which G_percent does not use
 */


int big_percent(unsigned long n, unsigned long d, int s)
{
    unsigned long x;
    static unsigned long prev = (unsigned long)-1;

    x = n * 100 / d;
    if (x % s)
	return 1;
    if (n <= 0 || n >= d || x != prev) {
	prev = x;
	fprintf(stderr, "%4ld%%\b\b\b\b\b", x);
	fflush(stderr);
    }
    if (x >= 100) {
	fprintf(stderr, "\n");
	prev = (unsigned long)-1;
    }

    return 0;
}
