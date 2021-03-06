#include  "gis.h"

G_ask_ellipse_name(spheriod)

char *spheriod;
{ 
	char buff[1024],answer[50];
        double aa,e2;
	char  *sph, *Tmp_file;
        FILE  *Tmp_fd = NULL;
	int  i;

        Tmp_file = G_tempfile ();
        if (NULL == (Tmp_fd = fopen (Tmp_file, "w"))) {
	    G_fatal_error("Cannot open temp file") ;
        }
        fprintf(Tmp_fd,"sphere\n");
        for (i=0; sph = G_ellipsoid_name(i); i++) {
          fprintf(Tmp_fd,"%s\n",sph);
        }

        fclose(Tmp_fd);

        for(;;) {
	  do {
	      fprintf(stderr,"\nPlease specify ellipsoid name\n");
	      fprintf(stderr,"Enter 'list' for the list of available ellipsoids\n");
	      fprintf (stderr, "Hit RETURN to cancel request\n");
	      fprintf(stderr,">");
          } while(!G_gets(answer));
          G_strip(answer); 
          if(strlen(answer)==0) return -1;
          if (strcmp(answer,"list") == 0) {
            if (isatty(1)) {
	      sprintf(buff,"more %s",Tmp_file);
            }
            else
	      sprintf(buff,"cat %s",Tmp_file);
            system(buff);
          }
          else {
            if (strcmp(answer,"sphere") == 0) break; 
            if (G_get_ellipsoid_by_name(answer,&aa,&e2) == 0) {
	      fprintf(stderr,"\ninvalid ellipsoid\n");
            }
            else break;
          }
        }
        sprintf(spheriod,"%s",answer);
        unlink(Tmp_file);
        if (strcmp(spheriod,"sphere") == 0) {
          return 2;
        }
        return 1;
}
