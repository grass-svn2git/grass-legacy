#include "gis.h"
#include <stdio.h>
#include "dig_structs.h"
#include "dig_defines.h"
#include "Vect.h"

int
where_am_i(name,mapset,color,fill,Dvect_color)
    char *name;
    char *mapset;
    char *color;
    int fill;
    char *Dvect_color;
{
    int screen_x, screen_y ;
    double east, north ;
    int button ;
    double D_get_d_north(), D_get_d_south() ;
    double D_get_d_east(), D_get_d_west() ;
    double D_d_to_u_row(), D_d_to_u_col() ;
    struct Map_info map;
    struct line_pnts *Points;
    int line_count;
    int number;
    int color_num;
    int area_perim();

/* initialize structure pointers */
    Points   =  Vect_new_line_struct ();
    G_begin_distance_calculations();
    G_begin_polygon_area_calculations();

    if (2 > Vect_open_old (&map, name,mapset))
	G_fatal_error ("Must run v.support on vector file first");

/* check to determine if "color" is a valid color */
    color_num = D_translate_color(color);
    if (color_num == 0)
     {
      fprintf(stderr,"\nColor:  '%s' is NOT a valid color.\n",color);
      fprintf(stderr,"The default color 'red' will be used instead.\n");
      fprintf(stderr,"Valid colors are:\n");
      fprintf(stderr,"white black yellow blue red green");
      fprintf(stderr," orange grey magenta aqua indigo violet brown\n\n");
     }

    screen_x = ((int)D_get_d_west()  + (int)D_get_d_east() ) / 2 ;
    screen_y = ((int)D_get_d_north() + (int)D_get_d_south()) / 2 ;

    fprintf(stderr, "\n\n\nButtons:\n") ;
    fprintf(stderr, "Left:   get area/perimeter\n") ;
    fprintf(stderr, "Middle: quit this\n");
    fprintf(stderr, "Right:  get area/perimeter\n\n") ;

    line_count = 8;
    do
    {
	R_get_location_with_pointer(&screen_x, &screen_y, &button) ;
	east = D_d_to_u_col((double)screen_x) ;
	north = D_d_to_u_row((double)screen_y) ;
        if (button != 2)
         {
          if (line_count > 22)
           {
	    fprintf(stderr, "\n\n\nButtons:\n") ;
            fprintf(stderr, "Left:   get area/perimeter\n") ;
            fprintf(stderr, "Middle: quit this\n");
            fprintf(stderr, "Right:  get area/perimeter\n\n") ;
            fprintf(stderr, "\n\n\n\n\n");
            fprintf(stderr,"POINT AT CROSS HAIRS:  %13.2f (N) %13.2f (E)\n",
            north,east);
            line_count = 13;
            number = area_perim(north,east,&map,Points,mapset,name,color,fill,Dvect_color);
            if (number == -1)
             {
              Vect_close(&map); 
              return;
             }
            if (number == 6)
             {
              fprintf(stderr,"\n\n");
              number += 4;
             }
            line_count += number;
           }
          else
           {
            fprintf(stderr,"POINT AT CROSS HAIRS:  %13.2f (N) %13.2f (E)\n",
            north,east);
            number = area_perim(north,east,&map,Points,mapset,name,color,fill,Dvect_color);
            if (number == -1)
             {
              Vect_close(&map); 
              return;
             }
            if (number == 6)
              line_count = 17;
            line_count += number;
           }
         }
	if(button == 2)
         {
          Vect_close(&map); 
	  return(0) ;
         }
    } while (button != 2) ;
}
