/* sos/rho2_num.c */

#if !defined(lint)  /* otherwise, it's too slow */

/*--------------------------------------------------------------------------*/

#include "geom/basic.h"
#include "geom/sos.h"
#include "internal.h"
#include "primitive.h"

/*--------------------------------------------------------------------------*/

Static_Declarations ("rho2_num", 3, 54, 6);

#include "../sos/primitive.i.c"

/*--------------------------------------------------------------------------*/

SoS_primitive_result * sos_rho2_num (i, j, k)
     int i, j, k;
     /* Returns significant term of Rho2 numerator. */
{
#ifdef __DEBUG__
  if (sos_proto_e_flag)
    {
      lia_clear ();
      print ("sos_rho2_num (%d,%d,%d)", i, j, k);
      print (" (");
      print ("%s,%s,%s;", Pi(i,1), Pi(i,2), Pi(i,3));
      print ("%s,%s,%s;", Pi(j,1), Pi(j,2), Pi(j,3));
      print ("%s,%s,%s;", Pi(k,1), Pi(k,2), Pi(k,3));
      print (")\n");
    }
#endif
/* C code generated by 'ccode' from 'gee' file "Rho2.num" */
Initialize ();
Epsilon_Term (0);
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (1);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Times ();
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Times ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Times ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Times ();
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Times ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (2);
Epsilon (i,3);
Epsilon (i,3);
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (4));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 3, 0));
Times ();
Times ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 3, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (4);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (5);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Times ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Times ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Times ();
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Times ();
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Times ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (6);
Epsilon (i,2);
Epsilon (i,3);
Push (Integer (4));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 2, 0));
Times ();
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 3, 0));
Times ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (7);
Epsilon (i,2);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (8);
Epsilon (i,2);
Epsilon (i,2);
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Integer (4));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Times ();
Times ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (9);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 3, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (10);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (11);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (12);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,2);
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (13);
Epsilon (i,1);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 1, 0));
Times ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 1, 0));
Times ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 1, 0));
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Times ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (14);
Epsilon (i,1);
Epsilon (i,3);
Push (Integer (4));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 1, 0));
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 3, 0));
Times ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (15);
Epsilon (i,1);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (16);
Epsilon (i,1);
Epsilon (i,2);
Push (Integer (4));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 1, 0));
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 2, 0));
Times ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (17);
Epsilon (i,1);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (18);
Epsilon (i,1);
Epsilon (i,1);
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Integer (4));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Times ();
Times ();
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (19);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 3, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (20);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (21);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (22);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (23);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,1);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Plus ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (24);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,1);
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (25);
Epsilon (j,3);
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (26);
Epsilon (j,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Minus ();
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Times ();
Times ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (27);
Epsilon (j,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (4));
Push (Minor2 (i, k, 3, 0));
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Minus ();
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (28);
Epsilon (j,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Plus ();
Minus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (29);
Epsilon (j,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (j, k, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (30);
Epsilon (j,3);
Epsilon (i,2);
Push (Integer (4));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Times ();
Times ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (31);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,3);
Push (Integer (8));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Times ();
Push (Integer (4));
Push (Minor2 (i, k, 2, 0));
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Minus ();
Times ();
Times ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (32);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (4));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (33);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (8));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (34);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,3);
Push (Integer (2));
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Plus ();
Minus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (35);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (4));
Push (Minor2 (j, k, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (36);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (4));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (37);
Epsilon (j,3);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (2));
Push (Minor2 (j, k, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (38);
Epsilon (j,3);
Epsilon (i,1);
Push (Integer (4));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Times ();
Times ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (39);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,3);
Push (Integer (8));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Times ();
Push (Integer (4));
Push (Minor2 (i, k, 1, 0));
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Minus ();
Times ();
Times ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (40);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (4));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (41);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,2);
Push (Integer (8));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 1, 0));
Times ();
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 2, 0));
Times ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (42);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (4));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (43);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Push (Integer (2));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (8));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Times ();
Times ();
Minus ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (44);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,3);
Push (Integer (2));
Push (Negative (Minor2 (j, k, 1, 0)));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Minus ();
Plus ();
Plus ();
Minus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (45);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (4));
Push (Minor2 (j, k, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (46);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,2);
Push (Integer (4));
Push (Minor2 (i, j, 2, 0));
Push (Minor2 (i, k, 2, 0));
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (47);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,2);
Epsilon (i,2);
Push (Integer (4));
Push (Minor2 (j, k, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (48);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,1);
Push (Integer (4));
Push (Minor2 (i, j, 1, 0));
Push (Minor2 (i, k, 1, 0));
Plus ();
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Coefficient (Pop ());
Epsilon_Term (49);
Epsilon (j,3);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,1);
Epsilon (i,1);
Push (Integer (2));
Push (Minor2 (j, k, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (50);
Epsilon (j,3);
Epsilon (j,3);
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (4));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Minus ();
Plus ();
Plus ();
Plus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (51);
Epsilon (j,3);
Epsilon (j,3);
Epsilon (i,3);
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Times ();
Push (Integer (4));
Push (Minor2 (j, k, 3, 0));
Times ();
Minus ();
Times ();
Push (Integer (2));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (4));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Minus ();
Plus ();
Plus ();
Plus ();
Plus ();
Times ();
Times ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (52);
Epsilon (j,3);
Epsilon (j,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (j, k, 2, 0));
Push (Integer (2));
Power ();
Push (Integer (4));
Push (Minor2 (i, k, 3, 0));
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Push (Minor2 (j, k, 3, 0));
Times ();
Minus ();
Times ();
Times ();
Push (Integer (4));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (j, k, 3, 0));
Times ();
Times ();
Push (Minor2 (j, k, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Minus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (53);
Epsilon (j,3);
Epsilon (j,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Push (Minor2 (i, k, 3, 0));
Push (Integer (2));
Push (Minor2 (j, k, 3, 0));
Times ();
Minus ();
Plus ();
Times ();
Coefficient (Pop ());
Epsilon_Term (54);
Epsilon (j,3);
Epsilon (j,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (1));
Coefficient (Pop ());
Finish ();
}

#endif  /* #if !defined(lint) */
