#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "dbmi.h"

/*!
 \fn dbColumn *db_get_table_column (dbTable *table, int n)
 \brief 
 \return 
 \param 
*/
dbColumn *
db_get_table_column (dbTable *table, int n)
{
    if (n < 0 || n >= table->numColumns)
	return ((dbColumn *)NULL);
    return &table->columns[n];
}

/*!
 \fn dbValue *db_get_column_value (dbColumn *column)
 \brief 
 \return 
 \param 
*/
dbValue *
db_get_column_value (dbColumn *column)
{
    return &column->value;
}

/*!
 \fn dbValue *db_get_column_default_value (dbColumn *column)
 \brief 
 \return 
 \param 
*/
dbValue *
db_get_column_default_value (dbColumn *column)
{
    return &column->defaultValue;
}

/*!
 \fn void db_set_column_sqltype (dbColumn *column, int sqltype)
 \brief 
 \return 
 \param 
*/
void
db_set_column_sqltype (dbColumn *column, int sqltype)
{
    column->sqlDataType = sqltype;
}

/*!
 \fn void db_set_column_host_type (dbColumn *column, int type)
 \brief 
 \return 
 \param 
*/
void
db_set_column_host_type (dbColumn *column, int type)
{
    column->hostDataType = type;
}

/*!
 \fn void db_get_column_host_type (dbColumn *column)
 \brief 
 \return 
 \param 
*/
int
db_get_column_scale (dbColumn *column)
{
    return column->scale;
}

/*!
 \fn void db_set_column_scale (dbColumn *column, int scale)
 \brief 
 \return 
 \param 
*/
void
db_set_column_scale (dbColumn *column, int scale)
{
    column->scale = scale;
}

/*!
 \fn int db_get_column_precision (dbColumn *column)
 \brief 
 \return 
 \param 
*/
int
db_get_column_precision (dbColumn *column)
{
    return column->precision;
}

