char *
db_whoami()
{
    char *cuserid();

    return cuserid((char *) 0);
}
