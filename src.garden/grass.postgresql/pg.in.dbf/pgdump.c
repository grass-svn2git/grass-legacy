/******************************************************************************
 * Copyright (c) 1998, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * 10/2003 Thierry Laronde <tlaronde@polynum.org> 
	- change PgDumpFromDBF in order to specify a custom DELIMITER
	- change DBFDumpASCII to take a parameter for specifying the
	delimiter
	- include "pgdump.h" for PgDumpFromDBF proto
	- change "float4" to "numeric" in order to not loose in precision
	XXX The infos I have found tell that DBF numeric field can be up
	to 20 digits long; numeric by default is (30,6) BUT can the numeric
	field of the DBF have more than a 6 scale length? In this case a
	safe way would be to define the conversion to numeric(40,20) but
	isn't it a bit too much?
 *
 * 12/2000 Federico Ponchio ponhio@dm.unipi.it (minor changes to memory 
 *  allocation for inserting in normal mode)
 * 02/2000 Alex Shevlakov sixote@yahoo.com	
 *			      
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "gis.h"
#include "shapefil.h"
#include <libpq-fe.h>
#include "glocale.h"
#include "pgdump.h"

typedef unsigned char uchar;

/* ok... i guess someone invented C++ just for this...*/

struct my_string
{
    char *data;
    int len;
    int totlen;
};

int init(struct my_string *str)
{
    str->data = (char *) malloc(1024 * sizeof(char));
    if (str->data == NULL) {
	fprintf(stderr, _("Failed to allocate new memory.\n"));
	exit(-1);
    }
	/* Initialize to null string or `strcat' used in `append' will 
	 * concatenate every random data available here till it
	 * finds some '\0' !
	 */
	*str->data = '\0'; 
    str->len = 1;
    str->totlen = 128;
    return (0);
}

int append(struct my_string *str, const char *s)
{
    int newlen;
    newlen = strlen(s);
    str->len += newlen;
    if (str->len >= str->totlen) {
	str->totlen = str->len;
	str->data = (char *) realloc(str->data, sizeof(char) * str->len);
	if (str->data == NULL) {
	    fprintf(stderr, _("Failed to allocate new memory.\n"));
	    exit(-1);
	}
    }
    strcat(str->data, s);
    return (0);
}

int clear(struct my_string *str)
{
    strcpy(str->data, "");
    str->len = 1;
    return (0);
}

int delete(struct my_string *str)
{
    if (str->data != NULL)
	free(str->data);
    return (0);
}

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/

static void *SfRealloc(void *pMem, int nNewSize)
{
    if (pMem == NULL)
	return ((void *) malloc(nNewSize));
    else
	return ((void *) realloc(pMem, nNewSize));
}

/************************************************************************/
/*                          DBFDumpASCII()                          	*/
/*                                                                      */
/*     		 Dumps DBF to DELIMITER-separated list. 			*/
/************************************************************************/

int DBFDumpASCII(DBFHandle psDBF, FILE * fp, char delim)
{
    int nRecordOffset;
    uchar *pabyRec;
    /*void      *pReturnField = NULL; */
    int hEntity = 0, iField = 0;

    /*static double dDoubleField; */
    static char *pszStringField = NULL;
    static int nStringFieldLen = 0;
    static char single_line[4096] = "";

	/* transform the delim char in a string for insertion via %s */
	char delim_string[2]; 
	delim_string[0] = delim;
	delim_string[1] = '\0';

    for (hEntity = 0; hEntity < psDBF->nRecords; hEntity++) {

	single_line[0] = '\0';

	nRecordOffset = psDBF->nRecordLength * hEntity + psDBF->nHeaderLength;

	fseek(psDBF->fp, nRecordOffset, 0);
	fread(psDBF->pszCurrentRecord, psDBF->nRecordLength, 1, psDBF->fp);
	psDBF->nCurrentRecord = hEntity;
	pabyRec = (uchar *) psDBF->pszCurrentRecord;

	for (iField = 0; iField < psDBF->nFields; iField++) {
	    char tmp_buf[1024] = "";



/* -------------------------------------------------------------------- */
/*	Ensure our field buffer is large enough to hold this buffer.	*/
/* -------------------------------------------------------------------- */
	    if (psDBF->panFieldSize[iField] + 1 > nStringFieldLen) {
		nStringFieldLen = psDBF->panFieldSize[iField] * 2 + 10;
		pszStringField =
		    (char *) SfRealloc(pszStringField, nStringFieldLen);
	    }

/* -------------------------------------------------------------------- */
/*	Extract the requested field.					*/
/* -------------------------------------------------------------------- */
	    strncpy(pszStringField, pabyRec + psDBF->panFieldOffset[iField],
		    psDBF->panFieldSize[iField]);
	    pszStringField[psDBF->panFieldSize[iField]] = '\0';

	    /*Remove white spaces if any */
#ifdef TRIM_DBF_WHITESPACE
	    if (1) {
		char *pchSrc, *pchDst;

		pchDst = pchSrc = pszStringField;
		while (*pchSrc == ' ')
		    pchSrc++;

		while (*pchSrc != '\0')
		    *(pchDst++) = *(pchSrc++);
		*pchDst = '\0';

		while (*(--pchDst) == ' ' && pchDst != pszStringField)
		    *pchDst = '\0';

	    }
#endif
	    if (!iField)
		snprintf(tmp_buf, 1024, "%s", pszStringField);
	    else if (iField == psDBF->nFields - 1)
		snprintf(tmp_buf, 1024, "%s%s\n", delim_string, pszStringField);
	    else
		snprintf(tmp_buf, 1024, "%s%s", delim_string, pszStringField);


	    strncat(single_line, tmp_buf, strlen(tmp_buf));

	}

	fwrite(single_line, strlen(single_line), 1, fp);

	if (ferror(fp)) {
	    fprintf(stderr, _("Error occurred while writing to tmp file!\n"));
	    fclose(fp);
	    exit(-1);
	}
    }



    return 0;
}

