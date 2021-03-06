C
C  *** / S R T C O M / ***
C
C  SORT COMMUNICATION COMMON BLOCK
C
      COMMON /SRTCOM/ VARTYP(ZMSRT),VARPOS(ZMSRT),
     X                VARLEN(ZMSRT),SORTYP(ZMSRT),
     X                LTUPLE,LTUMAX,LTUMIN,NSORT,NREAD,
     X                NSOVAR,FIXLT,OFFSET,NKSRT0
C
      PARAMETER (ZSRTCM=9+4*ZMSRT)
      INTEGER SRTCM0(ZSRTCM)
      EQUIVALENCE (SRTCM0(1),VARTYP(1))
C
      LOGICAL SORTYP
      LOGICAL FIXLT
C
C  VARIABLE DEFINITIONS:
C
C  THE SORT ROUTINES OPERATE ON SORT VARIABLES.  A SORT
C  VARIABLE IS A COLLECTION OF ADJACENT WORDS OF THE
C  SAME TYPE BEING SORTED IN THE SAME DIRECTION.  FOR
C  EXAMPLE, AN INTEGER ARRAY OR A MULTIWORD TEXT FIELD.
C  AS A SPECIAL CASE, SINGLE ITEMS ARE TREATED AS ARRAYS
C  OF LENGTH 1.
C
C  SORTYP(N)--ASCENDING OR DESCENDING SORT (TRUE OR FALSE)
C  VARTYP(N)--TYPE OF THE N-TH SORT VARIABLE
C  VARPOS(N)--POSITION IN TUPLE OF N-TH SORT VARIABLE
C             NOTE THAT FOR A DOUBLE PRECISION REAL
C             THIS IS FIRST WORD
C  VARLEN(N)--NUMBER OF WORDS IN THE N-TH SORT VARIABLE
C  OFFSET----LENGTH OF TUPLE PREFIX
C  FIXLT------.TRUE. IF FIXED LENGTH TUPLES
C             .FALSE. IF VARIABLE LENGTH TUPLES
C  LTUPLE-----IF FIXLT .TRUE. THEN LENGTH (WORDS) OF A TUPLE
C             OTHERWISE LENGTH (WORDS) OF SORT FILE
C  LTUMAX-----MAX LENGTH (WORDS) OF VARIABLE LENGTH TUPLE
C             INCLUDES THE FIRST WORD (LENGTH) OF TUPLE
C             HAS NO MEANING IF FIXLT .TRUE.
C  LTUMIN-----SAME AS LTUMAX EXCEPT MIN LENGTH
C  NSORT------NUMBER OF TUPLES ON THE SORT FILE
C  NREAD------NUMBER OF TUPLES CURRENTLY READ FROM THE SORT FILE
C  NSOVAR-----NUMBER OF SORT VARIABLES (LE 10)
C  NKSRT0 ------- TYPE OF SORT
C                 (1) SELECT SORT (WITH CID, NID AND IVAL)
C                 (2) ATTRIBUTE SORT (TALLY)
C                 (3) ATTRIBUTE AND ID SORT (BUILD)
C                 (4) DELETE DUPLICATES (SORT ALL ATTRIBUTES)
C
