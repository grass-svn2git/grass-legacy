/* %W% %G% */
/*
**  Written by Dave Gerdes  6/89
**  US Army Construction Engineering Research Lab
*/
#include    <stdio.h>
#include    <math.h>
#include    "gis.h"
#include    "digit.h"
#include    "dig_head.h"


#define MAIN
#define  USAGE  "Vcontour lines=linefile labels=labelfile"

/*
#define DEBUG
*/
/*  command line args */
static	char  *line_file = NULL ;
static	char  *label_file = NULL ;

static  int  load_args() ;

struct Command_keys vars[] = {
 { "lines", 1 },
 { "labels", 2 },
 { NULL,     0 }
};


main(argc, argv)
    int argc;
    char **argv;
{
    int   ret ;
    char *line_mapset, *label_mapset;


/*  check args and set flags  */
	
    ret = G_parse_command (argc, argv, vars, load_args) ;
    if (ret > 0)	/* Help was requested */
         exit (1);

    if (ret < 0  ||  line_file == NULL  ||   label_file == NULL)
    {
        fprintf (stderr, "%s: Command line error.\n\n Usage: %s\n",
		argv[0], USAGE);
        exit (-1);
    }

/* Show advertising */
    G_gisinit(argv[0]);
    printf("\n\n   Contours:\n\n") ;

    if ((line_mapset = G_find_file2 ("dig_plus", line_file, "")) == NULL)
	G_fatal_error ("Could not find DIG file %s\n", line_file);

    if ((label_mapset = G_find_file2 ("dig_plus", label_file, "")) == NULL)
	G_fatal_error ("Could not find DIG file %s\n", label_file);
    
    setup (line_file, line_mapset, label_file, label_mapset); 
    exit (0);
}

static
load_args (position, str)
    int position;
    char *str;
{
    switch(position)
    {
	case 1:
		line_file = G_store(str) ;
		break ;
	case 2:
		label_file = G_store(str) ;
		break ;
	default:
		break;
    }	/*  switch  */

    return (0);
}

#ifdef DEBUG
debugf (format, a, b, c, d, e, f, g, h, i, j, k, l)
    char *format;
    int a, b, c, d, e, f, g, h, i, j, k, l;
{
    fprintf (stderr, format, a, b, c, d, e, f, g, h, i, j, k, l);
}
#endif


/*DEBUG*/ 	struct head label_Head;

setup(line_file, line_mapset, label_file, label_mapset)
    char *line_file, *line_mapset, *label_file, *label_mapset;
{
	struct Map_info line_Map;
	struct Map_info label_Map;
	struct head line_Head;
/*DEBUG 	struct head label_Head; */
	FILE *plus_fp;
	struct Plus_head Plus_head;

	if ( ! line_mapset || !label_mapset)
		G_fatal_error ("No mapset specified.\n");

	dig__P_writeable (0);
	dig_P_init (label_file, label_mapset, &label_Map);
	dig_read_head_binary (label_Map.digit, &label_Head);

	dig__P_writeable (1);
	dig_P_init (line_file, line_mapset, &line_Map);
	dig_read_head_binary (line_Map.digit, &line_Head);

	/*
	** open up ATT file for read/write  so we can modify labels
	*/
	if ( (line_Map.att = fopen(line_Map.att_file, "r+")) == NULL )
	{
	    fprintf (stderr, "Cannot open ATT file '%s' for write\n", line_Map.att_file);
	    exit (-1);
	}


	doit (&line_Map, &label_Map);

	if (NULL == (plus_fp = fopen (line_Map.plus_file, "w")))
	{
	    fprintf (stderr, "Cant open Plus file for final write!\n");
	    exit (-1);
	}
	dig_map_to_head (&line_Map, &Plus_head);

	if (0 > dig_write_plus_file (plus_fp, &line_Map, &Plus_head))
	{
	    fprintf (stderr, "Error writing final plus file\n");
	    exit (-1);
	}

	fclose (line_Map.att);
	dig_P_fini (&line_Map);
	dig_P_fini (&label_Map);

	return(0) ;
}

