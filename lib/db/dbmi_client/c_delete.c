#include "dbmi.h"
#include "macros.h"

/*!
 \fn int db_delete (dbCursor *cursor)
 \brief 
 \return 
 \param 
*/
int
db_delete (dbCursor *cursor)
{
    int ret_code;

/* start the procedure call */
    db__set_protocol_fds (cursor->driver->send, cursor->driver->recv);
    DB_START_PROCEDURE_CALL(DB_PROC_DELETE);

/* send the argument(s) to the procedure */
    DB_SEND_TOKEN (&cursor->token);

/* get the return code for the procedure call */
    DB_RECV_RETURN_CODE(&ret_code);

    if (ret_code != DB_OK)
	return ret_code; /* ret_code SHOULD == DB_FAILED */

/* no results */
    return DB_OK;
}
