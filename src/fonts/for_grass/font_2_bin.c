/* read stdin, write font.bin */
long *index = 0;
long nchars = 0;
main ()
{
    long lseek();
    long offset;
    int font;
    int n;
    unsigned char x[256],y[256];
    int i;

    font = creat ("font.bin", 0666);
    if (font < 0)
    {
	perror ("font.bin");
	exit(1);
    }

/* index to the font ids will be written at end of font file.
 * pointer to index is written at top of file. write zero now
 */
    offset = 0;
    write (font, &offset, sizeof offset);

/* as each character comes in, add it to the index
 * then write the character to the font file
 * preceeded by the number of vectors in the character
 */
    while ((n = scanint (5)) > 0)
    {
	newchar (n, lseek (font, 0L, 1));
	n = scanint (3);

	for (i = 0; i < n; i++)
	{
	    if ( i == 32 || i == 68 || i == 104 || i ==140)
		getchar();	/* skip newlines? */
	    x[i] = getchar();
	    y[i] = getchar();
	}
	getchar();
	write (font, &n, sizeof (n));
	write (font, x, n);
	write (font, y, n);
    }

/* that's it. now write the index at the end, and the pointer
 * to the index at the head of the file
 */
    offset = lseek (font, 0L, 1);
    write (font, &nchars, sizeof(nchars));
    write (font, index, nchars * sizeof (*index));
    lseek (font, 0L, 0);
    write (font, &offset, sizeof(offset));
    close (font);

    exit(0);
}
scanint (n)
{
    char buf[20];
    int i,c;

    for (i=0; i < n; i++)
    {
	while ((c = getchar ()) == '\n')
		;
	if (c < 0) return -1;
	buf[i] = c;
    }
    buf[i] = 0;
    return atoi(buf);
}

/* add a new character to the index */
newchar (n, offset)
    long offset;
{
    char *malloc(), *realloc();
    n++;
    if (n > nchars)
    {
	if (nchars)
	    index = (long *)realloc (index, n * sizeof(*index));
	else
	    index = (long *)malloc (n * sizeof (*index));
	
	while (nchars < n)
	    index[nchars++] = 0;
    }
    index[n-1] = offset;
}
