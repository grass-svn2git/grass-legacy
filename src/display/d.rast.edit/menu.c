/*
   menus.c - popup menus
   
   Chris Rewerts, Agricultural Engineering, Purdue University
   May 1991

 */

#define GLOBAL
#include "edit.h"
char new_color[28];
char arrow_layer[128];

main_menu()
{
    static char *options[] = {
        " MAIN MENU",
        "  edit",
        "  redraw",
        "  zoom",
        "  arrow",
        "  number",
        "  options",
        "  exit",
        NULL };
    int background_color;
    int text_color;
    int div_color;
    int answer;

    background_color = D_translate_color("indigo");
    text_color = D_translate_color("white");
    div_color = D_translate_color("blue");
    R_font("romant");

    use_mouse();

   for(;;)
    {
        answer = D_popup(
               background_color,
               text_color,
               div_color,
               80,
               5,
               3,
               options
               );
    switch(answer)
    {
    case 1: 
    /* edit */
            change_made = 0;
            make_temporary_file();
            edit();
            if (change_made) 
                {
                make_new_cell_layer();
                
                if(strcmp(current_name, orig_name) == 0) 
                    {
                    strcpy(current_name, new_name);
                    strcpy(current_mapset, user_mapset);
                    }
                Dcell(current_name, current_mapset, 0);
                }
            use_mouse();
            break;
    case 2:
    /* redraw */
           R_close_driver();
           R_open_driver();
           Dcell(current_name, current_mapset, 0);
           use_mouse();
           break;
    case 3:
    /* zoom */
           R_close_driver();
           G_system("d.rast.zoom");
           R_open_driver();
           use_mouse();
           break;
    case 4:
    /* arrow */
           get_arrow_inputs();
           use_mouse();
           break;
        
    case 5:
    /* number */
           R_close_driver();
           G_system("d.rast.num g=black");
           R_open_driver();
           use_mouse();
           break;
        
    case 6:
    /* options */
           option_menu();
           use_mouse();
           break;

    case 7:
    /* exit */
          Dcell(orig_name, orig_mapset, 0);
          return(0);
    default:
           break;
    }
    }
}
/*---------------------------------------------------------------*/
option_menu()
{
    static char *options[] = {
        " OPTIONS",
        "  grid color",
        "  exit",
        NULL };
    int background_color;
    int text_color;
    int div_color;
    int answer;

    background_color = D_translate_color("indigo");
    text_color = D_translate_color("white");
    div_color = D_translate_color("blue");
    R_font("romant");

   for(;;)
    {
        answer = D_popup(
               background_color,
               text_color,
               div_color,
               80,
               15,
               3,
               options
               );
    switch(answer)
    {
    case 1: 
    /* grid color */
            color_menu("SELECT GRID COLOR");
            grid_color = D_translate_color(new_color);
            break;
    case 2:
    /* exit */

           return(0);
    default:
           break;
    }
    return(0);
    }
}

/*---------------------------------------------------------------*/
color_menu(title)
char *title;
{

  /* we vary the usage of the menu a bit to allow for a changeable
     title. We initialize with spaces then copy in the title later
  */ 
   static char *options[] = {
        "                    ",
        "      red",
        "      orange",
        "      yellow",
        "      green",
        "      blue",
        "      indigo",
        "      violet",
        "      gray",
        "      white",
        "      black",
        NULL };
    int background_color;
    int text_color;
    int div_color;
    int answer;

    if(strlen(title) > 20)
        error(1, "color_menu: title too long");
   
    strcpy(options[0], title);

    background_color = D_translate_color("indigo");
    text_color = D_translate_color("white");
    div_color = D_translate_color("blue");
    R_font("romant");

   for(;;)
    {
        answer = D_popup(
               background_color,
               text_color,
               div_color,
               80,
               25,
               3,
               options
               );
    switch(answer)
    {
    case 1: 
            strcpy(new_color, "red");
            break;
    case 2: 
            strcpy(new_color, "orange");
            break;
    case 3: 
            strcpy(new_color, "yellow");
            break;
    case 4: 
            strcpy(new_color, "green");
            break;
    case 5: 
            strcpy(new_color, "blue");
            break;
    case 6: 
            strcpy(new_color, "indigo");
            break;
    case 7: 
            strcpy(new_color, "violet");
            break;
    case 8: 
            strcpy(new_color, "gray");
            break;
    case 9: 
            strcpy(new_color, "white");
            break;
    case 10: 
            strcpy(new_color, "black");
            break;
    default:
           break;
    }
    return(0);
    }
}

