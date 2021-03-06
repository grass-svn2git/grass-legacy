/*
 *************************************************************************
 *   char *
 *   G_ask_cell_new(prompt, name)) 
 *       asks user to input name of a new cell file
 *
 *   char *
 *   G_ask_cell_old(prompt, name) 
 *       asks user to input name of an existing cell file
 *
 *   char *
 *   G_ask_cell_in_mapset(prompt, name)
 *       asks user to input name of an existing cell file in current mapset
 *
 *   char *
 *   G_ask_cell_any(prompt, name)
 *       asks user to input name of a new or existing cell file in
 *       the current mapset. Warns user about (possible) overwrite
 *       if cell file already exists
 *
 *   parms:
 *      char *prompt    optional prompt for user
 *      char *name      buffer to hold name of map found
 *
 *   returns:
 *      char *pointer to a string with name of mapset
 *       where file was found, or NULL if not found
 *
 *   note:
 *      rejects all names that begin with .
 **********************************************************************/

int lister();

char *
G_ask_cell_new (prompt,name)

	char *prompt;
	char *name;
{
	char *G_ask_new_ext();

	return G_ask_new_ext (prompt, name, "cell", "raster", "with titles", lister);
}

char *
G_ask_cell_old (prompt,name)

	char *prompt;
	char *name;
{
	char *G_ask_old_ext();

	return G_ask_old_ext (prompt, name, "cell", "raster", "with titles", lister);
}

char *
G_ask_cell_in_mapset (prompt,name)

	char *prompt;
	char *name;
{
	char *G_ask_in_mapset_ext();

	return G_ask_in_mapset_ext (prompt, name, "cell", "raster", "with titles", lister);
}

char *
G_ask_cell_any (prompt,name)

	char *prompt;
	char *name;
{
	char *G_ask_any_ext();

	return G_ask_any_ext (prompt, name, "cell", "raster", 1, "with titles", lister);
}

static
lister (name, mapset, buf)
    char *name;
    char *mapset;
    char *buf;
{
    char *G_get_cell_title();
    char *title;

    *buf = 0;
    if (*name == 0) return;

    strcpy (buf, title = G_get_cell_title (name, mapset));
    if (*buf == 0)
	strcpy (buf, "(no title)");
    free (title);
}
