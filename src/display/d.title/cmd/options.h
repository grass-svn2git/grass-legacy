
#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

GLOBAL char map_name[128] ;
GLOBAL char color[128] ;
GLOBAL float size ;
GLOBAL int type ;

#define NORMAL	1
#define FANCY	2
