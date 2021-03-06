#define GLOBAL
#include "global.h"

main(argc,argv) char *argv[];
{
    struct Categories cats;
    struct Colors colr;
    struct Ref group_ref;
    int nrows, ncols;
    int row;
    int band;
    int i;
    char msg[100];
    struct
    {
	struct Option *group, *subgroup, *sigfile, *class, *reject;
    } parm;
    struct
    {
	struct Flag *quiet;
    } flag;

    G_gisinit (argv[0]);

    parm.group = G_define_option();
    parm.group->key = "group";
    parm.group->type = TYPE_STRING;
    parm.group->required = YES;
    parm.group->description = "Imagery group to be classified";

    parm.subgroup = G_define_option();
    parm.subgroup->key = "subgroup";
    parm.subgroup->type = TYPE_STRING;
    parm.subgroup->required = YES;
    parm.subgroup->description = "Subgroup containing image files to be classified";

    parm.sigfile = G_define_option();
    parm.sigfile->key = "sigfile";
    parm.sigfile->type = TYPE_STRING;
    parm.sigfile->required = YES;
    parm.sigfile->description = "Signatures to use for classification";

    parm.class = G_define_option();
    parm.class->key = "class";
    parm.class->type = TYPE_STRING;
    parm.class->required = YES;
    parm.class->description = "Raster map to hold classification results";

    parm.reject = G_define_option();
    parm.reject->key = "reject";
    parm.reject->type = TYPE_STRING;
    parm.reject->required = NO;
    parm.reject->description = "Raster map to hold reject threshold results";

    flag.quiet = G_define_flag();
    flag.quiet->key = 'q';
    flag.quiet->description = "Run quietly";



    G_disable_interactive();
    if (G_parser(argc,argv))
	exit(1);

    class_name  = parm.class->answer;
    reject_name = parm.reject->answer;
    group       = parm.group->answer;
    subgroup    = parm.subgroup->answer;
    sigfile     = parm.sigfile->answer;

    open_files();

    nrows = G_window_rows();
    ncols = G_window_cols();

    if (!flag.quiet->answer)
	fprintf (stderr, "%s ... ", G_program_name());
    for (row = 0; row < nrows; row++)
    {
	if (!flag.quiet->answer)
	    G_percent(row, nrows, 2);
	for (band = 0; band < Ref.nfiles; band++)
	    if (G_get_map_row (cellfd[band], cell[band], row) < 0)
		exit(1);
	classify(class_cell, reject_cell, ncols);
	G_put_map_row (class_fd, class_cell, row);
	if (reject_fd > 0)
	    G_put_map_row (reject_fd, reject_cell, row);
    }
    if (!flag.quiet->answer)
	G_percent(row, nrows, 2);
    G_close_cell (class_fd);
    if (reject_fd > 0)
	G_close_cell (reject_fd);

    G_init_cats((CELL)S.nsigs,"Maximum Likelihood Classification",&cats);
    for (i=0; i < S.nsigs; i++)
    {
	if(*S.sig[i].desc)
	    G_set_cat ((CELL)(i+1),S.sig[i].desc, &cats);
    }
    G_write_cats (class_name, &cats);
    G_free_cats (&cats);

    if (reject_fd > 0)
    {
	char title[100];

	sprintf (title, "Rejection Probability for %s", class_name);

	G_init_cats((CELL)17,title,&cats);
	G_set_cats_title (title, &cats);
	G_set_cat ((CELL)0, "no data", &cats);
	G_set_cat ((CELL)1, "0.1%", &cats);
	G_set_cat ((CELL)2, "0.5%", &cats);
	G_set_cat ((CELL)3, "1%", &cats);
	G_set_cat ((CELL)4, "2%", &cats);
	G_set_cat ((CELL)5, "5%", &cats);
	G_set_cat ((CELL)6, "10%", &cats);
	G_set_cat ((CELL)7, "20%", &cats);
	G_set_cat ((CELL)8, "30%", &cats);
	G_set_cat ((CELL)9, "50%", &cats);
	G_set_cat ((CELL)10, "70%", &cats);
	G_set_cat ((CELL)11, "80%", &cats);
	G_set_cat ((CELL)12, "90%", &cats);
	G_set_cat ((CELL)13, "95%", &cats);
	G_set_cat ((CELL)14, "98%", &cats);
	G_set_cat ((CELL)15, "99%", &cats);
	G_set_cat ((CELL)16, "100%", &cats);
	G_set_cat ((CELL)17, "bad", &cats);
	G_write_cats (reject_name, &cats);
	G_free_cats (&cats);


	G_make_grey_scale_colors (&colr, (CELL) 1, (CELL) 16);

	G_set_color ((CELL)0, 0, 255, 0, &colr);
	G_set_color ((CELL)17, 255, 0, 0, &colr);
	G_write_colors (reject_name, G_mapset(), &colr);
	G_free_colors (&colr);
    }

/* associate the output files with the group */
    I_get_group_ref (group, &group_ref);
    I_add_file_to_group_ref (class_name, G_mapset(), &group_ref);
    if (reject_cell)
	I_add_file_to_group_ref (reject_name, G_mapset(), &group_ref);
    I_put_group_ref (group, &group_ref);
    make_history(class_name, group, subgroup, sigfile);
    exit(0);
}
