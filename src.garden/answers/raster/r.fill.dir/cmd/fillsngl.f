      INTEGER*2 DATA(20000,3)
      character*160 TOFILE, file
      CHARACTER*6 SYSTEM
      COMMON DATA
      PRINT *,'ENTER NL,NS,TOPO FILE,SYSTEM(UNIX,VMS,PRIME)'
      read(*,'(a)') file
      OPEN(UNIT=10,STATUS='OLD',file=file)
      READ(10,*)NL,NS
      READ(10,'(a)')TOFILE
      READ(10,'(a)')SYSTEM
      INFILE=11
      IF (SYSTEM.EQ.'VMS') GOTO 700
      IF (SYSTEM.EQ.'PRIME') GOTO 710
C
C     This section is for opening files under UNIX
C
      OPEN(UNIT=INFILE,STATUS='OLD',RECL=NS*2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=TOFILE)
      GOTO 13
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C     This section is for opening files under VMS
C
 700  CONTINUE
      OPEN(UNIT=INFILE,STATUS='OLD',RECL=(NS+1)/2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=TOFILE)
      GOTO 13
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C     This section is for opening files under PRIMOS
C
 710  CONTINUE
      OPEN(UNIT=INFILE,STATUS='OLD',RECL=NS,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=TOFILE)
      GOTO 13
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  THIS ROUTINE READS A DEM AND FILLS ALL ONE-CELL DEPRESSIONS
C  TO SAVE PROCESSING TIME IN A SUBSEQUENT BUILDING OF A 'LEVS'
C  IMAGE.
 13   I1=1
      I2=2
      I3=3
      READ(INFILE,REC=1)(DATA(J,1),J=1,NS)
      READ(INFILE,REC=2)(DATA(J,2),J=1,NS)
      READ(INFILE,REC=3)(DATA(J,3),J=1,NS)
      DO 10 I=2,NL-1
      DO 5 J=2,NS-1
      SMALLEST=MIN(DATA(J-1,I1),DATA(J,I1),DATA(J+1,I1),
     *   DATA(J-1,I2),DATA(J+1,I2),DATA(J-1,I3),DATA(J,I3),
     *   DATA(J+1,I3))
      IF(DATA(J,I2).LT.SMALLEST)DATA(J,I2)=SMALLEST
 5    CONTINUE
      WRITE(INFILE,REC=I)(DATA(J,I2),J=1,NS)
      ITEMP=I1
      I1=I2
      I2=I3
      I3=ITEMP
      IF(I.EQ.NL-1)GOTO 10
      READ(INFILE,REC=I+2)(DATA(J,I3),J=1,NS)
 10   CONTINUE
      STOP
      END
