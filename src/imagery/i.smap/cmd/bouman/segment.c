#include "defs.h"
#include "imagery.h"
#include "region.h"
#include "../shapiro/files.h"
#include "../shapiro/parms.h"

static void init_reg();
static int increment_reg();
static shift_img();
static shift_ll();


segment(S,parms,files)
  struct SigSet *S;            /* class parameters */
  struct files *files;
  struct parms *parms;
{
  int block_size;             /* size of subregion blocks */
  int ml;                     /* max likelihood? */
  int quiet;                  /* be quiet when running? */

  CELL ***img;                /* multispectral image, img[band][i][j] */
  int  wd,ht;                 /* image width and height */
  struct Region region;       /* specifies image subregion */
  int nbands;                 /* number of bands */
  int nclasses;               /* number of classes */
  LIKELIHOOD ****ll_pym;      /* pyramid of log likelihoods */
  unsigned char ***sf_pym;    /* pyramid of segmentations */
  int  D;                     /* number of levels in pyramid */
  double *alpha_dec;          /* class transition probabilities */
  int  vlevel;                /* level of verbose output */
  int  i;

  quiet = parms->quiet; /* run quietly? */
  ml    = parms->ml;	/* use maxl? */
  block_size = parms->blocksize;

  vlevel = quiet ? 0 : 1 ;
  wd = G_window_cols(); /* get width from GRASS */
  ht = G_window_rows(); /* get height from GRASS */

  /* make blocksize a power of 2 */
  if (block_size < 8) block_size = 8;
  for(i=0; (block_size>>i)>1; i++)
	{}
  block_size = 1<<i;

/**** this code may stay the same ******/
  nbands = S->nbands;
  nclasses = S->nclasses;

  /* Check for too many classes */
  if(nclasses>256) {
    G_fatal_error("number of classes must be < 256!");
    exit(1);
  }

  /* allocate alpha_dec parameters */
  D = levels(block_size,block_size);
  alpha_dec = (double *)G_malloc(D*sizeof(double));

  /* allocate image block */
  img = (CELL ***)multialloc(sizeof(CELL),3,nbands,block_size,block_size);

  /* allocate memory for log likelihood pyramid */
  ll_pym = (LIKELIHOOD ****)get_cubic_pyramid(block_size,block_size,nclasses,sizeof(LIKELIHOOD));

  /* allocate memory for segmentation pyramid */
  sf_pym = (unsigned char ***)get_pyramid(wd,ht,sizeof(char));

  /* tiled segmentation */
  init_reg(&region,wd,ht,block_size);
  extract_init(S);
  do {
    if(vlevel>=1) 
      printf("Processing rows %d-%d (of %d), cols=%d-%d (of %d)\n",
	region.ymin+1,region.ymax,ht,
	region.xmin+1,region.xmax,wd);
    shift_img(img,nbands,&region,block_size);
    /* this reads grass images into the block defined in region */
    read_block(img,&region,files);

    shift_ll(ll_pym,&region,block_size);
    extract(img,&region,ll_pym[0],S);

    if(ml)
      MLE(sf_pym[0],ll_pym[0],&region,nclasses);
    else {
      for(i=0; i<D; i++) alpha_dec[i]=1.0;
      seq_MAP(sf_pym,&region,ll_pym,nclasses,alpha_dec,vlevel);
    }


  } while(increment_reg(&region,wd,ht,block_size));

  write_img(sf_pym[0],wd,ht,S,parms,files);
}

static void init_reg(region,wd,ht,block_size)
struct Region *region;
int wd,ht;
int block_size;
{
   region->xmin = 0;
   region->ymin = 0;

   region->xmax = block_size;
   if(region->xmax>wd)
     region->xmax=wd;

   region->ymax = block_size;
   if(region->ymax>ht) 
     region->ymax=ht;

   region->free.left = 1;
   region->free.top = 1;
   region->free.right = 1;
   region->free.bottom = 1;
}

