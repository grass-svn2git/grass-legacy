
#include <stdio.h>
#include "gis.h"
#include "extr_areas.h"

static int col, row, top, bottom;
static CELL tl, tr, bl, br;
static struct COOR **v_list;
static struct COOR *h_ptr;
static CELL *buffer[2];
static int scan_length;
static int n_areas, area_num, n_equiv;
static struct area_table *a_list, *a_list_new, *a_list_old;
static struct equiv_table *e_list;

/* extract_areas - trace boundaries of polygons in file */

o_extract_areas()
{
  row = col = top = 0;			/* get started for read of first */
  bottom = 1;				/*   line from cell file */
  area_num = 0;
  scan_length = read_next();
  while (read_next())			/* read rest of file, one row at */
  {					/*   a time */
    for (col = 0; col < scan_length - 1; col++)
    {
      tl = *(buffer[top] + col);	/* top left in window */
      tr = *(buffer[top] + col + 1);	/* top right */
      bl = *(buffer[bottom] + col);	/* bottom left */
      br = *(buffer[bottom] + col + 1);	/* bottom right */
      update_list(nabors());
    }
    if (h_ptr != NULPTR)		/* if we have a loose end, */
      end_hline();			/*   tie it down */
    row++;
  }
  o_write_area(a_list,e_list,area_num,n_equiv);
  free(a_list);
  free(e_list);
}					/* extract_areas */

/* update_list - maintains linked list of COOR structures which resprsent */
/* bends in and endpoints of lines separating areas in input file; */
/* compiles a list of area to category number correspondences; */
/* for pictures of what each case in the switch represents, see comments */
/* before nabors() */

