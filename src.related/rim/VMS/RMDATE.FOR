      SUBROUTINE RMDATE(DAT)
      INCLUDE 'TEXT.DCK'
      INCLUDE 'SYSPAR.DCK'
C
C     *** VAX SYSTEM DEPENDENT ROUTINE ***
C
C  PURPOSE:   RETURN THE CURRENT DATE AS INTEGER
C
C  PARAMETERS:
C         DAT-----THE CURRENT DATE
C
      CALL IDATE(MM,DD,YY)
      CALL JULDAT(DD,MM,YY+1900,DAT)
      RETURN
      END
