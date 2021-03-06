      SUBROUTINE LNKPAG(THEROW)
      INCLUDE 'syspar.d'
C
C     DO PAGING AS NEEDED FOR THE LINK HEADER PAGES
C
C     PARAMETERS:
C         THEROW--INPUT - ROW WANTED
C                 OUTPUT - ACTUAL ROW TO USE IN THE BUFFER
      INCLUDE 'lnktbl.d'
      INCLUDE 'rimcom.d'
      INCLUDE 'f1com.d'
      INTEGER THEROW
C
C  TURN THE REQUESTED ROW INTO A RECORD AND OFFSET.
C
1     NNREC = ((THEROW - 1) / LPBUF) + 1
      NNROW = THEROW - ((NNREC - 1) * LPBUF)
C
C  SEE IF WE ALREADY HAVE THIS RECORD IN THE BUFFER.
C
      IF(NNREC.EQ.CLREC) GO TO 300
C
C  WE MUST DO PAGING.
C
C  SEE IF THE CURRENT RECORD IN THE BUFFER HAS BEEN MODIFIED.
C
      IF(LNKMOD.EQ.0) GO TO 100
C
C  WRITE OUT THE CURRENT RECORD.
C
      CALL RIOOUT(FILE1,CLREC,LNKBUF,LENBF1,IOS)
      IF(IOS.NE.0) RMSTAT = 2100 + IOS
C
C  READ IN THE NEEDED RECORD.
C
100   RELMOD = 0
      CALL RIOIN(FILE1,NNREC,LNKBUF,LENBF1,IOS)
      IF(IOS.EQ.0) GO TO 200
C
C  THERE WAS NO DATA ON THE FILE - WRITE SOME.
C
      CALL ZEROIT(LNKBUF,LENBF1)
      CALL RIOOUT(FILE1,0,LNKBUF,LENBF1,IOS)
      IF(IOS.NE.0) RMSTAT = 2100 + IOS
      LF1REC = LF1REC + 1
  200 CONTINUE
      CLREC = NNREC
C
C  SET THE POINTER TO THE ACTUAL ROW IN THE BUFFER.
C
  300 CONTINUE
      THEROW = NNROW
      RETURN
      END
