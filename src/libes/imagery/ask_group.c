/*************************************************************
* I_ask_group_old (prompt,group)
* I_ask_group_new (prompt,group)
* I_ask_group_any (prompt,group)
*
* prompt the user for an imagery group file name
*************************************************************/
#include <string.h>
#include "gis.h"
#include "imagery.h"
static int ask_group(char *,char *);


/*!
 * \brief prompt for an existing group
 *
 * Asks the user to enter the name of an existing <b>group</b>
 * in the current mapset.
 *
 *  \param prompt
 *  \param group
 *  \return int
 */

int I_ask_group_old (
    char *prompt,
    char *group)
{
    while(1)
    {
	if (*prompt == 0)
	    prompt = "Select an imagery group file";
	if (!ask_group(prompt, group))
	    return 0;
	if (I_find_group (group))
	    return 1;
	fprintf (stderr,"\n** %s - not found **\n\n", group);
    }
}


/*!
 * \brief prompt for new group
 *
 * Asks the user to enter a name for a <b>group</b> which does not exist
 * in the current mapset.
 *
 *  \param prompt
 *  \param group
 *  \return int
 */

int I_ask_group_new (
    char *prompt,
    char *group)
{
    while(1)
    {
	if (*prompt == 0)
	    prompt = "Enter a new imagery group file name";
	if (!ask_group(prompt, group))
	    return 0;
	if (!I_find_group (group))
	    return 1;
	fprintf (stderr,"\n** %s - exists, select another name **\n\n", group);
    }
}


/*!
 * \brief prompt for any valid group name
 *
 * Asks the user to enter a valid <b>group</b> name. The
 * <b>group</b> may or may not exist in the current mapset.
 *
 *  \param prompt
 *  \param group
 *  \return int
 */

int I_ask_group_any (
    char *prompt,
    char *group)
{
    if (*prompt == 0)
	prompt = "Enter a new or existing imagery group file";
    return ask_group(prompt, group);
}

static int ask_group( char *prompt,char *group)
{
    char buf[1024];

    while (1)
    {
	fprintf (stderr,"\n%s\n", prompt);
	fprintf (stderr,"Enter 'list' for a list of existing imagery groups\n");
	fprintf (stderr,"Enter 'list -f' for a verbose listing\n");
	fprintf (stderr,"Hit RETURN %s\n", G_get_ask_return_msg());
	fprintf (stderr,"> ");
	if (!G_gets(buf)) continue;

	G_squeeze (buf);
	fprintf (stderr,"<%s>\n", buf);
	if (*buf == 0) return 0;

	if (strcmp (buf, "list") == 0)
	    I_list_groups(0);
	else if (strcmp (buf, "list -f") == 0)
	    I_list_groups(1);
	else if (G_legal_filename(buf) < 0)
	    fprintf (stderr,"\n** <%s> - illegal name **\n\n", buf);
	else
	    break;
    }
    strcpy (group, buf);
    return 1;
}
