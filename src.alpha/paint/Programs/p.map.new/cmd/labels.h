#define MAXLABELS 10

struct labels
{
    int count;		/* number of labels files */
    char *name[MAXLABELS];
    char *mapset[MAXLABELS];
    char *other; /* text for various objects */
    char *texts; /* text instructions */
} ;

#ifdef MAIN
    struct labels labels;
#else
    extern struct labels labels;
#endif
