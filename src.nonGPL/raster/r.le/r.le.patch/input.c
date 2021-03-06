                        /********************************/
                        /*      r.le.patch/input.c      */
                        /*                              */
                        /*            2.2               */
                        /*                              */
                        /*      12/1/97 version         */
                        /*                              */
                        /*      Programmer: Baker       */
                        /*      Univ. of Wyoming        */
                        /********************************/

#include "patch.h" 

extern struct CHOICE *choice;

void user_input(argc,argv)  
int argc;
char **argv;

{

   int i,count=0;

					/* setup the GRASS parsing routine
					   structures to be used to read
					   in the user's parameter choices */

   struct Flag *run;
   struct Flag *cor;
   struct Flag *pat;
   struct Flag *bound;
   struct Flag *trace;
   struct Flag *units;
   struct Option *name;
   struct Option *sampling_method;
   struct Option *att;
   struct Option *size;
   struct Option *edge;
   struct Option *core;
   struct Option *shape;
   struct Option *shape_m;
   struct Option *fractal;
   struct Option *perim_method;
   struct Option *perimeter;
   struct Option *region;
   struct Option *out;


					/* initialize the GRASS GIS system */

   G_gisinit(argv[0]);	

					/* use the GRASS parsing routines
					   to read in the user's parameter
					   choices */

   cor = G_define_flag();
   cor->key          	= 'c';
   cor->description  	= "Output map 'interior' with patch cores";

   pat = G_define_flag();
   pat->key          	= 'n';
   pat->description  	= "Output map 'num' with patch numbers";

   bound = G_define_flag();
   bound->key		= 'p';
   bound->description	= "Include sampling area boundary as perimeter";

   trace = G_define_flag();
   trace->key          	= 't';
   trace->description  	= "Use 4 neighbor tracing instead of 8 neighbor";

   units = G_define_flag();
   units->key          	= 'u';
   units->description  	= "Output maps 'units_x' with sampling units for each scale x ";

   name = G_define_option();
   name->key          = "map";
   name->description  = "Raster map to be analyzed";
   name->type         = TYPE_STRING;
   name->gisprompt    = "old,cell,raster";
   name->required     = YES;

   sampling_method = G_define_option();
   sampling_method->answer 	= "w";
   sampling_method->key         = "sam";
   sampling_method->description = "Sampling method (choose only 1 method): \n\t\t w=whole map, u=units, m=moving window, r=regions";
   sampling_method->type        = TYPE_STRING;
   sampling_method->multiple	= NO;
   sampling_method->required    = NO;

   region = G_define_option();
   region->key          = "reg";
   region->description  = "Name of regions map, only when sam = r; omit otherwise";
   region->type         = TYPE_STRING;
   region->gisprompt    = "old,cell,raster";
   region->required     = NO;

   att = G_define_option();
   att->key     	= "att";
   att->description  	= "Attribute measures:  \n\t\t a1 = mean pixel attribute\ta2 = st. dev. pixel attribute\n\t\t a3 = mean patch attribute\ta4 = st. dev. patch attribute\n\t\t a5 = cover by gp\t\ta6 = density by gp\n\t\t a7 = total density";
   att->options  	= "a1,a2,a3,a4,a5,a6,a7";
   att->type         	= TYPE_STRING;
   att->multiple    	= YES;
   att->required     	= NO;

   size = G_define_option();
   size->key     	= "siz";
   size->description  	= "Patch size measures:   \n\t\t s1 = mean patch size\t\ts2 = st. dev. patch size\n\t\t s3 = mean patch size by gp\ts4 = st. dev. patch size by gp\n\t\t s5 = no. by size class\t\ts6 = no. by size class by gp";
   size->options  	= "s1,s2,s3,s4,s5,s6";
   size->type         	= TYPE_STRING;
   size->multiple     	= YES;
   size->required     	= NO;

   edge = G_define_option();
   edge->key		= "co1";
   edge->description	= "Edge width in pixels (integer) for use with co2";
   edge->type		= TYPE_INTEGER;
   edge->required	= NO;

   core = G_define_option();
   core->key     	= "co2";
   core->description  	= "Core size measures (required if co1 was specified):\n\t\t c1 = mean core size\t\tc2 = st. dev. core size\n\t\t c3 = mean edge size\t\tc4 = st. dev. edge size\n\t\t c5 = mean core size by gp\tc6 = st. dev. core size by gp\n\t\t c7 = mean edge size by gp\tc8 = st. dev. edge size by gp\n\t\t c9 = no. by size class\t\tc10 = no. by size class by gp";
   core->options  	= "c1,c2,c3,c4,c5,c6,c7,c8,c9,c10";
   core->type         	= TYPE_STRING;
   core->multiple     	= YES;
   core->required     	= NO;

   shape = G_define_option();
   shape->key     	= "sh1";
   shape->description  	= "Shape method (choose only 1 method):\n\t\t m1 = perim./area\t\tm2 = corr. perim./area\n\t\t m3 = rel. circum. circle";
   shape->type         	= TYPE_STRING;
   shape->multiple	= NO;
   shape->required     	= NO;

   shape_m = G_define_option();
   shape_m->key     		= "sh2";
   shape_m->description  	= "Shape measures (required if sh1 was specified):   \n\t\t h1 = mean patch shape\t\th2 = st. dev. patch shape\n\t\t h3 = mean patch shape by gp\th4 = st. dev. patch shape by gp\n\t\t h5 = no. by shape class\th6 = no. by shape class by gp";
   shape_m->options  		= "h1,h2,h3,h4,h5,h6";
   shape_m->type         	= TYPE_STRING;
   shape_m->multiple     	= YES;
   shape_m->required     	= NO;

   fractal = G_define_option();
   fractal->key     		= "fra";
   fractal->description  	= "Fractal dimension measures:   \n\t\t f1 = perim.-area fractal dim.";
   fractal->options  		= "f1";
   fractal->type         	= TYPE_STRING;
   fractal->required     	= NO;

   perimeter = G_define_option();
   perimeter->key     		= "per";
   perimeter->description  	= "Perimeter measures:   \n\t\t p1 = sum of perims.\t\tp4 = sum of perims. by gp\n\t\t p2 = mean perim.\t\tp5 = mean perim. by gp\n\t\t p3 = st. dev. perim.\t\tp6 = st. dev. perim. by gp";
   perimeter->options  		= "p1,p2,p3,p4,p5,p6";
   perimeter->type         	= TYPE_STRING;
   perimeter->multiple     	= YES;
   perimeter->required     	= NO;

   out = G_define_option();
   out->key          = "out";
   out->description  = "Name of output file for individual patch measures, when sam=w,u,r;\n\t    if out=head, then column headings will be printed";
   out->type         = TYPE_STRING;
   out->required     = NO;


   if (G_parser(argc,argv)) {
      exit(-1);
   }
					/* record the user inputs for map,
					   sam and out parameters */

   G_strcpy(choice->fn,name->answer);

   choice->wrum = sampling_method->answer[0];

   if (out->answer && choice->wrum != 'm')
      G_strcpy(choice->out,out->answer);
   else if (out->answer && choice->wrum == 'm') {
         printf("\n");
         printf("   ***************************************************\n");
         printf("    You can use the out parameter only when sam=w,u,r \n");
         printf("   ***************************************************\n");
         exit(0);
   } 
   else
      G_strcpy(choice->out,"");


					/* check for unacceptable values for
					   input parameters */

   if (strcmp(sampling_method->answer,"w") && 
       strcmp(sampling_method->answer,"u") &&
       strcmp(sampling_method->answer,"m") && 
       strcmp(sampling_method->answer,"r")) {
          printf("\n");
          printf("   ***************************************************\n");
          printf("    You input an unacceptable value for parameter sam \n");
          printf("   ***************************************************\n");
          exit(0);
   }

   if (shape->answer)
      if (strcmp(shape->answer,"m1") && strcmp(shape->answer,"m2") &&
         strcmp(shape->answer,"m3")) {
            printf("\n");
            printf("   ***************************************************\n");
            printf("    You input an unacceptable value for parameter sh1 \n");
            printf("   ***************************************************\n");
            exit(0);
      }
      

					/* check for multiple values for 
					   parameters that accept only 1 */

   if (sampling_method->answer)
      if (sampling_method->answers[1]) {
         printf("\n");
         printf("   **********************************************\n");
         printf("    You input multiple values for parameter sam, \n");
         printf("    but only one is allowed                      \n");
         printf("   **********************************************\n");
         exit(0);
      }

   if (shape->answer)
      if (shape->answers[1]) {
         printf("\n");
         printf("   **********************************************\n");
         printf("    You input multiple values for parameter sh1, \n");
         printf("    but only one is allowed                      \n");
         printf("   **********************************************\n");
         exit(0);
      }

					/* if the core flag -c is specified,
					   then set the choice->coremap flag
					   to 1 */

   choice->coremap = 0;
   if (!strcmp(sampling_method->answer,"w") && cor->answer)
      choice->coremap = 1;
   else if (strcmp(sampling_method->answer,"w") && cor->answer) {
      printf("\n");
      printf("   ***********************************************\n");
      printf("    You requested output of map 'core' with patch \n");
      printf("    cores, by using flag -c, but this option      \n");
      printf("    is only available when sam=w                  \n");
      printf("   ***********************************************\n");
      exit(0);
   }

					/* if the pat flag -n is specified,
					   then set the choice->coremap flag
					   to 1 */


   choice->patchmap = 0;
   if (!strcmp(sampling_method->answer,"w") && pat->answer)
      choice->patchmap = 1;
   else if (strcmp(sampling_method->answer,"w") && pat->answer) {
      printf("\n");
      printf("   **********************************************\n");
      printf("    You requested output of map 'num' with patch \n");
      printf("    numbers, by using flag -n, but this option   \n");
      printf("    is only available when sam=w                 \n");
      printf("   **********************************************\n");
      exit(0);
   }

 
   
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

					/* if the -u flag is specified, then
					   set the choice->units flag to 1 */

   choice->units = 0;
   if (!strcmp(sampling_method->answer,"u") && units->answer)
      choice->units = 1;
   else if (strcmp(sampling_method->answer,"u") && units->answer) {
      printf("\n");
      printf("   ***************************************************\n");
      printf("    You requested output of map 'units' with sampling \n");
      printf("    units, by using flag -u, but this option is only  \n");
      printf("    available when sam=u                              \n");
      printf("   ***************************************************\n");
      exit(0);
   }


					/* if the co1 parameter is specified,
					   then save the value of edge width
					   in choice->edge */

   choice->edge = 0;
   if (edge->answer)
      choice->edge = atoi(edge->answer);

					 /* if sampling_method is by REGION
					    get region file name.  Check to see
					    that the name was input */

   if (!strcmp(sampling_method->answer,"r")) {
      if (region->answer)
         G_strcpy(choice->reg,region->answer);
      else {
      	 printf("\n");
      	 printf("   ***********************************************\n");
      	 printf("    You requested sampling by region, but did not \n");
         printf("    input the name of the region using the reg=   \n");
      	 printf("    parameter                                     \n");    
      	 printf("   ***********************************************\n");
      	 exit(0);
      }
   }

   if (region->answer)
      if (strcmp(sampling_method->answer,"r")) {           
      	 printf("\n");
      	 printf("   ***********************************************\n");
      	 printf("    You requested sampling by region, by using    \n");
         printf("    the reg= parameter, but did not input the     \n");
         printf("    sam=r parameter                               \n");    
      	 printf("   ***********************************************\n");
      	 exit(0);
   }

         				/* initialize flag arrays in choice 
					   data structure. */

   for (i = 0; i < 3; i++) choice->Mx[i]    = 0;
   for (i = 0; i < 8; i++) choice->att[i]   = 0;
   for (i = 0; i < 8; i++) choice->size[i]  = 0;
   for (i = 0; i < 11; i++) choice->core[i] = 0;
   for (i = 0; i < 8; i++) choice->shape[i] = 0;
   for (i = 0; i < 8; i++) choice->perim[i] = 0;
         

   if (att->answer) {
      choice->att[0]=1; 		/* flag for attribute computations. */
      for (i = 0; att->answers[i] != NULL; i++) {
         count++;
              if (!strcmp(att->answers[i],"a1"))   choice->att[1]=1;
         else if (!strcmp(att->answers[i],"a2"))   choice->att[2]=1;
         else if (!strcmp(att->answers[i],"a3"))   choice->att[3]=1;
         else if (!strcmp(att->answers[i],"a4"))   choice->att[4]=1; 
         else if (!strcmp(att->answers[i],"a5"))   choice->att[5]=1;
         else if (!strcmp(att->answers[i],"a6"))   choice->att[6]=1;
         else if (!strcmp(att->answers[i],"a7"))   choice->att[7]=1;
      }
   }
              
   if (size->answer) {
      choice->size[0]=1; 		/* flag for size computations. */
      for (i = 0; size->answers[i] != NULL; i++) {
         count++;
              if (!strcmp(size->answers[i],"s1"))   choice->size[1] = 1;
         else if (!strcmp(size->answers[i],"s2"))   choice->size[2] = 1;
         else if (!strcmp(size->answers[i],"s3"))   choice->size[3] = 1;
         else if (!strcmp(size->answers[i],"s4"))   choice->size[4] = 1;
         else if (!strcmp(size->answers[i],"s5"))   choice->size[5] = 1;
         else if (!strcmp(size->answers[i],"s6")) { 
						    choice->size[6] = 1;
                                                    choice->size2   = 1;
		 }                      
      }
   }

   if (core->answer || edge->answer) {
      if (core->answer && edge->answer) {
         choice->core[0]=1; 		/* flag for core computations. */
         for (i = 0; core->answers[i] != NULL; i++) {
            count++;
                 if (!strcmp(core->answers[i],"c1"))   choice->core[1] = 1;
            else if (!strcmp(core->answers[i],"c2"))   choice->core[2] = 1;
            else if (!strcmp(core->answers[i],"c3"))   choice->core[3] = 1;
            else if (!strcmp(core->answers[i],"c4"))   choice->core[4] = 1;
            else if (!strcmp(core->answers[i],"c5"))   choice->core[5] = 1;
            else if (!strcmp(core->answers[i],"c6"))   choice->core[6] = 1;
            else if (!strcmp(core->answers[i],"c7"))   choice->core[7] = 1;
            else if (!strcmp(core->answers[i],"c8"))   choice->core[8] = 1;
            else if (!strcmp(core->answers[i],"c9"))   choice->core[9] = 1;
            else if (!strcmp(core->answers[i],"c10")) { 
						       choice->core[10] = 1;
					  	       choice->core2    = 1;
			}
		 }
      }
      else {
         if (cor->answer ) {
      	    printf("\n");
      	    printf("   ***********************************************\n");
      	    printf("    You requested output of map 'core' with patch \n");
      	    printf("    cores, by using flag -c, but did not input    \n");
            printf("    the co1 and co2 parameters                    \n");
      	    printf("   ***********************************************\n");
      	    exit(0);
         }
         else {
      	    printf("\n");
      	    printf("   ***********************************************\n");
      	    printf("    You requested core size measures, but did not \n");
	        printf("    input both parameter co1 and co2              \n");
      	    printf("   ***********************************************\n");
      	    exit(0);
         }
      }
   }

   if (shape->answer || shape_m->answer) {
      if (shape->answer && shape_m->answer) {
              if (!strcmp(shape->answer,"m1"))   choice->Mx[1] = 1;
         else if (!strcmp(shape->answer,"m2"))   choice->Mx[2] = 1;
         else if (!strcmp(shape->answer,"m3"))   choice->Mx[3] = 1;
         choice->shape[0]=1; 		/* flag for shape computations. */
         for (i = 0; shape_m->answers[i] != NULL; i++) {
            count++;
                 if (!strcmp(shape_m->answers[i],"h1")) choice->shape[1]=1;
            else if (!strcmp(shape_m->answers[i],"h2")) choice->shape[2]=1;
            else if (!strcmp(shape_m->answers[i],"h3")) choice->shape[3]=1;
            else if (!strcmp(shape_m->answers[i],"h4")) choice->shape[4]=1;
            else if (!strcmp(shape_m->answers[i],"h5")) choice->shape[5]=1;
            else if (!strcmp(shape_m->answers[i],"h6")) {
	    						   choice->shape[6]=1;
            						   choice->shape2  =1;
            }
         }
      }
      else {
      	 printf("\n");
      	 printf("   **********************************************\n");
         printf("    You requested shape measurement, but did not \n");
	     printf("    input both parameter sh1 and sh2             \n");
      	 printf("   **********************************************\n");
      	 exit(0);
      }
   }

   choice->fract = 0;
   if (fractal->answer) {
      choice->fract = 1;
      count++;
   }

   if (perimeter->answer) {
      choice->perim[0]=1; 		/* flag for perim. computations. */
      for (i = 0; perimeter->answers[i] != NULL; i++) {
         count++;
              if (!strcmp(perimeter->answers[i],"p1")) choice->perim[1]=1;
         else if (!strcmp(perimeter->answers[i],"p2")) choice->perim[2]=1;
         else if (!strcmp(perimeter->answers[i],"p3")) choice->perim[3]=1;
         else if (!strcmp(perimeter->answers[i],"p4")) choice->perim[4]=1;
         else if (!strcmp(perimeter->answers[i],"p5")) choice->perim[5]=1;
         else if (!strcmp(perimeter->answers[i],"p6")) choice->perim[6]=1;
      }
   }

   if (choice->wrum == 'm' && count > 25) {
      printf("\n");
      printf("   ****************************************************\n");
      printf("    You can only choose up to 25 simultaneous measures \n");
      printf("    when using sam=m.  Please redo your request.       \n");
      printf("   ****************************************************\n");
      exit(0);
   }

   if (!att->answer && !size->answer && !shape->answer && !shape_m->answer
      && !fractal->answer && !perimeter->answer && !core->answer) {
      printf("\n");
      printf("   **************************************************\n");
      printf("    You did not select any measures to be calculated \n");
      printf("   **************************************************\n");
      exit(0);
   }

}