/*!
 \fn void db_set_column_precision (dbColumn *column, int precision)
 \brief 
 \return 
 \param 
*/
void
db_set_column_precision (dbColumn *column, int precision)
{
    column->precision = precision;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_get_column_sqltype (column)
    dbColumn *column;
{
    return column->sqlDataType;
}

int
db_get_column_host_type (column)
    dbColumn *column;
{
    return column->hostDataType;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
/* returns column sqltype  or -1 on error*/
int
db_column_sqltype ( 
    dbDriver *driver,  
    char *tab, /* table name */ 
    char *col) /* column  name*/
{
    dbTable *table;
    dbString table_name;
    dbColumn *column;
    int ncol, cl, type;

    db_init_string(&table_name);
    db_set_string(&table_name, tab);
    
    if(db_describe_table (driver, &table_name, &table) != DB_OK)
       return -1;
    
    db_free_string ( &table_name );
    ncol = db_get_table_number_of_columns(table);
    for (cl = 0; cl < ncol; cl++) {
	column = db_get_table_column (table, cl);
	if ( strcmp (  db_get_column_name(column), col ) == 0 ) {
	    type = db_get_column_sqltype(column);
	    return type;
	}
    }
    
    return -1;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
/* returns column Ctype  or -1 on error */
int
db_column_Ctype ( 
    dbDriver *driver,  
    char *tab, /* table name */ 
    char *col) /* column  name*/
{
    int type;
    if ( ( type = db_column_sqltype ( driver, tab, col ) ) >= 0 ) {
	type = db_sqltype_to_Ctype(type); 
	return type;
    }

    return -1;
}
    
/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_has_defined_default_value(column)
    dbColumn *column;
{
    column->hasDefaultValue = DB_DEFINED;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_has_undefined_default_value(column)
    dbColumn *column;
{
    column->hasDefaultValue = DB_UNDEFINED;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_unset_column_has_default_value(column)
    dbColumn *column;
{
    column->hasDefaultValue = 0;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_test_column_has_default_value(column)
    dbColumn *column;
{
    return (column->hasDefaultValue != 0);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_test_column_has_defined_default_value(column)
    dbColumn *column;
{
    return (column->hasDefaultValue == DB_DEFINED);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_test_column_has_undefined_default_value(column)
    dbColumn *column;
{
    return (column->hasDefaultValue == DB_UNDEFINED);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_use_default_value(column)
    dbColumn *column;
{
    column->useDefaultValue = 1;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_unset_column_use_default_value(column)
    dbColumn *column;
{
    column->useDefaultValue = 0;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_test_column_use_default_value(column)
    dbColumn *column;
{
    return (column->useDefaultValue != 0);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_null_allowed(column)
    dbColumn *column;
{
    column->nullAllowed = 1;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_unset_column_null_allowed(column)
    dbColumn *column;
{
    column->nullAllowed = 0;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_test_column_null_allowed(column)
    dbColumn *column;
{
    return (column->nullAllowed != 0);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_get_column_length (column)
    dbColumn *column;
{
    return column->dataLen;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_length (column, length)
    dbColumn *column;
    int length;
{
    column->dataLen = length;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_select_priv_granted (column)
    dbColumn *column;
{
    column->select = DB_GRANTED;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_select_priv_not_granted (column)
    dbColumn *column;
{
    column->select = DB_NOT_GRANTED;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_get_column_select_priv (column)
    dbColumn *column;
{
    return column->select;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_update_priv_granted (column)
    dbColumn *column;
{
    column->update = DB_GRANTED;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_set_column_update_priv_not_granted (column)
    dbColumn *column;
{
    column->update = DB_NOT_GRANTED;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_get_column_update_priv (column)
    dbColumn *column;
{
    return column->update;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_init_column (column)
    dbColumn *column;
{
    db_zero ((void *)column, sizeof(dbColumn));
    db_init_string (&column->columnName);
    db_init_string (&column->description);
    db_init_string (&column->value.s);
    db_init_string (&column->defaultValue.s);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_set_column_name (column, name)
    dbColumn *column;
    char *name;
{
    return db_set_string (&column->columnName, name);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
char *
db_get_column_name (column)
    dbColumn *column;
{
    return db_get_string (&column->columnName);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_set_column_description (column, description)
    dbColumn *column;
    char *description;
{
    return db_set_string (&column->description, description);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
char *
db_get_column_description (column)
    dbColumn *column;
{
    return db_get_string (&column->description);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_free_column (column)
    dbColumn *column;
{
    db_free_string (&column->columnName);
    db_free_string (&column->value.s);
}

/*!
 \fn 
 \brief Get column structure by table and column name.
         Column is set to new dbColumn structure or NULL if column was not found
 \return: DB_OK
          DB_FAILED
 \param 
*/
int
db_get_column ( dbDriver *Driver, char *tname, char *cname, dbColumn **Column )
{
    int   i, ncols;
    dbTable *Table;
    dbColumn *Col, *NCol;
    dbString tabname;

    db_init_string(&tabname);
    db_set_string(&tabname, tname);

    if(db_describe_table (Driver, &tabname, &Table) != DB_OK) {
	 G_warning("Cannot describe table %s", tname);
	 return DB_FAILED;
    }

    *Column = NULL;

    ncols = db_get_table_number_of_columns(Table);
    G_debug (3, "ncol = %d", ncols );
	     
    for (i = 0; i < ncols; i++) {
        Col = db_get_table_column (Table, i);
	if ( G_strcasecmp ( db_get_column_name(Col), cname ) == 0 ) {
	    NCol = (dbColumn *) malloc ( sizeof ( dbColumn ) );
            db_init_column ( NCol );
	    db_set_string ( &(NCol->columnName), db_get_column_name(Col) );
	    db_set_string ( &(NCol->description), db_get_column_description(Col) );
	    NCol->sqlDataType = Col->sqlDataType;
	    NCol->hostDataType = Col->hostDataType;
	    db_copy_value ( &(NCol->value), &(Col->value) );
	    NCol->dataLen = Col->dataLen;
	    NCol->precision = Col->precision;
	    NCol->scale = Col->scale;
	    NCol->nullAllowed = Col->nullAllowed;
	    NCol->hasDefaultValue = Col->hasDefaultValue;
	    NCol->useDefaultValue = Col->useDefaultValue;
	    db_copy_value ( &(NCol->defaultValue), &(Col->defaultValue) );
	    NCol->select = Col->select;
	    NCol->update = Col->update;

	    *Column = NCol;
	    return DB_OK;
	}
    }
    return DB_OK;
}

