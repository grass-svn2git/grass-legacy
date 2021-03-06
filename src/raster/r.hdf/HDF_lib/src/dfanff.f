c***************************************************************************
c
c
c                         NCSA HDF version 3.2r3
c                            December 1, 1992
c
c NCSA HDF Version 3.2 source code and documentation are in the public
c domain.  Specifically, we give to the public domain all rights for future
c licensing of the source code, all resale rights, and all publishing rights.
c
c We ask, but do not require, that the following message be included in all
c derived works:
c
c Portions developed at the National Center for Supercomputing Applications at
c the University of Illinois at Urbana-Champaign, in collaboration with the
c Information Technology Institute of Singapore.
c
c THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
c SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
c WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
c
c***************************************************************************
C
C $Header: /hdf/hdf/v3.2r2/src/RCS/dfanff.f,v 1.1 1992/08/25 21:40:44 koziol beta $
C
C $Log: dfanff.f,v $
c Revision 1.1  1992/08/25  21:40:44  koziol
c Initial revision
c
C
C------------------------------------------------------------------------------
C File:     dfanFf.f
C Purpose:    Fortran stub routines for annotations
C Invokes:    dfanF.c
C Contents: 
C    dagllen      : get length of label of tag/ref
C    daglab       : get label of tag/ref
C    dagdlen      : get length of description of tag/ref
C    dagdesc      : get description of tag/ref
C    daplab       : put label of tag/ref
C    dapdesc      : put description of tag/ref
C    dallist      : get list of labels for a particular tag
C    daafid       : add file ID to file
C
C    dfangetlablen: get length of label of tag/ref
C    dfangetlabel : get label of tag/ref
C    dfangetdesclen: get length of description of tag/ref
C    dfangetdesc  : get description of tag/ref
C    dfanputlabel : put label of tag/ref
C    dfanputdesc  : put description of tag/ref
C    dfanlablist  : get list of labels for a particular tag
C    dfanaddfid   : add file ID to file 

C------------------------------------------------------------------------------


C------------------------------------------------------------------------------
C Name:    dagllen
C Purpose: get length of label of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want label
C Returns: length of label on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daiganl
C------------------------------------------------------------------------------

      integer function dagllen(filename, tag, ref)
      character *(*) filename
      integer tag, ref, daiganl

