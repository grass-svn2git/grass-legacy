
#include "graph.h"

R_pad_select (pad)
    char *pad;
{
    char result;

    _hold_signals(1);

    _send_ident (PAD_SELECT);
    _send_text (pad);
    _get_char (&result);

    _hold_signals(0);

    return result;
}
