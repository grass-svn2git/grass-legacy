#include "defs.h"
#include "imagery.h"
#include "region.h"

#define PI 3.141592654


void extract_init(S)
struct SigSet *S;
{
   int m; 
   int i;
   int b1,b2;
   int nbands;
   double *lambda;
   struct ClassSig *C;
   struct SubSig *SubS;

   nbands = S->nbands;
   /* allocate scratch memory */
   lambda = (double *)G_malloc(nbands*sizeof(double));

   /* invert matrix and compute constant for each subclass */

   /* for each class */
   for(m=0; m<S->nclasses; m++ ) {
     C = &(S->ClassSig[m]);

     /* for each subclass */
     for(i=0; i<C->nsubclasses; i++) {
        SubS = &(C->SubSig[i]);

        /* Test for symetric  matrix */
        for(b1=0; b1<nbands; b1++)
        for(b2=0; b2<nbands; b2++) {
          if(SubS->R[b1][b2]!=SubS->R[b2][b1]) {
            fprintf(stderr,"\nWarning: nonsymetric covariance for class %d ",m+1);
            fprintf(stderr,"Subclass %d\n",i+1);
          }
          SubS->Rinv[b1][b2] = SubS->R[b1][b2];
        }

        /* Test for positive definite matrix */
        eigen(SubS->Rinv,lambda,nbands);
        for(b1=0; b1<nbands; b1++) {
          if(lambda[b1]<=0.0) {
            fprintf(stderr,"Warning: nonpositive eigenvalues for class %d",m+1);
            fprintf(stderr,"Subclass %d\n",i+1);
          }
        }

        /* Precomputes the cnst */
        SubS->cnst = (-nbands/2.0)*log(2*PI);
        for(b1=0; b1<nbands; b1++) {
            SubS->cnst += - 0.5*log(lambda[b1]);
        }

        /* Precomputes the inverse of tex->R */
        invert(SubS->Rinv,nbands);
      }
    }
    free((char *)lambda);
}


void extract(img,region,ll,S)
CELL ***img;             /* multispectral image, img[band][i][j] */
LIKELIHOOD ***ll;        /* log likelihood, ll[i][j][class] */
struct Region *region;    /* region to extract */
struct SigSet *S;        /* class signatures */
{
    int  i,j;             /* column and row indexes */
    int  m;               /* class index */
    int  k;               /* subclass index */
    int  b1,b2;           /* spectral index */
    int  no_data;          /* no data flag */
    int  max_nsubclasses; /* maximum number of subclasses */
    int  nbands;          /* number of spectral bands */
    double *subll;        /* log likelihood of subclasses */
    double *diff; 
    double maxlike;
    double subsum;
    struct ClassSig *C;
    struct SubSig *SubS;

    nbands = S->nbands;

    /* determine the maximum number of subclasses */
    max_nsubclasses = 0;
    for(m=0; m<S->nclasses; m++ )
      if(S->ClassSig[m].nsubclasses>max_nsubclasses)
        max_nsubclasses = S->ClassSig[m].nsubclasses;

    /* allocate memory */
    diff = (double *)G_malloc(nbands*sizeof(double));
    subll = (double *)G_malloc(max_nsubclasses*sizeof(double));

    /* Compute log likelihood at each pixel and for every class. */

    /* for each pixel */
    for(i=region->ymin; i<region->ymax; i++)
    for(j=region->xmin; j<region->xmax; j++) {

      /* Check for no data condition */
      no_data = 1;
      for(b1=0; (b1<nbands)&&no_data; b1++)
         no_data = no_data&&(!img[b1][i][j]);

      if(no_data) {
        for(m=0; m<S->nclasses; m++ )
          ll[i][j][m] = 0.0;
      }
      else {
        /* for each class */
        for(m=0; m<S->nclasses; m++ ) {
          C = &(S->ClassSig[m]);

          /* compute log likelihood for each subclass */
          for(k=0; k<C->nsubclasses; k++) {
            SubS = &(C->SubSig[k]);
            subll[k] = SubS->cnst;
            for(b1=0; b1<nbands; b1++) {
              diff[b1] = img[b1][i][j] - SubS->means[b1];
              subll[k] -= 0.5*diff[b1]*diff[b1]*SubS->Rinv[b1][b1];
            }
            for(b1=0; b1<nbands; b1++) 
            for(b2=b1+1; b2<nbands; b2++)
              subll[k] -= diff[b1]*diff[b2]*SubS->Rinv[b1][b2];
          }

          /* shortcut for one subclass */
          if(C->nsubclasses==1) {
            ll[i][j][m] = subll[0];
          }
          /* compute mixture likelihood */
          else {
            /* find the most likely subclass */
            for(k=0; k<C->nsubclasses; k++)
            {
              if(k==0) maxlike = subll[k];
              if(subll[k]>maxlike) maxlike = subll[k];
            }

            /* Sum weighted subclass likelihoods */
            subsum = 0;
            for(k=0; k<C->nsubclasses; k++)
              subsum += exp( subll[k]-maxlike )*C->SubSig[k].pi;

            ll[i][j][m] = log(subsum) + maxlike;
          }
        }
      }
    }
    free((char *)diff);
    free((char *)subll);
}
