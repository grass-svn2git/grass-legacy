static char rcsid[]="$Header: /usr3/4.0/src.contrib/SCS/raster/r.in.erdas/cmd/RCS/main.c,v 1.8 1992/12/30 16:37:18 grass Exp $";
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "gis.h"
#define IAUTYP "NAHO"
#define MAXBND 7
#define MAXNUMBER 9999999999.0

/*****    The next two defines select the tarket system.         ******/
/*****    This is necessary becuase of a byte swapping problem   ******/
/*****    between the INTEL processors and other processors such ******/
/*****    as the WE 32000 used in the 3B2. The solution to this  ******/
/*****    problem is highly machine dependent and can also be    ******/
/*****    dependent on the means of transfering ERDAS files from ******/
/*****    the DOS world to the GRASS/UNIX environment.           ******/


/*****    End of machine dependent defines.			 *******/


#ifdef ATT_386
#define CONVHD() conv386hd(&erdashd)
#else
#define CONVHD() conv3b2hd(&erdashd)
#endif

int ERDFTYP;   /* ERDAS file type ( 4 or other) */


/******************* Structure for the header of the erdas file *********/
struct edheader {
	char hdwrd[6];
	short pack,nbands;
	char fil1[6];
	long rcols, rrows, rx, ry;
	char fill1[56];
	short maptyp, nclass;
	char fill2[14];
	short utyp;
	float area, mx, my, xcel, ycel;
};
/********************** End structure ************************************/


/******** Routine that does byte swapping for 2 or 4 byte word in place *******/
static void convbuf(buf,ftype)
char *buf;
int ftype;
{
	int i;
	char buf2[4];

	if (ftype != 4 && ftype != 2) {
		fprintf(stderr,"Error in type conversion\n");
		return;
	}
	if (ftype == 4){
		buf2[0] = *(buf+3);
		buf2[1] = *(buf+2);
		buf2[2] = *(buf+1);
		buf2[3] = *(buf);
	}
	else if (ftype == 2){
		buf2[0] = *(buf+1);
		buf2[1] = *(buf);
	}
	for (i=0;i<ftype;i++) *(buf+i) = buf2[i];
}
/******************** End convbuf ************************************/


/******** Routine to do byte swapping for necessary elements on 3B2 **********/
/******** this routine necessary because ERDAS file from the DOS system ******/
static void conv3b2hd(buf)
struct edheader *buf;
{
	convbuf(&(buf->pack),2);
	convbuf(&(buf->nbands),2);
	convbuf(&(buf->rcols),4);
	convbuf(&(buf->rrows), 4);
	convbuf(&(buf->rx), 4);
	convbuf(&(buf->ry),4);
	convbuf(&(buf->maptyp), 2);
	convbuf(&(buf->nclass),2);
	convbuf(&(buf->utyp),2);
	convbuf(&(buf->area), 4);
	convbuf(&(buf->mx), 4);
	convbuf(&(buf->my), 4);
	convbuf(&(buf->xcel), 4);
	convbuf(&(buf->ycel),4);
}
/********************* End conv3b2hd ******************************/

static void conv386hd(buf)
struct edheader *buf;
{
	/********************  No conversion necessary ********************/
}

/***************************  Not necessary ************************/
int read386(fd,buf,n)
int fd;
char *buf;
int n;
{
	int r,i;
	r = read(fd,buf,n);
	if (r > 0)
		for (i=0;i<r;i=i+2)
			convbuf(buf+i,2);
	return(r);
}
/********************************************************************/


/*********** Routine that prints out the ERDAS file header data ***********/
static void printhd(hd)
struct edheader *hd;
{
	int i;
	for (i=0;i<6;i++) fprintf(stderr,"%c",hd->hdwrd[i]);
	fprintf(stderr,"\n");
	fprintf(stderr,"pack type------ %d\n",hd->pack);
	fprintf(stderr,"number bands----------- %d\n",hd->nbands);
	fprintf(stderr,"number cols,rows------- %d, %d\n",hd->rcols,hd->rrows);
	fprintf(stderr,"starting coordinate --- %d, %d\n",hd->rx,hd->ry);
	fprintf(stderr,"map type -------------- %d\n",hd->maptyp);
	fprintf(stderr,"number classes -------- %d\n",hd->nclass);
	fprintf(stderr,"area ------------------ %f %c\n",hd->area,IAUTYP[hd->utyp]);
	fprintf(stderr,"map coordinate -------- %f, %f\n",hd->mx,hd->my);
	fprintf(stderr,"pixel size ------------ %f, %f\n\n",hd->xcel,hd->ycel);
}
/***************** End printhd *********************************************/


