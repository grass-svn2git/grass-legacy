/* %W% %G% */
main()
{
    double x;
    double tan();
    int i,j,deg,per;

    printf ("      slope\n");
    printf ("degrees  percent\n\n");
    for (i = 0; i < 15; i++)
    {
	for (j = 0; j < 6; j++)
	{
	    deg = j * 15 + i;
	    if (j) printf (" | ");
	    if (deg > 90) break;
	    if (deg == 90)
		printf ("90 (undefined)");
	    else
	    {
		x = (double) deg * 3.14159/180;
		per = tan(x) * 100.0 + .5;
		printf ("%2d %4d", deg, per);
	    }
	}
	printf ("\n");
    }
}
