#include "coin.h"

print_coin_hdr(Conformat)
char Conformat;
{
    char	unit_type[20];
    char	*G_date();
    char	*mapset,*location;
    char	north[30], south[30], east[30], west[30];

    mapset   = G_mapset () ;
    location = G_location () ;

    switch(Conformat){
	case 'a': sprintf(unit_type,"acres");			break;
	case 'h': sprintf(unit_type,"hectares");		break;
	case 'k': sprintf(unit_type,"square kilometers");	break;
	case 'm': sprintf(unit_type,"square miles");		break;
	case 'p': sprintf(unit_type,"percent cover");		break;
	case 'x': sprintf(unit_type,"percent of cols");		break;
	case 'y': sprintf(unit_type,"percent of rows");		break;
	default:  sprintf(unit_type,"cells");			break;
    }

    G_format_northing (window.north, north, window.proj);
    G_format_northing (window.south, south, window.proj);
    G_format_easting  (window.east,  east,  window.proj);
    G_format_easting  (window.west,  west,  window.proj);

    fprintf(dumpfile,"\n\n\n");
    fprintf(dumpfile,"+%78.78s+\n",midline);
    fprintf(dumpfile,"|%24.24sCOINCIDENCE TABULATION REPORT%25.25s|\n",
	    fill,fill);
    fprintf(dumpfile,"|%78.78s|\n",midline);
    fprintf(dumpfile,"| Location: %-16.14sMapset: %-17.15sDate: %-20.20s|\n",
	    location,mapset,G_date());
    fprintf(dumpfile,"|%78.78s|\n",fill);
    fprintf(dumpfile,"| Layer 1: %-15.15s-- %-50.49s|\n",map1name,title1);
    fprintf(dumpfile,"| Layer 2: %-15.15s-- %-50.49s|\n",map2name,title2);
    fprintf(dumpfile,"| Mask:    %-68.68s|\n",G_mask_info());
    fprintf(dumpfile,"|%78.78s|\n",fill);
    fprintf(dumpfile,"| Units:   %-68.68s|\n",unit_type);
    fprintf(dumpfile,"|%78.78s|\n",midline);
    fprintf(dumpfile, "| Window:%22.22sNorth: %-10s%31.31s|\n",
		fill,north,fill);
    fprintf(dumpfile,"|%14.14sWest: %-9s%19.19sEast: %-9s%15.15s|\n",
		fill,west,fill,east,fill);
    fprintf(dumpfile,"|%30.30sSouth: %-10s%31.31s|\n",
		fill,south,fill);
    fprintf(dumpfile,"+%78.78s+\n\n",midline);
}