static 
update_list(i)
    int i;
{
  struct COOR *new_ptr, *new_ptr1, *new_ptr2, *new_ptr3;
  struct COOR *get_ptr();
  CELL right, left;

  switch (i)
  {
    case 0:
      /* Vertical line - Just update width information */
      update_width(a_list + v_list[col]->left,0);
      break;
    case 1:
      /* Bottom right corner - Point in middle of new line */
      /* (growing) <- ptr2 -><- ptr1 -><- ptr3 -> (growing) */
      /*            (?, col) (row, col) (row, ?) */
      new_ptr1 = get_ptr();		/* corner point */
      new_ptr2 = get_ptr();		/* downward-growing point */
      new_ptr3 = get_ptr();		/* right-growing point */
      new_ptr1->bptr = new_ptr2;
      new_ptr1->fptr = new_ptr3;
      new_ptr2->bptr = new_ptr3->bptr = new_ptr1;
      new_ptr1->left = new_ptr2->right = new_ptr3->left = area_num;
      assign_area(tl,1);
      new_ptr1->right = new_ptr2->left = new_ptr3->right = area_num;
      assign_area(br,1);
      update_width(a_list_old,1);
      v_list[col] = new_ptr2;
      h_ptr = new_ptr3;
      break;
    case 3:
      /* Bottom left corner - Add point to line already under construction */
      /* (fixed) -><- original h_ptr -><- new_ptr -> (growing) */
      /*                (row, col)       (?, col) */
      new_ptr = get_ptr();		/* downward-growing point */
      h_ptr->col = col;
      h_ptr->fptr = new_ptr;
      new_ptr->bptr = h_ptr;
      new_ptr->left = h_ptr->left;
      new_ptr->right = h_ptr->right;
      update_width(a_list + new_ptr->left,3);
      v_list[col] = new_ptr;
      h_ptr = NULPTR;
      break;
    case 4:
      /* Top left corner - Join two lines already under construction */
      /* (fixed) -><- original v_list -><- (fixed) */
      /*                 (row, col) */
      equiv_areas(h_ptr->left,v_list[col]->right);
      equiv_areas(h_ptr->right,v_list[col]->left);
      v_list[col]->row = row;		/* keep downward-growing point */
      v_list[col]->fptr = h_ptr->bptr;	/*   and join it to predecessor */
      h_ptr->bptr->fptr = v_list[col];	/*   of right-growing point */
      free(h_ptr);			/* right-growing point disappears */
      o_write_line(v_list[col]);		/* try to write line */
      v_list[col] = h_ptr = NULPTR;	/* turn loose of pointers */
      break;
    case 5:
      /* Top right corner - Add point to line already under construction */
      /* (fixed) -><- original v_list -><- new_ptr -> (growing) */
      /*                 (row, col)        (row, ?) */
      new_ptr = get_ptr();		/* right-growing point */
      v_list[col]->row = row;
      new_ptr->bptr = v_list[col];
      new_ptr->left = v_list[col]->left;
      new_ptr->right = v_list[col]->right;
      v_list[col]->fptr = new_ptr;
      h_ptr = new_ptr;
      v_list[col] = NULPTR;
      break;
    case 6:
      /* T upward - End one vertical and one horizontal line */
      /*            Start horizontal line */
      v_list[col]->node = h_ptr->node = 1;
      left = v_list[col]->left;
      right = h_ptr->right;
      end_vline();
      end_hline();
      start_hline();
      h_ptr->bptr->node = 1;		/* where we came from is a node */
      h_ptr->left = h_ptr->bptr->left = left;
      h_ptr->right = h_ptr->bptr->right = right;
      break;
    case 7:
      /* T downward - End horizontal line */
      /*              Start one vertical and one horizontal line */
      h_ptr->node = 1;
      right = h_ptr->right;
      left = h_ptr->left;
      end_hline();
      start_hline();
      start_vline();
      h_ptr->bptr->node = v_list[col]->bptr->node = 1;
      h_ptr->left = h_ptr->bptr->left = left;
      h_ptr->right = h_ptr->bptr->right = v_list[col]->left = v_list[col]->bptr->left = area_num;
      assign_area(br,7);
      update_width(a_list_old,7);
      v_list[col]->right = v_list[col]->bptr->right = right;
      break;
    case 8:
      /* T left - End one vertical and one horizontal line */
      /*          Start one vertical line */
      h_ptr->node = v_list[col]->node = 1;
      right = h_ptr->right;
      left = v_list[col]->left;
      end_vline();
      end_hline();
      start_vline();
      v_list[col]->bptr->node = 1;	/* where we came from is a node */
      v_list[col]->left = v_list[col]->bptr->left = left;
      v_list[col]->right = v_list[col]->bptr->right = right;
      update_width(a_list + v_list[col]->left,8);
      break;
    case 9:
      /* T right - End one vertical line */
      /*           Start one vertical and one horizontal line */
      v_list[col]->node = 1;
      right = v_list[col]->right;
      left = v_list[col]->left;
      end_vline();
      start_vline();
      start_hline();
      v_list[col]->bptr->node = h_ptr->bptr->node = 1;
      h_ptr->left = h_ptr->bptr->left = left;
      h_ptr->right = h_ptr->bptr->right = v_list[col]->left = v_list[col]->bptr->left = area_num;
      assign_area(br,9);
      update_width(a_list_old,9);
      v_list[col]->right = v_list[col]->bptr->right = right;
      break;
    case 10:
      /* Cross - End one vertical and one horizontal line */
      /*         Start one vertical and one horizontal line */
      v_list[col]->node = h_ptr->node = 1;
      left = v_list[col]->left;
      right = h_ptr->right;
      end_vline();
      end_hline();
      start_vline();
      start_hline();
      v_list[col]->bptr->node = h_ptr->bptr->node = 1;
      h_ptr->left = h_ptr->bptr->left = left;
      v_list[col]->left = v_list[col]->bptr->left = h_ptr->right = h_ptr->bptr->right = area_num;
      assign_area(br,10);
      update_width(a_list_old,10);
      v_list[col]->right = v_list[col]->bptr->right = right;
      break;
  }						/* switch */
}						/* update_list */

