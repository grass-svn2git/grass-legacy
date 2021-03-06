#include "coin.h"

/* computes the approximate number of lines/pages an 80 column
 * report would take, and asks the user if this is ok
 */
check_report_size()
{
    long nlines;
    long npages;
    long npanels;
    char buf[100];


    npanels = ncat1 / 3;
    if (ncat1 % 3) npanels++;

    nlines = (12 + ncat2) * npanels + 11 + ncat2;
    npages = (nlines+65)/66;

    while(1)
    {
	printf ("\nThe coincidence table is %d rows by %d columns\n",
		ncat2, ncat1);
	printf("The report will require approximately %ld lines (%ld pages)\n",
		nlines, npages);
	printf("Do you want to continue? ");
	while(1)
	{
	    printf ("(y/n) ");
	    if (!G_gets(buf))
		break;
	    if (*buf == 'y' || *buf == 'Y') return;
	    if (*buf == 'n' || *buf == 'N') exit(0);
	}
    }
}
