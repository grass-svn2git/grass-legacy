
/*
*  This takes an ascii digit file in one coordinate system and converts
*  the map to another coordinate system.
*  Uses the transform library:  ../src/mapdev/libes/libtrans.a .
*
*  Written during the ice age of Illinois, 02/16/90, by the GRASS team, -mh.
*/

/*
*  Current is the existing file to be converted.
*  Trans is the new transformed file.
*/

#define MAIN
#define USAGE	"[mapin=name] [mapout=name] [coord=name] [verbose=(yes,no)]"

#include  "trans.h"
#include "options.h"

static  char  *Prog_name ;

main( argc, argv)
	int  argc ;
	char *argv[] ;
{

	char  current_name[150] ;
	char  trans_name[150] ;
	char  *current_mapset ;
	char  *trans_mapset ;
	struct  file_info  Current ;
	struct  file_info  Trans ;
	struct  file_info  Coord ;
	struct  command_flags  Flags ;

	extern int stash_away() ;
	extern int my_help() ;

	G_gisinit("Vector Transformation");

	Prog_name = argv[0] ;


/* Check command line */
	set_default_options( &Current, &Trans, &Coord, &Flags ) ;
	if ( argc > 1 )
	{

		G_set_parse_command_usage( my_help) ;
		if (G_parse_command(argc, argv, com_keys, stash_away))
		{
			fprintf(stderr,"Usage: %s %s\n", argv[0], USAGE) ;
			pr_options() ;
			exit(-1) ;
		}
	}

	open_vect_files ( &Current, &Trans, &Coord ) ;

	/*  read the header first, so that if there any errors reading the
	*   the header they don't have to type in the numbers before
        *   finding out.
	*/

	if ( 0 > read_head_ascii(Current.fp) )
	{
		fprintf( stderr, "\nERROR: Could not read the header information in the ascii digit file: %s .\n\n",  Current.full_name ) ;
		exit (-1) ;
	}

	create_transform_conversion( &Coord, &Flags ) ;

	transform_head_info() ;
	write_head_ascii( Trans.fp) ;


	if (Flags.verbose)
		printf("\n\n Now transforming the vectors ...") ;
	transform_digit_file( Current.fp, Trans.fp) ;

	fflush(Trans.fp) ;
	if (Coord.name[0] != NULL)
		fclose( Coord.fp) ;
	fclose( Current.fp) ;
	fclose( Trans.fp) ;
	if (Flags.verbose)
		printf("\n '%s' has finished the transformation of the vectors.\n", argv[0]) ;

	if ( open_att_files ( &Current, &Trans) )
	{
		if (Flags.verbose)
			printf("\nThere was no dig_att file to convert with this vector file. \n") ;
		exit(0) ;
	}

	transform_att_file( Current.fp, Trans.fp) ;

	if (Flags.verbose)
	{
		printf(" '%s' has finished the transformation of the vector's attribute file.\n", argv[0]) ;
		printf("Transformation is complete.\n") ;
	}
	fclose( Current.fp) ;
	fclose( Trans.fp) ;

	exit(0) ;

}


my_help()
{
	G_parse_command_usage( Prog_name, com_keys, USAGE_SHORT) ;
}

pr_options()
{
	fprintf(stderr,"\n    Options:\n") ;
	fprintf(stderr,"     mapin  - name of existing map to transform.\n") ;
	fprintf(stderr,"     mapout - name of transformed map.\n") ;
	fprintf(stderr,"     coord  - name of file holding transformation coordinates.\n") ;
	fprintf(stderr,"     verbose  - print the stats or not (default is yes).\n") ;
}