/* end_vline, end_hline - end vertical or horizontal line and try */
/* to write it out */

static 
end_vline()
{
  v_list[col]->row = row;
  v_list[col]->fptr = v_list[col];
  o_write_line(v_list[col]);
  v_list[col] = NULPTR;
}

static 
end_hline()
{
  h_ptr->col = col;
  h_ptr->fptr = h_ptr;
  o_write_line(h_ptr);
  h_ptr = NULPTR;
}

/* start_vline, start_hline - begin line in vertical or horizontal */
/* direction */

static 
start_vline()
{
  struct COOR *new_ptr1, *new_ptr2;
  struct COOR *get_ptr();

  new_ptr1 = get_ptr();
  new_ptr2 = get_ptr();
  new_ptr1->fptr = new_ptr2;
  new_ptr2->bptr = new_ptr1->bptr = new_ptr1;
  new_ptr2->fptr = NULPTR;
  v_list[col] = new_ptr2;
}

static 
start_hline()
{
  struct COOR *new_ptr1, *new_ptr2;
  struct COOR *get_ptr();

  new_ptr1 = get_ptr();
  new_ptr2 = get_ptr();
  new_ptr1->bptr = new_ptr2->bptr = new_ptr1;
  new_ptr1->fptr = new_ptr2;
  new_ptr2->fptr = NULPTR;
  h_ptr = new_ptr2;
}

/* get_ptr - allocate storage for yet another COOR structure */

static struct COOR *
get_ptr()
{
  static struct COOR *ptr;
  char *G_malloc();

  ptr = (struct COOR *) G_malloc(sizeof (struct COOR));
  if (ptr == NULPTR)
  {
    fprintf(stderr,"OUT OF MEMORY\n");
    exit(-1);
  }
  ptr->row = row;
  ptr->col = col;
  ptr->fptr = ptr->bptr = NULPTR;
  ptr->node = ptr->left = ptr->right = 0;
  return(ptr);
}

/* nabors - check 2 x 2 matrix and return case from table below */

/*    *--*--*      *--*--*      *--*--*      *--*--*    */
/*    |  |  |      |     |      |     |      |     |    */
/*    *  |  *      *  *--*      *-----*      *--*  *    */
/*    |  |  |      |  |  |      |     |      |  |  |    */
/*    *--*--*      *--*--*      *--*--*      *--*--*    */

/*       0            1            2            3       */

/*    *--*--*      *--*--*      *--*--*      *--*--*    */
/*    |  |  |      |  |  |      |  |  |      |     |    */
/*    *--*  *      *  *--*      *--*--*      *--*--*    */
/*    |     |      |     |      |     |      |  |  |    */
/*    *--*--*      *--*--*      *--*--*      *--*--*    */

/*       4            5            6            7       */

/*    *--*--*      *--*--*      *--*--*      *--*--*    */
/*    |  |  |      |  |  |      |  |  |      |     |    */
/*    *--*  *      *  *--*      *--*--*      *     *    */
/*    |  |  |      |  |  |      |  |  |      |     |    */
/*    *--*--*      *--*--*      *--*--*      *--*--*    */
/*							*/
/*       8            9            10           11      */

nabors()
{
  if (tl != tr)				/* 0, 4, 5, 6, 8, 9, 10 */
  {
    if (tl != bl)			/* 4, 6, 8, 10 */
    {
      if (bl != br)			/* 8, 10 */
      {
        if (tr != br)
          return(10);
        else
          return(8);
      }
      else				/* 4, 6 */
      {
        if (tr != br)
          return(6);
        else
          return(4);
      }
    }
    else				/* 0, 5, 9 */
    {
      if (bl != br)			/* 0, 9 */
      {
        if (tr != br)
          return(9);
        else
          return(0);
      }
      else
        return(5);
    }
  }
  else					/* 1, 2, 3, 7, 11 */
  {
    if (tl != bl)			/* 2, 3, 7 */
    {
      if (bl != br)			/* 3, 7 */
      {
        if (tr != br)
          return(7);
        else
          return(3);
      }
      else
        return(2);
    }
    else				/* 1, 11 */
    {
      if (bl != br)
        return(1);
      else
        return(11);
    }
  }
}

