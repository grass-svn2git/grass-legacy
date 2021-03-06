#include "tree.h"

/* =========================================================================
 * rcr_wr_supp generates cats, hist, and colr support files
 * for named and overlay nodes
 */

rcr_wr_supp(node) struct Node *node ;
{
    int status ;
    int num_cats ;

    if (node == NULL)
	    return ;

    switch(node->oper)
    {
    case LEAF_OPR:
	    status = 1 ;
	    break ;

    case NAM_OPR:
	    /* make the categories support file */
	    if (make_cats_comb(node, &num_cats) == -1)
		    status = 0 ;
	    else
	    {
		    /* make the history support file */
		    if (make_hist_comb(node) == -1)
			    status = 0 ;
		    else
		    {
			    /* make the color support file */
			    if (make_colr_comb(node, num_cats) == -1)
				    status = 0 ;
			    else
			    {
				    /* unary operator has left child only */
				    status = rcr_wr_supp(node->left) ;
			    }
		    }
	    }
	    break ;

    case COV_OPR:
    case OV1_OPR:
    case OV2_OPR:
    case OV3_OPR:
    case OV4_OPR:
    case GRP_OPR:
	    /* unary operators have left child only */
	    status = rcr_wr_supp(node->left) ;
	    break ;

    default:
	    status = rcr_wr_supp(node->left) ;
	    status = rcr_wr_supp(node->rite) ;
	    break ;
    }
    return(status) ;
}

make_cats_comb(node, num_cats) 
    struct Node *node ;
    int *num_cats ;
{
    struct Categories pcats ;
    int stat;

/* Establish information in the cats structure */
    if (G_read_cats( node->name, node->mapset, &pcats) == -1)
	    return(-1) ;

/* Base category info on what is being named */
    switch(node->left->oper)
    {
    case OV1_OPR:
    case OV2_OPR:
    case OV3_OPR:
    case OV4_OPR:
	    G_set_cat (0,  "<no data>", &pcats);
	    G_set_cat (1,  "OVERLAY 1------ ", &pcats) ;
	    G_set_cat (2,  "OVERLAY --2---- ", &pcats) ;
	    G_set_cat (3,  "        1|2| |  ", &pcats) ;
	    G_set_cat (4,  "OVERLAY ----3-- ", &pcats) ;
	    G_set_cat (5,  "        1| |3|  ", &pcats) ;
	    G_set_cat (6,  "         |2|3|  ", &pcats) ;
	    G_set_cat (7,  "        1|2|3|  ", &pcats) ;
	    G_set_cat (8,  "OVERLAY ------4 ", &pcats) ;
	    G_set_cat (9,  "        1| | |4 ", &pcats) ;
	    G_set_cat (10, "         |2| |4 ", &pcats) ;
	    G_set_cat (11, "        1|2| |4 ", &pcats) ;
	    G_set_cat (12, "         | |3|4 ", &pcats) ;
	    G_set_cat (13, "        1| |3|4 ", &pcats) ;
	    G_set_cat (14, "         |2|3|4 ", &pcats) ;
	    G_set_cat (15, "        1|2|3|4 ", &pcats) ;
	    break ;
    default:
	    break ;
    }

    *num_cats = pcats.num;
    stat = G_write_cats( node->name, &pcats);
    G_free_cats (&pcats);
    if (stat < 0)
	    return(-1) ;

    return(0) ;
}

make_hist_comb( node ) 
struct Node *node ;
{
    struct History phist ;

    /*
    if (node->exist)
	    return(0) ;
    */

/* Construct some default history information */
    sprintf(phist.mapid, "%s : %s", G_date(), node->name) ;
    sprintf(phist.title, "%s", node->name) ;
    sprintf(phist.mapset, "%s", G_getenv("MAPSET")) ;

    sprintf(phist.creator, "%s", G_whoami()) ;
    sprintf(phist.maptype, "raster") ;
    sprintf(phist.edhist[0],"Generated by %s program", G_program_name()) ;

    phist.edlinecnt=1 ;

    G_write_history(node->name, &phist) ;
    return(0) ;
}

make_colr_comb( node, num_cats ) 
struct Node *node ;
int num_cats ;
{
    struct Colors color ;

/* make color combos */
    G_init_colors (&color) ;
    if (num_cats <16)
    {
	make_16_colors(&color);
    }
    else
    {
	G_make_random_colors (&color,1,num_cats) ;
    }

    G_write_colors(node->name, node->mapset, &color) ;

    return(0) ;
}
