#include <libpq-fe.h>

/* cursors */
typedef struct _cursor {
    PGresult *res;
    int nrows;             /* number of rows in query result */
    int row;               /* current row */
    dbToken token;
    int type;              /* type of cursor: SELECT, UPDATE, INSERT */
    int *cols;             /* indexes of known (type) columns */ 
    int ncols;             /* number of known columns */
} cursor;  

typedef struct {
        char *host, *port, *options, *tty, *dbname, *user, *password;
} PGCONN;

/* Postgres data types defined in GRASS */
typedef enum {     /* name in pg_type, aliases */
  PG_TYPE_UNKNOWN,  /* all types not supported by GRASS */

  PG_TYPE_INT2,	    /* int2,	smallint */
  PG_TYPE_INT4,      /* int4,	integer, int */
  PG_TYPE_SERIAL,    /* serial */
  PG_TYPE_OID,       /* oid */

  PG_TYPE_REAL,      /* real,   float4 */
  PG_TYPE_FLOAT8,    /* float8, double precision */
  PG_TYPE_NUMERIC,   /* numeric, decimal */
  
  PG_TYPE_CHAR,      /* char,	character */
  PG_TYPE_BPCHAR,    /* ??? blank padded character, oid of this type is returned for char fields */
  PG_TYPE_VARCHAR,   /* varchar,	character varying */
  
  PG_TYPE_TEXT,      /* text */
  
  PG_TYPE_DATE,      /* date */
  PG_TYPE_TIME,      /* time */
  PG_TYPE_TIMESTAMP, /* timestamp */
} PG_TYPES;


#ifdef MAIN
    PGconn *pg_conn; /* Database connection */
    int (*pg_types)[2] = NULL;   /* array of types, first is internal code, second PG_TYPE_* */
    int pg_ntypes = 0;
    dbString *errMsg = NULL; /* error message */
#else
    extern PGconn *pg_conn; 
    extern dbString *errMsg;
    extern int (*pg_types)[2];
    extern int pg_ntypes;
#endif 


