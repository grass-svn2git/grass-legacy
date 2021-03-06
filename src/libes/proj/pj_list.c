#ifndef lint
static char RCSID[] = "@(#)$Id: pj_list.c,v 4.7 1992/07/14 01:29:01 gie Exp $";
#endif
/* Projection System: List of projections
** When adding a projection both of the following statements
** need to be updated.
*/
#define __PJ_LIST
#define __PJ_LIB
#include "projects.h"
	/* Prototypes */

#define PPJ PROTO((PJ *))

extern void
   *pj_aea PPJ,    *pj_aeqd PPJ,   *pj_bipc PPJ, 
   *pj_bonne PPJ,  *pj_cea PPJ,    *pj_eck4 PPJ, 
   *pj_eck6 PPJ,   *pj_eqc PPJ,    *pj_eqdc PPJ, 
   *pj_gnom PPJ,   *pj_laea PPJ,   *pj_lcc PPJ,
   *pj_merc PPJ,   *pj_mill PPJ,   *pj_moll PPJ, 
   *pj_nsper PPJ,  *pj_omerc PPJ,  *pj_ortho PPJ, 
   *pj_poly PPJ,   *pj_sinu PPJ,   *pj_stere PPJ, 
   *pj_tmerc PPJ,  *pj_ups PPJ,    *pj_utm PPJ, 
   *pj_vandg PPJ,  *pj_cass PPJ,   *pj_tpers PPJ, 
   *pj_tcea PPJ,   *pj_ocea PPJ,   *pj_parab PPJ, 
   *pj_gall PPJ,   *pj_eck5 PPJ,   *pj_wink1 PPJ, 
   *pj_hammer PPJ, *pj_august PPJ, *pj_hataea PPJ,
   *pj_mbtfps PPJ, *pj_mbtfpq PPJ, *pj_putp2 PPJ,
   *pj_eck3 PPJ,   *pj_mbtfpp PPJ, *pj_putp5 PPJ, 
   *pj_quau PPJ,   *pj_dense PPJ,  *pj_robin PPJ, 
   *pj_aitoff PPJ, *pj_wintri PPJ, *pj_eck1 PPJ,
   *pj_eck2 PPJ,   *pj_boggs PPJ,  *pj_pconic PPJ,
   *pj_rpoly PPJ,  *pj_airy PPJ,   *pj_bacon PPJ, 
   *pj_eisen PPJ,  *pj_fourn PPJ,  *pj_lagrng PPJ,
   *pj_nicol PPJ,  *pj_ortel PPJ,  *pj_vandg2 PPJ,
   *pj_vandg3 PPJ, *pj_vandg4 PPJ, *pj_wag7 PPJ, 
   *pj_leac PPJ,   *pj_loxim PPJ,  *pj_apian PPJ, 
   *pj_cc PPJ,     *pj_tcc PPJ,    *pj_collg PPJ, 
   *pj_goode PPJ,  *pj_chamb PPJ,  *pj_lsat PPJ,
   *pj_alsk PPJ,   *pj_gs50 PPJ,   *pj_tpeqd PPJ;

