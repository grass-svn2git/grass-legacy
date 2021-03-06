
#include "graph.h"

R_pad_get_item (item, list, count)
    char *item;
    char ***list;
    int *count;
{
    char result;

    _hold_signals(1);

    _send_ident (PAD_GET_ITEM);
    _send_text (item);
    _get_char (&result);

    if (result == OK)
	_get_list (list, count);

    _hold_signals(0);

    return result;
}
