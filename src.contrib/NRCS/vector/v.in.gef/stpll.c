/* @(#)stpll.c	1.1   02/24/88 */
/* created by: R.L.Glenn
*      Determines the Latitude 'lat' and Longitude 'lon' for the point
*      located NOR feet NORTH and EAS feet EAST in FIPS code SFIP,CFIP
*      zone.
*      Programmed for North-western Quadrisphere ONLY
*****/
#include <ctype.h>
#include <stdio.h>
#include <math.h>
static int zones[] = { 
101, 102, 201, 202, 203, 301, 302, 401, 402, 403, 404, 405, 406, 407, 501,
502, 503, 600, 700, 901, 902, 903, 1001, 1002, 1101, 1102, 1103, 1201, 1202,
1301, 1302, 1401, 1402, 1501, 1502, 1601, 1602, 1701, 1702, 1703, 1801, 1802,
1900, 2001, 2002, 2101, 2102, 2103, 2111, 2112, 2113, 2201, 2202, 2203, 2301,
2302, 2401, 2402, 2403, 2501, 2502, 2503, 2601, 2602, 2701, 2702, 2703, 2800,
2900, 3001, 3002, 3003, 3101, 3102, 3103, 3104, 3200, 3301, 3302, 3401, 3402,
3501, 3502, 3601, 3602, 3701, 3702, 3800, 3901, 3902, 4001, 4002, 4100, 4201,
4202, 4203, 4204, 4205, 4301, 4302, 4303, 4400, 4501, 4502, 4601, 4602, 4701,
4702, 4801, 4802, 4803, 4901, 4902, 4903, 4904 };
static double coeff[][11] = { 
500000.,309000.,1822.,21.00903,.99996,.3817065,0.0,0.0,0.0,0.0,0.0,
500000.,315000.,1792.,25.53386,.9999333333,.3817477,0.0,0.0,0.0,0.0,0.0,
500000.,396600.,1852.,16.62358,.9999,.3816485,0.0,0.0,0.0,0.0,0.0,
500000.,402900.,1852.,16.62358,.9999,.3816485,0.0,0.0,0.0,0.0,0.0,
500000.,409500.,1852.,16.62358,.9999333333,.3815948,0.0,0.0,0.0,0.0,0.0,
2000000.,331200.,29277593.61,29732882.87,.9999359370,.5818991407,2126.,46.35656,3.81452,3.26432,0.0,
2000000.,331200.,31014039.23,31511724.20,.9999184698,.5596906871,2033.,56.94711,3.8155,3.08256,0.0,
2000000.,439200.,24245358.05,24792436.23,.9998946358,.6538843192,2441.,26.75847,0.80992,3.93575,0.0,
2000000.,439200.,25795850.31,26312257.65,.9999146793,.6304679732,2336.,30.81964,3.81147,3.70114,0.0,
2000000.,433800.,27057475.85,27512992.04,.9999291792,.6122320427,2256.,35.52018,3.81265,3.52998,0.0,
2000000.,428400.,28182405.33,28652931.96,.9999407628,.5965871443,2189.,10.35494,3.81362,3.39020,0.0,
2000000.,424800.,30194145.54,30649424.27,.9999221277,.5700119219,2076.,52.10305,3.81523,3.16593,0.0,
2000000.,418500.,31846570.92,32271267.72,.9999541438,.5495175982,1992.,.16335,3.81642,3.00292,0.0,
4186692.58,426000.,30891382.10,35055396.31,.9999885350,.5612432071,2040.,22.88096,3.81572,3.09520,0.0,
2000000.,379800.,24751897.68,25086068.20,.9999568475,.6461334829,2406.,24.62308,3.81044,3.85610,0.0,
2000000.,379800.,25781376.91,26243052.74,.9999359117,.6306895773,2337.,29.65162,3.81146,3.70326,0.0,
2000000.,379800.,26977133.89,27402231.82,.9999453995,.6133780528,2261.,34.26662,3.81257,3.54046,0.0,
600000.,261900.,23659233.56,23914389.02,.9999831405,.6630594147,2483.,19.67980,3.80929,4.03278,0.0,
500000.,271500.,2271.,30.53702,.9999950281,.3811454,0.0,0.0,0.0,0.0,0.0,
500000.,291600.,1453.,26.09287,.9999411765,.382109,0.0,0.0,0.0,0.0,0.0,
500000.,295200.,1453.,26.09287,.9999411765,.382109,0.0,0.0,0.0,0.0,0.0,
2000000.,304200.,36030443.05,36454924.53,.9999484343,.5025259,1802.,26.11701,3.80898,2.65643,0.0,
500000.,295800.,1792.,25.53386,.9999,.3817593,0.0,0.0,0.0,0.0,0.0,
500000.,303000.,1792.,25.53386,.9999,.3817593,0.0,0.0,0.0,0.0,0.0,
500000.,403800.,2491.,18.35156,.9999473684,.3807624,0.0,0.0,0.0,0.0,0.0,
500000.,410400.,2491.,18.35156,.9999473684,.3807624,0.0,0.0,0.0,0.0,0.0,
500000.,416700.,2491.,18.35156,.9999333333,.3806227,0.0,0.0,0.0,0.0,0.0,
500000.,318000.,2191.,37.04639,.999975,.3811074,0.0,0.0,0.0,0.0,0.0,
500000.,324600.,2191.,37.04639,.9999411765,.3811332,0.0,0.0,0.0,0.0,0.0,
500000.,308400.,2241.,32.84965,.9999666667,.3811064,0.0,0.0,0.0,0.0,0.0,
500000.,313500.,2241.,32.84965,.9999666667,.3811064,0.0,0.0,0.0,0.0,0.0,
2000000.,336600.,22736950.34,23162461.59,.9999453686,.6777445518,2551.,20.02265,3.80827,4.19479,0.0,
2000000.,336600.,23936585.11,24374096.67,.9999483705,.6587010213,2463.,22.59905,3.80959,3.98630,0.0,
2000000.,352800.,25644959.12,25979068.57,.9999568556,.6327148646,2346.,27.97215,3.81133,3.72376,0.0,
2000000.,354600.,26896024.48,27351521.50,.99993592,.6145281068,2266.,34.4102,3.81250,3.55102,0.0,
2000000.,303300.,26371820.68,26724051.82,.9999620817,.6220672671,2299.,30.63364,3.81202,3.62113,0.0,
2000000.,308700.,27467860.75,27832235.64,.9999453808,.6064623718,2231.,36.57874,3.81301,3.47771,0.0,
2000000.,333000.,33624568.36,34079629.33,.9999147417,.5287006734,1907.,12.68515,3.81758,2.84511,0.0,
2000000.,328800.,36271389.35,36756553.45,.9999257458,.5000126971,1792.,28.55026,3.81911,2.63885,0.0,
2000000.,328800.,41091749.54,41576762.39,.9998947956,.4540068519,1612.,59.30342,3.82138,2.27436,25.0,
500000.,246600.,2621.,15.15187,.9999,.380618,0.0,0.0,0.0,0.0,0.0,
500000.,252600.,2561.,16.25668,.9999666667,.3806575,0.0,0.0,0.0,0.0,0.0,
800000.,277200.,25989474.99,26369112.76,.9999498485,.6276341196,2323.,59.69369,3.81166,3.67392,0.0,
600000.,257400.,23111975.14,23549477.32,.9999645506,.6717286561,2523.,19.53138,3.8087,4.12738,0.0,
200000.,253800.,23784678.44,23924398.02,.9999984844,.6610953994,2474.,19.47463,3.80943,4.01174,0.0,
500000.,301200.,2481.,18.72150,.9999428571,.3807283,0.0,0.0,0.0,0.0,0.0,
500000.,308700.,2481.,18.72150,.9999090909,.3807541,0.0,0.0,0.0,0.0,0.0,
500000.,319500.,2481.,18.72150,.9999090909,.3805361,0.0,0.0,0.0,0.0,0.0,
2000000.,313200.,20041716.18,20589420.09,.9999410344,.7227899381,2768.,22.25085,3.80501,4.68430,36.,
2000000.,303600.,21001715.22,21594768.40,.9999509058,.7064074100,2687.,50.76661,3.80622,4.46875,35.,
2000000.,303600.,22564848.51,23069597.22,.9999450783,.6805292633,2564.,22.23938,3.80808,4.15706,33.,
2000000.,335160.,18984319.62,19471398.75,.9999028166,.7412196637,2861.,24.63011,3.80362,5.01609,0.0,
2000000.,339300.,20006679.72,20493457.15,.9999220223,.7233880702,2771.,20.89747,3.80497,4.76197,0.0,
2000000.,338400.,21327006.06,21874349.14,.9999220448,.7009277824,2661.,20.12517,3.80662,4.46959,0.0,
500000.,319800.,1772.,28.62716,.99996,.3817257,0.0,0.0,0.0,0.0,0.0,
500000.,325200.,1822.,21.00903,.9999411765,.3816986,0.0,0.0,0.0,0.0,0.0,
500000.,325800.,2141.,41.66790,.9999333333,.3812643,0.0,0.0,0.0,0.0,0.0,
500000.,333000.,2141.,41.66790,.9999333333,.3812422,0.0,0.0,0.0,0.0,0.0,
500000.,340200.,2161.,39.76857,.9999411765,.3812362,0.0,0.0,0.0,0.0,0.0,
2000000.,394200.,18689498.40,19157874.26,.9999714855,.7464518080,2888.,20.21285,3.80322,5.09490,0.0,
2000000.,394200.,19432939.76,19919806.36,.9999220151,.7333538278,2821.,21.96779,3.80422,4.90135,0.0,
2000000.,394200.,20500650.51,21096820.93,.9999107701,.7149012442,2729.,21.1582,3.8056,4.64814,0.0,
2000000.,360000.,23004346.29,23368977.46,.9999645501,.6734507906,2531.,19.30504,3.80858,4.14653,0.0,
2000000.,358200.,24104561.06,24590781.86,.9999220725,.6560764003,2451.,24.68139,3.80977,3.95865,0.0,
500000.,416100.,2076.,48.30429,.9999,.3812311,0.0,0.0,0.0,0.0,0.0,
500000.,420000.,2076.,48.30429,.9999,.3812311,0.0,0.0,0.0,0.0,0.0,
500000.,426900.,2076.,48.30429,.9999,.3812311,0.0,0.0,0.0,0.0,0.0,
500000.,258000.,2541.,16.76677,.9999666667,.3807327,0.0,0.0,0.0,0.0,0.0,
2000000.,268800.,2321.,27.02745,.9999750295,.3810845,0.0,0.0,0.0,0.0,0.0,
500000.,375600.,1852.,16.62358,.9999090909,.3816135,0.0,0.0,0.0,0.0,0.0,
500000.,382500.,1852.,16.62358,.9999,.3816204,0.0,0.0,0.0,0.0,0.0,
500000.,388200.,1852.,16.62358,.9999166667,.3816288,0.0,0.0,0.0,0.0,0.0,
500000.,267600.,2391.,22.84247,.9999666667,.3808377,0.0,0.0,0.0,0.0,0.0,
500000.,275700.,2391.,22.84247,.9999375,.380845,0.0,0.0,0.0,0.0,0.0,
500000.,282900.,2391.,22.84247,.9999375,.380875,0.0,0.0,0.0,0.0,0.0,
2000000.,266400.,24235000.80,24462545.30,.9999949,.6540820950,2442.,20.6424,3.8099,3.9378,0.0,
2000000.,284400.,29637059.47,30183611.25,.999872551,.57717077,2106.,51.60353,3.8148,3.22483,0.0,
2000000.,361800.,18819849.05,19215516.01,.9999358426,.7441333961,2876.,22.5795,3.80339,5.05972,0.0,
2000000.,361800.,19661027.79,20086977.18,.9999358523,.7293826040,2801.,20.45445,3.80452,4.84504,0.0,
2000000.,297000.,24048738.51,24559158.47,.9999391411,.6569503193,2455.,23.48125,3.80971,3.96783,0.0,
2000000.,297000.,25522875.81,26027071.12,.9999359346,.6345195439,2354.,28.63705,3.81121,3.74048,0.0,
2000000.,352800.,28657871.66,29082831.70,.9999454101,.5901470744,2161.,42.56887,3.81402,3.33440,0.0,
2000000.,352800.,30382831.06,30838032.96,.9999359432,.5676166827,2066.,52.48935,3.81537,3.14645,0.0,
2000000.,433800.,20836250.94,21383852.48,.999894581,.7091860222,2701.,22.08858,3.80602,4.57382,0.0,
2000000.,433800.,22341309.43,22888667.15,.9998946058,.6841473833,2581.,22.74104,3.80782,4.26823,0.0,
2000000.,279900.,23755351.27,24211050.37,.999956841,.6615397363,2476.,21.57953,3.8094,4.01753,0.0,
2000000.,279900.,24577800.67,24984826.43,.9999595012,.6487931668,2418.,23.87979,3.81026,3.88319,0.0,
500000.,257400.,2456.,19.72344,.99999375,.380922,0.0,0.0,0.0,0.0,0.0,
2000000.,291600.,30630125.53,31127724.75,.9999454207,.5644973800,2053.,53.44099,3.81555,3.12127,0.0,
2000000.,291600.,32252126.30,32676887.65,.9999326284,.54465157,1972.,3.57839,3.81669,2.94381,0.0,
2000000.,360000.,20922704.09,21366697.03,.999939116,.7077381841,2694.,18.93392,3.80612,4.55529,0.0,
2000000.,361200.,21993575.61,22461937.05,.9999068931,.6898519579,2608.,21.54370,3.80742,4.33519,0.0,
2000000.,309600.,29010231.09,29535149.91,.999948403,.5854397296,2141.,44.28313,3.81431,3.29422,0.0,
2000000.,365400.,29456907.29,29972959.94,.9999108771,.5795358654,2116.,48.58548,3.81466,3.24452,0.0,
2000000.,351000.,32187809.58,32691654.54,.9998726224,.5453944146,1975.,5.95074,3.81665,2.97107,0.0,
2000000.,361200.,34851703.46,35337121.23,.9998817443,.5150588857,1852.,21.62181,3.81832,2.7455,0.0,
2000000.,356400.,37261509.20,37807440.38,.9998632433,.4899126408,1752.,37.19059,3.81962,2.56899,0.0,
2000000.,354600.,41091749.54,41576762.39,.9998947956,.4540068519,1612.,59.30342,3.82138,2.33094,0.0,
2000000.,401400.,23894872.45,24229110.29,.9999568422,.6593554910,2466.,21.96231,3.80955,3.99323,0.0,
2000000.,401400.,25117176.75,25664114.42,.9998988207,.6405785926,2381.,29.30066,3.81081,3.80024,0.0,
2000000.,401400.,27025955.35,27432812.88,.9999512939,.6126873424,2258.,34.16878,3.81262,3.53414,0.0,
500000.,261000.,2541.,16.76677,.9999642857,.380742,0.0,0.0,0.0,0.0,0.0,
2000000.,282600.,26230200.09,26576444.45,.9999483859,.6241178597,2308.,30.78682,3.81189,3.64047,0.0,
2000000.,282600.,27434800.06,27811312.71,.9999454027,.6069248249,2233.,36.41072,3.81298,3.48187,0.0,
2000000.,435000.,18798081.67,19205863.43,.9999422551,.7445203390,2878.,22.15711,3.80336,5.06556,0.0,
2000000.,433800.,19832653.52,20289119.60,.9999145875,.7263957947,2786.,21.72121,3.80474,4.80336,0.0,
2000000.,286200.,25305029.12,25715126.55,.999940746,.6377729696,2368.,57.52979,3.81099,3.77244,0.0,
2000000.,291600.,26639323.45,27070620.78,.9999256928,.6181953936,2282.,33.82207,3.81227,3.58491,0.0,
2000000.,324000.,20124133.05,20489179.67,.9999453461,.7213707913,2761.,19.04034,3.80511,4.73451,0.0,
2000000.,324000.,21050746.99,21430913.91,.9999407059,.7055766312,2683.,48.81363,3.80628,4.52782,0.0,
2000000.,324000.,22161432.25,22672134.66,.9999325474,.6871032423,2595.,20.01691,3.80761,4.30274,0.0,
500000.,378600.,2431.,20.83533,.9999411765,.3808422,0.0,0.0,0.0,0.0,0.0,
500000.,386400.,2431.,20.83533,.9999411765,.3808422,0.0,0.0,0.0,0.0,0.0,
500000.,391500.,2431.,20.83533,.9999411765,.3808422,0.0,0.0,0.0,0.0,0.0,
500000.,396300.,2431.,20.83533,.9999411765,.3808422,0.0,0.0,0.0,0.0,0.0 };

