/******************************************
 * $GISABSE/etc/echo [-n] args
 *
 * echos its args to stdout
 * suppressing the newline if -n specified
 *
 * replaces the standard UNIX echo which
 * varies from machine to machine
 *******************************************/
main(argc, argv) char *argv[];
{
    int i;
    int newline;
    int any;

    newline = 1;
    any = 0;

    for (i = 1; i < argc; i++)
	if (strcmp (argv[i],"-n") == 0)
	    newline = 0;
	else
	    printf ("%s%s", any++?" ":"", argv[i]);
    if (any && newline)
	printf ("\n");
}
