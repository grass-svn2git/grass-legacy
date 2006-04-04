#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <grass/gis.h>

enum state
{
    S_TOPLEVEL,
    S_MODULE,
    S_FLAG,
    S_OPTION
};

struct context {
    struct GModule *module;
    struct Option *option;
    struct Flag *flag;
    struct Option *first_option;
    struct Flag *first_flag;
    int state;
    FILE *fp;
    int line;
};

static void parse_toplevel(struct context *ctx, const char *cmd)
{
    if (strcasecmp(cmd, "module") == 0)
    {
	ctx->state = S_MODULE;
	ctx->module = G_define_module();
	return;
    }

    if (strcasecmp(cmd, "flag") == 0)
    {
	ctx->state = S_FLAG;
	ctx->flag = G_define_flag();
	if (!ctx->first_flag)
	    ctx->first_flag = ctx->flag;
	return;
    }

    if (strcasecmp(cmd, "option") == 0)
    {
	ctx->state = S_OPTION;
	ctx->option = G_define_option();
	if (!ctx->first_option)
	    ctx->first_option = ctx->option;
	return;
    }

    fprintf(stderr, "Unknown command \"%s\" at line %d\n",
	    cmd, ctx->line);
}

static void parse_module(struct context *ctx, const char *cmd, const char *arg)
{

    if (strcasecmp(cmd, "label") == 0)
    {
	ctx->module->label = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "description") == 0)
    {
	ctx->module->description = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "end") == 0)
    {
	ctx->state = S_TOPLEVEL;
	return;
    }

    fprintf(stderr, "Unknown module parameter \"%s\" at line %d\n",
	    cmd, ctx->line);
}

static void parse_flag(struct context *ctx, const char *cmd, const char *arg)
{
    if (strcasecmp(cmd, "key") == 0)
    {
	ctx->flag->key = arg[0];
	return;
    }

    if (strcasecmp(cmd, "answer") == 0)
    {
	ctx->flag->answer = atoi(arg);
	return;
    }

    if (strcasecmp(cmd, "label") == 0)
    {
	ctx->flag->label = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "description") == 0)
    {
	ctx->flag->description = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "guisection") == 0)
    {
	ctx->flag->guisection = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "end") == 0)
    {
	ctx->state = S_TOPLEVEL;
	return;
    }

    fprintf(stderr, "Unknown flag parameter \"%s\" at line %d\n",
	    cmd, ctx->line);
}

static int parse_type(struct context *ctx, const char *arg)
{
    if (strcasecmp(arg, "integer") == 0)
	return TYPE_INTEGER;

    if (strcasecmp(arg, "double") == 0)
	return TYPE_DOUBLE;

    if (strcasecmp(arg, "string") == 0)
	return TYPE_STRING;

    fprintf(stderr, "Unknown type \"%s\" at line %d\n",
	    arg, ctx->line);

    return TYPE_STRING;
}

static int parse_boolean(struct context *ctx, const char *arg)
{
    if (strcasecmp(arg, "yes") == 0)
	return YES;

    if (strcasecmp(arg, "no") == 0)
	return NO;

    fprintf(stderr, "Unknown boolean value \"%s\" at line %d\n",
	    arg, ctx->line);

    return NO;
}

static void parse_option(struct context *ctx, const char *cmd, const char *arg)
{
    if (strcasecmp(cmd, "key") == 0)
    {
	ctx->option->key = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "type") == 0)
    {
	ctx->option->type = parse_type(ctx, arg);
	return;
    }

    if (strcasecmp(cmd, "required") == 0)
    {
	ctx->option->required = parse_boolean(ctx, arg);
	return;
    }

    if (strcasecmp(cmd, "multiple") == 0)
    {
	ctx->option->multiple = parse_boolean(ctx, arg);
	return;
    }

    if (strcasecmp(cmd, "options") == 0)
    {
	ctx->option->options = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "key_desc") == 0)
    {
	ctx->option->key_desc = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "label") == 0)
    {
	ctx->option->label = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "description") == 0)
    {
	ctx->option->description = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "descriptions") == 0)
    {
	ctx->option->descriptions = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "answer") == 0)
    {
	ctx->option->answer = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "gisprompt") == 0)
    {
	ctx->option->gisprompt = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "guisection") == 0)
    {
	ctx->option->guisection = strdup(arg);
	return;
    }

    if (strcasecmp(cmd, "end") == 0)
    {
	ctx->state = S_TOPLEVEL;
	return;
    }

    fprintf(stderr, "Unknown option parameter \"%s\" at line %d\n",
	    cmd, ctx->line);
}

