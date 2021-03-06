
#include <stdio.h>
#include "AtoG.h"

/* A_labs_cmd.c (cmd-line version of GenToDigAreaLabels.c -- dks)
 *
 * function defined:
 *
 * GenToDigAreaLabels(pts_file,txt_file,atts_file,cats_filename,
 *                       catcol, attcol /added 2/91--dks/)
 *
 *   FILE *pts_file,       - ARC/INFO Generate format label-points file 
 *        *txt_file,       - text file assoc. integer category values
 *                           with lines of descriptive text. 
 *        *atts_file;      - GRASS vector attribute (dig_atts) file
 *   char *cats_filename;  - GRASS vector category (dig_cats) filename
 *
 * PURPOSE: Create a GRASS vector attribute (dig_atts) and a vector
 *          category (dig_cat) file from a label-points file (pts_file)
 *          and a label-text file (txt_file).
 *
 * NOTES: 
 *
 *   1) At the request of Ron Glenn of the USDA-SCS, this routine 
 *      determines which column of the label-text (txt_file) to 
 *      use for label-point-ID numbers by looking for the second
 *      column in the file that has a 1 in the 1st row, 2 in the
 *      second row, 3 in the 3rd row, and so on.  Each label-point
 *      ID number ties a row in the label-text file to a row in the
 *      labels-points file.  This technique is used because one 
 *      cannot count on the first column in the label-text file
 *      being the label-point ID number.
 *  
 *   2) this routine prompts the user to enter the number of the
 *      column in the txt_file that should be used for GRASS
 *      category numbers, and the number of the column that
 *      should be used for attribute text.
 *
 *   3) If the user-specified category column contains any thing
 *      that is not an integer this routine will die gracefully.
 *
 *
 *   4) DKS: 4.0 changes: formerly this routine displayed column numbers    
 *      to the user as NumCols-1 (in effect, treating Rphysical column
 *      #1 as a non-column.  However, since the program counts all columns
 *      in its data processing routines, NumCols itself is displayed.
 *
 *
 * 
 * Dave Johnson
 * DBA Systems, Inc.
 * 10560 Arrowhead Drive
 * Fairfax, Virginia 22030
 *
 */

#include "gis.h"
#include "digit.h"
#include "dig_head.h"

GenToDigAreaLabels(pts_file,txt_file,atts_file,cats_filename, 
				   idcol, catcol, attcol)
FILE *txt_file,
*pts_file,
*atts_file;
char *cats_filename;
int idcol, catcol, attcol; 
{
	struct Categories new_cats;
	double  east, north;
	int	id,
	NumCats,
	NumCols,
	NumLines,
	CatStat,
	CatNum,
	IDCol,
	AttCol,
	CatCol,
	done;
	char    txtbuf[512];
	char    inbuf[512];
	char    tmpbuf[512];
	char    AttText[512];

#ifdef DEBUG
	printf("GenToDigAreaLabels\n");
#endif

	AttText[0]=0;

	/* count the columns in the text_file */
	rewind(txt_file);
	if (!fgets(txtbuf,512,txt_file)) return (-1);  /* skip headers record */
	if (!fgets(txtbuf,512,txt_file)) return (-1);  /* skip headers record */
	NumCols = CountColumns(txtbuf);
	NumLines = CountLines(txt_file);

/*  FUNCTION CALL EXPUNGED--by dks, to make consistent with policy on line side,
	already done by Glenn.  Besides consistency, needed to handle header-less
	files. FindIDCol rewinds before counting, so progr fails with header-less
	textfiles. I could have removed rewind from FindIDCol, but decided to take
	it out so polygon search and line search have similar requirements.
**
* determine which column in the txt_file is the label-point ID
 * number.  this is done by looking for the second column in the 
 * file that has a 1 on the 1st line, 2 on the 2nd line, 3 on the
 * 3rd line, and so on...

	if ((IDCol=FindIDColumn(txt_file)) < 0)
	{
		printf("\n\nThe LABEL-TEXT file has been scanned.\n");
		printf("There is not enough information in the file to create GRASS\n");
		printf("attribute and category files\n");
		return(-1);
	}
*****/

	/* tell the user how many columns were found in the txt_file and
 * which one was found to be the label-point ID column
 */
	printf("\n\nThe LABEL-TEXT file has been scanned. There are %d\n",NumLines);
	printf("lines in the file and %d columns in the file\n", NumCols);


	if (NumCols == 2)
	{
		IDCol = 1; /*added by dks*/
		CatCol = 2;
		AttCol = -1;
		printf("\nBecause there are only 2 columns, column 2 is assumed to be\n");
		printf("the category number column\n");
	}
	else 
	{
		IDCol = idcol;
		CatCol = catcol;
		if (IDCol<1 || IDCol>NumCols)
		{
			printf("\n\n<%d> = Invalid number for textfile ID Column.\n", IDCol);
			printf ("Exiting\n");
			exit (-1);
		}
		if (CatCol<1 || CatCol>NumCols)
		{
			printf("\n\n<%d> = Invalid number for textfile Category Column.\n"), CatCol;
			printf ("Exiting\n");
			exit (-1);
		}

		if (attcol == 0)
			AttCol = -1;
		else
		{
			AttCol =  attcol;
			if (AttCol > NumCols)
			{
				fprintf(stderr, "Invalid number for Attribute Column.\n");
				exit (-1);
			}
		}
	 }

	printf("\nWorking...\n");

	/* count number of different categories in the category file and use
 * this number to initialize a category structure
 */
	rewind(txt_file);
	if (!fgets(txtbuf,512,txt_file)) return (-1);  /* skip headers record */
	NumCats = CountCats(txt_file,CatCol);
	G_init_cats((CELL)NumCats,(char *)NULL,&new_cats);

	/* LOOP - through the pts_file */
	done = 0;
	while (!done)
	{
		/* read a line */
		if (!fgets(inbuf,1024,pts_file)) return (-1);
		strcpy(tmpbuf, inbuf);
		if (strcmp(G_squeeze(tmpbuf),"END") == 0)
		{
			/* end of file reached */
			done = 1;
		}
		else
		{
		    process_inp(tmpbuf);
		    if (sscanf(tmpbuf,"%d %lf %lf",&id,&east,&north) == 3)
		    {
/*dks: BUG???--why rewind this every time if the points are sequential?*/
			rewind(txt_file);
			if (!fgets(txtbuf,512,txt_file)) return (-1);  /* skip headers record */
			/* find the point's category value in the label-txt file */
			if ((CatStat=FindAttAndCat(txt_file,AttCol,CatCol,IDCol,id,AttText,&CatNum))<-1)
			{
				G_fatal_error("Reading label-text file");
				exit(-1);
			}

			/* write point and its category number to the att. file */
			write_att(atts_file,'A',east,north,CatNum);

			/* find an attribute string in the label-text file */
			/* set the attribute string in the category structure */
			if (G_set_cat((CELL)CatNum,AttText,&new_cats) != 1)
				G_fatal_error("Do_area_labs: call to G_set_cats");
		   }
	   	   else
		   {
			/* error */
			G_fatal_error("reading LABEL-POINTS file");
		   }
                }
	}

	sprintf(tmpbuf,"Arc/Info Areas for %s",cats_filename);
	G_set_cats_title(tmpbuf,&new_cats);
	if (G_write_vector_cats(cats_filename,&new_cats) != 1)
		G_fatal_error("Do_area_labs: writing dig_cats file");

	return(0);
}
