#ifdef NO_PROTO
# define P(s) ()
#else
# define        P(s) s
#endif

/* The same for old and new, format independent */
Site *G_site_new_struct P((RASTER_MAP_TYPE cattype, int ndim, int ns, int nd));
void G_site_free_struct P((Site *s));
int G_site_in_region P((Site *s, struct Cell_head *region));
int G_site_d_cmp P((const void *a, const void *b));
int G_site_c_cmp P((const void *a, const void *b));
int G_site_s_cmp P((const void *a, const void *b));
char *G_site_format P((Site *s, char *fs, int id));
SITE_ATT * G_sites_get_atts P((FILE * ptr, int* cat));
int G_sites_get_fields P((FILE * ptr, char*** cnames, int** ctypes, int** ndx));
void G_sites_free_fields P((int ncols, char** cnames, int* ctypes, int* ndx));

/* Old version used by v.in.sites */
FILE * G_oldsites_open_old P((char *name, char *mapset));
int G_oldsite_describe P((FILE *p, int *dims, int *cat, int *strs, int *dbls));
int G_oldsite_get P((FILE *p, Site *s));
int G__oldsite_get ( FILE *, Site *, int);

/* New version based on vectors used in old, not updated sites modules */
int G_site_get P((FILE *p, Site *s));
int G_site_put P((FILE *p, Site *s));
int G_site_describe P((FILE *p, int *dims, int *cat, int *strs, int *dbls));
int G_site_get_head P((FILE *p, Site_head *head));
int G_site_put_head P((FILE *p, Site_head *head));
FILE * G_sites_open_old P((char *name, char *mapset));
FILE * G_sites_open_new P((char *name));
void G_sites_close ( FILE * );
char * G_find_sites P((char *name, char *mapset));
char * G_find_sites2 P((char *name, char *mapset));
char * G_ask_sites_new P((char *prompt, char *name));
int G__site_put ( FILE *, Site *, int);
#undef P

