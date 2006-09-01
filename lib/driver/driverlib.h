
#define DEF_WIDTH  640
#define DEF_HEIGHT 480

#include "driver.h"

extern const struct driver *driver;

/* Utility Functions */

/* Color.c */
void color(int);
int get_color_offset(void);
/* Font.c */
int font_is_freetype(void);
/* Reset_colors.c */
void reset_color(int,int,int,int);
/* Set_window.c */
int window_clip(double *,double *,double *,double *);
int window_box_clip(double *,double *,double *,double *);
/* Text2.c */
void drawchar(double,double,double,double,unsigned char);
void soft_text_ext(int x,int,double,double,double,const char *);
void get_text_ext (int *,int *,int *,int *);
void soft_text(int,int,double,double,double,const char *);
void onechar(int,int,double,double,double,unsigned char);
/* Text3.c */
void soft_text_freetype(int,int,double,double,double,const char *);
void soft_text_ext_freetype(int,int,double,double,double,const char *);
void get_text_ext_freetype(int*,int*,int*,int*);

/* clip.c */
int clip(double,double,double,double,double *,double *,double *,double *);
/* color_support.c */
int get_fixed_color(int);
void get_fixed_color_array(int *,const int *,int);
void assign_standard_color(int,int);
int get_standard_color(int);
int get_max_std_colors(void);
/* font.c */
int font_init(const char *);
int get_char_vects(unsigned char,int *,unsigned char **,unsigned char **);
/* font_freetype.c */
int font_init_freetype(const char *);
int font_init_charset(const char *);
const char *font_get_freetype_name(void);
const char *font_get_charset(void);

/* connect_{sock,fifo}.c */
#ifdef USE_G_SOCKS
int get_connection_sock(int,int *,int *,int);
int prepare_connection_sock(const char *,const char *);
#else
int get_connection_fifo(const char *,int *,int *,int);
#endif /* USE_G_SOCKS */
int check_connection(const char *,const char *);

/* command.c */
void command_init(int,int);
int get_command(char *);
int process_command(int);

/* parse_ftcap.c */
extern struct FT_CAP *parse_freetypecap(void);
extern void free_freetypecap(struct FT_CAP *ftcap);