C 0 is DFAN_LABEL
      dagllen = daiganl(filename, tag, ref, 0, len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    daglab
C Purpose: get label of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want label
C          label: space to return label in
C          maxlen: size of space to return label in
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daigann
C------------------------------------------------------------------------------

      integer function daglab(filename, tag, ref, label, maxlen)
      character *(*) filename, label
      integer tag, ref, maxlen, daigann

C 0 is DFAN_LABEL
      daglab = daigann(filename, tag, ref, label, maxlen, 0,
     +                                                  len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dagdlen
C Purpose: get length of description of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want description
C Returns: length of description on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daiganl
C------------------------------------------------------------------------------

      integer function dagdlen(filename, tag, ref)
      character *(*) filename
      integer tag, ref, daiganl

C 1 is DFAN_DESC
      dagdlen = daiganl(filename, tag, ref, 1, len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dagdesc
C Purpose: get description of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want description
C          desc: space to return description in
C          maxlen: size of space to return description in
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daigann
C------------------------------------------------------------------------------

      integer function dagdesc(filename, tag, ref, desc, maxlen)
      character *(*) filename, desc
      integer tag, ref, daigann, maxlen

C 1 is DFAN_DESC
      dagdesc = daigann(filename, tag, ref, desc, maxlen, 1,
     +                                                  len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    daplab
C Purpose: put label of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which this is the label
C          label: label to write to file
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daipann
C------------------------------------------------------------------------------

      integer function daplab(filename, tag, ref, label)
      character *(*) filename, label
      integer tag, ref, daipann

C 0 is DFAN_LABEL
      daplab = daipann(filename, tag, ref, label, len(label), 0,
     +                                                   len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dapdesc
C Purpose: put description of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which this is the description
C          desc: description to write to file
C          desclen: length of description
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daipann
C------------------------------------------------------------------------------

      integer function dapdesc(filename, tag, ref, desc, desclen)
      character *(*) filename, desc
      integer tag, ref, desclen, daipann

C 1 is DFAN_DESC
      dapdesc = daipann(filename, tag, ref, desc, desclen, 1,
     +                                               len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dallist
C Purpose: Return list of refs and labels for a given tag
C Inputs:  filename: name of HDF file
C          tag: tag to get list of refs and labels for
C          reflist: array to place refs in
C          labellist: array of strings to place labels in
C          listsize: size of ref and label lists
C          maxlen: maximum length allowed for label
C          startpos: beginning from the startpos'th entry, upto listsize
C                entries will be returned.
C Returns: number of entries on success, -1 on error with DFerror set
C Users:   HDF users, utilities, other routines
C Invokes: dailist
C Method:  call dailist
C Remarks: none
C------------------------------------------------------------------------------

      integer function dallist(filename, tag, reflist, labellist,
     +                                      listsize, maxlen, startpos)
      character *(*) filename, labellist
      integer tag, reflist(*), listsize, maxlen, startpos, dailist

      dallist = dailist(filename, tag, reflist, labellist,
     +                     listsize, maxlen, startpos, len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    daafid
C Purpose: add file ID to file
C Inputs:  dfile: pointer to HDF file
C          id: id to write to file
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daipann
C------------------------------------------------------------------------------

      integer function daafid(file, id)
      integer file, daiafid
      character*(*) id

      daafid = daiafid(file, id, len(id) )
      return
      end


CEND7MAX


C------------------------------------------------------------------------------
C Name:    dfangetlablen
C Purpose: get length of label of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want label
C Returns: length of label on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daignal
C------------------------------------------------------------------------------

      integer function dfangetlablen(filename, tag, ref)
      character *(*) filename
      integer tag, ref, daiganl

C 0 is DFAN_LABEL
      dfangetlablen = daiganl(filename, tag, ref, 0, len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfangetlabel
C Purpose: get label of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want label
C          label: space to return label in
C          maxlen: size of space to return label in
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daigann
C------------------------------------------------------------------------------

      integer function dfangetlabel(filename, tag, ref, label, maxlen)
      character *(*) filename, label
      integer tag, ref, maxlen, daigann

C 0 is DFAN_LABEL
      dfangetlabel = daigann(filename, tag, ref, label, maxlen, 0,
     +                                                  len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfangetdesclen
C Purpose: get length of description of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want description
C Returns: length of description on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daiganl
C------------------------------------------------------------------------------

      integer function dfangetdesclen(filename, tag, ref)
      character *(*) filename
      integer tag, ref, daiganl

C 1 is DFAN_DESC
      dfangetdesclen = daiganl(filename, tag, ref, 1, len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfangetdesc
C Purpose: get description of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which we want description
C          desc: space to return description in
C          maxlen: size of space to return description in
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daigann
C------------------------------------------------------------------------------

      integer function dfangetdesc(filename, tag, ref, desc, maxlen)
      character *(*) filename, desc
      integer tag, ref, daigann, maxlen

C 1 is DFAN_DESC
      dfangetdesc = daigann(filename, tag, ref, desc, maxlen, 1,
     +                                                  len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfanputlabel
C Purpose: put label of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which this is the label
C          label: label to write to file
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daipann
C------------------------------------------------------------------------------

      integer function dfanputlabel(filename, tag, ref, label)
      character *(*) filename, label
      integer tag, ref, daipann

C 0 is DFAN_LABEL
      dfanputlabel = daipann(filename, tag, ref, label, len(label), 0,
     +                                                   len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfanputdesc
C Purpose: put description of tag/ref
C Inputs:  filename: name of HDF file
C          tag, ref: tag/ref of item of which this is the description
C          desc: description to write to file
C          desclen: length of description
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daipann
C------------------------------------------------------------------------------

      integer function dfanputdesc(filename, tag, ref, desc, desclen)
      character *(*) filename, desc
      integer tag, ref, desclen, daipann

C 1 is DFAN_DESC
      dfanputdesc = daipann(filename, tag, ref, desc, desclen, 1,
     +                                               len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfanlablist
C Purpose: Return list of refs and labels for a given tag
C Inputs:  filename: name of HDF file
C          tag: tag to get list of refs and labels for
C          reflist: array to place refs in
C          labellist: array of strings to place labels in
C          listsize: size of ref and label lists
C          maxlen: maximum length allowed for label
C          startpos: beginning from the startpos'th entry, upto listsize
C                entries will be returned.
C Returns: number of entries on success, -1 on error with DFerror set
C Users:   HDF users, utilities, other routines
C Invokes: dailist
C Method:  call dailist
C Remarks: none
C------------------------------------------------------------------------------

      integer function dfanlablist(filename, tag, reflist, labellist,
     +                                      listsize, maxlen, startpos)
      character *(*) filename, labellist
      integer tag, reflist(*), listsize, maxlen, startpos, dailist

      dfanlablist = dailist(filename, tag, reflist, labellist,
     +                     listsize, maxlen, startpos, len(filename))
      return
      end


C------------------------------------------------------------------------------
C Name:    dfanaddfid
C Purpose: add file ID to file
C Inputs:  dfile: pointer to HDF file
C          id: id to write to file
C Returns: 0 on success, -1 on failure with DFerror set
C Users:   HDF HLL users, utilities, other routines
C Invokes: daipann
C------------------------------------------------------------------------------

      integer function dfanaddfid(file, id)
      integer file, daiafid
      character*(*) id

      DFANaddfid = daiafid(file, id, len(id) )
      return
      end


