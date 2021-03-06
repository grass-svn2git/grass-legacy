/***************************************************************************
*
*
*                         NCSA HDF version 3.2r3
*                            December 1, 1992
*
* NCSA HDF Version 3.2 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
*
* We ask, but do not require, that the following message be included in all
* derived works:
*
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign, in collaboration with the
* Information Technology Institute of Singapore.
*
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
*
****************************************************************************
*/
#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 1.5 $";
#endif
/*
$Header: /hdf/hdf/v3.2r2/src/RCS/dfr8.c,v 1.5 1992/10/30 18:36:52 koziol beta $

$Log: dfr8.c,v $
 * Revision 1.5  1992/10/30  18:36:52  koziol
 * Fixed casts in a bunch of functions to make the SGI ANSI compiler happy
 *
 * Revision 1.4  1992/10/23  00:14:11  koziol
 * Changed all DFIstr*() and DFImem*() calls to HDstr*() and HDmem*() calls
 * #ifdef'd out the macros Jason defined for Hopen, Hclose, etc. for Vsets
 * Replaced Vset VFREESPACE and VGETSPACE calls with actual calls to HDfreespace
 * and HDgetspace
 * Added a MS-Windows lower lower for file I/O (which may not be completely working
 *
 * Revision 1.3  1992/10/12  18:11:51  koziol
 * Updated for v3.2r2 release
 *
 * Revision 1.2  1992/09/17  20:02:56  koziol
 * Included Shiming's bugfix to RIG stuff
 *
 * Revision 1.1  1992/08/25  21:40:44  koziol
 * Initial revision
 *
*/
/*-----------------------------------------------------------------------------
 * File:    dfr8.c
 * Purpose: read and write 8-bit Raster Image Groups
 * Invokes: df.c, dfcomp.c, dfgroup.c, dfrig.h
 * Contents:
 *  DFR8getdims: retrieve information about 8-bit image dimensions
 *  DFR8getimage: retrieve 8-bit image and associated palette
 *  DFR8setpalette: specify palette to be used with subsequent 8-bit images
 *  DFR8Iputimage: internal routine that write 8-bit images to files
 *  DFR8putimage: write 8-bit image into an HDF file
 *  DFR8addimage: append another 8-bit image to an HDF file
 *  DFR8getrig: read in a raster image group for 8-bit images
 *  DFR8putrig: write out a raster image group for 8-bit images
 *  DFR8nimages: number of images in HDF file
 *  DFR8readref: get image with this reference number next
 *  DFR8writeref: put image with this reference number next
 *  DFR8restart: forget info about last file accessed - restart from beginning
 *  DFR8lastref: return reference number of last element read or written
 *  DFR8Iopen: open/reopen file
 *  DFR8Iriginfo: obtain info about next RIG/RI8 to get
 * Remarks: A RIG specifies attributes associated with an image - palette,
 *          dimension, compression, color compensation etc.
 *          The palette for an 8-bit image is assumed to always be 768 bytes
 *          The palette is arranged as RGBRGB...
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "herr.h"
#include "dfrig.h"

static int foundRig = -1;      /* -1: don't know if HDF file has RIGs */
                               /* 0: No RIGs, try for RI8s etc. */
                               /* 1: RIGs used, ignore RI8s etc. */
static DFRrig Readrig;         /* information about RIG being read */
static DFRrig Writerig;        /* information about RIG being written */
static int Newdata = 0;        /* does Readrig contain fresh data? */
static uint16 Writeref=0;      /* ref of next image to put in this file */
static int Newpalette=(-1);    /* -1 = no palette is associated */
                               /* 0 = palette already written out */
                               /* 1 = new palette, not yet written out */
static uint8 Palette[768];      /* to store palette for 8-bit images */
static uint16 Refset=0;        /* Ref of image to get next */
static uint16 Lastref = 0;     /* Last ref read/written */
static DFRrig Zrig = {         /* empty RIG for initialization */
    {0, 0}, {0, 0, {0, 0}, 0, 0, {0, 0}},
    {0, 0}, {0, 0, {0, 0}, 0, 0, {0, 0}},
    {0, 0}, {0, 0, {0, 0}, 0, 0, {0, 0}},
    0, 0, (float32)0.0, (float32)0.0,
    {(float32)0.0, (float32)0.0, (float32)0.0},
    {(float32)0.0, (float32)0.0, (float32)0.0},
    {(float32)0.0, (float32)0.0, (float32)0.0},
    {(float32)0.0, (float32)0.0, (float32)0.0}, NULL
};

