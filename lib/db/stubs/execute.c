#include "dbmi.h"

int
db_driver_execute_immediate (SQLstatement)
    dbString *SQLstatement;
{
    db_procedure_not_implemented("db_execute_immediate");
    return DB_FAILED;
}

/* Implemented only in some drivers */
int
db_driver_begin_transaction ( void )
{
    G_debug (2, "Begin transaction" );
    return DB_OK;
}

/* Implemented only in some drivers */
int
db_driver_commit_transaction ( void )
{
    G_debug (2, "Commit transaction" );
    return DB_OK;
}
