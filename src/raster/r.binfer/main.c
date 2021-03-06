/*
**                               
** Filename: main.c
**                                 
**                                  
** Author: Kurt A. Buehler           
** Date: Fri Feb 10 08:41:42 EST 1989
** Version: 1.0                        
**                                      
*/

#include <stdio.h>
#include "symtab.h"
extern int yylineno;
extern char yytext[];
extern FILE *yyin;
int verbose;
int probabilitymaps;
int combinedmap;
int colortable;
char *result ;
extern struct symtab table;

main(argc,argv)
int argc;
char **argv;
{
    struct symtab *ptr;
    FILE *fopen();


    probabilitymaps = 1;
    combinedmap = 1;
    colortable = Ramp;

    G_gisinit(argv[0]);
    parse_arglist(argc,argv);
    ptr = (struct symtab *)yyparse();
    fclose(yyin);
    fprintf(stderr,"Creating intermediate maps\n");
    do_reclass(ptr);
    engine(ptr,result);
    fprintf(stderr, "Removing intermediate maps\n");
    cleanup_reclass(ptr);
}



yyerror(s)
char *s;
{
    fprintf(stderr,"\n%s: line number %d at or near \"%s\"\n",s,yylineno,yytext);
    exit(0);
}


parse_arglist(argc,argv)
int argc;
char **argv;
{
    char errmsg[80];
	
	struct Option *infile ;
	struct Option *outfile ;
	struct Flag *verbose_flag ;

	verbose_flag              = G_define_flag() ;
	verbose_flag->key         = 'v' ;
	verbose_flag->description = "Run verbosely with debugging output" ;

	infile                    = G_define_option() ;
	infile->key               = "input" ;
	infile->description       = "file containing instructions" ;
	infile->type              = TYPE_STRING ;
	infile->required          = YES ;

	outfile                   = G_define_option() ;
	outfile->key              = "output" ;
	outfile->description      = "file to capture output information" ;
	outfile->type             = TYPE_STRING ;
	outfile->answer           = "r.binfer.out" ;
	outfile->required         = NO ;

	if (G_parser(argc, argv))
		exit(1) ;

    verbose = verbose_flag->answer ;;
	if ((yyin = fopen(infile->answer,"r")) == NULL )
	{
		fprintf(stderr,"Couldn't open scriptfile %s\n",infile->answer);
		G_usage() ;
		exit(1) ;
	}

	result = outfile->answer ;
}
