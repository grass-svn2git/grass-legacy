#include "dbmi.h"
#include "macros.h"

db_delete_database (driver, handle)
    dbDriver *driver;
    dbHandle *handle;
{
    int ret_code;

/* start the procedure call */
    db__set_protocol_fds (driver->send, driver->recv);
    DB_START_PROCEDURE_CALL(DB_PROC_DELETE_DATABASE);

/* send the arguments to the procedure */
    DB_SEND_HANDLE (handle);

/* get the return code for the procedure call */
    DB_RECV_RETURN_CODE(&ret_code);

    if (ret_code != DB_OK)
	return ret_code; /* ret_code SHOULD == DB_FAILED */

/* no results */
    return DB_OK;
}