/*---------------------------------------------------------------*/
map_type_menu()
{
    static char *options[] = {
        " ASPECT MAP TYPE",
        "  grass",
        "  agnps",
        "  answers",
        "  exit",
        NULL };
    int background_color;
    int text_color;
    int div_color;
    int answer;

    background_color = D_translate_color("indigo");
    text_color = D_translate_color("white");
    div_color = D_translate_color("blue");
    R_font("romant");

   for(;;)
    {
        answer = D_popup(
               background_color,
               text_color,
               div_color,
               80,
               15,
               3,
               options
               );
    switch(answer)
    {
    case 1: 
            strcpy(new_color, "grass");
            return(1);
    case 2:
            strcpy(new_color, "agnps");
            return(2);
    case 3:
            strcpy(new_color, "answers");
            return(3);
    case 4:
            return(-1);
    default:
            return(-1);
    }
    }
}
/*---------------------------------------------------------------*/
arrow_options()
{
    static char *options[] = {
        "SET PROGRAM OPTIONS?",
        "  NO:  use default options",
        "  YES: set options now",
        NULL };
    int background_color;
    int text_color;
    int div_color;
    int answer;

    background_color = D_translate_color("indigo");
    text_color = D_translate_color("white");
    div_color = D_translate_color("blue");
    R_font("romant");

   for(;;)
    {
        answer = D_popup(
               background_color,
               text_color,
               div_color,
               80,
               15,
               3,
               options
               );
    switch(answer)
    {
    case 1: 
            return(0);
    case 2:
            return(1);
    default:
           return(0);;
    }
    }
}
/*---------------------------------------------------------------*/

get_arrow_inputs()
{

    char line[198];
    char map_type[28], arrow_color[28], x_color[28], unknown_color[28];
    int m_type;

           m_type = map_type_menu();
           if(m_type < 1)
               return(0);
           strcpy(map_type, new_color);
           if(arrow_options())
               {
               arrow_map();
               strcpy(map_type, new_color);
               color_menu("COLOR FOR ARROWS");
               strcpy(arrow_color, new_color);
               color_menu("COLOR FOR X's");
               strcpy(x_color, new_color);
               color_menu("COLOR FOR ?'s");
               strcpy(unknown_color, new_color);
               }
           else
           /* set defaults */
               {
               strcpy(arrow_layer, "-");
               switch(m_type)
                   {
                   case 1:
                       strcpy(arrow_color, "green");
                       strcpy(x_color, "white");
                       strcpy(unknown_color, "red");
                       break;
                   case 2:
                       strcpy(arrow_color, "black");
                       strcpy(x_color, "white");
                       strcpy(unknown_color, "red");
                       break;
                   case 3:
                       strcpy(arrow_color, "green");
                       strcpy(x_color, "black");
                       strcpy(unknown_color, "white");
                       break;
                   default:
                       strcpy(arrow_color, "green");
                       strcpy(x_color, "white");
                       strcpy(unknown_color, "red");
                       break;
                  }
               }
           sprintf(line, "d.rast.arrow type=%s arrow=%s x=%s unk=%s",
           map_type, arrow_color, x_color, unknown_color);
printf(line);
           R_close_driver();
           G_system(line);
           R_open_driver();

}

/*---------------------------------------------------------------*/
arrow_map()
{
    static char *options[] = {
        "USE DISPLAYED MAP AS INPUT?",
        "  NO:  enter other name now",
        "  YES: use displayed map",
        NULL };
    int background_color;
    int text_color;
    int div_color;
    int answer;
    char *mapset;

    background_color = D_translate_color("indigo");
    text_color = D_translate_color("white");
    div_color = D_translate_color("blue");
    R_font("romant");

   for(;;)
    {
        answer = D_popup(
               background_color,
               text_color,
               div_color,
               80,
               15,
               3,
               options
               );
    switch(answer)
    {
    case 1: 
            fprintf(stderr, "\n     +-------------------------------------------+\n");
            fprintf(stderr, "     |            Text input needed              |\n");
            fprintf(stderr, "     +-------------------------------------------+\n\n");
            if ((mapset = G_ask_cell_old("Enter name of aspect map to use for arrows",
                arrow_layer)) ==NULL)
            {
                error(0, "cell layer not found");
            }
            return(0);
    case 2:
            strcpy(arrow_layer, "-");
            return(0);
    default:
           return(0);;
    }
    }
}