#ifdef PERM_OUT
#ifndef VMS
int32 DFR8Iopen();
#else /*VMS*/
int32 _DFR8Iopen();
#endif
#endif

typedef struct R8dim {
    uint16 xd;
    uint16 yd;
} R8dim;                       /* dimensions of raster-8 image */

/* private functions */
#ifdef PROTOTYPE
PRIVATE intn DFR8Iputimage(char *filename, VOIDP image, int32 xdim, int32 ydim,
                         uint16 compress, int op);
PRIVATE int DFR8getrig(int32 file_id, uint16 ref, DFRrig *rig);
PRIVATE int DFR8putrig(int32 file_id, uint16 ref, DFRrig *rig, int wdim);
PRIVATE int32 DFR8Iopen(char *filename, int access);
PRIVATE int DFR8Iriginfo(int32 file_id);
#else
PRIVATE intn DFR8Iputimage();
PRIVATE int DFR8getrig();
PRIVATE int DFR8putrig();
PRIVATE int32 DFR8Iopen();
PRIVATE int DFR8Iriginfo();
#endif

uint8 R8tbuf[512];

/*-----------------------------------------------------------------------------
 * Name:    DFR8getdims
 * Purpose: get dimensions of next image from RIG, also if there is a palette
 * Inputs:  filename: name of HDF file
 *          pxdim, pxdim, pointer to locations for returning x,y dimensions
 *          pispal: pointer to location for rtning whether there is a palette
 * Returns: 0 on success, -1 on failure with DFerror set
 *          *pxdim, *pydim are set to dimensions of the next image
 *          *pispal is set to 1 if a palette is associated with it, else 0
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFR8Iopen, DFclose, DFR8Iriginfo, DFIerr
 * Remarks: will also handle file with just raster-8 tags: RI8, CI8, ID8, IP8
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
intn DFR8getdims(char *filename, int32 *pxdim, int32 *pydim, intn *pispal)
#else
intn DFR8getdims(filename, pxdim, pydim, pispal)
    char *filename;
    int32 *pxdim, *pydim;
    intn *pispal;
#endif
{
    char *FUNC="DFR8getdims";
    int32 file_id;

    HEclear();

    if (!filename || !*filename || !pxdim || !pydim) {
        HERROR(DFE_ARGS);
        return FAIL;
    }

    file_id = DFR8Iopen(filename, DFACC_READ);
    if (file_id == FAIL) {
       return FAIL;
    }

    if (DFR8Iriginfo(file_id) == FAIL) /* reads next RIG or RI8 from file */
        return(HDerr(file_id)); /* on error, close file and return -1 */

    Newdata = 1;
    *pxdim = Readrig.descimage.xdim;
    *pydim = Readrig.descimage.ydim;
    if (pispal) *pispal = Readrig.lut.tag ? 1 : 0; /* is there a palette */

    return(Hclose(file_id));
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8getimage
 * Purpose: get next image from a RIG, get palette also if desired
 * Inputs:  filename: name of HDF file
 *          image: space to read image into
 *          xdim, ydim: dimensions of space allocated by user for image
 *          pal: 768-byte space for palette, null if palette not wanted
 * Returns: 0 on success, -1 on failure with DFerror set
 *          image in image, palette in pal
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFR8Iopen, DFR8Iriginfo, HDerr, DFclose, DFgetelement, DFgetcomp
 * Remarks: Will also get RI8s and CI8s if no RIGs in file
 *          Normally,DFR8getdims is called first and it finds next image to get
 *          But if that is not called, DFR8getimage will itself find next image
 *          Automatically decompresses images
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
intn DFR8getimage(char *filename, uint8 *image, int32 xdim, int32 ydim,
                uint8 *pal)
#else
intn DFR8getimage(filename, image, xdim, ydim, pal)
    char *filename;
    int32 xdim, ydim;
    uint8 *image;
    uint8 *pal;
