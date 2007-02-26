/******************************************************************************
 * gmath.h
 * Top level header file for gmath units

 * @Copyright David D.Gray <ddgray@armadce.demon.co.uk>
 * 27th. Sep. 2000
 * Last updated: $Id$
 *

 * This file is part of GRASS GIS. It is free software. You can 
 * redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 ******************************************************************************/

#ifndef GMATH_H_
#define GMATH_H_

#include <grass/config.h>
#if defined(HAVE_LIBLAPACK) && defined(HAVE_LIBBLAS) && defined(HAVE_G2C_H)
 /* only include if available */
#include <grass/la.h>
#endif

/* fft.c */
int fft(int, double *[2], int, int, int);
int fft2(int, double (*)[2], int, int, int);
/* gauss.c */
float gauss(int);
/* max_pow2.c */
long max_pow2 (long n);
long min_pow2 (long n);
/* rand1.c */
float rand1(int);
/* del2g.c */
int del2g(double *[2], int, double);
/* getg.c */
int getg(double, double *[2], int);
/* eigen.c */
int eigen(double **, double **, double *, int);
int egvorder2(double *, double **, long);
int transpose2(double **, long);
/* jacobi.c */
#define MX 9
int jacobi(double [MX][MX], long, double [MX], double [MX][MX]);
int egvorder(double [MX], double [MX][MX], long);
int transpose(double [MX][MX], long);
/* mult.c */
int mult (double *v1[2], int size1, double *v2[2], int size2, double *v3[2], int size3);

#endif /* GMATH_H_ */

