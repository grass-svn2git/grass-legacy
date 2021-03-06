#include <stdio.h>
#define OLD 1
#define NEW 2

I_vask_group_new (info, group, cancel_msg)
    char **info;
    char *group;
    char *cancel_msg;
{
    return ask (info, group, (char *) NULL, NEW, 1, cancel_msg);
}

I_vask_group_old (info, group, cancel_msg)
    char **info;
    char *group;
    char *cancel_msg;
{
    return ask (info, group, (char *) NULL, OLD, 1, cancel_msg);
}

I_vask_subgroup_new (info, group, subgroup, both, cancel_msg)
    char **info;
    char *group;
    char *subgroup;
    char *cancel_msg;
{
    return ask (info, group, subgroup, NEW, both, cancel_msg);
}

I_vask_subgroup_old (info, group, subgroup, both, cancel_msg)
    char **info;
    char *group;
    char *subgroup;
    char *cancel_msg;
{
    return ask (info, group, subgroup, OLD, both, cancel_msg);
}

static
ask (info, group, subgroup, type, both, cancel_msg)
    char **info;
    char *group, *subgroup;
    char *cancel_msg;
{
    int line;
    char tgroup[30], tsubgroup[30];

/* read the current group and save its name */
    if (both)
    {
	I_get_group (group);
    }
    else
    {
	if (subgroup == NULL)
	    G_fatal_error ("vask_group: illegal use by programmer");
	if(!I_find_group(group))
		return 0;
    }

    strcpy (tgroup, group);

/* do the same for the subgroup */
    if (subgroup != NULL)
    {
	if (type == OLD)
	    I_get_subgroup (group, subgroup);
	else
	    *subgroup = 0;
	strcpy (tsubgroup, subgroup);
    }

/* set up the screen prompts */
    V_clear();
    if (cancel_msg != NULL && *cancel_msg != 0)
	V_intrpt_msg (cancel_msg);
    line = 0;
    while (*info)
	V_line (line++, *info++);
    line += 2;
    if (both)
    {
	V_line (line, "GROUP:                           (list will show available groups)");
	V_ques (group, 's', line++, 10, 20);
    }
    else
    {
	V_line (line, "GROUP:");
	V_const (group, 's', line++, 10, 20);
    }
    if (subgroup != NULL)
    {
	V_line (line, "SUBGROUP:                        (list will show available subgroups)");
	V_ques (subgroup, 's', line++, 10, 20);
    }


    while(1)
    {
/* ask the questions */
	V_intrpt_ok();
	if (!V_call())
	    return 0;

/* check the group name */
	G_strip (group);
	if (subgroup != NULL)
	    G_strip (subgroup);
	if (*group == 0 || strcmp (group, "list") == 0)
	{
	    I_list_groups (1);
	    strcpy (group, tgroup);
	    continue;
	}
	strcpy (tgroup, group);
/* if type is OLD, or looking for subgroup as well, group must exist */
	if (subgroup != NULL || type == OLD)
	{
	    if (!I_find_group(group))
	    {
		printf ("group [%s] not found\n", group);
		I_list_groups (1);
		continue;
	    }
	}
/*
 * new group request is checked for legal names
 * caller must check if the group exists or not using I_find_group()
 */
	else if (G_legal_filename (group) >= 0)
	    break;
	else
	{
	    printf ("[%s] ** illegal group name **", group);
	    I_list_groups (1);
	    continue;
	}

/* check the subgroup */
	if (subgroup != NULL)
	{
	    if (*subgroup == 0 || strcmp (subgroup, "list") == 0)
	    {
		I_list_subgroups (group, 1);
		strcpy (subgroup, tsubgroup);
		continue;
	    }
	    strcpy (tsubgroup, subgroup);
	    if (type == OLD && !I_find_subgroup(group, subgroup))
	    {
		printf ("subgroup [%s] not found\n", subgroup);
		I_list_subgroups (group, 1);
		continue;
	    }
	    if (type == NEW && I_find_subgroup(group, subgroup))
	    {
		printf ("subgroup [%s] already exists. choose another name\n",
		    subgroup);
		I_list_subgroups (group, 1);
		continue;
	    }
	    if (type == NEW && G_legal_filename (subgroup) < 0)
	    {
		printf ("[%s] ** illegal subgroup name **\n", subgroup);
		I_list_subgroups (group, 1);
		continue;
	    }
	}
	break;
    }
    I_put_group (group);
    if (subgroup != NULL)
	I_put_subgroup (group, subgroup);
    return 1;
}
