/*****************************************************************************/
/***                                                                       ***/
/***                                fft()                                  ***/
/***   Calculactes the fast Fourier transform of an array of real numbers  ***/
/***   Algorithm derived from Cooley et al (1969).                         ***/
/***   Implemented in Press et al (1988), Numerical Recipes in C.	   ***/
/***   GRASS coding, Ali R. Vali, University of Texas.			   ***/
/***   Modified by David B. Satnik, Central Washington University,	   ***/
/***               Jo Wood, V1.0, 19th October, 1994                       ***/
/***                                                                       ***/
/*****************************************************************************/

#include "frac.h"
#include <math.h>

/*****************************************************************************/
/*                                                                           */
/* Fast Fourier Transform Routine for two-dimensional array                  */
/* Adapted from N.M. Brenner Algorithm (Numerical Recipes in C pp. 407-412)  */
/*                                                                           */
/* Input arguments: i_sign - direction of transform :                        */
/*                                -1 -> transform, +1 -> inverse transform   */
/*                  DATA   - pointer to a complex linear array in row major  */
/*                           order containing the data and the result.       */
/*                  NN     - value of DATA dimension                         */
/*                  dimc   - value of image column dimension (max power of 2)*/
/*                  dimr   - value of image row dimension (max power of 2)   */
/*                                                                           */
/*****************************************************************************/

fft(i_sign,DATA,NN,dimc,dimr)
int    i_sign;
double *DATA[2];
int   dimc, dimr, NN;
{
    int    i, j, ip1, ip2, i1, i2, i3, i2rev, i3rev, ibit, ifp1, ifp2, k1, k2;
    int    N, Nprev, Nrem;
    double tempr, tempi, wr, wi, wpr, wpi, wtemp, theta, norm;
    double swr, swi;

    /* Apply normalization factor */

    norm = 1.0 / sqrt((double)NN);
    for (i = 0; i < NN; i++)
      for (j = 0; j < 2; j++)
        *(DATA[j]+i) *= norm;

    /* Begin FFT algorithm */
    Nprev = 1;
    N = dimr;

    for (i = 0; i < 2; i++)
    {
        Nrem = NN / (N * Nprev);
        ip1 = Nprev * N;

        /* This is the bit reversal section */

        i2rev = 0;
        for (i2 = 0; i2 < ip1; i2 += Nprev)
        {
            if (i2 < i2rev)
            {
                for (i1 = i2; i1 < (i2+Nprev); i1++)
                    for (i3 = i1; i3 < NN; i3 += ip1)
                    {
                        i3rev = i2rev + i3 - i2;
                        tempr = *(DATA[0]+i3);
                        tempi = *(DATA[1]+i3);
                        *(DATA[0]+i3) = *(DATA[0]+i3rev);
                        *(DATA[1]+i3) = *(DATA[1]+i3rev);
                        *(DATA[0]+i3rev) = tempr;
                        *(DATA[1]+i3rev) = tempi;
                    }
            }   /* end if */
            ibit = ip1 >> 1;
            while ((ibit >= Nprev) && (i2rev >= ibit))
            {
                i2rev -= ibit;
                ibit >>= 1;
            }
            i2rev += ibit;
        }   /* end bit reversal section */

        /* Begin Danielson-Lanczos section of algorithm */

        ifp1 = Nprev;
        while (ifp1 < ip1)
        {
            ifp2 = 2 * ifp1;
            theta = i_sign * TWOPI / (ifp2 / Nprev);
            wpr = cos(theta);
            wpi = sin(theta);
            wr = 1.0;
            wi = 0.0;
            for (i3 = 0; i3 < ifp1; i3 += Nprev)
            {
                for (i1 = i3; i1 < (i3+Nprev); i1++)
                    for (i2 = i1; i2 < NN; i2 += ifp2)

                    /* Danielson-Lanczos formula */
                    {
                        k1 = i2;
                        k2 = k1 + ifp1;
                        swr = wr;
                        swi = wi;
                        tempr = swr * *(DATA[0]+k2) - swi * *(DATA[1]+k2);
                        tempi = swr * *(DATA[1]+k2) + swi * *(DATA[0]+k2);
                        *(DATA[0]+k2) = *(DATA[0]+k1) - tempr;
                        *(DATA[1]+k2) = *(DATA[1]+k1) - tempi;
                        *(DATA[0]+k1) = *(DATA[0]+k1) + tempr;
                        *(DATA[1]+k1) = *(DATA[1]+k1) + tempi;
                    }

                /* trigonometric recurrence */
                wtemp = wr;
                wr = wr * wpr - wi * wpi;
                wi = wi * wpr + wtemp * wpi;
            }
            ifp1 = ifp2;

        } 

        Nprev = N;
        N = dimc;
    }

    return(0);
}   
