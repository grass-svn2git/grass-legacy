.TH m.ipf
.SH NAME
\fIm.ipf\fR \- Iterative proportional fitting for error matrices.
.br	
.I (GRASS Data Import/Processing Program)
.SH SYNOPSIS
\fBm.ipf \fR[\fB-emz\fR] [\fBinput\*=\fIname\fR] [\fBformat\*=\fIstring\fR]
[\fBstop\*=\fIvalue\fR]
.SH DESCRIPTION
.I m.ipf 
uses an error or confusion matrix produced by 
.I r.coin
or 
.I r.kappa,
smooths zero counts, 
and does iterative proportional fitting to normalize the matrix.
.LP
\fBFlags:\fR
.IP \fB-e\fR 18
Indicate when the iterative algorithm finished.
.IP \fB-m\fR 18
Print the marginals (row and column totals) with each matrix.
.IP \fB-z\fR 18
Print the intermediate (smoothed) matrix.
.LP
\fBParameters:\fR
.IP \fBinput\*=\fIname\fR 18
The input file must have the following format:
the first line contains an integer K
which is the number of rows and columns in the matrix;
the remainder of the file is the matrix, i.e.,
K lines, each containing K integers.
If the input is not specified on the command line, it may
come from standard input.
.IP \fBformat\*=\fIstring\fR 18
Specifies the format conversion string used to print the results.
Default is \fI%7.3f\fR. For details, see 
.I printf(3).
.IP \fBstop\*=\fIvalue\fR 18
The stopping criteria is a floating point number which actually
specifies an integer maximum number of iterations and a fractional
change in marginal. The default, 100.01, specifies
that the interative proportional fitting will stop
at 100 iterations or when marginals do not change by 0.01,
whichever comes first.
.SH EXAMPLE
For the following input,
.RS
.TS
tab(:);
lnnn
nnnn.
3
712:    0:   12
0:  584:    2
18:    0:  434
.TE
.RE

zero counts in the matrix will be smoothed:

.RS
.TS
tab(:);
lnnn
nnnn.
711.249:  0.438: 12.314 
  0.443:583.289:  2.268 
 18.309:  0.273:433.418 
.TE
.RE

and the matrix will be normalized to yield:

.RS
.TS
tab(:);
lnnn
nnnn.
  0.969:0.001:0.022 
  0.001:0.999:0.004 
  0.031:0.001:0.973 
.TE
.RE
.SH PROGRAM NOTES
Iterative proportional curve fitting is useful when comparing
the output of image classification algorithms (for example,
\fIi.maxlik\fR and \fIi.smap\fR),
especially when training fields (signatures) and/or test fields
are different. The diagonals of the normalized matrix can be used 
in a Tukey multiple comparison test.
.SH SEE ALSO
.I r.coin,
.I r.kappa,
.I printf(3),
and
Zhuang, X., B.A. Engel, X. Xiong, and C. Johanssen. 1994.
Analysis of Classification Results of Remotely
Sensed Data and Evaluation of Classification Algorithms,
\fI Photogrammetric Engineering and Remote Sensing\fR
(in press)
.SH BUGS
Please send all bug fixes and comments to the author.
.SH AUTHOR
James Darrell McCauley, Purdue University 
.if n .br 
(mccauley@ecn.purdue.edu)
