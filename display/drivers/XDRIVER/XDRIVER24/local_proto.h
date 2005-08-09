/* Box_abs.c */
int Box_abs(int, int, int, int);
/* Can_do.c */
int can_do_float(void);
/* Clr_table.c */
int Color_table_float(void);
Colormap InitColorTableFixed(Colormap cmap);
int Color_table_fixed(void);
/* Draw_line.c */
int draw_line(int, int, int, int);
/* Draw_point.c */
int draw_point(int, int);
/* line_attr.c */
int line_width(int);
/* Get_w_box.c */
int Get_location_with_box(int, int, int *, int *, int *);
/* Get_w_line.c */
int Get_location_with_line(int, int, int *, int *, int *);
/* Get_w_pnt.c */
int Get_location_with_pointer(int *, int *, int *);
/* Graph_Clse.c */
int Graph_Close(void);
/* Graph_Set.c */
int Graph_Set(int, char **);
/* Panel.c */
int Panel_save(char *, int, int, int, int);
int Panel_restore(char *);
int Panel_delete(char *);
/* Plylne_abs.c */
int Polyline_abs(int *, int *, int);
int Polyline_rel(int *, int *, int);
/* Polygn_abs.c */
int Polygon_abs(int *, int *, int);
int Polygon_rel(int *, int *, int);
/* Serve_Xevent.c */
int Get_Xevent(long event_mask, XEvent *event);
int Service_Xevent(int);
/* alloc.c */
XPoint *AllocXPoints(int);
