#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  If you want to add a new command:
        1. Declare a function that takes argc and argv as arguments.
        2. Add an entry to the cmds[] array.
        3. Implement the function at the end of this file.
*/

struct command {
    const char *name;
    int (*func)(int, char**);
    const char *help_short;
    const char *help_long;
    int end;    // 0 (default) for commands, 1 for end-of-array sentinel
};
void print_usage();
int cmd_help(int, char**);
int cmd_world(int, char**);
int cmd_exit(int, char**);

struct command cmds[] = {
    {
        .name = "help",
        .func = cmd_help,
        .help_short = "Get help for a command.",
        .help_long = "help <command>\n\tget help for a command",
    },
    {
        .name = "world",
        .func = cmd_world,
        .help_short = "View a world.",
        .help_long = "run <worldname>\n\tview world.",
    },
    {
        .name = "",
        .func = cmd_exit,
        .help_short = "",
        .help_long = "",
        .end = 1, // end-of-array sentinel
    },
};

int die(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
	return 0;
}

int get_cmd_index_for_name( char *needle )
{
    int cmds_index = 0;
    while( !cmds[cmds_index].end )
    {
        if( !strcmp( cmds[cmds_index].name, needle ) )
        {
            return cmds_index;
        }
        cmds_index++;
    }

    return -1;
}

int main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
        print_usage();
        return 1;
    }

    char *exe_name = argv[0];
    char *command_name = argv[1];
    int cmd_index = get_cmd_index_for_name( command_name );
    if( cmd_index == -1 )
    {
        printf( "%s: '%s' is not a command.\n", exe_name, command_name );
        return 1;
    }
    else
    {
        return cmds[cmd_index].func( argc, argv );
    }
}

void print_usage()
{
    printf( "usage: ff7tool <command> <[args]>\n"
            "\n"
            "The available commands are:\n" );
    int cmds_index=0;
    while( !cmds[cmds_index].end )
    {
        printf( "\t%s\t%s\n", cmds[cmds_index].name, cmds[cmds_index].help_short);
        cmds_index++;
    }

    printf( "\n"
            "See 'ff7tool help <command>' to read about a specific command.\n" );
}

int cmd_help( int argc, char *argv[] )
{
    if( argc < 3 )
    {
        fprintf( stderr, "usage: ff7tool help <command>\n" );
        return 1;
    }

    char *command_name = argv[2];
    int cmds_index=0;
    while( !cmds[cmds_index].end )
    {
        if( !strcmp( cmds[cmds_index].name, command_name ) )
        {
            printf( "%s\n", cmds[cmds_index].help_long );
            return 0;
        }
        cmds_index++;
    }
    printf( "No command %s\n", command_name );
    return 1;
}

int cmd_world (int argc, char *argv[])
{
	// implemented in worldview.c
	extern int worldview(int,char**);
	return worldview(argc, argv);
}

int cmd_exit( int argc, char *argv[] )
{
    return 0;
}