/* read_next - read another line from input file */

static int 
read_next()
{
  int n;

  top = bottom++;			/* switch top and bottom, */
  bottom = 1 & bottom;			/*   which are always 0 or 1 */
  n = o_read_row(buffer[bottom]);
  return(n);
}

/* alloc_bufs - allocate buffers we will need for storing cell file */
/* data, pointers to extracted lines, area number information */

o_alloc_bufs(size)
    int size;
{
  char *G_calloc();
  int i;

  buffer[0] = (CELL *) G_calloc(size, sizeof(CELL));
  buffer[1] = (CELL *) G_calloc(size, sizeof(CELL));
  v_list = (struct COOR **) G_malloc(size * sizeof(*v_list));
  n_areas = n_equiv = 500;		/* guess at number of areas, equivs */
  a_list = (struct area_table *) G_malloc(n_areas * sizeof(struct area_table));
  for (i = 0; i < n_areas; i++)
  {
    (a_list + i)->width = (a_list + i)->row = (a_list + i)->col = 0;
    (a_list + i)->free = 1;
  }
  a_list_new = a_list_old = a_list;
  e_list = (struct equiv_table *) G_malloc(n_equiv * sizeof(struct equiv_table));
  for (i = 0; i < n_equiv; i++)
  {
    (e_list + i)->mapped = (e_list + i)->count = 0;
    (e_list + i)->ptr = NULL;
  }
}

/* equiv_areas - force two areas to be equivalent and generate */
/* mapping information */

static int 
equiv_areas(a1,a2)
    int a1, a2;
{
  int small, large, small_obj, large_obj;

  if (a1 == a2)
    return(0);
  if (a1 < a2)
  {
    small = a1;
    large = a2;
  }
  else
  {
    small = a2;
    large = a1;
  }
  while (large >= n_equiv)		/* make sure our equivalence tables */
    more_equivs();			/*   are large enough */
  if ((e_list + large)->mapped)
  {
    if ((e_list + small)->mapped)	/* small mapped, large mapped */
    {
      large_obj = (e_list + large)->where;
      small_obj = (e_list + small)->where;
      if (large_obj == small_obj)	/* both mapped to same place */
        return(0);
      if (small_obj < large_obj)	/* map where large goes to where */
        map_area(large_obj,small_obj);	/*   small goes */
      else				/* map where small goes to where */
        map_area(small_obj,large_obj);	/*   large goes */
    }
    else				/* small not mapped, large mapped */
    {
      large_obj = (e_list + large)->where;
      if (small == large_obj)		/* large already mapped to small */
        return(0);
      if (small < large_obj)		/* map where large goes to small */
        map_area(large_obj,small);
      else				/* map small to where large goes */
        map_area(small,large_obj);
    }
  }
  else
  {
    if ((e_list + small)->mapped)	/* small mapped, large not mapped */
      map_area(large,(e_list + small)->where);
    else				/* small not mapped, large not mapped */
      map_area(large,small);
  }
  return (0);
}

/* map_area - establish a mapping from one area to another */
/* the area number mapping looks like the following: */
/*   area numbers index into e_list to get equiv_table structures */
/*   for each of these structures, */
/*   .mapped indicates whether area number is mapped */
/*   if area number is mapped */
/*     .where tells to what other area number */
/*   if area number is not mapped */
/*     .count tells how many areas are mapped to this one */
/*     if count != 0 */
/*       .ptr gives a pointer to the list of area numbers */

