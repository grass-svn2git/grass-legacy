C
C  *** / I N C O R E / ***
C
C  CONTROL VARIABLES FOR INCORE BUFFER MANAGEMENT
C
      COMMON /INCORE/ BLOCKS(3,20),NEXT,LIMIT,NUMBL
      INTEGER BLOCKS
C
C  VARIABLE DEFINITIONS:
C     BLOCKS--ARRAY WITH POINTERS AND DIMENSIONS OF INCORE BLOCKS
C         ROW 1---STARTING POSITION
C         ROW 2---NUMBER OF ROWS
C         ROW 3---NUMBER OF COLUMNS
C     NEXT----NEXT AVAILABLE ADDRESS IN THE BUFFER
C     LIMIT---LAST WORD IN THE BUFFER
C     NUMBL---NUMBER OF BLOCKS DEFINED
C
