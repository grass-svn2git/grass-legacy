C
C  *** / W H C O M / ***
C
C  WHERE CLAUSE COMMON BLOCK
C
      PARAMETER (WHAND=1,WHOR=2,WHNOT=3)
C
      COMMON /WHCOM/ NBOO,BOO(ZMWHR),KATTP(ZMWHR),
     1    KATTL(ZMWHR),KATTY(ZMWHR),
     2    KOMTYP(ZMWHR),KOMPOS(ZMWHR),KOMLEN(ZMWHR),
     3    KOMPOT(ZMWHR),KSTRT,MAXTU
     2   ,LIMTU,WHRVAL(ZMWHVL),WHRLEN(ZMWHLL),KBOOP
C
      PARAMETER (ZWHCOM=5+8*ZMWHR+ZMWHVL+ZMWHLL)
      INTEGER WHCOM0(ZWHCOM)
      EQUIVALENCE (WHCOM0(1),NBOO)
C
C  VARIABLE DEFINITIONS:
C              NBOO----NUMBER OF SIMPLE BOOLEAN CONDITIONS
C              BOO-----MOD 10 = BOOLEAN OPERATOR TO NEXT VALUE         )
C                      DIV 10 = LOGICAL NESTING LEVEL (1 = TOP)
C              KATTP---ATTRIBUTE COLUMN NUMBER
C              KATTL---ARRAY OF ATTRIBUTE LENGTHS IN WORDS EXCEPT TEXT
C                      IS IN CHARACTERS (0 IF "TUPLE" WHERE CLAUSE)
C              KATTY---ARRAY OF ATTRIBUTE TYPES
C                      (0 IF "TUPLE" WHERE CLAUSE)
C              KOMTYP--ARRAY OF BOOLEAN COMPARISON INTEGER IDENTIFIERS
C              KOMPOS--ARRAY OF POSITION POINTERS IN WHRVAL
C                      FOR THE START OF THE VALUE LIST
C                        OR SECOND ATTRIBUTE COLUMN NUMBER
C              KOMLEN--ARRAY INDICATING THE NUMBER OF ITEMS IN THE
C                      VALUE LIST
C              KOMPOT--ARRAY OF POINTERS TO WHRLEN
C              KSTRT---RECORD NUMBER OF THE STARTING NODE IN THE
C                      B-TREE IF KEY PROCESSING IS USED
C              MAXTU---MAXIMUM TUPLE NUMBER REQUESTED OR 0
C              LIMTU---MAXIMUM NUMBER OF TUPLES TO ACTUALLY PROCESS
C              WHRVAL--ARRAY OF VALUES POINTED TO BY KOMPOS
C              WHRLEN--ARRAY OF "ATTLEN" STYLE INFORMATION
C              WHRTUP--RELATION TUPLE POINTERS (IN BUFFER)
C              KBOOP---KEY ATTRIBUTE POINTER
C              ZWHCOM--LENGTH OF WHCOM IN WORDS
