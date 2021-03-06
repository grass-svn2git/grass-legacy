/*******************************************************
void Critical_area_analys()

MODIFICATIONS by:
Michael Foster, AIWQ Center 
501 ASI Building 
Penn State University
University Park, PA 16802
Tel./FAX 814-865-3375/3048
Email: mike_foster@agcs.acs.psu.edu

PURPOSE OF FUNCTION:

1. To read in agnps.dat input file of AGNPS
and determine the number of cells.

2. To open agnps.nps output file of AGNPS and
read in the nutrient management data.

3. Prepare rules for generating raster maps of 
AGNPS output.

4. Write map title file for each AGNPS output.

5. Generate rules for reclassing. 

*************************************************************/


#include <stdio.h>
#include <string.h>
#include <sys/param.h>


/* parameter "input_nps_filename" added by Dave Peterson, April 1996; parameter
   passes back input filename entered by user with .nps extension added
*/
void 
Critical_area_analys (char *input_nps_filename)
{


  /* Definitions of variables                          */
  /* The variable names are indentical to the name used*/
  /* in AGNPS output.                                  */


  /*
     numb ---- Number of Base cells
     nd   ---- Number of division cells 
     csn  ---- Cell sediment nitrogen           lb/acre
               (Nitrogen in the total
                   sediment of the cell)       

     sdn  ---- Sediment attached nitrogen       lb/arce
               (Nitrogen in sediment leaving
                   the cell)
     cn   ---- Soluble nitrogen concentration   
               in cell runoff                   lb/arce
     tsn  ---- Total soluble nitrogen           lb/arce
     rppmn---- Soluble nitrogen concentration   ppm 

     csp  ---- Cell sediment phosphorus         lb/arce
               (Phosphorus in total sediment 
                   of the cell)   
     sdp  ---- Sediment attached phosphorus     lb/arce
               (Phosphorus in sediment leaving
                   the cell)
     cp   ---- Soluble Phosphorus in cell runoff lb/arce

     tsp  ---- Total soluble phosphorus         lb/arce
     rppmp---- Soluble phosphorus concentration ppm
     cc   ---- Cell COD yield                   lb/arce
     tscod---- Total soluble COD                lb/arce
     rppmc---- Soluble COD concentration        ppm
     sedy ---- Sediment yied in a cell          tons
     ce -----  Cell erosion                     tons/acre
     */

     /* define agnps output variables                */
     int numb, nd;
     float da, csn, sdn, cn, tsn, rppmn, csp, sdp, sedy,ce;
     float cp, tsp, rppmp, cc, tscod, rppmc, runof;

     /* define pointers for selected agnps output     */
     /* parameters for purpose of dynamically         */
     /* allocating memory for these variables         */

     float *ptrunof, *ptsdn, *ptcn, *pttsn, *ptsdp, *ptcp, *pttsp, *ptcc,*pttscod, *ptsedy, *ptce;

    FILE *infile;
    FILE *outfile;
    FILE *tfile;
    FILE *temp1;
    FILE *temp2;
    FILE *temp3;
    FILE *temp4;
    FILE *temp5;
    FILE *temp6;
    FILE *temp7;
    FILE *temp8; 
    FILE *temp9;
    FILE *temp10;
    FILE *temp11;
    FILE *temp12;


    char *file_selected;
/*
ADDED BY MIKE FOSTER 2-19-96
*/
    char file_entered[30];
    char infilename[30];
    char outfilename[30];
    char teststring[30];
/*
END ADDITIONS BY MIKE FOSTER 2-19-96
*/
    int getstring_ok;
    int getdir_ok;
    char cmd[100];
    char filename[100], label;
    int  slength, t, m;
    char s[15], line[100];
    char *directory;
    char cdir[80];
    int  basecell,tcell;
    float cellsize;
    float  cvtf= 4.4167;   /* cvtf = the convert factor of lb/arce.in */
                           /*        to mg/L.  The latter is used as  */
                           /*        measurement fo water quality.    */ 
                           /*        Where in is the runoff per arce. */

    int tt;
  /* Define auxiliary variables              */
    int t1,t2, i;        
    float t3,t4,t5,t6,t7,t8,t9,t10;

  /* Define external variables               */
    extern int nlimit, plimit, elimit, climit;

  /* initialize the variables and arrays     */

    for(i=0; i<=99; i++) {
       line[i] = ' ';
       filename[i] = ' ';
       }

    strcpy(teststring,"                             ");

  /* Open the agnps.dat file and read in the number  */
  /* of total cells(including divided cells)         */

  /*  First, change the working directory to the user*/
  /*  selected simulation.                           */


/* ASK FOR THE FILENAME IN THE MODIFIED VERSION */
/* MIKE FOSTER 2-19-96 */

      system("clear");
      printf("\nPlease enter the output file name without its .nps extension\n");
      printf("(ex: gaston) >");
      gets(file_entered);
      strcpy(infilename,file_entered);
      strcpy(input_nps_filename, file_entered);
      strcat(input_nps_filename, ".nps");
      strcat(infilename,".dat");
      if((infile = fopen(infilename,"r")) == NULL) {
         fprintf(stderr,"Can't open file: %s\n",infilename);
         sprintf(cmd,"Bad_dir_message.sh %s.nps\n",file_entered); 
         system(cmd); 
         return;
         }
      fgets(line,80,infile);
      fgets(line,80,infile);
      fgets(line,80,infile);
      fgets(line,80,infile);
      fgets(line,80,infile);
      sscanf(line,"%f %d %d",&cellsize,&basecell,&tcell);
      fclose(infile);


   /*  Use malloc to allocate dynamic memory for the */
   /*  variables to be read in from the agnps output */
   /*  file.                                         */


       ptrunof = (float *)malloc(tcell*sizeof(runof));
       ptsdn=(float *)malloc(tcell*sizeof(sdn));
       pttsn =(float *)malloc(tcell*sizeof(tsn));
       ptsdp=(float *)malloc(tcell*sizeof(sdp));
       pttsp =(float *)malloc(tcell*sizeof(tsp));
       pttscod =(float *)malloc(tcell*sizeof(tscod));
       ptsedy =(float *)malloc(tcell*sizeof(sedy));
       ptce  =(float *)malloc(tcell*sizeof(ce));

   /* Open agnps output file to read in the nutrient */
   /* data to the corresponding addresses allocated  */
   /* by malloc.                                     */

   strcpy(outfilename,file_entered);
   strcat(outfilename,".nps"); 
      if( (outfile=fopen(outfilename,"r")) == NULL) {
          fprintf(stderr,"The AGNPS output file: %s\n",outfilename);
          fprintf(stderr,"does NOT exist. Please\n");
          fprintf(stderr,"run AGNPS to generate \n");
          fprintf(stderr,"the ouput file for this\n");
          fprintf(stderr,"simulation\n");
          sprintf(cmd,"Bad_dir_message.sh %s.nps",file_entered);
          system(cmd);
          return;
          }


      while(  (fgets(line,80,outfile)) != NULL) {
         if( (strncmp(line,"PESTICIDE",8)) ==0) {
            for(i=0; i<=tcell-1; i++) {
               fgets(line,80,outfile);
      sscanf(line,"%d %d %f %f %f %f %f %f",&t3,&t4,&t5,&t6,&t7,&t8,&t9,&t10);
               *ptsdn = t7;
               *pttsn  = t9;
               ptsdn++;
               pttsn++;
               fgets(line,80,outfile);
      sscanf(line,"%f %f %f %f %f %f %f %f",&t3,&t4,&t5,&t6,&t7,&t8,&t9,&t10);
               *ptsdp = t4;
               ptsdp++;
               *pttsp  = t6;
               pttsp++;
               *pttscod  = t9;
               pttscod++;
               }
            }
         if((strncmp(line,"SOIL_LOSS",9)) == 0) {
           for(i=0; i<=tcell-1; i++) {
               fgets(line,80,outfile);
               sscanf(line,"%d %d %f %f",&t1,&t2,&t3,&t4);
               *ptrunof = t4;
               ptrunof++;
               fgets(line,80,outfile);
               fgets(line,80,outfile);
               fgets(line,80,outfile);
               fgets(line,80,outfile);
               fgets(line,80,outfile);
               fgets(line,80,outfile);
               sscanf(line,"%f %f %f %f %f",&t3,&t4,&t5,&t6,&t7);
/*
MODIFIED TO BE CORRECT! BY MIKE FOSTER 4-2-96
*/
               *ptce = t3; /*cell erosion in tons per acre */
               *ptsedy = t6; /* sediment yield in tons*/ 
               ptce++;
               ptsedy++;
               }
            }
         }

    /* reset the pointers for the variables to their intial point*/
      ptsdn = ptsdn - tcell;
      pttsn  = pttsn  - tcell;
      ptsdp = ptsdp - tcell;
      pttsp  = pttsp  - tcell;
      pttscod  = pttscod - tcell;
      ptrunof  = ptrunof  - tcell;
      ptsedy = ptsedy - tcell;
      ptce = ptce - tcell; 
      fclose(outfile);


    /* Prepare rules for generating raster maps  for */
    /* the contaminents interested based on the level*/
    /* of contamination and a critirea.              */



    /* Create a mask file for file selection         */
      tfile = fopen("maps","w");
      fprintf(tfile,"This is the directory containing rules\n");
      fclose(tfile);


    /* nitrogen leaving the cell as sediment attachment*/
      temp1 = fopen("N_sed.rules","w");

    /* Total soluble nitrogen */
      temp2 = fopen("N_sol.rules","w");

    /* Phosphorus leaving the cell as sediment attachment*/
      temp3 = fopen("P_sed.rules","w");

    /* Total soluble phosphorus     */
      temp4 = fopen("P_sol.rules","w");

    /* Total Soluble COD in this cell                        */
      temp5 = fopen("COD.rules","w");

    /* Sediment  yield in this cell                  */

      temp6  = fopen("cellerosion.rules","w");
      temp7  = fopen("sedyield.rules","w");

    /* Title file for Nitrogen                       */
      temp8 = fopen("N.title","w");

    /* Title file for Phosphorus                     */
      temp9 = fopen("P.title","w");

    /* Title file for Sediment                       */
      temp10 = fopen("cellerosion.title","w");
      temp11 = fopen("sedyield.title","w");

    /* Title file for COD                            */
      temp12 = fopen("COD.title","w");



    /* Write title file for each contaminant         */

   /* ALTERED FOR MODIFIED VERSION
   MIKE FOSTER 2-18-96
   */

       fclose(temp8);
       fclose(temp9);
       fclose(temp10);
       fclose(temp11);
       fclose(temp12);

    /* generate rules for map reclassfication        */

       fprintf(temp1,"0 = 0    nodata\n");
       fprintf(temp2,"0 = 0    nodata\n");
       fprintf(temp3,"0 = 0    nodata\n");
       fprintf(temp4,"0 = 0    nodata\n");
       fprintf(temp5,"0 = 0    nodata\n");
       fprintf(temp6,"0 = 0    nodata\n");
       fprintf(temp7,"0 = 0    nodata\n");

      for(i=1; i<=tcell; i++) {

         runof = *ptrunof;

        /* for nitrogen in sediment  */
         t3 = *ptsdn;
         if(t3 <= 0.001) {
            t3 = 0.0;
            }
         t4 = t3;

/* 
USE unmodified AGNPS output instead of
AGNPS output divided by its threshold value
MIKE FOSTER 2-19-96
*/
         m = t4 * 100;

         fprintf(temp1,"%i",i);

         fprintf(temp1," =  %i",m);

         fprintf(temp1," %7.2f",t4);
         fprintf(temp1,"lbs/acre\n");


        /* for soluble nitrogen  */    
         t3 = *pttsn;
         if(t3 <= 0.001) {
            t3 = 0.0;
            }
         t4 = t3;
/* 
USE unmodified AGNPS output instead of
AGNPS output divided by its threshold value
MIKE FOSTER 2-19-96
*/
         m = t4 * 100;

         fprintf(temp2,"%i",i);

         fprintf(temp2," =  %i",m);

         fprintf(temp2," %7.2f",t4);
         fprintf(temp2,"lbs/acre\n");


        /* for phosphorus in sediment   */    
         t3 = *ptsdp;
         if(t3 <= 0.001) {
            t3 = 0.0;
            }
         t4 = t3;

/* 
USE unmodified AGNPS output instead of
AGNPS output divided by its threshold value
MIKE FOSTER 2-19-96
*/
         m = t4 * 100;

         fprintf(temp3,"%i",i);

         fprintf(temp3," =  %i",m);

         fprintf(temp3," %7.2f",t4);
         fprintf(temp3,"lbs/acre\n");

/* for soluble phosphorus  */    
         t3 = *pttsp;
         if(t3 <= 0.001) {
            t3 = 0.0;
            }
         t4 = t3;
/* 
USE unmodified AGNPS output instead of
AGNPS output divided by its threshold value
MIKE FOSTER 2-19-96
*/
         m = t4 * 100;


         fprintf(temp4,"%i",i);
         fprintf(temp4," =  %i",m);

         fprintf(temp4," %7.2f",t4);
         fprintf(temp4,"lbs/acre\n");


        /* for Total Soluble COD*/    
         t3 = *pttscod;
         if(t3 <= 0.001) {
            t3 = 0.0;
            }
         t4 = t3;

/* 
USE unmodified AGNPS output instead of
AGNPS output divided by its threshold value
MIKE FOSTER 2-19-96
*/
         m = t4 * 100;

         fprintf(temp5,"%i",i);

         fprintf(temp5," =  %i",m);

         fprintf(temp5," %7.2f",t4);
         fprintf(temp5,"lbs/acre\n");


        /* for cell erosion in tons per acre */
         t3 = *ptce;
         if(t3 <= 0.001) {
            t3 = 0.0;
            }
         t4 = t3;
         m = t4 * 100;
         fprintf(temp6,"%i",i);
         fprintf(temp6," =  %i",m);
         fprintf(temp6," %7.2f",t4);
         fprintf(temp6,"tons/acre\n");

         /* for sediment yield in tons */
         t3 = *ptsedy;
         if(t3 <= 0.001) {
           t3 = 0.0;
           }
         t4 = t3;
         m = t4 * 100;
         fprintf(temp7,"%i",i);
         fprintf(temp7," = %i",m);
         fprintf(temp7," %7.2f",t4);
         fprintf(temp7,"tons\n");

         ptsdn++;
         pttsn++;
         ptsdp++;
         pttsp++;
         pttscod++;
         ptrunof++;
         ptce++;
         ptsedy++;
         }



    /* Close the files opened                    */

         fclose(temp1);
         fclose(temp2);
         fclose(temp3);
         fclose(temp4);
         fclose(temp5);
         fclose(temp6);
         fclose(temp7);

      return;  
      }