/*
**  C - Contours
**  T - Text labels
*/
doit (Cmap, Tmap)
    struct Map_info *Cmap, *Tmap;
{
    int Tline, Cnode;
    P_LINE *TLine;
    P_NODE *CNode;
    double N_N, N_S, N_E, N_W;
    double box_height, box_width;
    int num_found, loop_cnt;
    plus_t A, B;
    double A_Dist, B_Dist;
    plus_t A_Best, B_Best;
    static struct line_pnts Points;
    static int first_time = 1;
    double dist1, dist2;
    int Aline, Bline;
    int att, cat;
    int ret;
/*
    FILE *fpp;
*/

    /*
    fpp = fopen ("/tmp/corridor", "w");
    dig_write_head_binary (fpp, label_Head);
    */

/*DEBUG*/ fprintf (stderr, "IN DOIT\n");
    if (first_time)
    {
	Points.alloc_points = 0;
	first_time = 0;
    }

    for (Tline = 1 ; Tline <= Tmap->n_lines ; Tline++)
    {
	num_found = 0;
	A = B = 0;

/*DEBUG*/ fprintf (stderr, "LABEL NUMBER: %d\n", Tline);
	TLine = &(Tmap->Line[Tline]);
	cat = Tmap->Att[TLine->att].cat;
	if (!LINE_LABELED (TLine))
	{
	    fprintf (stderr, "Textfile Line %d NOT labelled\n", Tline);
	    continue;
	}

	/* read in Label outline */
	if (0 > dig__Read_line (&Points, Tmap->digit, TLine->offset))
	{
	    fprintf (stderr, "Error reading LABEL dig file\n");
	    exit (-1);
	}

	/*
	corridor (Cmap, Tmap, Tline, &Points, fpp);
	*/
	    A_Best = B_Best = 0;
#include "corridor.c"

	/* hack code to see something work: */
	/* always take the node found w/ the earliest past */
	for (loop_cnt = 0 ; num_found < 2 && loop_cnt < 50 ; loop_cnt ++)
	{
	    /* this line should work outside of the loop */
	    A_Best = B_Best = 0;
	    box_height = TLine->N - TLine->S;
	    box_width  = TLine->E - TLine->W;
	    N_N = TLine->N + (loop_cnt * .5 * box_height);
	    N_S = TLine->S - (loop_cnt * .5 * box_height);
	    N_E = TLine->E + (loop_cnt * .5 * box_width);
	    N_W = TLine->W - (loop_cnt * .5 * box_width);

	    for (Cnode = 1 ; Cnode <= Cmap->n_nodes ; Cnode++)
	    {
		CNode = &(Cmap->Node[Cnode]);

		/* skip if not the end of a line */
		if (CNode->n_lines != 1)
		    continue;

		/* skip if line is already labelled something else */
		if ((att = Cmap->Line[abs (CNode->lines[0])].att) &&
		    Cmap->Att[att].cat != cat)
		    continue;

		/* and skip if not in new test BBOX */
		if (CNode->y < N_S || CNode->y > N_N) continue;
		if (CNode->x < N_W || CNode->x > N_E) continue;


		/* A */
		dist1 = dist2 = HUGE;
		if (!A)
		    dist1 = dig_distance2_point_to_line (CNode->x,CNode->y,Points.x[0],Points.y[0],Points.x[1],Points.y[1]);
		if (!B)
		    dist2 = dig_distance2_point_to_line (CNode->x,CNode->y,Points.x[2],Points.y[2],Points.x[3],Points.y[3]);
		if (dist1 == dist2)	/* never gets here? */
		{
		    if (dist1 == HUGE)
			continue;
		}
		if (dist1 <= dist2)
		{
		    if (!A_Best)
		    {
			A_Best = Cnode;
			A_Dist = dist1;
		    }
		    else
		    {
			if (dist1 < A_Dist)
			{
			    A_Best = Cnode;
			    A_Dist = dist1;
			}
		    }
		}
		else 
		{
		    if (!B_Best)
		    {
			B_Best = Cnode;
			B_Dist = dist2;
		    }
		    else
		    {
			if (dist2 < B_Dist)
			{
			    B_Best = Cnode;
			    B_Dist = dist2;
			}
		    }
		}
	    }

	    if (!A)
	    {
		if (A_Best)
		{
		    A = A_Best;
		    num_found++;
		}
	    }
	    if (!B)
	    {
		if (B_Best)
		{
		    B = B_Best;
		    num_found++;
		}
	    }
	}
	if (num_found < 2)
	{
	    fprintf (stderr, "FAILED to find node for Tline %d\n", Tline);
	    continue;
	}

	/* otherwise, we should have two nodes whose lines belong
	** to this label!
	*/
	Aline = abs(Cmap->Node[A].lines[0]);
	Bline = abs(Cmap->Node[B].lines[0]);
/*DEBUG*/ fprintf (stderr, "Found:  Aline %d   Bline %d\n", Aline, Bline);

	ret = label_lines (Cmap, Aline, Tmap->Att[TLine->att].cat);
	if (ret < 0)
	    fprintf (stderr, "Could not create new Label A\n");
	ret = label_lines (Cmap, Bline, Tmap->Att[TLine->att].cat);
	if (ret < 0)
	    fprintf (stderr, "Could not create new Label B\n");
    }
/*
    fclose (fpp);
*/
}

