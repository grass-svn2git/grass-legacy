/*
 ****************************************************************************
 *
 * MODULE:       Vector library 
 *              
 * AUTHOR(S):    Original author CERL, probably Dave Gerdes.
 *               Update to GRASS 5.7 Radim Blazek.
 *
 * PURPOSE:      Lower level functions for reading/writing/manipulating vectors.
 *
 * COPYRIGHT:    (C) 2001 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *              License (>=v2). Read the file COPYING that comes with GRASS
 *              for details.
 *
 *****************************************************************************/
#include <stdio.h>
#include <grass/Vect.h>

/*
 **  Written by Dave Gerdes  9/1988
 **  US Army Construction Engineering Research Lab
 */


/* 
 ** 
 **  This code is a quick hack to allow the writing of portable
 **  binary data files.
 **  The approach is to take known values and compare them against
 **  the current machine's internal representation.   A cross reference
 **  table is then built, and then all file reads and writes must go through
 **  through these routines to correct the numbers if need be.
 **
 **  As long as the byte switching is symetrical, the conversion routines
 **  will work both directions.

 **  The integer test patterns are quite simple, and their choice was
 **  arbitrary, but the float and double valued were more critical.

 **  I did not have a specification for IEEE to go by, so it is possible
 **  that I have missed something.  My criteria were:
 **
 **  First, true IEEE numbers had to be chosen to avoid getting an FPE.
 **  Second, every byte in the test pattern had to be unique.   And
 **  finally, the number had to not be sensitive to rounding by the 
 **  specific hardware implementation.
 **
 **  By experimentation it was found that the number  1.3333  met
 **  all these criteria for both floats and doubles

 **  See the discourse at the end of this file for more information
 **  
 **
 */

#define TEST_PATTERN 1.3333
#define LONG_TEST 0x01020304
#define INT_TEST 0x01020304
#define SHORT_TEST 0x0102

static double u_d = TEST_PATTERN;
static float u_f = TEST_PATTERN;
static long u_l = LONG_TEST;
static int u_i = INT_TEST;
static short u_s = SHORT_TEST;

/* dbl_cmpr holds the bytes of an IEEE representation of  TEST_PATTERN */
static const unsigned char dbl_cmpr[] =
    { 0x3f, 0xf5, 0x55, 0x32, 0x61, 0x7c, 0x1b, 0xda };
/* flt_cmpr holds the bytes of an IEEE representation of  TEST_PATTERN */
static const unsigned char flt_cmpr[] = { 0x3f, 0xaa, 0xa9, 0x93 };
static const unsigned char lng_cmpr[] = { 0x01, 0x02, 0x03, 0x04 };
static const unsigned char int_cmpr[] = { 0x01, 0x02, 0x03, 0x04 };
static const unsigned char shrt_cmpr[] = { 0x01, 0x02 };

/* Find native sizes */
int nat_dbl = sizeof(double);
int nat_flt = sizeof(float);
int nat_lng = sizeof(long);
int nat_int = sizeof(int);
int nat_shrt = sizeof(short);

int dbl_order;
int flt_order;
int lng_order;
int int_order;
int shrt_order;

unsigned char dbl_cnvrt[sizeof(double)];
unsigned char flt_cnvrt[sizeof(float)];
unsigned char lng_cnvrt[sizeof(long)];
unsigned char int_cnvrt[sizeof(int)];
unsigned char shrt_cnvrt[sizeof(short)];

/*
 * match search_value against each char in basis. 
 * return offset or -1 if not found
 */

static int find_offset(const unsigned char *basis, unsigned char search_value,
		       int size)
{
    int i;

    for (i = 0; i < size; i++)
	if (basis[i] == search_value)
	    return (i);

    return (-1);
}

static int find_offsets(const void *pattern, unsigned char *cnvrt,
			const unsigned char *cmpr, int port_size,
			int nat_size, const char *typename)
{
    int big, ltl;
    int i;

    for (i = 0; i < port_size; i++) {
	int off = find_offset(pattern, cmpr[i], nat_size);

	if (off < 0)
	    G_fatal_error("could not find '%x' in %s", cmpr[i], typename);

	cnvrt[i] = off;
    }

    big = ltl = 1;

    for (i = 0; i < port_size; i++) {
	if (cnvrt[i] != (nat_size - port_size + i))
	    big = 0;		/* isn't big endian */
	if (cnvrt[i] != (port_size - 1 - i))
	    ltl = 0;		/* isn't little endian */
    }

    if (big)
	return ENDIAN_BIG;

    if (ltl)
	return ENDIAN_LITTLE;

    return ENDIAN_OTHER;
}

void port_init(void)
{
    static int done;

    if (done)
	return;

    done = 1;

    /* Following code checks only if all assumptions are fullfilled */
    /* Check sizes */
    if (nat_dbl != PORT_DOUBLE)
	G_fatal_error("sizeof(double) != %d", PORT_DOUBLE);
    if (nat_flt != PORT_FLOAT)
	G_fatal_error("sizeof(float) != %d", PORT_DOUBLE);
    if (nat_lng < PORT_LONG)
	G_fatal_error("sizeof(long) < %d", PORT_LONG);
    if (nat_int < PORT_INT)
	G_fatal_error("sizeof(int) < %d", PORT_INT);
    if (nat_shrt < PORT_SHORT)
	G_fatal_error("sizeof(short) < %d", PORT_SHORT);

    /* Find for each byte in big endian test pattern (*_cmpr) 
     * offset of corresponding byte in machine native order.
     * Look if native byte order is little or big or some other (pdp)
     * endian.
     */

    dbl_order =
	find_offsets(&u_d, dbl_cnvrt, dbl_cmpr, PORT_DOUBLE, nat_dbl,
		     "double");
    flt_order =
	find_offsets(&u_f, flt_cnvrt, flt_cmpr, PORT_FLOAT, nat_flt, "float");
    lng_order =
	find_offsets(&u_l, lng_cnvrt, lng_cmpr, PORT_LONG, nat_lng, "long");
    int_order =
	find_offsets(&u_i, int_cnvrt, int_cmpr, PORT_INT, nat_int, "int");
    shrt_order =
	find_offsets(&u_s, shrt_cnvrt, shrt_cmpr, PORT_SHORT, nat_shrt,
		     "short");
}
