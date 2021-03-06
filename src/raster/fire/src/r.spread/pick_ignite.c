/******************************************************************************
 *
 *	pick_ignite.c 	pick an ignition based on probability
 *	
 * Usage: pick_ignite (fuelmoisture)
 *
 * Notes: pick_ignite() pick an ignition idetermined by fine fuel moisture
 * based on the probability simplified from Rothermel (1983): Predicting
 * ?????????????????? 
 * Author: Jianping Xu, Rutgers University
 * Date: 06/11/1994
 ******************************************************************************/
#include <values.h>
#include <math.h>

int p[18] = {100, 100, 100, 90, 80, 70, 60, 50, 40,   /*adapt the "average"*/
	      40,  30,  30, 20, 20, 20, 10, 10, 10};  /*10-50 shading, 80-89F*/

pick_ignite (i)
int i;
{
	return ((100.0*rand()/MAXINT) <= p[i]);
}
