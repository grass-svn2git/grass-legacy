#include "dbmi.h"
#include "dbstubs.h"

static char *rfind();
static int make_parent_dir();
static int make_dir();

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
db_driver_mkdir (path, mode, parentdirs)
    char *path;
    int mode;
    int parentdirs;
{
    if (parentdirs)
    {
	if (make_parent_dir (path, mode) != DB_OK)
	    return DB_FAILED;
    }
    return make_dir (path, mode);
}

/* make a directory if it doesn't exist */
/* this routine could be made more intelligent as to why it failed */
static int
make_dir (path, mode)
    char *path;
    int mode;
{
    if (db_isdir(path) == DB_OK)
	return DB_OK;
    if (mkdir (path, mode) == 0)
	return DB_OK;
    db_syserror(path);
    return DB_FAILED;
}

static
make_parent_dir(path, mode)
    char *path;
    int mode;
{
    char *slash;
    int stat;

    slash = rfind(path,'/');
    if (slash == NULL || slash == path)
	return DB_OK; /* no parent dir to make. return ok */

    *slash = 0; /* add NULL to terminate parentdir string */
    if (access(path,0) == 0) /* path exists, good enough */
    {
	stat = DB_OK;
    }
    else if (make_parent_dir (path, mode) != DB_OK)
    {
	stat = DB_FAILED;
    }
    else if(make_dir (path, mode) == DB_OK)
    {
	stat = DB_OK;
    }
    else
    {
	stat = DB_FAILED;
    }
    *slash = '/';  /* put the slash back into the path */
    return stat;
}

static 
char *rfind(string, c)
    char *string;
    char c;
{
    char *found;

    found = NULL;
    while (*string)
    {
	if (*string == c)
	    found = string;
	string++;
    }
    return found;
}
