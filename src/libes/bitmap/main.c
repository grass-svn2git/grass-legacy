#include <stdio.h>
#include "bitmap.h"

main(argc, argv)
    char *argv[];
{
    int SIZE;
    struct BM *map, *map2;
    int i, x, y;
    int dump;
    FILE *fp;

    if (argc < 2)
	SIZE = 11;
    else
	SIZE = atoi (argv[1]);

    if(NULL != getenv ("NODUMP"))
	dump = 0;
    else
	dump = 1;

    map = BM_create (SIZE, SIZE);

    /* turn on bits in X pattern */
    for (i = 0 ; i < SIZE ; i++)
    {
	BM_set (map, i, i, 1);
	BM_set (map, (SIZE-1)-i, i, 1);
    }


    if (dump)
	dump_map (map);
    printf ( "Size = %d\n", BM_get_map_size (map));

    printf ( "\n\n");

    /* now invert it */
    for (y = 0 ; y < SIZE ; y++)
	for (x = 0 ; x < SIZE ; x++)
	    BM_set(map, x, y, !BM_get (map, x, y));

    if (dump)
	dump_map (map);

    printf ( "Size = %d\n", BM_get_map_size (map));

    {
	fp = fopen ("dumpfile", "w");
	BM_file_write (fp, map);
	fclose (fp);

	fp = fopen ("dumpfile", "r");
	map2 = BM_file_read (fp);
	fclose (fp);
	dump_map (map2);
    }

    BM_destroy (map);
    BM_destroy (map2);
}

dump_map(map)
    struct BM *map;
{
    int x, y;

    for (y = 0 ; y < map->rows ; y++)
    {
	for (x = 0 ; x < map->cols ; x++)
	{
	    printf ( "%c", BM_get(map, x, y) ? '#' : '.');

	}
	printf ( "\n");
    }
}
