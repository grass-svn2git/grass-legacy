#include "globals.h"


/**____________________________________________________________________________

Retyrns: 
	0 - user entered info and hit ESC
	1 - user hit CONTROL-C 
____________________________________________________________________________**/

#define DEG_TO_RADS 0.01745329
#define RAD_TO_DEGS 57.29578


mod_init_info(have_old, init_info)
int  have_old ;
Camera_Expose *init_info;
{
   double omega_deg, phi_deg, kappa_deg; 
   double omega_var, phi_var, kappa_var;
   int status;

	if( ! have_old)
	{
          /* make zero */
	}

        /* convert from radians to degrees */
        omega_deg   = init_info->omega_init * RAD_TO_DEGS;
        phi_deg     = init_info->phi_init   * RAD_TO_DEGS;
        kappa_deg   = init_info->kappa_init * RAD_TO_DEGS;

        omega_var   = init_info->omega_var * RAD_TO_DEGS;
        phi_var     = init_info->phi_var   * RAD_TO_DEGS;
        kappa_var   = init_info->kappa_var * RAD_TO_DEGS;


	V_clear() ;
	V_line(1,"                   Please provide the following information:") ;
        V_line(2,"+------------------------------------------------------------------------------+");
	V_line(4,"     Initial Camera Exposure X-coordinate Meters:");
	V_line(5,"     Initial Camera Exposure Y-coordinate Meters:");
	V_line(6,"     Initial Camera Exposure Z-coordinate Meters:");
	V_line(7,"     Initial Camera Omega (roll) degrees:");
	V_line(8,"     Initial Camera Phi (pitch)  degrees:");
	V_line(9,"     Initial Camera Kappa (yaw)  degrees:");

	V_line(11,"     Apriori standard devation X-coordinate Meters:");
	V_line(12,"     Apriori standard devation Y-coordinate Meters:");
	V_line(13,"     Apriori standard devation Z-coordinate Meters:");
	V_line(14,"     Apriori standard devation Omega (roll) degrees:");
	V_line(15,"     Apriori standard devation Phi (pitch)  degrees:");
	V_line(16,"     Apriori standard devation Kappa (yaw)  degrees:");

	V_line(18,"     Use these values at run time? (1=yes, 0=no)"); 
        V_line(19,"+------------------------------------------------------------------------------+");

	V_ques( &(init_info->XC_init),     'd', 4,  60, 15-1) ;
        V_ques( &(init_info->YC_init),     'd', 5,  60, 15-1) ;
	V_ques( &(init_info->ZC_init),     'd', 6,  60, 15-1) ;

	V_ques( &omega_deg,  'd', 7,  60, 15-1) ;
        V_ques( &phi_deg,    'd', 8,  60, 15-1) ;
	V_ques( &kappa_deg,  'd', 9,  60, 15-1) ;

        V_ques( &(init_info->XC_var),      'd',11,  60, 15-1) ;
	V_ques( &(init_info->YC_var),      'd',12,  60, 15-1) ;
        V_ques( &(init_info->ZC_var),      'd',13,  60, 15-1) ;

	V_ques( &omega_var,   'd',14,  60, 15-1) ;
        V_ques( &phi_var,     'd',15,  60, 15-1) ;
	V_ques( &kappa_var,   'd',16,  60, 15-1) ;
	V_ques( &(init_info->status),      'i',18,  60, 2) ; 


        status  = 1;
	for (;;) {
	   V_intrpt_ok();
	   if(!V_call()) {
	       goto end;
           }

           /* TODO check values ?? */
           break;
        }
     
        status = 0;
 
        /* convert back to radians */
        init_info->omega_init = omega_deg * DEG_TO_RADS;
        init_info->phi_init   = phi_deg   * DEG_TO_RADS;
        init_info->kappa_init = kappa_deg * DEG_TO_RADS;

        init_info->omega_var = omega_var * DEG_TO_RADS;
        init_info->phi_var   = phi_var   * DEG_TO_RADS;
        init_info->kappa_var = kappa_var * DEG_TO_RADS;

end:
   return (status);
}