/************** Routine to get the selected bands from the user **************/
static void getbands(out,bands)
int out[];
short bands;
{
	char line[150];
	int length, i;

	for (i=0;i<bands;i++) out[i]=i+1;
	out[i]=0;
	i = 0;
	printf("Do you want to select a subset of the bands in the ERDAS file (y/n)[n] ");
	if (G_gets(line)) {
		if (line[0] == 'y') {
			printf("Enter the selcted bands one per line stop with a cariage return\n");
			while (length > 0 && i <= MAXBND) {
				printf(":");
				G_gets(line);
				length = strlen(line);
				if (length > 0) out[i++]=atoi(line);
			}
			while (i < MAXBND) out[i++] = 0;
		}
	}
	/* DEBUG for (i=0;i<MAXBND;i++) fprintf(stderr,"%d-%d\n",i,out[i]);*/
}
/************** End getbands **********************************************/


/********* Routine to get selected sub window ***************************/
int getwin(row,col,lrow,lcol)
double *row,*col,*lrow,*lcol;
{
	double srow = -1.0, scol = -1.0, nrow = MAXNUMBER, ncol = MAXNUMBER;
	char OK = 0,line[150];

	printf("Do you want to subwindow the erdas file (y/n)[n] ");

	if (G_gets(line)) {
		if (line[0] == 'y') {
			while (srow < *row) {
				printf("Enter starting row ");
				if (G_gets(line))
					sscanf(line,"%lf",&srow);
			}
			while (scol < *col) {
				printf("Enter starting col ");
				if (G_gets(line))
					sscanf(line,"%lf",&scol);
			}
			while (nrow > (*lrow-*row+1)) {
				printf("Enter number of rows ");
				if (G_gets(line))
					sscanf(line,"%lf",&nrow);
			}
			while (ncol > (*lcol -*col+1)) {
				printf("Enter number of cols ");
				if (G_gets(line))
					sscanf(line,"%lf",&ncol);
			}
			*row = srow;
			*col = scol;
			*lrow = srow + nrow -1;
			*lcol = scol + ncol -1;
			return(1);
		}
	}
	else return(0);
}
/************ End getwin **********************************/


/******************** Routine to put data into CELL file ****************/
int put_row (fd, buf, row, pack)
int fd, pack;
unsigned char *buf;
int row;
{
	CELL *c,*cellbuf;
	int ncols;
	short *buf1;


	buf1 = (short*) buf;
	ncols = G_window_cols();
	if ((cellbuf = (CELL *)G_malloc(ncols*sizeof(CELL))) == NULL) {
		fprintf(stderr,"memory error\n");
		exit(0);
	}
	c = cellbuf;
	while(ncols-- > 0) {
		if (pack == 2){
			*c++ = *buf1++;
		}else{
			*c++ = *buf++;
		}
	}

	/*************** DEBUG ********************
    ncols = G_window_cols(); c = cellbuf;
    while(ncols-- > 0) {
fprintf(stderr," %d ",*c++);
	}
fprintf(stderr,"\n");
******************************************/

	if (G_put_map_row (fd, cellbuf) < 0) return (-1);
	free(cellbuf);
	return (0);
}
/******************** End put_row ***************************/


/******** Routine that sets the current window to ERDAS file specs row col ****/
int set_window (firstrow, lastrow, firstcol, lastcol)
double firstrow,lastrow,firstcol,lastcol;

