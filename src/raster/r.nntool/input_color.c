#include "globals.h"

#define INP_COLOR 2

int Display_color = NUM_GREEN;
char *Display_color_name = NAME_GREEN;
static int use = 1;

input_color()
{
  int red();
  int green();
  int blue();
  int grey();
  int black();
  int yellow();
  int brown();
  int orange();
  int purple();
  int white();

  static Objects objects[] =
    {
      INFO("Display Color: ",&use),
      MENU(" Green ",green,&use),
      MENU(" Red ",red, &use),
      MENU(" Blue ",blue, &use),
      MENU(" Yellow ",yellow,&use),
      MENU(" Orange ",orange,&use),
      MENU(" Brown ",brown,&use),
      MENU(" Purple ",purple,&use),
      MENU(" White ",white,&use),
      MENU(" Grey ",grey,&use),
      MENU(" Black ",black,&use),
      {0}
    };
  
  Input_pointer (objects);
  Menu_msg ("");
  return(INP_COLOR);
}



int green()
{
  Display_color = NUM_GREEN;
  Display_color_name = NAME_GREEN;
  return (1);
}

int red()
{
  Display_color = NUM_RED;
  Display_color_name = NAME_RED;
  return (1);
}

int blue()
{
  Display_color = NUM_BLUE;
  Display_color_name = NAME_BLUE;
  return (1);
}

int yellow()
{
  Display_color = NUM_YELLOW;
  Display_color_name = NAME_YELLOW;
  return (1);
}

int orange()
{
  Display_color = NUM_ORANGE;
  Display_color_name = NAME_ORANGE;
  return (1);
}

int brown()
{
  Display_color = NUM_BROWN;
  Display_color_name = NAME_BROWN;
  return (1);
}

int purple()
{
  Display_color = NUM_PURPLE;
  Display_color_name = NAME_PURPLE;
  return (1);
}

int white()
{
  Display_color = NUM_WHITE;
  Display_color_name = NAME_WHITE;
  return (1);
}

int grey()
{
  Display_color = NUM_GREY;
  Display_color_name = NAME_GREY;
  return (1);
}

int black()
{
  Display_color = NUM_BLACK;
  Display_color_name = NAME_BLACK;
  return (1);
}
