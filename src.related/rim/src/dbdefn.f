      SUBROUTINE DBDEFN(*)
      INCLUDE 'syspar.d'
C
C     DEFINE THE DATABASE SCHEMA
C
C  SYNTAX:  DEFINE <DATABASE>
C           <COMMANDS>
C           END
C
C
      INCLUDE 'ascpar.d'
      INCLUDE 'tokens.d'
      INCLUDE 'rimcom.d'
      INCLUDE 'flags.d'
      INCLUDE 'cflags.d'
      INCLUDE 'files.d'
      INCLUDE 'prom.d'
C
      LOGICAL EQKEYW
      LOGICAL EQ, NE
      INTEGER NAMOWN(Z)
C
 
      NUMELE  = 0
      NEWCSN = 0
      CALL RMDATE(TDAY)
C
C     SET THE PROMPT CHARACTER
C
      CALL PRMSET('SET','RIM_DEF:')
C
C     CHECK THE DATA BASE NAME.
C
      IF (ITEMS.GE.2) THEN
C
C     NAME SPECIFIED ON DEFINE COMMAND
C
         CALL SYSDBG(2,DBSTAT)
         IF(DBSTAT.NE.0) GO TO 999
C
C        CHECK THE DATABASE AND OPEN IT
C
         CALL DBOPEN (DBFNAM,.TRUE.)
         IF((RMSTAT.NE.15).AND.(RMSTAT.NE.0)) THEN
            CALL WARN(RMSTAT,0,0)
            GO TO 999
         ENDIF
      ELSE
C
C     USE CURRENT DATABASE
C
         IF (.NOT.DFLAG) THEN
            CALL WARN(2,0,0)
            GOTO 999
         ENDIF
      ENDIF
 
 
      CALL MSG(' ','BEGIN DEFINITIONS FOR ','+')
      IF (DFLAG) THEN
         CALL MSG(' ','EXISTING DATABASE: ','+')
         RFLAG = 1
      ELSE
         CALL MSG(' ','NEW DATABASE: ','+')
         CALL ZMOVE(OWNER,USERID)
      ENDIF
      CALL AMSG(DBNAME,ZC,' ')
      NEWCSN = 1
 
C
C  PROCESS DB DEFINITION CAOMMANDS
C
300   CALL LODREC
350   IF(EQKEYW(1,'COLUMNS')) GO TO 400
      IF(EQKEYW(1,'ATTRIBUTES')) GO TO 400
      IF(EQKEYW(1,'TABLES')) GO TO 500
      IF(EQKEYW(1,'RELATIONS')) GO TO 500
      IF(EQKEYW(1,'LINKS')) GO TO 700
      IF(EQKEYW(1,'PASSWORDS')) GO TO 750
      IF(EQKEYW(1,'OWNER')) GO TO 800
      IF(EQKEYW(1,'END')) GO TO 900
C
C  ERROR.
C
      CALL WARN(4,0,0)
      GO TO 300
C
C  PROCESS ATTRIBUTES.
C
400   CALL LODELE(NUMELE)
      GO TO 350
C
C
C  PROCESS RELATIONS.
C
500   CONTINUE
      CALL LODREL(NUMELE)
      GO TO 350
C
C
C  PROCESS LINKS.
C
700   CONTINUE
      CALL LODLNK
      GO TO 350
C
C  PROCESS PASSWORDS.
C
750   CONTINUE
      CALL LODPAS
      GO TO 350
C
C     PROCESS OWNER
C
800   IF (ITEMS.GE.2) THEN
         CALL LXSREC(2,NAMOWN,ZC)
      ELSE
         CALL ZMOVE(NAMOWN,NONE)
      ENDIF
      IF (NE(OWNER,NONE) .AND. NE(OWNER,NAMOWN)) THEN
         CALL MSG('E','YOU ARE NOT THE DATABASE OWNER.',' ')
         ERROR = ERROR + 1
         GO TO 300
      ENDIF
      CALL ZMOVE(OWNER,NAMOWN)
      CALL ZMOVE(USERID,OWNER)
      GOTO 300
C
C  PROCESS END.
C
  900 CONTINUE
C
C  SET THE RETURN CODE AND MAKE SURE A SCHEMA HAS BEEN DEFINED
C
      IF(NEWCSN.EQ.0) GO TO 999
      CALL MSG(' ','DATABASE DEFINITIONS COMPLETED.',' ')
C
C  BUFFER THE SCHEMA AND DATABASE OUT
C
      DFLAG = .TRUE.
      IFMOD = .TRUE.
      CALL DBOPEN (DBFNAM,.FALSE.)
      IF(RMSTAT.NE.0) CALL WARN(RMSTAT,0,0)
C
C
C RESET THE PROMPT CHARACTER TO R
C
999   CALL PRMSET('RESET',' ')
      CALL BLKCLR(10)
      RETURN 1
      END
