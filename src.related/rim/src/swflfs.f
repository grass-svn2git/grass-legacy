      SUBROUTINE SWFLFS(INFIL,OUTFIL,BUFFER,LBUF,LPRU,DPRU)
      INCLUDE 'syspar.d'
C
C  PURPOSE  DRIVER FOR OUT-OF-CORE SORT
C           OF FIXED LENGTH TUPLES
C
C  METHOD   A LEAST COST SORT STRATEGY
C           IS ESTABLISHED BASED UPON
C           MACHINE DEPENDENT PARAMETERS
C           THE COST IS BASED UPON
C           COST FOR POSITIONING ON
C           MASS STORAGE,MASS STORAGE
C           TRANSFERS,IN-CORE MOVEMENT
C           OF DATA AND COMPARISON OF
C           DATA.
C           AN N-ARY SORT/MERGE STRATEGY
C           IS CHOOSEN WHERE 2 LE N LE 9
C           N IS THE NUMBER OF CHAINS
C           OF DATA THAT IS MERGED IN
C           ONE SINGLE MERGE. EACH SORT PASS
C           MAY REQUIRE SEVERAL SUCH MERGES.
C
C
C
C
C  DEFINITION OF VARIABLES
C
C  INFIL   FILE UNIT CONTAING IINPUT TUPLES
C         INFIL IS UNFORMATTED (BINARY)
C         EACH TUPLE IS WRITTEN AS A
C         RECORD AS FOLLOWS
C         FOR FIXED LENGTH RECORDS
C           WRITE(INFIL) (TUP(I),I=1,LTUPLE)
C         FOR VARIABLE LENGTH RECORDS
C           WRITE(INFIL) L,(TUP(I),I=1,L)
C
C  OUTFIL  FILE UNIT TO CONTAIN OUTPUT TUPLES
C          OUTFIL MAY EQ INFIL
C          FORMAT OF OUTFIL IS THE
C          SAME AS THAT OF INFIL
C
C  BUFFER  INCORE SCRATCH AREA              (ANY,SCRATCH)
C
C  LBUF    LENGTH OF BUFFER                 (INT,I)
C
C  LPRU    QUANTUM LENGTH OF RANDOM         (INT,I)
C          FILE RECORDS
C
C  DPRU    DELTA QUANTUM LENGTH OF          (INT,I)
C          RANDOM FILE RECORDS.
C          THE LENGTH OF SUCH A RECORD
C          MUST EQUAL
C          I*LPRU+DPRU
C
C  DEFINITION OF LOCAL VARIABLES
C
C  I1     SCRATCH
C  I2     SCRATCH,NO OF PAGES IN INITIAL
C         OFLOADING
C  I3     SCRATCH,NO OF SORT PASSES,NOT COUNTING
C         ACTIONS ON SEQUENTIAL FILES
C         OF WHOLE RANDOM FILES
C  I4     SCRATCH
C  I5     SCRATCH
C  I6     LOW COST SORT ORDER
C  I7     NO OF INCORE PAGES IN INITIAL
C         PASS WHERE SEQUENTIAL FILE IS
C         OFFLOADED
C  I8     SCRATCH,NO OF TUPLES PER RAN FILE PAGE
C  I9     SCRATCH,NO OF PAGES ON RANDOM FILES
C  I10    SCRATCH,LENGTH OF RANDOM FILE PAGE
C  COST   COST OF OPTIMUM SORT STRATEGY
C  NTUREC NO OF TUPLES PER RANDOM FILE PAGE
C  NRECS  NO OF PAGES ON RANDOM SCRATCH FILE
C  LREC   LENGTH OF RANDOM FILE PAGE
C  NPASS  NO OF SORT PASSES,NOT COUNTING
C         ACTIONS ON SEQUENTIAL FILES
C         ONE PASS CONTAINS ONE COMPLETE
C         WRITE AND ONE COMPLETE READ
C         OF WHOLE RANDOM FILES
C
      INCLUDE 'srtcom.d'
      INCLUDE 'rimcom.d'
      DIMENSION BUFFER(1)
      INTEGER CHAIN1,OUTREC
      LOGICAL SWITCH
      REAL COST, A1
C
      LTUP = LTUPLE
      I6 = 0
      I1 = 2*LPRU
      I11 = 2*DPRU
      DO 100 I=2,9
      I1 = I1 + LPRU
      I11 = I11 + DPRU
      I10 = LPRU*((LBUF-I11)/I1) + DPRU
      IF(I10 .LT. LTUP) GO TO 110
      I8 = (I10-2)/LTUP
      I2 = (LBUF-I10)/(I10+I8)
