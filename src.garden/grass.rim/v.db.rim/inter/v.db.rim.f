C234567890

        PROGRAM VDBRIM

        INTEGER*4 IARGC, NUMARG, RETVAL, TOPLEVEL
        CHARACTER*256 AR0STR
        CHARACTER*256 AR1STR


        NUMARG = IARGC()
        CALL GETARG(0, AR0STR)

        IF (NUMARG.GT.0) THEN
                CALL GETARG(1, AR1STR)
        ENDIF

        RETVAL = TOPLEVEL(NUMARG,AR0STR,AR1STR)

        STOP

        END

