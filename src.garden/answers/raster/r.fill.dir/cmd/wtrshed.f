      IMPLICIT INTEGER*2 (A-Z)
      CHARACTER*160 DIFILE
      CHARACTER*160 MAFILE, tfile
      CHARACTER*6 SYSTEM
      LOGICAL DONE,ACTIVITY
      DIMENSION DIR(20000,3),MASK(20000,3)
      COMMON DIR,MASK
      PRINT *,'ENTER NL,NS,DIR FILE,MASK FILE,SYSTEM(UNIX,VMS,PRIME)'
      read(*,'(a)') tfile
      OPEN(UNIT=10,STATUS='OLD',file=tfile)
      READ(10,*)NL,NS
      READ(10,'(a)')DIFILE
      READ(10,'(a)')MAFILE
      READ(10,'(a)')SYSTEM
C-- DFILE IS THE SDIR FILE THAT IS MADE BY DOSDIR, I*2
      DFILE=13
C-- MFILE IS THE STARTER FILE,0 FOR MASK,-1 FOR NO LABEL YET,
C    AND A NUMBER WHERE THE USER WISHES A WATERSHED TO TERMINATE
      MFILE=12
      IF (SYSTEM.EQ.'VMS') GOTO 700
      IF (SYSTEM.EQ.'PRIME') GOTO 710
C
C     This section is for opening files under UNIX
C
      OPEN(UNIT=DFILE,STATUS='OLD',RECL=NS*2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=DIFILE)
      OPEN(UNIT=MFILE,STATUS='OLD',FORM='UNFORMATTED',
     *   ACCESS='DIRECT',file=MAFILE,RECL=NS*2)
      GOTO 13
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C     This section is for opening files under VMS
C
 700  CONTINUE
      OPEN(UNIT=DFILE,STATUS='OLD',RECL=(NS+1)/2,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=DIFILE)
      OPEN(UNIT=MFILE,STATUS='OLD',FORM='UNFORMATTED',
     *   ACCESS='DIRECT',file=MAFILE,RECL=(NS+1)/2)
      GOTO 13
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C     This section is for opening files under PRIMOS
C
 710  CONTINUE
      OPEN(UNIT=DFILE,STATUS='OLD',RECL=NS,
     *   FORM='UNFORMATTED',ACCESS='DIRECT',
     *   file=DIFILE)
      OPEN(UNIT=MFILE,STATUS='OLD',FORM='UNFORMATTED',
     *   ACCESS='DIRECT',file=MAFILE,RECL=NS)
      GOTO 13
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 13   UPDOWN=-1
      PASS=0
 33   I1=1
      I2=2
      I3=3
      UPDOWN=-UPDOWN
      DONE=.TRUE.
      PASS=PASS+1
      PRINT *,'PASS',PASS
      DO 3 II=1,NS+2
      INDEX=1
      IF(UPDOWN.EQ.-1)INDEX=3
 3    MASK(II,INDEX)=0
      DO 5 I=1,3
      MASK(1,I)=0
 5    MASK(NS+2,I)=0
      INDEX2=0
      IF(UPDOWN.EQ.-1)INDEX2=NL-2
      INDEX1=1
      IF(UPDOWN.EQ.-1)INDEX1=0
      DO 6 I=1,2
      READ(DFILE,REC=I+INDEX2)(DIR(II,I+INDEX1),II=2,NS+1)
 6    READ(MFILE,REC=I+INDEX2)(MASK(II,I+INDEX1),II=2,NS+1)
      DO 10 I=1,NL
 15   ACTIVITY=.FALSE.
      DO 20 J=2,NS+1
      IF(MASK(J,I2).NE.-1)GOTO 20
      dirx=dir(J,I2)
      IF(DIRX.NE.0) GOTO 16
      MASK(J,I2) = 0
      ACTIVITY = .TRUE.
      GOTO 20
 16   IF(MASK(J+1,I1).LT.0.OR.DIRX.NE.1)GOTO 21
      MASK(J,I2)=MASK(J+1,I1)
      ACTIVITY=.TRUE.
      GOTO 20
 21   IF(MASK(J+1,I2).LT.0.OR.DIRX.NE.2)GOTO 22
      MASK(J,I2)=MASK(J+1,I2)
      ACTIVITY=.TRUE.
      GOTO 20
 22   IF(MASK(J+1,I3).LT.0.OR.DIRX.NE.4)GOTO 23
      MASK(J,I2)=MASK(J+1,I3)
      ACTIVITY=.TRUE.
      GOTO 20
 23   IF(MASK(J,I3).LT.0.OR.DIRX.NE.8)GOTO 24
      MASK(J,I2)=MASK(J,I3)
      ACTIVITY=.TRUE.
      GOTO 20
 24   IF(MASK(J-1,I3).LT.0.OR.DIRX.NE.16)GOTO 25
      MASK(J,I2)=MASK(J-1,I3)
      ACTIVITY=.TRUE.
      GOTO 20
 25   IF(MASK(J-1,I2).LT.0.OR.DIRX.NE.32)GOTO 26
      MASK(J,I2)=MASK(J-1,I2)
      ACTIVITY=.TRUE.
      GOTO 20
 26   IF(MASK(J-1,I1).LT.0.OR.DIRX.NE.64)GOTO 27
      MASK(J,I2)=MASK(J-1,I1)
      ACTIVITY=.TRUE.
      GOTO 20
 27   IF(MASK(J,I1).LT.0.OR.DIRX.NE.128)GOTO 20
      MASK(J,I2)=MASK(J,I1)
      ACTIVITY=.TRUE.
      GOTO 20
 20   IF(ACTIVITY)DONE=.FALSE.
      IF(ACTIVITY)GOTO 15
      INDEX=I
      IF(UPDOWN.EQ.-1)INDEX=NL-I+1
      WRITE(MFILE,REC=INDEX)(MASK(II,I2),II=2,NS+1)
      IF(I.EQ.NL)GOTO 10
      IF(UPDOWN.EQ.-1)GOTO 18
      ITEMP=I1
      I1=I2
      I2=I3
      I3=ITEMP
      IF(I.NE.NL-1)GOTO 65
      DO 64 II=1,NS+2
 64   MASK(II,I3)=0
      GOTO 10
 65   READ(MFILE,REC=I+2)(MASK(II,I3),II=2,NS+1)
      READ(DFILE,REC=I+2)(DIR(II,I3),II=2,NS+1)
      GOTO 10
C--DOING A BOTTOM TO TOP PASS
 18   ITEMP=I3
      I3=I2
      I2=I1
      I1=ITEMP
      IF(I.NE.NL-1)GOTO 19
      DO 17 II=1,NS+2
 17   MASK(II,I1)=O
      GOTO 10
 19   READ(MFILE,REC= NL-I-1)(MASK(II,I1),II=2,NS+1)
      READ(DFILE,REC=NL-I-1)(DIR(II,I1),II=2,NS+1)
 10   CONTINUE
      IF(.NOT.DONE)GOTO 33
      STOP
      END
