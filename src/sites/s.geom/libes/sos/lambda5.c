/* sos/lambda5.c */

/*--------------------------------------------------------------------------*/

#include "basic.h"
#include "sos.h"
#include "internal.h"
#include "primitive.h"

/*--------------------------------------------------------------------------*/

Static_Declarations ("lambda5", 5, 49, 4);

#include "primitive.i.c"

/*--------------------------------------------------------------------------*/

SoS_primitive_result * sos_lambda5 (i, j, k, l, m)
     int i, j, k, l, m;
     /* Returns significant term of Lambda5 epsilon-determinant.
        Assumes indices in proper range, pairwise different, and sorted. */
{
#ifdef __DEBUG__
  if (sos_proto_e_flag)
    {
      lia_clear ();
      print ("sos_lambda5 (%d,%d,%d,%d,%d)", i, j, k, l, m);
      print (" (");
      print ("%s,%s,%s,%s,1;", Pi(i,1), Pi(i,2), Pi(i,3), Pi(i,4));
      print ("%s,%s,%s,%s,1;", Pi(j,1), Pi(j,2), Pi(j,3), Pi(j,4));
      print ("%s,%s,%s,%s,1;", Pi(k,1), Pi(k,2), Pi(k,3), Pi(k,4));
      print ("%s,%s,%s,%s,1;", Pi(l,1), Pi(l,2), Pi(l,3), Pi(l,4));
      print ("%s,%s,%s,%s,1;", Pi(m,1), Pi(m,2), Pi(m,3), Pi(m,4));
      print (")\n");
    }
#endif
/* C code generated by 'ccode' from 'gee' file "Lambda5.out" */
Initialize ();
Epsilon_Term (0);
Positive_Coefficient (Minor5 (i, j, k, l, m, 1, 2, 3, 4, 0));
Epsilon_Term (1);
Epsilon (i,4);
Negative_Coefficient (Minor4 (j, k, l, m, 1, 2, 3, 0));
Epsilon_Term (2);
Epsilon (i,3);
Positive_Coefficient (Minor4 (j, k, l, m, 1, 2, 4, 0));
Epsilon_Term (3);
Epsilon (i,2);
Negative_Coefficient (Minor4 (j, k, l, m, 1, 3, 4, 0));
Epsilon_Term (4);
Epsilon (i,1);
Positive_Coefficient (Minor4 (j, k, l, m, 2, 3, 4, 0));
Epsilon_Term (5);
Epsilon (j,4);
Positive_Coefficient (Minor4 (i, k, l, m, 1, 2, 3, 0));
Epsilon_Term (6);
Epsilon (j,4);
Epsilon (i,3);
Positive_Coefficient (Minor3 (k, l, m, 1, 2, 0));
Epsilon_Term (7);
Epsilon (j,4);
Epsilon (i,2);
Negative_Coefficient (Minor3 (k, l, m, 1, 3, 0));
Epsilon_Term (8);
Epsilon (j,4);
Epsilon (i,1);
Positive_Coefficient (Minor3 (k, l, m, 2, 3, 0));
Epsilon_Term (9);
Epsilon (j,3);
Negative_Coefficient (Minor4 (i, k, l, m, 1, 2, 4, 0));
Epsilon_Term (10);
Epsilon (j,3);
Epsilon (i,2);
Positive_Coefficient (Minor3 (k, l, m, 1, 4, 0));
Epsilon_Term (11);
Epsilon (j,3);
Epsilon (i,1);
Negative_Coefficient (Minor3 (k, l, m, 2, 4, 0));
Epsilon_Term (12);
Epsilon (j,2);
Positive_Coefficient (Minor4 (i, k, l, m, 1, 3, 4, 0));
Epsilon_Term (13);
Epsilon (j,2);
Epsilon (i,1);
Positive_Coefficient (Minor3 (k, l, m, 3, 4, 0));
Epsilon_Term (14);
Epsilon (j,1);
Negative_Coefficient (Minor4 (i, k, l, m, 2, 3, 4, 0));
Epsilon_Term (15);
Epsilon (k,4);
Negative_Coefficient (Minor4 (i, j, l, m, 1, 2, 3, 0));
Epsilon_Term (16);
Epsilon (k,4);
Epsilon (i,3);
Negative_Coefficient (Minor3 (j, l, m, 1, 2, 0));
Epsilon_Term (17);
Epsilon (k,4);
Epsilon (i,2);
Positive_Coefficient (Minor3 (j, l, m, 1, 3, 0));
Epsilon_Term (18);
Epsilon (k,4);
Epsilon (i,1);
Negative_Coefficient (Minor3 (j, l, m, 2, 3, 0));
Epsilon_Term (19);
Epsilon (k,4);
Epsilon (j,3);
Positive_Coefficient (Minor3 (i, l, m, 1, 2, 0));
Epsilon_Term (20);
Epsilon (k,4);
Epsilon (j,3);
Epsilon (i,2);
Negative_Coefficient (Minor2 (l, m, 1, 0));
Epsilon_Term (21);
Epsilon (k,4);
Epsilon (j,3);
Epsilon (i,1);
Positive_Coefficient (Minor2 (l, m, 2, 0));
Epsilon_Term (22);
Epsilon (k,4);
Epsilon (j,2);
Negative_Coefficient (Minor3 (i, l, m, 1, 3, 0));
Epsilon_Term (23);
Epsilon (k,4);
Epsilon (j,2);
Epsilon (i,1);
Negative_Coefficient (Minor2 (l, m, 3, 0));
Epsilon_Term (24);
Epsilon (k,4);
Epsilon (j,1);
Positive_Coefficient (Minor3 (i, l, m, 2, 3, 0));
Epsilon_Term (25);
Epsilon (k,3);
Positive_Coefficient (Minor4 (i, j, l, m, 1, 2, 4, 0));
Epsilon_Term (26);
Epsilon (k,3);
Epsilon (i,2);
Negative_Coefficient (Minor3 (j, l, m, 1, 4, 0));
Epsilon_Term (27);
Epsilon (k,3);
Epsilon (i,1);
Positive_Coefficient (Minor3 (j, l, m, 2, 4, 0));
Epsilon_Term (28);
Epsilon (k,3);
Epsilon (j,2);
Positive_Coefficient (Minor3 (i, l, m, 1, 4, 0));
Epsilon_Term (29);
Epsilon (k,3);
Epsilon (j,2);
Epsilon (i,1);
Positive_Coefficient (Minor2 (l, m, 4, 0));
Epsilon_Term (30);
Epsilon (k,3);
Epsilon (j,1);
Negative_Coefficient (Minor3 (i, l, m, 2, 4, 0));
Epsilon_Term (31);
Epsilon (k,2);
Negative_Coefficient (Minor4 (i, j, l, m, 1, 3, 4, 0));
Epsilon_Term (32);
Epsilon (k,2);
Epsilon (i,1);
Negative_Coefficient (Minor3 (j, l, m, 3, 4, 0));
Epsilon_Term (33);
Epsilon (k,2);
Epsilon (j,1);
Positive_Coefficient (Minor3 (i, l, m, 3, 4, 0));
Epsilon_Term (34);
Epsilon (k,1);
Positive_Coefficient (Minor4 (i, j, l, m, 2, 3, 4, 0));
Epsilon_Term (35);
Epsilon (l,4);
Positive_Coefficient (Minor4 (i, j, k, m, 1, 2, 3, 0));
Epsilon_Term (36);
Epsilon (l,4);
Epsilon (i,3);
Positive_Coefficient (Minor3 (j, k, m, 1, 2, 0));
Epsilon_Term (37);
Epsilon (l,4);
Epsilon (i,2);
Negative_Coefficient (Minor3 (j, k, m, 1, 3, 0));
Epsilon_Term (38);
Epsilon (l,4);
Epsilon (i,1);
Positive_Coefficient (Minor3 (j, k, m, 2, 3, 0));
Epsilon_Term (39);
Epsilon (l,4);
Epsilon (j,3);
Negative_Coefficient (Minor3 (i, k, m, 1, 2, 0));
Epsilon_Term (40);
Epsilon (l,4);
Epsilon (j,3);
Epsilon (i,2);
Positive_Coefficient (Minor2 (k, m, 1, 0));
Epsilon_Term (41);
Epsilon (l,4);
Epsilon (j,3);
Epsilon (i,1);
Negative_Coefficient (Minor2 (k, m, 2, 0));
Epsilon_Term (42);
Epsilon (l,4);
Epsilon (j,2);
Positive_Coefficient (Minor3 (i, k, m, 1, 3, 0));
Epsilon_Term (43);
Epsilon (l,4);
Epsilon (j,2);
Epsilon (i,1);
Positive_Coefficient (Minor2 (k, m, 3, 0));
Epsilon_Term (44);
Epsilon (l,4);
Epsilon (j,1);
Negative_Coefficient (Minor3 (i, k, m, 2, 3, 0));
Epsilon_Term (45);
Epsilon (l,4);
Epsilon (k,3);
Positive_Coefficient (Minor3 (i, j, m, 1, 2, 0));
Epsilon_Term (46);
Epsilon (l,4);
Epsilon (k,3);
Epsilon (i,2);
Negative_Coefficient (Minor2 (j, m, 1, 0));
Epsilon_Term (47);
Epsilon (l,4);
Epsilon (k,3);
Epsilon (i,1);
Positive_Coefficient (Minor2 (j, m, 2, 0));
Epsilon_Term (48);
Epsilon (l,4);
Epsilon (k,3);
Epsilon (j,2);
Positive_Coefficient (Minor2 (i, m, 1, 0));
Epsilon_Term (49);
Epsilon (l,4);
Epsilon (k,3);
Epsilon (j,2);
Epsilon (i,1);
Coefficient (Integer (1));
Finish ();
}