int PgDumpFromDBF(char *infile, int normal_user, char delim, const char *null_string)
{

    DBFHandle hDBF;
    char buf[256] = "";

    int i, j;
    char *dbname, *pp;
					

    struct my_string SQL_create;
    struct my_string SQL_insert;
    struct my_string chunks;
    struct my_string fldstrng;

    static char name[128] = "";

    PGconn *pg_conn;
    PGresult *res;
    char *pghost;

    FILE *fp;
    char *tmpfile_nm;

	/* transform the delim char in a string for insertion via %s */
	char delim_string[2]; 
	delim_string[0] = delim;
	delim_string[1] = '\0';

    init(&SQL_create);
    init(&SQL_insert);
    init(&chunks);
    init(&fldstrng);

    /* Check DATABASE env variable */
    if ((dbname = G__getenv("PG_DBASE")) == NULL) {
	fprintf(stderr,
		_
		("Please run g.select.pg to identify a current database.\n"));
	exit(-1);
    }
/* -------------------------------------------------------------------- */
/*      Extract basename of dbf file.                                   */
/* -------------------------------------------------------------------- */
    for (pp = infile + strlen(infile) - 1;
	 pp != infile - 1 && (isalnum(*pp) || *pp == '_' || *pp == '.');
	 pp--) {
    }
    strcpy(name, pp + 1);

    pp = strrchr(name, '.');
    if (pp != NULL)
	*pp = '\0';

    /* Open the dbf file */
    hDBF = NULL;
    hDBF = DBFOpen(infile, "r");

    if (hDBF == NULL) {
	sprintf(buf, _("%s - DBF not found, or wrong format.\n"), infile);
	G_fatal_error(buf);
    }


    for (i = 0; i < DBFGetFieldCount(hDBF); i++) {
	char field_name[128];
	int field_width;
	char *fld = "";


	DBFFieldType ftype;

	ftype = DBFGetFieldInfo(hDBF, i, field_name, &field_width, NULL);

	switch (ftype) {
	case 0:
	    fld = "text";
	    break;
	case 1:
	    if (field_width <= 7)
		fld = "int4";
	    else
		fld = "int8";
	    break;
	case 2:
	    fld = "numeric";
	    break;
	case 3:
	    G_fatal_error(_("Invalid field type - bailing out"));
	    break;
	}

	/*chunks -for create stmt */
	append(&chunks, field_name);
	append(&chunks, " ");
	append(&chunks, fld);
	append(&chunks, ",");

	/*fldstrng - for insert stmt */

	append(&fldstrng, field_name);
	append(&fldstrng, ",");

    }
    /*stripping last commas */
    pp = strrchr(chunks.data, ',');
    if (pp != NULL)
	*pp = '\0';

    pp = strrchr(fldstrng.data, ',');
    if (pp != NULL)
	*pp = '\0';

    append(&SQL_create, "create table ");
    append(&SQL_create, name);
    append(&SQL_create, " (");
    append(&SQL_create, chunks.data);
    append(&SQL_create, ")");

    pghost = G__getenv("PG_HOST");

    pg_conn = PQsetdb(pghost, NULL, NULL, NULL, G_getenv("PG_DBASE"));
    if (PQstatus(pg_conn) == CONNECTION_BAD) {
	fprintf(stderr, _("Error Quering Postgres:%s\n"),
		PQerrorMessage(pg_conn));
	PQfinish(pg_conn);
	exit(-1);
    }
    fprintf(stdout, _("Executing %s\n"), SQL_create.data);
    res = PQexec(pg_conn, SQL_create.data);

    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
	fprintf(stderr,
		_
		("\n**********************\n%s\nPlease make sure that created table name is not used by another table.\n"),
		PQresultErrorMessage(res));
	PQclear(res);
	PQfinish(pg_conn);
	DBFClose(hDBF);
	exit(-1);
    }

    PQclear(res);

    if (!normal_user) {

	char nm[32] = "";
	uchar ch = 'y';


	fprintf(stdout,
		_("Additionally dump to ASCII file (enter full Unix name or hit <Enter> for none):\n"));
	if (fgets(buf, sizeof(buf), stdin) == NULL || !strlen(buf)) {
	    fprintf(stdout, _("OK, writing to temporary file\n"));
	    tmpfile_nm = G_tempfile();
	    ch = 'n';

	}
	else {
	    sscanf(buf, "%s", nm);
	    if (strlen(nm))
		tmpfile_nm = nm;
	    else {
		fprintf(stdout, _("OK, writing to temporary file\n"));
		tmpfile_nm = G_tempfile();
		ch = 'n';
	    }

	}

	if ((fp = fopen(tmpfile_nm, "w")) == NULL) {
	    fprintf(stderr,
		    _("File write error on temporary file %s\nHint: Check write permissions for current catalogue"),
		    tmpfile_nm);
	    append(&SQL_insert, "drop table ");
	    append(&SQL_insert, name);

	    res = PQexec(pg_conn, SQL_insert.data);
	    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
		fprintf(stderr, "Error: drop table:%s\n",
			PQerrorMessage(pg_conn));

	    PQclear(res);
	    PQfinish(pg_conn);
	    DBFClose(hDBF);
	    exit(-1);
	}

	DBFDumpASCII(hDBF, fp, delim);

	fclose(fp);

	clear(&SQL_insert);
	append(&SQL_insert, "copy ");
	append(&SQL_insert, name);
	append(&SQL_insert, " from '");
	append(&SQL_insert, tmpfile_nm);
	append(&SQL_insert, "' using delimiters '");
	append(&SQL_insert, delim_string);
	append(&SQL_insert, "' with null as '");
	append(&SQL_insert, null_string);
	append(&SQL_insert, "'");

	fprintf(stdout, _("Executing %s\n"), SQL_insert.data);

	res = PQexec(pg_conn, SQL_insert.data);

	if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {

	    fprintf(stderr,
		    _("********************\nERROR:%s\nThe table has NOT been created.\nYou must be Postgres superuser to COPY table. Choose normal user dumpmode.\n"),
		    PQresultErrorMessage(res));

	    PQclear(res);
	    clear(&SQL_insert);
	    append(&SQL_insert, "drop table ");
	    append(&SQL_insert, name);

	    res = PQexec(pg_conn, SQL_insert.data);
	    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
		fprintf(stderr, "Error: drop table:%s\n",
			PQerrorMessage(pg_conn));

	    PQclear(res);

	    if (ch != 'y')
		unlink(tmpfile_nm);
	    PQfinish(pg_conn);
	    DBFClose(hDBF);
	    exit(-1);
	}
	if (ch != 'y')
	    unlink(tmpfile_nm);
	fprintf(stdout,
		_("\nTable %s successfully copied into Postgres. Congratulations!\n"),
		name);
	PQclear(res);
    }
    else {
	struct my_string valstrng;
	init(&valstrng);

	/*Loop over records */
	for (i = 0; i < hDBF->nRecords; i++) {
	    clear(&valstrng);
	    /*   char valstrng[1024]=""; */

	    for (j = 0; j < DBFGetFieldCount(hDBF); j++) {

		char field_name[15];
		/*      char        c_tmpbuf[1024]; */
		char fld[1024];

		DBFFieldType ftype;

		ftype = DBFGetFieldInfo(hDBF, j, field_name, NULL, NULL);

		switch (ftype) {
		case 0:
		    append(&valstrng, "'");
		    append(&valstrng, DBFReadStringAttribute(hDBF, i, j));
		    append(&valstrng, "'");
		    break;
		case 1:
		    snprintf(fld, 1024, "%d",
			     DBFReadIntegerAttribute(hDBF, i, j));
		    append(&valstrng, fld);

		    break;
		case 2:
		    snprintf(fld, 1024, "%f",
			     DBFReadDoubleAttribute(hDBF, i, j));
		    append(&valstrng, fld);
		    break;
		case 3:
		    G_fatal_error(_("Invalid field type - bailing out"));
		    break;
		}
		append(&valstrng, ",");
		/*valstrng -for insert stmt */
	    }

	    pp = strrchr(valstrng.data, ',');
	    if (pp != NULL)
		*pp = '\0';
	    clear(&SQL_insert);
	    append(&SQL_insert, "insert into ");
	    append(&SQL_insert, name);
	    append(&SQL_insert, " (");
	    append(&SQL_insert, fldstrng.data);
	    append(&SQL_insert, ") values (");
	    append(&SQL_insert, valstrng.data);
	    append(&SQL_insert, ")");

	    fprintf(stdout, _("Executing %s\n"), SQL_insert.data);

	    res = PQexec(pg_conn, SQL_insert.data);
	    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
		fprintf(stderr, "Error in INSERT:%s\n",
			PQerrorMessage(pg_conn));
	    PQclear(res);
	}
	fprintf(stdout,
		_
		("\nSuccessfully inserted %d records to Postgres table %s\n"),
		hDBF->nRecords, name);
    }

    PQfinish(pg_conn);
    DBFClose(hDBF);

    return 0;
}
