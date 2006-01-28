#include <stdlib.h>
#include <sys/types.h>


#ifndef USE_RAND

#if defined(__CYGWIN__) || defined(__APPLE__) || defined(__MINGW32__)
#define lrand48() ((long)((double) rand() * (1<<31) / RAND_MAX))
#define srand48(sv) (srand((unsigned)(sv)))
#else
extern long lrand48();
extern void srand48();
#endif 

extern time_t time();

long 
make_rand (void)
{
    return lrand48();
}

void 
init_rand (void)
{
    srand48( (long) time( (time_t *) 0) );
}

#else

static long 
labs (int n)
{
    return n < 0 ? (-n) : n;
}

long 
make_rand (void)
{
    return (labs(rand() + (rand() << 16)));
}

void 
init_rand (void)
{
    srand(getpid());
}

#endif
/* vim: set softtabstop=4 shiftwidth=4 expandtab: */
