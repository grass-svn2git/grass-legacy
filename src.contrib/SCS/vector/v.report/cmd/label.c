#include <stdio.h>
char *
print_label (s, len, pflag, spacing, dot)
    char *s;
    char dot;
{
    char *x;
    int n;
    int i;

/* strip away leading spaces */
    while (*s == ' ')
	s++;

/* if it all fits, then just print it, and add spaces to pad to len */
    n = strlen (s);
    if (n <= len)
    {
	if (pflag)
	{
	    i = 0;
	    while (*s)
	    {
		putchar (*s++);
		i++;
	    }
	    while(n++ < len)
		putchar (spacing && ++i%spacing == 0 ? dot : ' ');
	}
	return NULL;
    }

/* back up from len chars to first space */
    for (x = s + len; x != s; x--)
	if (*x == ' ')
	    break;
/* special case. If entire string has no spaces, just print len chars */
    if (*x != ' ')
	x = s+len;
    else
    {
/* back up to first space */
	while (*x == ' ')
	    x--;
	x++;
    }

    i = 0;
    while (s != x)
    {
	if (pflag)
	    putchar (*s);
	i++;
	s++;
	len--;
    }
    if (pflag)
    {
	while (len-- > 0)
	    putchar (spacing && ++i%spacing == 0 ? dot : ' ');
    }
    return s;
}
