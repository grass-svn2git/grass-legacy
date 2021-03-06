#include "tree.h"

rcr_wr_line(node, win_row) struct Node *node ; int win_row ;
{
    if (node == NULL)
	    return ;
    switch(node->oper)
    {
    case LEAF_OPR:
	    break ;

    case NAM_OPR:
	    /* the name operator has the power to write out maps */
	    G_put_map_row(node->cellfd, node->cbuf) ;

	    /* name operator is UNARY!! */
	    rcr_wr_line(node->left, win_row) ;
	    break ;

    case COV_OPR:
    case OV1_OPR:
    case OV2_OPR:
    case OV3_OPR:
    case OV4_OPR:
	    /* overlay and cover operators are UNARY!! */
	    rcr_wr_line(node->left, win_row) ;
	    break ;

    case GRP_OPR:
	    /* group operator is UNARY!! */
	    rcr_wr_line(node->left, win_row) ;
	    break ;

    default:	/* binary operator */
	    rcr_wr_line(node->left, win_row) ;
	    rcr_wr_line(node->rite, win_row) ;
	    break ;
    }

    return ;
}
/* ========================================================================= */