{
	struct Cell_head window;

	window.south  = -(lastrow + .5);
	window.north  = -(firstrow - .5);
	window.west   = firstcol - .5;
	window.east   = lastcol  + .5;
	window.cols   = lastcol - firstcol + 1.0;
	window.rows   = lastrow - firstrow + 1.0;
	window.ns_res = window.ew_res = 1.0;

	window.proj   = 0;
	window.zone   = 0;
	window.format = 0;
	window.compressed = 0;

	G_set_cell_format(0);
	printf("The image row and column numbers are being used as the\nGRASS window coordinates.\n");

	if(G_set_window (&window) < 0)
		return -1;
	return G_put_window (&window);
}
/*************** End set_window *************************************/


/******** Routine that sets the current window to ERDAS file specs UTM ****/
int set_uwindow (hd,firstrow, lastrow, firstcol, lastcol)
struct edheader *hd;
double firstrow,lastrow,firstcol,lastcol;

{
	struct Cell_head window;


	window.north  = (hd->my - (firstrow - hd->ry) * hd->ycel) + hd->ycel/2;
	window.south  = (hd->my - (lastrow - hd->ry+1) * hd->ycel) + hd->ycel/2;
	window.west  = (hd->mx + (firstcol - hd->rx) * hd->xcel) - hd->ycel/2;
	window.east  = (hd->mx + (lastcol - hd->rx+1) * hd->xcel) - hd->ycel/2;
	window.ns_res = hd->ycel;
	window.ew_res = hd->xcel;

	window.proj   = 99;
	window.zone   = 0;
	/*window.format = 0;
	window.compressed = 0;
	G_set_cell_format(0);*/

	printf("UTM coordinates used remember that the UTM zone is unknown\nand must be set using the grass support function on the header file.\n");

	if(G_set_window (&window) < 0)
		return -1;
	return G_put_window (&window);
}
/*************** End set_window *************************************/


/*********** Routine to handle 4 bit packed data  *********/
/* Need to consider the last nibble of rows with odd number of elements */

int do4bit(fd,buf,size,row,lastnibble)
int fd,row,*lastnibble;
unsigned char *buf;
unsigned size;
{
	unsigned char *tmpbuf;
	int n,i,oddrow,oddcol;

	/* check for odd number of columns */
	oddcol = size % 2;
	/* is this the 1st (odd) or 2nd (even) row that we're on */
	oddrow = row % 2;
	/* make some space */
	tmpbuf = (unsigned char *)malloc(size);
	/* if we have an odd number of cols, we need to keep last nibble for next row */
	if (oddcol){
		n = read(fd,tmpbuf,(size+oddrow)/2);
		if (!oddrow)
			*buf = *lastnibble;
		for (i=0;i<n;i++){
			if(oddrow){
				*(buf+i*2) = *(tmpbuf+i) & 017;
				*lastnibble = *(buf+i*2+1) = (*(tmpbuf+i) & ~017) >> 4;
			} else{
				*(buf+i*2+1) = *(tmpbuf+i) & 017;
				*(buf+i*2+2) = (*(tmpbuf+i) & ~017) >> 4;
			}
		}
	} else{
		/* we have an even number of cols so its a little easier */
		n = read(fd,tmpbuf,size/2);
		for (i=0;i<n;i++){
			*(buf+i*2) = *(tmpbuf+i) & 017;
			*(buf+i*2+1) = (*(tmpbuf+i+1) & ~ 017) >> 4;
		}
	}
	return(2*n);
}
/************* End do4bit **********************************************/



/*********** Routine to handle 16 bit packed data  *********/

int do16bit(fd,buf,size)
int fd;
unsigned char *buf;
unsigned size;
{
	int n;
	unsigned char *tmpbuf;

	tmpbuf = (unsigned char*)G_malloc(size*2);
	n = read(fd,tmpbuf,size*2);
	swab(tmpbuf, buf, size*2);
	return(2*n);
}
/************* End do16bit **********************************************/



