/* %W% %G% */
static unsigned char cyan[]={
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static unsigned char yellow[]={
4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,
4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,
4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,
4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,
4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0,4,3,2,1,0
};
static unsigned char magenta[]={
4,4,4,4,4,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,
4,4,4,4,4,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,
4,4,4,4,4,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,
4,4,4,4,4,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,
4,4,4,4,4,3,3,3,3,3,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0
};
cym (n, c, y, m)
    unsigned char n;
    int *c, *y, *m;
{
    if (n > 124) n = 0;

    *c = cyan[n];
    *y = yellow[n];
    *m = magenta[n];
}