int main(int argc, char *argv[])
{
    struct context ctx;
    struct Option *option;
    struct Flag *flag;
    const char *filename;

    ctx.module = NULL;
    ctx.option = NULL;
    ctx.flag = NULL;
    ctx.first_option = NULL;
    ctx.first_flag = NULL;
    ctx.state = S_TOPLEVEL;

    if (argc < 2)
    {
	fprintf(stderr, "Usage: %s <filename> [<argument> ...]\n", argv[0]);
	return 1;
    }

    filename = argv[1];
    G_debug ( 2, "filename = %s", filename );

    ctx.fp = fopen(filename, "r");
    if (!ctx.fp)
    {
	perror("Unable to open script file");
	return 1;
    }

    G_gisinit((char *) filename);

    for (ctx.line = 1; ; ctx.line++)
    {
	char buff[4096];
	char *cmd, *arg;

	if (!fgets(buff, sizeof(buff), ctx.fp))
	    break;

	arg = strchr(buff, '\n');
	if (!arg)
	{
	    fprintf(stderr, "Line too long or missing newline at line %d\n", ctx.line);
	    return 1;
	}
	*arg = '\0';

	if (buff[0] != '#' || buff[1] != '%')
	    continue;

	cmd = buff + 2;
	G_strip(cmd);

	arg = strchr(cmd, ':');

	if (arg)
	{
	    *(arg++) = '\0';
	    G_strip(cmd);
	    G_strip(arg);
	}

	switch (ctx.state)
	{
	case S_TOPLEVEL:	parse_toplevel(&ctx, cmd);	break;
	case S_MODULE:		parse_module  (&ctx, cmd, arg);	break;
	case S_FLAG:		parse_flag    (&ctx, cmd, arg);	break;
	case S_OPTION:		parse_option  (&ctx, cmd, arg);	break;
	}
    }

    if (fclose(ctx.fp) != 0)
    {
	perror("Error closing script file");
	return 1;
    }

    if (G_parser(argc - 1, argv + 1) < 0)
	return 1;

    /* Because shell from MINGW and CygWin converts all variables
     * to uppercase it was necessary to use uppercase variables.
     * Set both until all scripts are updated */
    for (flag = ctx.first_flag; flag; flag = flag->next_flag)
    {
	char buff[12];
	sprintf(buff, "GIS_FLAG_%c=%d", flag->key, flag->answer ? 1 : 0);
	putenv(G_store(buff));

	sprintf(buff, "GIS_FLAG_%c=%d", toupper(flag->key), flag->answer ? 1 : 0);
	putenv(G_store(buff));
    }

    for (option = ctx.first_option; option; option = option->next_opt)
    {
	char buff[1024];
	sprintf(buff, "GIS_OPT_%s=%s", option->key, option->answer ? option->answer : "");
	putenv(G_store(buff));

        G_str_to_upper(option->key);
	sprintf(buff, "GIS_OPT_%s=%s", option->key, option->answer ? option->answer : "");
	putenv(G_store(buff));
    }

#ifdef __MINGW32__
    execlp( "sh", "sh", filename, "@ARGS_PARSED@", NULL);
#else
    execl(filename, filename, "@ARGS_PARSED@", NULL);
#endif

    perror("execl() failed");
    return 1;
}