#endif
{
    char *FUNC="DFR8getimage";
    int32 file_id;

    HEclear();

    if (!filename || !*filename || !image
       || (xdim<=0) || (ydim<=0)) {
       HERROR(DFE_ARGS);
        return FAIL;
    }

    file_id = DFR8Iopen(filename, DFACC_READ);
    if (file_id == FAIL) {
       return(FAIL);
    }

    if (!Newdata) {            /* if Readrig not fresh */
        if (DFR8Iriginfo(file_id) == FAIL) /*reads next RIG or RI8 from file */
            return(HDerr(file_id)); /* on error, close file and return -1 */
    }
    Newdata = 0;               /* read new RIG next time */

    if ((Readrig.descimage.xdim > xdim) || (Readrig.descimage.ydim > ydim)) {
       HERROR(DFE_ARGS);
        return(HDerr(file_id));
    }

    /* read image */
    if (Readrig.descimage.compr.tag) { /* compressed image */
        if (DFgetcomp(file_id, Readrig.image.tag, Readrig.image.ref, image,
                     Readrig.descimage.xdim, Readrig.descimage.ydim,
                     Readrig.descimage.compr.tag) == FAIL)
            return(HDerr(file_id));
    } else {                   /* non-compressed raster image */
        if (Hgetelement(file_id, Readrig.image.tag,
                       Readrig.image.ref, (uint8 *)image) == FAIL)
            return(HDerr(file_id));
    }

    if (xdim > Readrig.descimage.xdim) {
       int32 off1, off2;
       int32 x, y;

       off1 = (Readrig.descimage.ydim - 1) * xdim;
       off2 = (Readrig.descimage.ydim - 1) * Readrig.descimage.xdim;
       for (y = Readrig.descimage.ydim - 1; y > 0; y-- ) {
           for (x = Readrig.descimage.xdim - 1; x >= 0; x--)
               image[off1+x] = image[off2+x];
           off1 -= xdim;
           off2 -= Readrig.descimage.xdim;
       }
    }

    if (pal && Readrig.lut.tag) { /* read palette */
        if (Hgetelement(file_id, Readrig.lut.tag, 
                               Readrig.lut.ref,(uint8 *)pal) == FAIL)
            return(HDerr(file_id));
    }
    return(Hclose(file_id));
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8setpalette
 * Purpose: set palette for subsequent images
 * Inputs:  pal: palette to set
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: none
 * Remarks: if pal is NULL, no palette is associated with subsequent images
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
int DFR8setpalette(uint8 *pal)
#else
int DFR8setpalette(pal)
    uint8 *pal;
#endif
{
/*    char *FUNC="DFR8setpalette"; */
    int i;

/*    HEclear(); */

    if (!pal) {
        Newpalette = -1;       /* no palette */
        Writerig.lut.tag = 0;
        Writerig.lut.ref = 0;   /* forget tag/ref of previous palette */
        Writerig.desclut.xdim = 0;
        Writerig.desclut.ncomponents = 0;
    } else {                   /* store palette */
        for (i=0; i<768; i++)
            Palette[i] = pal[i];
        Newpalette = 1;
    }
    return SUCCEED;
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8Iputimage
 * Purpose: Internal routine to write RIG to file
 * Inputs:  filename: name of HDF file
 *          image: image to be written to file
 *          xdim, ydim: dimensions of image
 *          compress: compression scheme to be used on image, 0 if none
 *                    possible values are DFTAG_RLE and DFTAG_IMC
 *          op: 0 will overwrite existing file, 1 will append image to file
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF systems programmers, DFR8putimage, DFR8addimage
 * Invokes: DFR8Iopen, DFclose, DFputelement, DFdup, DFR8putrig, DFputcomp,
 *          HDerr
 * Remarks: Palette will be associated with image is isPalette is 1
 *          Palette will be written to file if not written before (Palref=0)
 *          Creates both RIG and RI8/CI8 tags, to accomodate older programs
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
PRIVATE intn DFR8Iputimage(char *filename, VOIDP image, int32 xdim, int32 ydim,
                         uint16 compress, int op)
#else
PRIVATE intn DFR8Iputimage(filename, image, xdim, ydim, compress, op)
    char *filename;
    int32 xdim, ydim;
    VOIDP image;
    uint16 compress;            /* compression scheme */
    int op;                     /* 0 is a put, 1 is a putnext */
#endif
{
    char *FUNC="DFR8Iputimage";
    int access;                        /* create if op 0, write if op 1 */
    int32 file_id;
    uint16 r8tag;              /*RIG and raster tags of image being written */
    uint8 *pal;                 /* pointer to palette to be written */
    uint8 newpal[768];          /*Imcomp creates new palette to be associated*/
    int wdim;                  /* have dimensions already been written out? */

    HEclear();

    if (!filename || !*filename || !image
       || (xdim<=0) || (ydim<=0)) {
       HERROR(DFE_ARGS);
        return FAIL;
    }
    pal = (Newpalette>=0) ? Palette : NULL;
    access = op ? DFACC_WRITE : DFACC_CREATE;

    file_id = DFR8Iopen(filename, access);
    if (file_id==FAIL) {
       return FAIL;
    }

    if (!Writeref) Writeref = Hnewref(file_id);
    if (Writeref == 0) {
       return FAIL;
    }

    /* write out image */
    if (compress) {
        if (DFputcomp(file_id, DFTAG_CI, Writeref, (uint8*)image, xdim, ydim,
                     (uint8*)pal, (uint8*)newpal, compress) == FAIL)
            return(HDerr(file_id));
        Writerig.image.tag = DFTAG_CI;
        if (compress==DFTAG_IMC) {
            pal = newpal;      /* Imcomp creates new pal */
            Newpalette = 1;    /* write out palette */
        }
    } else {                   /* image need not be compressed */
        if (Hputelement(file_id, DFTAG_RI, Writeref, 
                                     (uint8 *)image, xdim*ydim) == FAIL)
            return(HDerr(file_id));
        Writerig.image.tag = DFTAG_RI;
    }
    Writerig.image.ref = Writeref;
    Writerig.descimage.ncomponents = 1;
    Writerig.aspectratio = (float32)1.0;

    /* Write out Raster-8 tags for those who want it */
    r8tag = compress ?
       ((compress==DFTAG_RLE) ? DFTAG_CI8 : DFTAG_II8) : DFTAG_RI8;
    if (Hdupdd(file_id, r8tag, Writeref, Writerig.image.tag, Writeref) == FAIL)
        return(HDerr(file_id));

    /* Write out palette */
    if (pal) {                 /* if there is a palette */
        if (Newpalette==1) {   /* write palette */
            if (Hputelement(file_id, DFTAG_LUT, Writeref, 
                                   (uint8 *)pal, (int32) 768) == FAIL)
                return(HDerr(file_id));
            Writerig.lut.tag = DFTAG_LUT;
            Writerig.lut.ref = Writeref;
            Writerig.desclut.xdim = 768;
            Writerig.desclut.ncomponents = 1;
        }
        if (compress!=DFTAG_IMC) Newpalette = 0;
       /* if IMCOMP, original palette not written out */

       /* put in Raster-8 stuff also, for those who want it */
       Hdeldd(file_id, DFTAG_IP8, Writeref);
        if (Hdupdd(file_id, DFTAG_IP8, Writeref, Writerig.lut.tag,
                  Writerig.lut.ref) == FAIL)
            return(HDerr(file_id));
    }

    /* Write out RIG */
    if ((Writerig.descimage.xdim==xdim) && (Writerig.descimage.ydim==ydim) &&
       (Writerig.descimage.compr.tag==compress))
        wdim = 0;
    else {
        wdim = 1;
        Writerig.descimage.xdim = xdim;
        Writerig.descimage.ydim = ydim;
        Writerig.descimage.compr.tag = compress;
    }
    if (DFR8putrig(file_id, Writeref, &Writerig, wdim)
       == FAIL)                /* writes ID, NT */
        return(HDerr(file_id));

    Lastref = Writeref;                /* remember ref written */

    Writeref = 0;               /* don't know ref to write next */

    return(Hclose(file_id));
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8putimage
 * Purpose: Write RIG to HDF file
 * Inputs:  filename: name of HDF file
 *          image: image to be written to file
 *          xdim, ydim: dimensions of image
 *          compress: compression scheme to be used on image, 0 if none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFR8Iputimage
 * Remarks: overwrites existing HDF file
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
int DFR8putimage(char *filename, VOIDP image, int32 xdim, int32 ydim,
                uint16 compress)
#else
int DFR8putimage(filename, image, xdim, ydim, compress)
    char *filename;
    int32 xdim, ydim;
    VOIDP image;
    uint16 compress;
#endif
{
    return(DFR8Iputimage(filename, image, xdim, ydim, compress, 0));
}


/*-----------------------------------------------------------------------------
 * Name:    DFR8addimage
 * Purpose: Append RIG to HDF file
 * Inputs:  filename: name of HDF file
 *          image: image to be written to file
 *          xdim, ydim: dimensions of image
 *          compress: compression scheme to be used on image, 0 if none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFR8Iputimage
 * Remarks: inserts image into existing file, will create file if necessary
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
int DFR8addimage(char *filename, VOIDP image, int32 xdim, int32 ydim,
                uint16 compress)
#else
int DFR8addimage(filename, image, xdim, ydim, compress)
    char *filename;
    int32 xdim, ydim;
    VOIDP image;
    uint16 compress;
#endif
{
    return(DFR8Iputimage(filename, image, xdim, ydim, compress, 1));
}


/*****************************************************************************/
/* This is the next lower layer - procedures to get and put a RIG. */
/* These are specific to 8-bit */
/*****************************************************************************/

/*-----------------------------------------------------------------------------
 * Name:    DFR8getrig
 * Purpose: Read a RIG into memory
 * Inputs:  file_id: pointer to HDF file containing RIG
 *          ref: reference number of RIG to get
 *          rig: struct in which to place info obtained
 * Returns: 0 on success, -1 on failure with DFerror set
 *          contents of RIG in the struct rig
 * Users:   HDF programmers, utilities, DFR8getdims,DFR8getimage
 * Invokes: DFdiget, DFdinext, DFIcheck, DFgetelement
 * Remarks: assumes 8-bit
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
PRIVATE int DFR8getrig(int32 file_id, uint16 ref, DFRrig *rig)
#else
PRIVATE int DFR8getrig(file_id, ref, rig)
    int32 file_id;
    uint16 ref;
    DFRrig *rig;
#endif
{
    char *FUNC="DFR8getrig";
    uint16 elt_tag;
    uint16 elt_ref;
    uint8 ntstring[4];

    HEclear();

    if (!HDvalidfid(file_id) || !ref || !rig) {
       HERROR(DFE_ARGS);
        return FAIL;
    }

    if (DFdiread(file_id, DFTAG_RIG, ref) == FAIL) /* read RIG into memory */
        return FAIL;

    *rig = Zrig;               /* fill rig with zeroes */
    while (DFdiget(&elt_tag, &elt_ref) != FAIL) {
       /*get next tag/ref from RIG */
        switch (elt_tag) {     /* process tag/ref */
            case DFTAG_CI:
            case DFTAG_RI:
                rig->image.tag = elt_tag; /* put tag/ref in struct */
                rig->image.ref = elt_ref;
                break;
            case DFTAG_LUT:
                rig->lut.tag = elt_tag;
                rig->lut.ref = elt_ref;
                break;
            case DFTAG_ID:     /* read description info */
                if (Hgetelement(file_id, elt_tag, elt_ref, 
                                           (uint8 *)R8tbuf) != FAIL) {
                    uint8 *p;
                    p = R8tbuf;
                    INT32DECODE(p, rig->descimage.xdim);
                    INT32DECODE(p, rig->descimage.ydim);
                    UINT16DECODE(p, rig->descimage.nt.tag);
                    UINT16DECODE(p, rig->descimage.nt.ref);
                    INT16DECODE(p, rig->descimage.ncomponents);
                    INT16DECODE(p, rig->descimage.interlace);
                    UINT16DECODE(p, rig->descimage.compr.tag);
                    UINT16DECODE(p, rig->descimage.compr.ref);
                } else
                    return FAIL;
                if (rig->descimage.ncomponents!=1) {
                    HERROR(DFE_BADCALL);
                    return FAIL;
                }
                if (rig->descimage.nt.tag==0) break; /* old RIGs */

               /* read NT */
                if (Hgetelement(file_id, rig->descimage.nt.tag,
                               rig->descimage.nt.ref, ntstring) == FAIL)
                    return FAIL;
                if ((ntstring[2]!=8) || (ntstring[1]!=DFNT_UCHAR)) {
                    HERROR(DFE_BADCALL);
                    return FAIL;
                }
                break;
            default:           /* ignore unknown tags */
                break;
        }
    }
    return(0);
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8putrig
 * Purpose: Write RIG struct out to HDF file
 * Inputs:  file_id: HDF file pointer
 *          ref: ref to put RIG with
 *          rig: struct containing RIG info to put
 *          wdim: if 1, write out new description records. if 0 already written
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF programmers, utilities, DFR8Iputimage, other routines
 * Invokes: DFIcheck, DFdistart, DFdiadd, DFdiend, DFputelement
 * Remarks: assumes 8-bit.  Writes out NT if necessary, ID, ID8 if told to
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
PRIVATE int DFR8putrig(int32 file_id, uint16 ref, DFRrig *rig, int wdim)
#else
PRIVATE int DFR8putrig(file_id, ref, rig, wdim)
    int32 file_id;
    uint16 ref;
    DFRrig *rig;
    int wdim;
#endif
{
    char *FUNC="DFR8putrig";
    static uint16 prevdimref=0; /*ref of previous dimension record, to reuse */
    R8dim im8dim;
    uint8 ntstring[4];

    HEclear();

    if (!HDvalidfid(file_id) || !ref) {
       HERROR(DFE_ARGS);
        return FAIL;
    }

    if (!rig->descimage.nt.tag) {
       /* construct and write out NT */
        ntstring[0] = DFNT_VERSION; /* version */
        ntstring[1] = DFNT_UCHAR; /* type */
        ntstring[2] = 8;       /* width: RIG data is 8-bit chars */
        ntstring[3] = DFNTC_BYTE; /* class: data are numeric values */
        if (Hputelement(file_id, DFTAG_NT, ref, ntstring, (int32) 4)  == FAIL)
            return FAIL;
        rig->descimage.nt.tag = DFTAG_NT;
        rig->descimage.nt.ref = ref;
    }

    im8dim.xd = (uint16)rig->descimage.xdim;
    im8dim.yd = (uint16)rig->descimage.ydim;
    if (wdim) {
        uint8 *p;

        p = R8tbuf;
        INT32ENCODE(p, rig->descimage.xdim);
        INT32ENCODE(p, rig->descimage.ydim);
        UINT16ENCODE(p, rig->descimage.nt.tag);
        UINT16ENCODE(p, rig->descimage.nt.ref);
        INT16ENCODE(p, rig->descimage.ncomponents);
        INT16ENCODE(p, rig->descimage.interlace);
        UINT16ENCODE(p, rig->descimage.compr.tag);
        UINT16ENCODE(p, rig->descimage.compr.ref);
        if (Hputelement(file_id, DFTAG_ID, ref, R8tbuf,(int32)(p-R8tbuf))
           == FAIL)
            return FAIL;
       /* write out ID8 */
        p = R8tbuf;
        UINT16ENCODE(p, im8dim.xd);
        UINT16ENCODE(p, im8dim.yd);
        if (Hputelement(file_id, DFTAG_ID8, ref, R8tbuf, (int32) 4) == FAIL)
            return FAIL;
        prevdimref = ref;
    }
    if (!prevdimref) {
        HERROR(DFE_ARGS);
        return FAIL;
    }

    /* prepare to start writing rig */
    /* ### NOTE: the second parameter to this call may go away */
    if (DFdisetup(10) == FAIL)
       return FAIL;            /* max 10 tag/refs in set */

    /* add tag/ref to RIG - image description, image and palette */
    if (DFdiput(DFTAG_ID, prevdimref) == FAIL)
       return FAIL;

    if (DFdiput(rig->image.tag, rig->image.ref) == FAIL)
       return FAIL;

    if (rig->lut.ref
       && DFdiput(rig->lut.tag, rig->lut.ref) == FAIL)
       return FAIL;

    /* write out RIG */
    return(DFdiwrite(file_id, DFTAG_RIG, ref));
}


/*-----------------------------------------------------------------------------
 * Name:    DFR8nimages
 * Purpose: How many images are present in this file?
 * Inputs:  filename: name of HDF file
 * Returns: number of images  on success, -1 on failure with DFerror set
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8Iopen, DFclose, DFnumber
 * Remarks: the number is the number of RIGs if RIGs are present
 *          If not, it is the number of RI8s + number of CI8s
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
int DFR8nimages(char *filename)
#else
int DFR8nimages(filename)
    char *filename;
#endif
{
    char *FUNC="DFR8nimages";
    int32 file_id;
    int nimages=0;

    HEclear();

    /* should use reopen if same file as last time - more efficient */
    file_id = DFR8Iopen(filename, DFACC_READ);
    if (file_id == FAIL)
       return FAIL;

    /* find next rig */
    if (foundRig) {            /* either RIGs present or don't know */
        nimages = Hnumber(file_id, DFTAG_RIG); /* count number of RIGs */
        if (nimages>0) {
            foundRig = 1;
            if (Hclose(file_id) == FAIL)
               return FAIL;
            return(nimages);
        }
        foundRig = 0;
    }
    nimages = Hnumber(file_id, DFTAG_RI8);
    nimages += Hnumber(file_id, DFTAG_CI8);
    if (Hclose(file_id) == FAIL)
       return FAIL;
    return(nimages);
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8readref
 * Purpose: Set ref of image to get next
 * Inputs:  filename: file to which this applies
 *          ref: reference number of next get
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8Iopen, DFIfind
 * Remarks: checks if image with this ref exists
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
intn DFR8readref(char *filename, uint16 ref)
#else
intn DFR8readref(filename, ref)
    char *filename;
    uint16 ref;
#endif
{
    char *FUNC="DFR8readref";
    int32 file_id;
    int32 aid;

    HEclear();

    file_id = DFR8Iopen(filename, DFACC_READ);
    if (file_id == FAIL)
       return FAIL;
    if ((aid = Hstartread(file_id, DFTAG_RIG, ref)) == FAIL
       && (aid = Hstartread(file_id, DFTAG_RI8, ref)) == FAIL
       && (aid = Hstartread(file_id, DFTAG_CI8, ref)) == FAIL) {
       HERROR(DFE_NOMATCH);
       return(HDerr(file_id));
    }
    Refset = ref;
    Newdata = 0;
    Hendaccess(aid);
    return(Hclose(file_id));
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8writeref
 * Purpose: Set ref of image to put next
 * Inputs:  filename: file to which this applies
 *          ref: reference number of next put
 * Returns: 0 on success, -1 on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFR8Iopen, DFIfind
 * Remarks: none
 *---------------------------------------------------------------------------*/

/* shut lint up */
/* ARGSUSED */
#ifdef PROTOTYPE
int DFR8writeref(char *filename, uint16 ref)
#else
int DFR8writeref(filename, ref)
    char *filename;
    uint16 ref;
#endif
{
    HEclear();

    Writeref = ref;
    return SUCCEED;
}

static char Lastfile[DF_MAXFNLEN];          /* last file opened */

/*-----------------------------------------------------------------------------
 * Name:    DFR8restart
 * Purpose: Do not remember info about file - get again from first image
 * Inputs:  none
 * Returns: 0 on success
 * Users:   HDF programmers
 * Remarks: Just reset Lastfile to NULL
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
int DFR8restart(void)
#else
int DFR8restart()
#endif
{
    Lastfile[0] = '\0';
    return SUCCEED;
}


/*-----------------------------------------------------------------------------
 * Name:    DFR8lastref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: none
 * Method:  return Lastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
int DFR8lastref(void)
#else
int DFR8lastref()
#endif
{
    return((int) Lastref);
}



/*************************************************************************/
/*----------------------- Internal routines -----------------------------*/
/*************************************************************************/


/*-----------------------------------------------------------------------------
 * Name:    DFR8Iopen
 * Purpose: open or reopen a file
 * Inputs:  filename: name of file to open
 *          access : access mode
 * Returns: file pointer on success, NULL on failure with DFerror set
 * Users:   HDF systems programmers, all the RIG routines
 * Invokes: DFopen
 * Remarks: This is a hook for someday providing more efficient ways to
 *          reopen a file, to avoid re-reading all the headers
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
PRIVATE int32 DFR8Iopen(char *filename, int access)
#else
PRIVATE int32 DFR8Iopen(filename, access)
    char *filename;
    int access;
#endif
{

    int32 file_id;

    /* use reopen if same file as last time - more efficient */
    if (HDstrncmp(Lastfile,filename,DF_MAXFNLEN) || (access==DFACC_CREATE)) {
                               /* treat create as different file */
        file_id = Hopen(filename, access, 0);
       if (file_id == FAIL) {
           return FAIL;
       }
        foundRig = -1;         /* don't know if any RIGs in file */
        Refset = 0;            /* no ref to get set for this file */
        Newdata = 0;
        Readrig = Zrig;                /* blank out read/write RIGs */
        Writerig = Zrig;
        if (Newpalette!=(-1)) Newpalette = 1; /* need to write out palette */
    } else {
        file_id = Hopen(filename, access, 0);
       if (file_id == FAIL) {
           return FAIL;
       }
    }

    HDstrncpy(Lastfile, filename, DF_MAXFNLEN);
    /* remember filename, so reopen may be used next time if same file */
    return(file_id);
}

/*-----------------------------------------------------------------------------
 * Name:    DFR8Iriginfo
 * Purpose: Getinformation about next RIG or Raster-8 in file
 * Inputs:  file_id: pointer to DF file
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF systems programmers
 * Invokes: DFIfind, DFgetelement
 * Remarks: checks for RIGs first, then RI8s
 *          if Refset set, gets image with that ref, if any
 *---------------------------------------------------------------------------*/

#ifdef PROTOTYPE
PRIVATE int DFR8Iriginfo(int32 file_id)
#else
PRIVATE int DFR8Iriginfo(file_id)
    int32 file_id;
#endif
{
    char *FUNC="DFR8riginfo";
    uint16 riref=0, ciref=0;
    int32 aid=FAIL;
    uint16 ref;

    HEclear();
    /* find next rig */
    if (foundRig) {            /* either RIGs present or don't know */
       if (!Refset && Readrig.image.ref)
           aid = Hstartread(file_id, DFTAG_RIG, Readrig.image.ref);
       do {
           if (Refset) {
               aid = Hstartread(file_id, DFTAG_RIG, Refset);
           } else {
               if (!Readrig.image.ref) {
                   aid = Hstartread(file_id, DFTAG_RIG, DFREF_WILDCARD);
               } else {
                   if (aid != FAIL
                       && Hnextread(aid, DFTAG_RIG, DFREF_WILDCARD,
                                    DF_CURRENT) == FAIL) {
                       Hendaccess(aid);
                       aid = FAIL;
                   }
               }
           }
           if (aid == FAIL) {
               if (foundRig==1) { /*RIGs present, but no more to return */
                   HERROR(DFE_NOMATCH);
                   return FAIL;
               }
               foundRig = 0; /* No RIGs present in file */
           }
           /* RIG found */
           if (aid != FAIL)   {
              Hinquire(aid, (int32*)NULL, (uint16*)NULL, &ref,
                    (int32*)NULL, (int32*)NULL, (int32*)NULL,
                    (int16*)NULL, (int16*)NULL);
              if (DFR8getrig(file_id, ref, &Readrig) == FAIL) {
                 if (Refset || (HEvalue(1) != DFE_BADCALL)) {
                     Refset = 0;
                     Hendaccess(aid);
                     return FAIL;
                 }
                 Readrig.image.ref = ref;
              } else {
                 foundRig = 1;
                 Refset = 0;
              }
           }
       } while ( (aid != FAIL) && (HEvalue(1) == DFE_BADCALL));
       if (aid != FAIL) Hendaccess(aid);
    }
    if (Refset || !foundRig) { /* No RIGs present, look for RI8 and CI8 */
       /* look for Refset if DFR8ref called, else look for next ref */
       if (Refset) {
           aid = Hstartread(file_id, DFTAG_RI8, Refset);
       } else {
           if (Readrig.image.ref) {
               aid = Hstartread(file_id, DFTAG_RI8, Readrig.image.ref);
               if (aid != FAIL
                   && Hnextread(aid, DFTAG_RI8, DFREF_WILDCARD,
                                DF_CURRENT) == FAIL) {
                   Hendaccess(aid);
                   aid = FAIL;
               }
           } else {
               aid = Hstartread(file_id, DFTAG_RI8, DFREF_WILDCARD);
           }
       }
       if (aid != FAIL) {
           Hinquire(aid, (int32*)NULL, (uint16*)NULL, &riref,
                    (int32*)NULL, (int32*)NULL, (int32*)NULL,
                    (int16*)NULL, (int16*)NULL);
           Hendaccess(aid);
       }

       if (Refset) {
           aid = Hstartread(file_id, DFTAG_CI8, Refset);
       } else {
           if (Readrig.image.ref) {
               aid = Hstartread(file_id, DFTAG_CI8, Readrig.image.ref);
               if (aid != FAIL
                   && Hnextread(aid, DFTAG_CI8, DFREF_WILDCARD,
                                DF_CURRENT) == FAIL) {
                   Hendaccess(aid);
                   aid = FAIL;
               }
           } else {
               aid = Hstartread(file_id, DFTAG_CI8, DFREF_WILDCARD);
           }
       }
       if (aid != FAIL) {
           Hinquire(aid, (int32*)NULL, (uint16*)NULL, &ciref,
                    (int32*)NULL, (int32*)NULL, (int32*)NULL,
                    (int16*)NULL, (int16*)NULL);
           Hendaccess(aid);
       }

        Refset = 0;
        if (!riref && !ciref) {
           HERROR(DFE_NOMATCH);
            return FAIL;
        }
        if ((!ciref) || (riref && (riref<ciref))) {

           /* next image is RI8 */

           Readrig.image.ref = riref;
            Readrig.image.tag = DFTAG_RI8;
        } else {

           /* next image is CI8 */

           Readrig.image.ref = ciref;
            Readrig.image.tag = DFTAG_CI8;
            Readrig.descimage.compr.tag = DFTAG_RLE;
        }

        if (Hgetelement(file_id, DFTAG_ID8, Readrig.image.ref, R8tbuf)
           != FAIL) {
            uint8 *p;
            p = R8tbuf;
            UINT16DECODE(p, Readrig.descimage.xdim);
            UINT16DECODE(p, Readrig.descimage.ydim);
        } else
           return FAIL;

        if ((aid = Hstartread(file_id, DFTAG_IP8, Readrig.image.ref))
           != FAIL) {
            Readrig.lut.tag = DFTAG_IP8;
            Readrig.lut.ref = Readrig.image.ref;

           Hendaccess(aid);
        }
    }
    Lastref = Readrig.image.ref; /* remember ref read */
    return SUCCEED;
}