static int increment_reg(region,wd,ht,block_size)
struct Region *region;
int wd,ht;
int block_size;
{

   /* shift one block to right */
   if(region->xmax<wd) {
     region->xmin = region->xmax;
     region->xmax = region->xmin + block_size;
     if(region->xmax>wd) region->xmax = wd;
   }
   else {
     /* shift one block down */
     if(region->ymax<ht) {
       region->xmin = 0;
       region->xmax = region->xmin + block_size;
       if(region->xmax>wd) region->xmax = wd;

       region->ymin = region->ymax;
       region->ymax = region->ymax + block_size;
       if(region->ymax>ht) region->ymax = ht;
     }
     /* finished shifting */
     else {
       return(0);
     }
   }

   if(region->xmin == 0) region->free.left = 1;
   else region->free.left = 0;

   if(region->ymin == 0) region->free.top = 1;
   else region->free.top = 0;

   region->free.right = 1;
   region->free.bottom = 1;

   return(1);
}

static shift_img(img,nbands,region,block_size)
CELL ***img;
int nbands;
struct Region *region;
int block_size;
{
    static int xoffset=0;
    static int yoffset=0;
    int xdelta;
    int ystart,ystop,ydelta;
    int b,i;

    xdelta = region->xmin - xoffset;
    ydelta = region->ymin - yoffset;
    xoffset = region->xmin;
    yoffset = region->ymin;

    ystart = region->ymin;
    ystop  = ystart + block_size;

    for(b=0; b<nbands; b++) {
      img[b] -= ydelta;
      for(i=ystart; i<ystop; i++ )
        img[b][i] -= xdelta;
    }
}

static shift_ll(ll_pym,region,block_size)
LIKELIHOOD ****ll_pym;
struct Region *region;
int block_size;
{
    static int first=1;
    static int xoffset[20];
    static int yoffset[20];

    int xdelta;
    int ystart,ystop,ydelta;
    int D;
    int k,i;
    int block_size_k;
    struct Region region_buff;

    /* if first time, set offsets to 0 */
    if(first) {
      D=levels(block_size,block_size);
      for(k=0; k<=D; k++) 
        xoffset[k]=yoffset[k]=0;
      first=0;
    }

    /* save region information */
    copy_reg(region,&region_buff);

    /* subtract pointer offsets for each scale */
    D=levels(block_size,block_size);
    block_size_k = block_size;
    for(k=0; k<=D; k++) {
      xdelta = region->xmin - xoffset[k];
      ydelta = region->ymin - yoffset[k];
      xoffset[k] = region->xmin;
      yoffset[k] = region->ymin;

      ystart = region->ymin;
      ystop  = ystart + block_size_k;

      ll_pym[k] -= ydelta;
      for(i=ystart; i<ystop; i++ )
        ll_pym[k][i] -= xdelta;
      dec_reg(region);
      block_size_k = block_size_k/2;
    }

    /* replace region information */
    copy_reg(&region_buff,region);
}


#ifdef COMMENTED_OUT
read_block(img,wd,ht,nbands,region,infn)
CELL ***img;
int  wd;
int  ht;
int  nbands;
struct Region *region;
char *infn;
{
     static first=1;
     static unsigned char ***img_buf;

     int b,i,j;
     int xstop,ystop;
     char *infn_num;

     /* allocate image buffer first time called */
     if(first) {
       img_buf = (unsigned char ***)multialloc(sizeof(unsigned char),3,nbands,ht,wd);

       /* allocate memory for name extension */
       infn_num = (char *)G_malloc((strlen(optarg)+10)*sizeof(char));

       /* read data */
       if(nbands==1) {
         read_img(img_buf[0],wd,ht,infn);
       } 
       else { 
         for(b=1; b<=nbands; b++) {
           sprintf(infn_num,"%s.%d",infn,b);
           read_img(img_buf[b-1],wd,ht,infn_num);
         } 
       }
       first=0;
     }

     /* copy data from buffer */
     xstop = region->xmax; if(xstop>wd) xstop=wd;
     ystop = region->ymax; if(ystop>ht) ystop=ht;
     for(b=0; b<nbands; b++) 
     for(i=region->ymin; i<ystop; i++)
     for(j=region->xmin; j<xstop; j++)
       img[b][i][j] = img_buf[b][i][j];
}
#endif