struct PJ_LIST
pj_list[] = {
	"aea",		pj_aea,		"Albers Egual Area - Inv",
	"aeqd",		pj_aeqd,	"Azimuthal equidistant - Inv",
	"airy",		pj_airy,	"Airy",
	"aitoff",	pj_aitoff,	"Aitoff",
	"alsk",		pj_alsk,	"Alaska Mod.-Stereographics - Inv",
	"apian",	pj_apian,	"Apian Globular - Inv",
	"august",	pj_august,	"August Epicycloidal",
	"bacon",	pj_bacon,	"Bacon Globular",
	"bipc",		pj_bipc,	"Bipolar Conic - Inv",
	"boggs",	pj_boggs,	"Boggs Eumorphic",
	"bonne",	pj_bonne,	"Bonne - Inv",
	"cass",		pj_cass,	"Cassini - Inv",
	"cc",		pj_cc,		"Central Cylindrical - Inv",
	"cea",		pj_cea,		"Cylindrical Equal Area - Inv",
	"chamb",	pj_chamb,	"Chamberlin Trimetric",
	"collg",	pj_collg,	"Collignon - Inv",
	"dense",	pj_dense,	"Denoyer Semi-Elliptical",
	"eck1",		pj_eck1,	"Eckert I - Inv",
	"eck2",		pj_eck2,	"Eckert II - Inv",
	"eck3",		pj_eck3,	"Eckert III - Inv",
	"eck4",		pj_eck4,	"Eckert IV - Inv",
	"eck5",		pj_eck5,	"Eckert V - Inv",
	"eck6",		pj_eck6,	"Eckert VI - Inv",
	"eisen",	pj_eisen,	"Eisenlohr",
	"eqc",		pj_eqc,		"Equidistant Cylindrical - Inv",
	"eqdc",		pj_eqdc,	"Equidistant Conic - Inv",
	"fourn",	pj_fourn,	"Fournier Globular",
	"gall",		pj_gall,	"Gall (Stereographic) - Inv",
	"goode",	pj_goode,	"Goode Homolosine",
	"gnom",		pj_gnom,	"Gnomonic - Inv",
	"gs50",		pj_gs50,	"50 State U.S. Mod.-Stereographic - Inv",
	"hammer",	pj_hammer,	"Hammer (Elliptical)",
	"hataea",	pj_hataea,	"Hatano Asymmetrical Equal Area - Inv",
	"lagrng",	pj_lagrng,	"Lagrange",
	"laea",		pj_laea,	"Lambert Azimuthal Equal Area - Inv",
	"leac",		pj_leac,	"Lambert Equal Area Conic - Inv",
	"lcc",		pj_lcc,		"Lambert Conformal Conic - Inv",
	"loxim",	pj_loxim,	"Loximuthal - Inv",
	"lsat",		pj_lsat,	"LANDSAT Space Oblique Mercator - Inv",
	"mbtfpp",	pj_mbtfpp,	"McBryde-Thomas Flat-Polar Parabolic - Inv",
	"mbtfps",	pj_mbtfps,	"McBryde-Thomas Flat-Polar Sinusoidal - Inv",
	"mbtfpq",	pj_mbtfpq,	"McBryde-Thomas Flat-Polar Quartic - Inv",
	"merc",		pj_merc,	"Mercator - Inv",
	"mill",		pj_mill,	"Miller - Inv",
	"moll",		pj_moll,	"Mollweides - Inv",
	"nicol",	pj_nicol,	"Nicolosi Globular",
	"nsper",	pj_nsper,	"General Vertical Persepective - Inv",
	"ocea",		pj_ocea,	"Oblique Cylindrical Equal Area - Inv",
	"omerc",	pj_omerc,	"Oblique Mercator - Inv",
	"ortel",	pj_ortel,	"Ortelius",
	"ortho",	pj_ortho,	"Orthographic - Inv",
	"parab",	pj_parab,	"Caster Parabolic - Inv",
	"pconic",	pj_pconic,	"Perspective Conic",
	"poly",		pj_poly,	"Polyconic (American) - Inv",
	"putp2",	pj_putp2,	"Putnins P2' - Inv",
	"putp5",	pj_putp5,	"Putnins P5 - Inv",
	"rpoly",	pj_rpoly,	"Rectangular Polyconic",
	"quau",		pj_quau,	"Quartic Authalic - Inv",
	"robin",	pj_robin,	"Robinson - Inv",
	"sinu",		pj_sinu,	"Sinusoidal - Inv",
	"stere",	pj_stere,	"Stereographic - Inv",
	"tcc",		pj_tcc,		"Transverse Central Cylindrical - Inv",
	"tcea",		pj_tcea,	"Transverse Cylindrical Equal Area - Inv",
	"tmerc",	pj_tmerc,	"Transverse Mercator - Inv",
	"tpeqd",	pj_tpeqd,	"Two Point Equidistant",
	"tpers",	pj_tpers,	"Tilted perspective - Inv",
	"ups",		pj_ups,		"Universal Polar Stereographic - Inv",
	"utm",		pj_utm,		"Universal Transverse Mercator - Inv",
	"vandg",	pj_vandg,	"Van der Grinten - Inv",
	"vandg2",	pj_vandg2,	"Van der Grinten II",
	"vandg3",	pj_vandg3,	"Van der Grinten III",
	"vandg4",	pj_vandg4,	"Van der Grinten IV",
	"wag7",		pj_wag7,	"Wagner VII",
	"wink1",	pj_wink1,	"Winkel I - Inv",
	"wintri",	pj_wintri,	"Winkel Tripel",
	0,		0,	"list terminator"
};
