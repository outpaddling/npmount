/***************************************************************************
 *  Description:
 *      Non-privileged mount tool
 *
 *  History: 
 *  Date        Name        Modification
 *  2023-06-03  Jason Bacon Begin
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>
#include <string.h>         // strcmp(), strerror()
#include <ctype.h>          // issapce()
#include <stdlib.h>         // exit()
#include <sys/stat.h>
#include <errno.h>
#include <fnmatch.h>
#include <grp.h>            // getgrnam()
#include <unistd.h>         // execlp()
#include <sys/wait.h>
#include <xtend/string.h>   // strblank()
#include <xtend/proc.h>     // xt_get_user_name()
#include "protos.h"

#define PERMS           fputs("Permissions should be -rw-r--r--.\n", stderr)
#define CONFIG_FILE     "/usr/local/etc/npmount.conf"
#define MAX_GROUP_LEN   64
#define MAX_PATTERN_LEN 64
#define MAX_LINE_LEN    128
#define MAX_GROUPS      100     // FIXME: Get real value from sysconf()
#define USER_NAME_MAX   64

int     main(int argc,char *argv[])

{
    char    *subcommand, *mount_point;
    int     len;
    
    switch(argc)
    {
	case 3:
	    subcommand = argv[1];
	    mount_point = argv[2];
	    break;
	
	default:
	    usage(argv);
	    return EX_USAGE;    // Useless, but to silence compiler
    }
    
    len = strlen(mount_point) - 1;
    if ( mount_point[len] == '/' )
	mount_point[len] = '\0';
    
    // FIXME: Maybe push checks into np_cmd() and make it available
    // via a library
    if ( strcmp(subcommand, "mount") == 0 )
	return np_cmd(subcommand, mount_point);
    else if ( strcmp(subcommand, "umount") == 0 )
	return np_cmd(subcommand, mount_point);
    else
	usage(argv);
}


int     np_cmd(const char *command, const char *mount_point)

{
    FILE        *config_fp;
    struct stat st;
    char        line[MAX_LINE_LEN + 1],
		*group_name,
		*pattern,
		*p,
		user_name[USER_NAME_MAX + 1];
    struct group    *group_st;
    gid_t       groups[MAX_GROUPS];
    int         ngroups, status;
    
    if ( stat(CONFIG_FILE, &st) != 0 )
    {
	fprintf(stderr, "npmount: Cannot stat %s: %s\n", CONFIG_FILE, strerror(errno));
	return EX_NOINPUT;
    }
    
    if ( st.st_uid != 0 )
    {
	fprintf(stderr, "npmount: %s must be owned by root.\n", CONFIG_FILE);
	fprintf(stderr, "Check the contents of %s and verify that you have not been hacked.\n", CONFIG_FILE);
	PERMS;
	return EX_CONFIG;
    }
    
    if ( st.st_mode & S_IWGRP )
    {
	fprintf(stderr, "npmount: %s is group-writable.\n", CONFIG_FILE);
	fprintf(stderr, "Check the contents of %s and verify that you have not been hacked.\n", CONFIG_FILE);
	PERMS;
	return EX_CONFIG;
    }
    
    if ( st.st_mode & S_IWOTH )
    {
	fprintf(stderr, "npmount: %s is world-writable.\n", CONFIG_FILE);
	fprintf(stderr, "Check the contents of %s and verify that you have not been hacked.\n", CONFIG_FILE);
	PERMS;
	return EX_CONFIG;
    }
    
    if ( (config_fp = fopen(CONFIG_FILE, "r")) == NULL )
    {
	fprintf(stderr, "Cannot open %s: %s\n", CONFIG_FILE, strerror(errno));
	PERMS;
	return EX_NOINPUT;
    }
    
    while ( fgets(line, MAX_LINE_LEN + 1, config_fp) != NULL )
    {
	if ( (line[0] != '#') && ! strblank(line) )
	{
	    // We know the line is not blank at this point
	    for (p = line; isspace(*p); ++p)
		;
	    group_name = strsep(&p, " \t");
	    
	    while ( isspace(*p) && (*p != '\n') )
		++p;
	    if ( *p == '\n' )
	    {
		fprintf(stderr, "npmount: Malformed %s.\n", CONFIG_FILE);
		fputs("Lines should begin with '#' or contain \"group  glob-pattern\"\n", stderr);
		return EX_CONFIG;
	    }
	    pattern = strsep(&p, " \t\n");
	    
	    // Does this config file entry match the mount point?
	    if ( fnmatch(pattern, mount_point, FNM_PATHNAME) == 0 )
	    {
		// Is the user a member of the group allowed to [un]mount?
		group_st = getgrnam(group_name);
		if ( group_st == NULL )
		{
		    fprintf(stderr, "npmount: No such group: %s\n",
			    group_name);
		    return EX_DATAERR;
		}
		
		if ( (ngroups = getgroups(MAX_GROUPS, groups)) == -1 )
		{
		    fprintf(stderr, "npmount: Could not get user's group list: %s\n",
			    strerror(errno));
		    return EX_SOFTWARE;
		}
		
		// FIXME: Is there a portable way to check for remote
		// sessions, so we can limit unmount to the console user?
		
		for (int c = 0; c < ngroups; ++c)
		{
		    if ( groups[c] == group_st->gr_gid )
		    {
			// Group and mount point match, no need to read more
			fclose(config_fp);
			
			if ( setuid(0) == 0 )
			{
			    if ( fork() == 0 )
			    {
				execlp(command, command, mount_point, NULL);
				fprintf(stderr, "npmount: exec failed: %s\n",
					strerror(errno));
				return EX_SOFTWARE;
			    }
			    else
			    {
				wait(&status);
				if ( status == 0 )
				    return EX_OK;
				else
				    // status set by wait is may be > 255, which
				    // would return as 0 since RV is 1 byte
				    return EX_SOFTWARE;
			    }
			}
			else
			{
			    fprintf(stderr, "npmount: setuid() failed.\n");
			    fprintf(stderr, "Is the suid bit set?\n");
			}
		    }
		}
		
		xt_get_user_name(user_name, USER_NAME_MAX + 1);
		fprintf(stderr, "npmount: User %s is not a member of group %s.\n",
			user_name, group_name);
		fclose(config_fp);
		return EX_NOPERM;
	    }
	}
    }
    
    fprintf(stderr, "npmount: %s does not match any allowed mount point.\n",
	    mount_point);
    fclose(config_fp);
    
    return EX_NOPERM;
}


void    usage(char *argv[])

{
    fprintf(stderr, "Usage: %s [u]mount /mount/path\n", argv[0]);
    exit(EX_USAGE);
}
