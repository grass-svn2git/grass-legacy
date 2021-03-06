				/********************************/
				/*    r.le.trace/user_input.c	*/
				/*				*/
				/*		2.2		*/
				/*				*/
				/*       12/1/97 version	*/
				/*				*/
				/*      Programmer: Baker	*/
				/*      Univ. of Wyoming	*/
				/********************************/


#include "r.le.trace.h"

extern struct CHOICE *choice ;

void user_input(argc,argv)  
int argc ;
char **argv ;

{


   struct Flag *pat;
   struct Flag *bound;
   struct Flag *trace;
   struct Option *name;
   struct Option *out;

   pat = G_define_flag();
   pat->key          	= 'n';
   pat->description  	= "Output map 'num' with patch numbers";

   bound = G_define_flag();
   bound->key		= 'p';
   bound->description	= "Include sampling area boundary as perimeter";

   trace = G_define_flag();
   trace->key          	= 't';
   trace->description  	= "Use 4 neighbor tracing instead of 8 neighbor";

   name = G_define_option() ;
   name->key          = "map" ;
   name->description  = "Raster map to be analyzed";
   name->type         = TYPE_STRING;
   name->gisprompt    = "old,cell,raster";
   name->required     = YES ;


   out = G_define_option() ;
   out->key = "out" ;
   out->description = "Name of output file to store patch data";
   out->type = TYPE_STRING ;
   out->required = NO ;


   if (G_parser(argc,argv))
      exit(-1) ;

   G_strcpy(choice->fn,name->answer) ;

   if (out->answer)
      G_strcpy(choice->out,out->answer) ;
   else
      G_strcpy(choice->out,"") ;

					/* if the pat flag -n is specified,
					   then set the choice->coremap flag
					   to 1 */


   choice->patchmap = 0;
   if (pat->answer)
      choice->patchmap = 1;
   
					/* if the 4 neighbor tracing flag -t
					   is specified, then set the 
					   choice->trace flag to 1 */

   choice->trace = 1;
   if (trace->answer)
      choice->trace = 0;

					/* if the -p flag is specified, then
					   set the choice->perim2 flag to 0 */

   if (bound->answer)
      choice->perim2 = 0;
   else
      choice->perim2 = 1;



}
