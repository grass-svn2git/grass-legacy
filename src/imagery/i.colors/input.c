#include "globals.h"

static int active = 0;
static int replot;

#define TEXT_COLOR BLACK
#define FILL_COLOR GREY
#define OUTLINE_COLOR WHITE

/* Input: drive mouse. returns status of handler that returns != 0 */
Input_pointer (objects)
    Objects *objects;
{
    return mouse (objects,0,0,0);
}

Input_box (objects,ax,ay)
    Objects *objects;
{
    return mouse (objects,ax,ay,1);
}

Input_keyboard (keyboard)
    int (*keyboard)();
{
    int stat;

    Menu_msg("Keyboard input required");
    stat = (*keyboard)();
    if (active)
	use_mouse_msg();

    Menu_msg("");
    return stat;
}

static
mouse (objects,ax,ay,box)
    Objects *objects;
{
    int first;
    int stat;
    int x,y,button;
    Objects *obj;
    Objects *find();


    first = !active;
    active = 1;
    if (first)
	use_mouse_msg();

    if (box)
    {
	x = ax + 20;
	y = ay + 20;
    }
    stat = 0;
    replot = 1;
    while (stat == 0)
    {
	if (replot)
	{
	    replot = 0;
	    draw_objects (objects);
	}
	if (box)
	    Mouse_box_anchored (ax, ay, &x, &y, &button) ;
	else
	    Mouse_pointer (&x, &y, &button) ;

	if (!(obj = find (objects, x, y)))
	    continue;

	switch (obj->type)
	{
	case MENU_OBJECT:
	case OTHER_OBJECT:
		stat = (*obj->handler)(x,y,button);
		break;
	case OPTION_OBJECT:
		select_option (objects, obj);
		draw_option_boxes(objects);
		break;
	}
    }

/* if we are first call, mark not active
 * indicate that objects above use must be replotted.
 */
    if (first)
	active = 0;
    Menu_msg("");

    return stat;
}


static
use_mouse_msg()
{
    /*
    G_clear_screen();
    */
    printf (" Use mouse now\r");
    fflush (stdout);
}

static
draw_objects (objects)
    Objects *objects;
{
    Objects *obj;
    int top, bottom, left, right;
    int size, edge;


/* erase the menu window */
    Erase_view (VIEW_MENU);
    R_flush();

/* determine sizes and text indentation */
    size = VIEW_MENU->nrows - 4;
    edge = 2;

    R_text_size (size, size);

    left  = VIEW_MENU->left;
    top   = VIEW_MENU->top;
    bottom  = VIEW_MENU->bottom;


/* put the (boxed) text on the menu view */
    for (obj = objects; obj->type; obj++)
    {
	if (!visible(obj))
	    continue;
	switch (obj->type)
	{
	case OPTION_OBJECT:
	case MENU_OBJECT:
		right = left + 2*edge + Text_width (obj->label);
		obj->left = left;
		obj->right = right;
		obj->top = top;
		obj->bottom = bottom;

		R_standard_color (FILL_COLOR);
		Solid_box (top, bottom, left, right);

		R_standard_color (TEXT_COLOR);
		Text (obj->label, top, bottom, left, right, edge, 0);

		R_standard_color (OUTLINE_COLOR);
		Outline_box (top, bottom, left, right);

		left = right;
		break;

	case INFO_OBJECT:
		if (*obj->label == 0) break;
		if (*obj->status < 0) break;
		right = left + 2*edge + Text_width (obj->label);
		R_standard_color (WHITE);
		Text (obj->label, top, bottom, left, right, edge, 0);

		left = right;
		break;
	}
    }
    draw_option_boxes (objects);
    R_flush();
}

static
Objects *
find (objects, x, y)
    Objects *objects;
{
    Objects *other;
    other = NULL;
    for (; objects->type; objects++)
    {
	if (!visible (objects))
	    continue;
	switch (objects->type)
	{
	case MENU_OBJECT:
	case OPTION_OBJECT:
	    if (x >= objects->left && x <= objects->right
	    &&  y >= objects->top  && y <= objects->bottom)
		    return objects;
	    break;
	case OTHER_OBJECT:
	    other = objects;
	    break;
	}
    }
    return other;
}

static
select_option (objects, obj)
    Objects *objects, *obj;
{
    while (objects->type)
    {
	if (objects->type == OPTION_OBJECT && *objects->status >= 0 &&
	    objects->binding == obj->binding)
		*objects->status = 0;
	objects++;
    }
    *obj->status = 1;
}

static
draw_option_boxes (objects)
    Objects *objects;
{
    Objects *x;

    R_standard_color (OUTLINE_COLOR);
    for (x = objects; x->type; x++)
    {
	if (x->type == OPTION_OBJECT && *x->status == 0)
	    Outline_box (x->top, x->bottom, x->left, x->right);
    }
    R_standard_color (GREEN);
    for (x = objects; x->type; x++)
    {
	if (x->type == OPTION_OBJECT && *x->status > 0)
	    Outline_box (x->top, x->bottom, x->left, x->right);
    }
}

static
visible(object)
    Objects *object;
{
    if (object->type == OPTION_OBJECT)
	return (*object->status >= 0);
    return (*object->status > 0);
}

Menu_msg(msg)
    char *msg;
{
    int size, edge;

    size = VIEW_MENU->nrows - 4;
    edge = 2;

    Erase_view (VIEW_MENU);

    if (*msg)
    {
	R_text_size (size, size);
	R_standard_color (WHITE);
	Text (msg, VIEW_MENU->top, VIEW_MENU->bottom,
		   VIEW_MENU->left, VIEW_MENU->right, edge, 0);
    }
    R_flush();
    replot = 1;
}

Start_mouse_in_menu()
{
    Set_mouse_xy (
	(VIEW_MENU->left+2*VIEW_MENU->right)/3,
	(VIEW_MENU->top+VIEW_MENU->bottom)/2);
}