label_lines (map, line, cat)
    struct Map_info *map;
    int line;
    int cat;
{
    int tmp, next_line, prev_line;

    line = abs (line);

    label_line (map, line, cat);

    prev_line = line;
    while (1)
    {
	if (0 == (next_line = get_next_line (map, -prev_line)))
	    break;
	/* if ran into another label */
	if (map->Line[abs(next_line)].att &&
	    (tmp = map->Att[map->Line[abs(next_line)].att].cat) != cat)
	{
	    fprintf (stderr, "Label %d ran into label %d, Line %d\n", cat, tmp, next_line);
	    break;
	}

	label_line (map, next_line, cat);
	prev_line = next_line;
    }

    prev_line = -line;
    while (1)
    {
	if (0 == (next_line = get_next_line (map, -prev_line)))
	    break;
	/* if ran into another label */
	if (map->Line[abs(next_line)].att &&
	    (tmp = map->Att[map->Line[abs(next_line)].att].cat) != cat)
	{
	    fprintf (stderr, "Label %d ran into label %d, Line %d\n", cat, tmp, next_line);
	    break;
	}

	label_line (map, next_line, cat);
	prev_line = next_line;
    }

    return (0);
}

get_next_line (map, prev_line)
    struct Map_info *map;
    int prev_line;
{
    P_NODE *Node;
    int next_node;

    if (prev_line < 0)
	next_node = map->Line[abs(prev_line)].N2;
    else
	next_node = map->Line[prev_line].N1;

    Node = &(map->Node[next_node]);
    if (Node->n_lines == 1)
	return (0);
    if (Node->n_lines > 2)
    {
	fprintf (stderr, "Node #%d: %d lines intersect here\n", next_node, Node->n_lines);
	return (0);
    }

    if (Node->lines[0] == prev_line)
	return Node->lines[1];
    else
	return Node->lines[0];
}
    

label_line (map, line, cat)
    struct Map_info *map;
    int line;
    int cat;
{
    int att;
    double x, y;
    int line_type;
    P_LINE *Line;
    P_ATT *Att;
    static struct line_pnts Points;
    static int first_time = 1;

    if (first_time)
    {
	Points.alloc_points = 0;
	first_time = 0;
    }

    line = abs (line);
    Line = &(map->Line[line]);
    if (0 > dig__Read_line (&Points, map->digit, Line->offset)) 
    {
	fprintf (stderr, "Label_line: Can't read line %d offset %ld\n", line, Line->offset);
	exit (-1);
    }

    line_type = Line->type;

    /* area and line lines all get labelled as LINE */
    if (line_type == AREA)
        line_type = LINE;
 
    /* remove old label from screen */
 
    get_line_center (&x, &y, &Points);
 
    if (Line->att) /* if already exists, change it */
    {
        att = Line->att;
        Att = &(map->Att[att]);
        Att->cat = cat;
        Att->x = x;
        Att->y = y;
        dig_update_att (map, att);
    }
    else
    {   
        att = dig_new_att (map, x, y, line_type, line, cat);
        if (att < 0)
            return (-1);
        Line->att = att;
    }
    return (0);
}
