#include <grass/gis.h>
#ifdef HAVE_CURSES_H
#include <grass/vask.h>
#include <grass/imagery.h>
#include <stdlib.h>

int I_v_exec()
{
    V_intrpt_ok();
    if (!V_call()) exit(0);
    return 0;
}
#endif
