#include "dbmi.h"

int
db_driver_execute_immediate (SQLstatement)
    dbString *SQLstatement;
{
    db_procedure_not_implemented("db_execute_immediate");
    return DB_FAILED;
}