stpll(STZON,lat,lon,NOR,EAS) 
	int *STZON;
	float *lat, *lon;
 	double *NOR, *EAS;
{
      int ii, ier, icode;
      double FLAT, FLON, FAC1, FAC2, COEFF[11];
      double SAPRX,S,DIFF,WSEC,SINPI,COSPI,FAC7,FAC8,CON1;
      double PHR,SA,S1,DLON1,THETR,TETS,FLONS,R,YPRI;
      double XPR,SG1,SG,SM,W,WR,PHIS,PHIR,DPHI,PHI;

      icode = 0;
      for (ier=0;ier<=112;ier++)
	{
	icode = *STZON - zones[ier];
        if ((*STZON - zones[ier]) == 0)
	   {
	   icode = 1;
	   break;
	   }
	}
      if (icode == 0)
	{
	*STZON = -1; return; }
      if (icode == 1)
	{
/* Get the coefficient parameters for this state/zone */
	for (ii=0; ii<=10; ii++)
	 { COEFF[ii] = coeff[ier][ii]; }
        FAC1 = 60.0;
        FAC2 = 3600.0;
        FAC7 = 3600.0;
        FAC8 = 57.2957795E0;
        CON1 = FAC7*FAC8;
	if (COEFF[9] == 0.0)
	  { /*     MERCATOR PROJECTION    */
          XPR = *EAS-COEFF[0]+.1E-12;
          SG1 = XPR-COEFF[5] * ((XPR/10.E4)*(XPR/10.E4)*(XPR/10.E4));
          SG = XPR-COEFF[5] * ((SG1/10.E4)*(SG1/10.E4)*(SG1/10.E4));
          SM = .3048006099E0*SG/COEFF[4];
          W = 60.E0*COEFF[2]+COEFF[3]+.9873675553E-2*(*NOR)/COEFF[4];
          WR = W/CON1;
          SINPI = sin(WR);
          COSPI = cos(WR);
          PHIS = W+SINPI*COSPI*(1047.54671E0+COSPI*COSPI*(6.19276E0+.050912E0*COSPI*COSPI));
          PHIR = PHIS/CON1;
          SINPI = sin(PHIR);
          COSPI = cos(PHIR);
          DPHI = 25.52381E-10*(SM*SM)*(SINPI/COSPI)*(1.E0-.6768658E-2*SINPI*SINPI)*(1.E0-.6768658E-2*SINPI*SINPI);
          PHI = PHIS-DPHI;
          PHR = PHI/CON1;
          SA = SM-4.0831E0 * ((SM/1.E5)*(SM/1.E5)*(SM/1.E5));
          S1 = SM-4.0831E0 * ((SA/1.E5)*(SA/1.E5)*(SA/1.E5));
          DLON1 = S1*sqrt(1.E0-.6768658E-2*sin(PHR)*sin(PHR))/(30.92241724E0*cos(PHR));
          FLON = DLON1+3.9174E0 * ((DLON1/1.E4)*(DLON1/1.E4)*(DLON1/1.E4));
          FLON = DLON1+3.9174E0 * ((FLON/1.E4)*(FLON/1.E4)*(FLON/1.E4));
          THETR = FLON*sin(PHIR)/CON1;
          *(lon) = (COEFF[1]-FLON)/FAC7;
          *(lat) = PHI/FAC7;
          }
	if (COEFF[9] != 0.0)
          {           /*     LAMBERT PROJECTION   */
          THETR = atan((*EAS-COEFF[0])/(COEFF[3]-*NOR));
          TETS = THETR*FAC7*FAC8;
          FLONS = COEFF[1]-TETS/COEFF[5];
          R = (COEFF[3]-*NOR)/cos(THETR);
          YPRI = *NOR-2.E0*R*sin(THETR/2.E0)*sin(THETR/2.E0);
          SAPRX = (COEFF[3]-COEFF[2]-YPRI)/COEFF[4];
          S = SAPRX;
          S1 = S;
          for (ii=1;ii<=100;ii++)
            {
	    S = SAPRX/(1.E0 + ((S/1.E8)*(S/1.E8)) *COEFF[8] - ((S/1.E8)*(S/1.E8)*(S/1.E8))*COEFF[9]);
            DIFF = fabs(S1-S);
            S1 = S;
            if (DIFF <= 0.01E0) break;
            }
          WSEC = 60.E0*COEFF[6]+COEFF[7]-.987367553E-2*S;
          WR = WSEC/CON1;
          SINPI = sin(WR);
          COSPI = cos(WR);
          PHIS = WSEC+SINPI*COSPI*(1047.54671E0+COSPI*COSPI*(6.19276E0+0.050912E0*COSPI*COSPI));
          *(lat) = PHIS/FAC7;
          *(lon) = FLONS/FAC7;
          }
        }
      return;
}
