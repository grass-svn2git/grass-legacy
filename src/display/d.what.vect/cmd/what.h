/* openvect.c */
char *openvect(char *);
/* what.c */
int what(int, int, int, int, int, int, char *, char *);
int show_buttons(int, int);
char * colr_str(int);
/*flash.c*/
void flash_area(struct Map_info *, plus_t, struct line_pnts *, int);
void flash_line(struct Map_info *, plus_t, struct line_pnts *, int);
/* attr.c */
int disp_attr(char *, char *, int);

#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL	extern
#endif

GLOBAL	char **vect;
GLOBAL	int nvects;
GLOBAL	struct Map_info *Map;
GLOBAL	struct Categories *Cats;

