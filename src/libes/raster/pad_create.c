
#include "graph.h"

R_pad_create (pad)
    char *pad;
{
    char result;

    _hold_signals(1);

    _send_ident (PAD_CREATE);
    _send_text (pad);
    _get_char (&result);

    _hold_signals(0);

    return result;
}
