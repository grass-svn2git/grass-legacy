#define SELECT "SELECT colname FROM syscolumns, systables\n"
#define SELECTV "SELECT colname,coltype,collength FROM syscolumns, systables\n"
#define WHERE "WHERE systables.tabname='%s'\n"
#define JOIN "AND systables.tabid=syscolumns.tabid\n"
#define ORDER "ORDER BY colname\n"
