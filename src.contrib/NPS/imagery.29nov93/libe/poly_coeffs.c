/*======================================================================
                             poly_coeffs.c

======================================================================*/

#include "ortho_image.h"

I_get_poly_coefs_data (group) 
Rectify_Group *group;
{

   group->coefs = (Coeffs_Poly *) G_malloc (sizeof (Coeffs_Poly));
   get_poly_coeffs_data(group);
}


get_poly_coeffs_data(group)
Rectify_Group    *group;
{
Coeffs_Poly  *coeffs;
char           msg[100];


    /* make auxilary visiable */
    coeffs = (Coeffs_Poly *) group->coefs;

}

