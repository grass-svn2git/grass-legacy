#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <grass/glocale.h>
#include "global.h"

#define INCR 20

int read_range (void)
{
  struct FPRange drange;
  struct Range range;
  CELL tmp_min, tmp_max;
  DCELL tmp_dmin, tmp_dmax;
  char buff[1024];
  int i;

  /* read the fpranges and ranges of all input maps */
  for(i=0; i<noi; i++)
  {
     if(G_read_fp_range(name[i], mapset[i], &drange)<=0)
     {
        sprintf(buff, "Can't read f_range for map %s", name[i]);
        G_fatal_error(buff);
     }
     G_get_fp_range_min_max(&drange, &tmp_dmin, &tmp_dmax);

     if(G_read_range(name[i], mapset[i], &range)<=0)
     {
        sprintf(buff, "Can't read range for map %s", name[i]);
        G_fatal_error(buff);
     }
     G_get_range_min_max(&range, &tmp_min, &tmp_max);
     if(!i || tmp_max > old_max || G_is_c_null_value(&old_max))
        old_max = tmp_max;
     if(!i || tmp_min < old_min || G_is_c_null_value(&old_min))
        old_min = tmp_min;
     if(!i || tmp_dmax > old_dmax || G_is_d_null_value(&old_dmax))
        old_dmax = tmp_dmax;
     if(!i || tmp_dmin < old_dmin || G_is_d_null_value(&old_dmin))
        old_dmin = tmp_dmin;
  } /* for loop */

  return 0;
}  

int 
report_range (void)
{
  char buff[1024], buff2[300];

  if(G_is_d_null_value(&old_dmin) || G_is_d_null_value(&old_dmax))
      G_message (_("Old data range is empty"));
  else
  {
      sprintf(buff, "%.10f",old_dmin);
      sprintf(buff2, "%.10f",old_dmax);
      G_trim_decimal(buff);
      G_trim_decimal(buff2);
      G_message (_("Old data range is %s to %s"), buff, buff2);
  }
  if(G_is_c_null_value(&old_min) || G_is_c_null_value(&old_max))
      G_message (_("Old integer data range is empty"));
  else
      G_message (_("Old integer data range is %d to %d"), 
		       (int)old_min, (int)old_max);

  return 0;
}

int 
read_rules (void)
{
   char buf[256];
   DCELL dLow, dHigh;
   CELL iLow, iHigh;
   int line, n, nrules=0;
   int first=1;
   DCELL dmin, dmax;
   CELL cmin, cmax;
   char *prompt = "Add more quant rules?";

   read_range();
   report_range();
   fprintf(stderr,
    "\nEnter the rule or 'help' for the format description or 'end' to exit:\n");
   G_quant_init(&quant_struct);
   for (line=1;;line++)
   {
      if (isatty(0)) fprintf (stdout, "> ");
      if (!fgets(buf,1024,stdin)) return 0;
      for (n=0;buf[n];n++)
	   if(buf[n] == ',')
	      buf[n] = ' ';
      G_strip(buf);
      G_chop(buf);
      if (*buf == 0) continue;
      if (*buf == '#') continue;
      if (strcmp(buf, "end") == 0) {
	if(nrules == 0) break; /* if no new rules have been specified */

	/* give warning when quant rules do not cover the whole range of map */
	G_quant_get_limits(&quant_struct, &dmin, &dmax, &cmin, &cmax);
	if ((dmin > old_dmin || dmax < old_dmax) && !first) {
	  fprintf (stdout,
		"Warning: quant rules do not cover the whole range map\n");
	  if (G_yes(prompt, 1))
	    continue;
	}
	break;
      }

      if (strcmp(buf, "help") == 0)
      {
	  fprintf(stdout, "Enter a rule in one of these formats:\n");
	  fprintf(stdout, "float_low:float_high:int_low:int_high\n");
	  fprintf(stdout, "float_low:float_high:int_val  (i.e. int_high == int_low)\n");
	  fprintf(stdout, "*:float_val:int_val           (interval [inf, float_val])\n");
	  fprintf(stdout, "float_val:*:int_val           (interval [float_val, inf])\n");
      }	       

      /* we read and record into quant table all values, even int as doubles
	 we convert the range and domain values to the right format when we 
	 lookup the values in the quant table */
      switch (sscanf (buf, "%lf:%lf:%d:%d", &dLow, &dHigh, &iLow, &iHigh))
      {
        case 3:  G_quant_add_rule(&quant_struct, dLow, dHigh, iLow, iLow);
		 nrules++;
      	         first = 0;
		 break;

        case 4:  G_quant_add_rule(&quant_struct, dLow, dHigh, iLow, iHigh);
		 nrules++;
      	         first = 0;
		 break;

        default:
	    if(sscanf (buf, "%lf:*:%d", &dLow, &iLow)==2)
	    {
	       G_quant_set_pos_infinite_rule(&quant_struct, dLow, iLow);
	       nrules++;
      	       first = 0;
            }	

	    else if(sscanf (buf, "*:%lf:%d", &dHigh, &iLow)==2) 
	    {
	       G_quant_set_neg_infinite_rule(&quant_struct, dHigh, iLow);
	       nrules++;
      	       first = 0;
            }

	    else if (strcmp(buf, "help") == 0)
	       break;

	    else
	       fprintf(stderr, "%s is not a valid rule\n", buf);
	    break;
      } /* switch */
    } /* loop */

  return nrules;
}