static int 
map_area(x,y)
    int x, y;				/* map x to y */
{
  int n, i, *p;

  (e_list + x)->mapped = 1;
  (e_list + x)->where = y;
  if ((a_list + x)->width > (a_list + y)->width)
  {
    (a_list + y)->width = (a_list + x)->width;
    (a_list + y)->row = (a_list + x)->row;
    (a_list + y)->col = (a_list + x)->col;
  }
  if (add_to_list(x,y))			/* if x is not already in y's list */
  {
    n = (e_list + x)->count;
    p = (e_list + x)->ptr;
    for (i = 0; i < n; i++)		/* map everything that is currently */
    {					/*   mapped onto x onto y; because */
      (e_list + *p)->where = y;		/*   of this reshuffle, only one */
      add_to_list(*p++,y);		/*   level of mapping is ever needed */
    }
  }
}

/* add_to_list - add another area number to an equivalence list */

static int 
add_to_list(x,y)
    int x, y;
{
  int n, i;
  struct equiv_table *e_list_y;

  e_list_y = e_list + y;
  n = e_list_y->count;
  if (n == 0)				/* first time through--start list */
  {
    e_list_y->length = 20;		/* initial guess at storage needed */
    e_list_y->ptr = (int *) G_malloc(e_list_y->length * sizeof(int));
    *(e_list_y->ptr) = x;
    (e_list_y->count)++;
  }
  else					/* list already started */
  {
    for (i = 0; i < n; i++)		/* check whether x is already */
    {					/*   in list */
      if (*(e_list_y->ptr + i) == x)
        return(0);			/* if so, we don't need to add it */
    }
    if (n + 1 >= e_list_y->length)	/* add more space for storage */
    {					/*   if necessary */
      e_list_y->length += 10;
      e_list_y->ptr = (int *) G_realloc(e_list_y->ptr,e_list_y->length * sizeof(int));
    }
    *(e_list_y->ptr + n) = x;		/* add x to list */
    (e_list_y->count)++;
  }
  return(1);				/* indicate addition made */
}

/* assign_area - make current area number correspond to the passed */
/* category number and allocate more space to store areas if necessary */

static int 
assign_area(cat,kase)
    CELL cat;
    int kase;
{
  a_list_new->free = 0;
  a_list_new->cat = cat;
  area_num++;
  if (area_num >= n_areas)
    more_areas();
  a_list_old = a_list + area_num - 1;
  a_list_new = a_list + area_num;
}

/* more_areas - allocate larger space to store area correspondences */

static int 
more_areas()
{
  char *G_realloc();
  int old_n, i;

  old_n = n_areas;
  n_areas += 250;
  a_list = (struct area_table *) G_realloc(a_list,n_areas * sizeof(struct area_table));
  for (i = old_n; i < n_areas; i++)
  {
    (a_list + i)->width = -1;
    (a_list + i)->free = 1;
  }
}

/* more_equivs - allocate more space to construct equivalence information */

more_equivs()
{
  char *G_realloc();
  int old_n, i;

  old_n = n_equiv;
  n_equiv += 250;
  e_list = (struct equiv_table *) G_realloc(e_list,n_equiv * sizeof(struct equiv_table));
  for (i = old_n; i < n_equiv; i++)
  {
    (e_list + i)->mapped = (e_list + i)->count = 0;
    (e_list + i)->ptr = NULL;
  }
}

/* update_width - update position of longest horizontal strip in an area */

static int 
update_width(ptr,kase)
    struct area_table *ptr;
    int kase;
{
  int w, j, a;
  struct equiv_table *ep;

  a = ptr - a_list;
  ep = e_list + a;
  for (j = col + 1, w = 0; *(buffer[bottom] + j) == br && j < scan_length; j++, w++)
  { }
  if (a < n_equiv)
  {
    if (ep->mapped)
      ptr = a_list + ep->where;
  }
  if (w > ptr->width)
  {
    ptr->width = w;
    ptr->row = row;
    ptr->col = col;
  }
}
