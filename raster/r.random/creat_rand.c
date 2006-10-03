#include <stdlib.h>
#include <sys/types.h>
#include <grass/config.h>

#ifndef USE_RAND

#ifndef HAVE_DRAND48
#define lrand48() (((long) rand() ^ ((long) rand() << 16)) & 0x7FFFFFFF)
#define srand48(sv) (srand((unsigned)(sv)))
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