main(argc,argv)
int argc;
char *argv[];
{
	struct edheader erdashd;
	unsigned size;
	int i,n,band,erdf,pack,outband[MAXBND+1],offset,lastnibble;
	unsigned char *buf,*startbuf;
	float *fptr, fixint;

	/****************  For GRASS ********************/
	double firstrow,lastrow,firstcol,lastcol;
	int row,new[MAXBND], showhead;
	char grassname[20],*tst;
	/************************************************/
	/*********add for grass4.0*****************/
	struct Option *erdasopt, *outopt, *trlopt;
	struct Option *start_row, *start_col, *num_rows, *num_cols, *sel_bands;
	struct Flag *headflag;


	G_gisinit(argv[0]);

	headflag = G_define_flag();
	headflag->key = 'l';
	headflag->description = "List the ERDAS header only";

	erdasopt = G_define_option();
	erdasopt->key             = "input";
	erdasopt->type            =  TYPE_STRING;
	erdasopt->required        =  YES;
	erdasopt->description     = "Erdas input file name";

	outopt = G_define_option();
	outopt->key             = "output";
	outopt->type            =  TYPE_STRING;
	outopt->required        =  YES;
	outopt->description     = "output prefix of the GRASS raster files to be created.";

	trlopt = G_define_option();
	trlopt->key             = "trailer";
	trlopt->type            =  TYPE_STRING;
	trlopt->required        =  NO;
	trlopt->description     = "Erdas trailer input file name";

	sel_bands = G_define_option();
	sel_bands->key             = "bands";
	sel_bands->type            =  TYPE_INTEGER;
	sel_bands->required        =  NO;
	sel_bands->multiple	   = YES;
	sel_bands->description     = "Selected bands to extract.";

	start_row = G_define_option();
	start_row->key 		= "srow";
	start_row->type		= TYPE_INTEGER;
	start_row->required	= NO;
	start_row->description	= "Starting row.";

	start_col = G_define_option();
	start_col->key 		= "scol";
	start_col->type		= TYPE_INTEGER;
	start_col->required	= NO;
	start_col->description	= "Starting column.";

	num_rows = G_define_option();
	num_rows->key 		= "rows";
	num_rows->type		= TYPE_INTEGER;
	num_rows->required	= NO;
	num_rows->description	= "Number of rows to extract.";

	num_cols = G_define_option();
	num_cols->key 		= "cols";
	num_cols->type		= TYPE_INTEGER;
	num_cols->required	= NO;
	num_cols->description	= "Number of columns to extract.";


	/* heeeerrrrrre's the   PARSER */
	if (G_parser (argc, argv))
		exit (-1);

	showhead = headflag->answer;

	if ((erdf = open(erdasopt->answer,0)) < 0) {
		fprintf(stderr,"Error can not open ERDAS file\n");
		exit(0);
	}
	n = read(erdf,&erdashd,128);
	if (n!=128) {
		fprintf(stderr,"Error in reading header\n");
		exit(0);
	}
	ERDFTYP = 0;
	if (erdashd.hdwrd[4] == '7' && erdashd.hdwrd[5] == '4')
		ERDFTYP = 4;
	CONVHD();
/*skip anything less than 7.4 file type */
	if ((erdashd.hdwrd[4] - 0x30) < 7 || (erdashd.hdwrd[5] - 0x30) < 4)
		G_fatal_error("ERDAS files before version 7.4 are not supported.");
/*	if (ERDFTYP != 4){
		fptr = (float *)&(erdashd.rrows);
		fixint = *fptr;
		erdashd.rrows = fixint;
		fptr = (float *)&(erdashd.rcols);
		fixint = *fptr;
		erdashd.rcols = fixint;
		fptr = (float *)&(erdashd.rx);
		fixint = *fptr;
		erdashd.rx = fixint;
		fptr = (float *)&(erdashd.ry);
		fixint = *fptr;
		erdashd.ry = fixint;
	}
*/
	printhd(&erdashd);
	if (showhead)
		exit(1);

	pack = erdashd.pack;
	if (pack !=0 && pack !=1 && pack !=2)
	{
		fprintf("Error can not handle pack type %d",pack);
		exit(0);
	}

	size = erdashd.rcols;

	/* make it big enough for 16 bit stuff */
	buf = (unsigned char *)malloc(size*2);
	if (buf==NULL) {
		fprintf(stderr,"Error in memory allocation\n");
		exit(0);
	}

	for (i=0;i<erdashd.nbands;i++) outband[i]=i+1;
	outband[i]=0;	
	if (sel_bands->answers)
		for(i=0; (sel_bands->answers[i] != NULL) && i<erdashd.nbands; i++){
			/*printf("%d %s \n", atoi(sel_bands->answers[i]),sel_bands->answers[i]);*/
			outband[i]=atoi(sel_bands->answers[i]);
		}

	/*getbands(outband,erdashd.nbands);*/

	if (start_row->answer != NULL){
		firstrow = atol(start_row->answer);
		if (firstrow < 1 || firstrow > erdashd.rrows)
			G_fatal_error("Invalid starting row");
	}
	else
		firstrow = erdashd.ry;
	if (num_rows->answer != NULL){
		lastrow = (double)atol(num_rows->answer) + firstrow -1;
		if (lastrow < firstrow || lastrow > erdashd.rrows)
			G_fatal_error("Invalid number of rows");
	}
	else
		lastrow = erdashd.ry + erdashd.rrows-1;

	if (start_col->answer != NULL){
		firstcol = (double)atol(start_col->answer);
		if (firstcol < 1 || firstcol > erdashd.rcols)
			G_fatal_error("Invalid starting column");
		offset = firstcol-1;
	}
	else{
		firstcol = erdashd.rx;
		offset = 0;
	}
	if (num_cols->answer != NULL){
		lastcol = (double)atol(num_cols->answer) + firstcol -1;
		if (lastcol < firstcol || lastcol > erdashd.rcols)
			G_fatal_error("Invalid number of columnss");
	}
	else
		lastcol = erdashd.rx + erdashd.rcols-1;

/*	if ((getwin(&firstrow,&firstcol,&lastrow,&lastcol))) 
		offset = firstcol-1;
	else 
		offset=0;
*/
	if (erdashd.maptyp == 1) {
		if (set_uwindow(&erdashd,firstrow,lastrow,firstcol,lastcol) <0) {
			fprintf(stderr,"Error in setting GRASS window cordsn");
			exit(0);
		}
	}
	else
		if (set_window(firstrow,lastrow,firstcol,lastcol) <0) {
			fprintf(stderr,"Error in setting GRASS window cordsn");
			exit(0);
		}

	i=0;
	for (band=outband[0];band!=0;band=outband[++i]) {
		sprintf(grassname,"%s.%d",outopt->answer,band);
		if ((new[band]=G_open_cell_new(grassname)) <= 0) {
			fprintf(stderr,"Error in opening grass cell file band %d",band);
			exit(0);
		}
	}

	/* if using 16 bit data, double position of offset in buf */
	if (pack == 2) offset = offset*2;
	startbuf = buf+offset ;
	i =0;
	for (row=erdashd.ry;row<=lastrow;row++) {
		for (band=1;band<=erdashd.nbands;band++) {
			switch(pack){
			case 0:
				n=read(erdf,buf,size);
				break;
			case 1:
				n=do4bit(erdf,buf,size,row,&lastnibble);
				break;
			case 2:
				n=do16bit(erdf,buf,size);
				break;
			}
			/*if (n!=size) {
			fprintf(stderr,"Error in reading row %d, size = %d n=%d\n",row,size,n);
			exit(0);
			}
*/
			if (row >= firstrow) {
				for (i=0;i<erdashd.nbands;i++) {
					if (band == outband[i]) {
						if (put_row(new[band],startbuf,row,pack) < 0) {
							fprintf(stderr,"Error in putting row %d band %d\n",row,band);
							exit(0);
						}
					}
				}
			}
		}
		putchar('.');
		fflush(stdout);
	}
	putchar('\n');

	i=0;
	for (band=outband[0];band!=0;band=outband[++i]){
		printf("creating file %s.%d\n",outopt->answer,band);
		G_close_cell(new[band]);
		if (trlopt->answer){
			 do_color(trlopt->answer,outopt->answer,band);
			 do_label(trlopt->answer,outopt->answer,band, erdashd.nclass);
		}
	}
}
