#define SELECT   "SELECT (pg_attribute.attname)\n"
#define SELECTV  "SELECT (pg_attribute.attname, pg_type.typname)\n"
#define WHERE    "where  pg_class.relname = \"%s\"\n"
#define JOIN     "and    pg_attribute.atttypid = pg_type.oid\n and pg_attribute.attrelid=pg_class.oid \n"
#define ORDER    "sort   by attname\n"