C
C  I2 IS NO OF INCORE BLOCKS IN
C     INITIAL PASS
C
      I9 =(NSORT+I8-1)/I8
      I3 = 1
      I4 = I2
   10 CONTINUE
      I5 = I4
      I4 = I4*I + I5
      IF (I4 .GE. I9) GO TO 20
      I4 = I4 - I5
      I3 = I3 + 1
      GO TO 10
   20 CONTINUE
C
      CALL SWCOST(I3,I9,I10,I,A1)
      IF(I6 .GT. 0) GO TO 30
      GO TO 35
   30 CONTINUE
      IF(A1 .GE. COST) GO TO 90
   35 COST = A1
      I7 = I2
      I6 = I
      NTUREC = I8
      NRECS = I9
      NPASS = I3
      LREC = I10
   90 CONTINUE
      IF(I3 .EQ. 1) GO TO 110
  100 CONTINUE
  110 CONTINUE
      IF (LREC.LE.0) THEN
         RMSTAT = 3000
         GOTO 999
      ENDIF
C
C  OPTIMUM SORT STRATEGY DETERMINED
C
C  OPEN SORT SCRATCH FILES
C
      CALL SYSDEL(ZCSRT1)
      CALL SYSDEL(ZCSRT2)
      CALL RIOOPN(ZCSRT1,ZNSRT1,LREC,IOS)
      IF (IOS.NE.0) GOTO 998
      CALL RIOOPN(ZCSRT2,ZNSRT2,LREC,IOS)
      IF (IOS.NE.0) GOTO 998
      CALL SWFILO(BUFFER,LTUP,LREC,NTUREC,I7*NTUREC,
     X            INFIL,ZNSRT1)
C
C     NPASS IS THE NUMBER OF RANDOM TO RANDOM MERGES
C     NI IS THE NUMBER OF CHAINS ON THE INPUT FILE
C     NO IS THE NUMBER OF CHAINS ON THE OUTPUT FILE
C     NCHAIN IS THE NUMBER OF CHAINS TO MERGE
C     LCHAIN IS THE NUMBER OF PAGES PER INPUT CHAIN
C
      LCHAIN = I7
      NCHAIN = I6
      NI = (NRECS-1)/LCHAIN
      NI = NI + 1
      NO = NI
      SWITCH = .TRUE.
C
C     OUTER LOOP ON THE NUMBER OF PASSES
C
      NPASS = NPASS - 1
      IF(NPASS.EQ.0) GO TO 250
      DO 200 I=1,NPASS
      NI = NO
      NO = (NI-1)/NCHAIN
      NO = NO + 1
      SWITCH = .NOT. SWITCH
      INC = LCHAIN*NCHAIN
C
C     INNER LOOP ON NUMBER OF OUTPUT CHAINS
C
      DO 150 J=1,NO
      CHAIN1 = (J-1)*INC + 1
      OUTREC = CHAIN1
      IF(I.EQ.1) OUTREC = 0
      NCH = NCHAIN
      IF(J.EQ.NO) NCH = NI - (NO-1)*NCHAIN
      IF(SWITCH) CALL SWSMFL(BUFFER,CHAIN1,NCH,LCHAIN,OUTREC,J,NTUREC,
     X       LTUP,LREC,ZNSRT2,ZNSRT1)
      IF(.NOT.SWITCH) CALL SWSMFL(BUFFER,CHAIN1,NCH,LCHAIN,OUTREC,J,
     X       NTUREC,LTUP,LREC,ZNSRT1,ZNSRT2)
  150 CONTINUE
      LCHAIN = LCHAIN * NCHAIN
  200 CONTINUE
  250 CONTINUE
C
C     CALL SWUNLO TO CREATE OUTPUT SEQUENTIAL FILE
C
      CHAIN1 = 1
      OUTREC = 1
      NCH = NO
      IF(SWITCH) CALL SWUNLO(BUFFER,CHAIN1,NCH,LCHAIN,
     X      LTUP,LREC,ZNSRT1,OUTFIL)
      IF(.NOT.SWITCH) CALL SWUNLO(BUFFER,CHAIN1,NCH,LCHAIN,
     X      LTUP,LREC,ZNSRT2,OUTFIL)
C
C     RETURN THE SCRATCH RANDOM FILES
C
      CALL RIOCLO(ZNSRT1)
      CALL RIOCLO(ZNSRT2)
C
      CALL SYSDEL(ZCSRT1)
      CALL SYSDEL(ZCSRT2)
      GOTO 999
 
998   RMSTAT = 3000 + IOS
 
999   RETURN
      END
