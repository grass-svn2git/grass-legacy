#include "dbmi.h"
#include "macros.h"

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
db_open_update_cursor (driver, table_name, select, cursor, mode)
    dbDriver *driver;
    dbString *table_name;
    dbString *select;
    dbCursor *cursor;
    int mode;
{
    int ret_code;

    db_init_cursor (cursor);
    cursor->driver = driver;

/* start the procedure call */
    db__set_protocol_fds (driver->send, driver->recv);
    DB_START_PROCEDURE_CALL(DB_PROC_OPEN_UPDATE_CURSOR);

/* send the argument(s) to the procedure */
    DB_SEND_STRING (table_name);
    DB_SEND_STRING (select);
    DB_SEND_INT (mode);

/* get the return code for the procedure call */
    DB_RECV_RETURN_CODE(&ret_code);

    if (ret_code != DB_OK)
	return ret_code; /* ret_code SHOULD == DB_FAILED */

/* get the results */
    DB_RECV_TOKEN(&cursor->token);
    DB_RECV_INT(&cursor->type);
    DB_RECV_INT(&cursor->mode);
    DB_RECV_TABLE_DEFINITION(&cursor->table);
    db_alloc_cursor_column_flags (cursor);
    return DB_OK;
}
