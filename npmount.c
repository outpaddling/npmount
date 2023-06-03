/***************************************************************************
 *  Description:
 *      Non-privileged mount tool
 *
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2023-06-03  Jason Bacon Begin
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>
#include <string.h>     // strcmp()
#include <stdlib.h>     // exit()
#include "npmount.h"

int     main(int argc,char *argv[])

{
    char    *subcommand, *mount_point;
    
    switch(argc)
    {
	case 3:
	    subcommand = argv[1];
	    mount_point = argv[2];
	    break;
	
	default:
	    usage(argv);
    }
    
    if ( strcmp(subcommand, "mount") == 0 )
	return np_mount(mount_point);
    else if ( strcmp(subcommand, "umount") == 0 )
	return np_umount(mount_point);
    else
	usage(argv);
    
    return EX_OK;
}


int     np_mount(const char *mount_point)

{
    return EX_OK;
}


int     np_umount(const char *mount_point)

{
    mount_point_t   mount_points[MAX_MOUNT_POINTS];
    
    read_config(mount_points);
    
    //fnmatch();
    //setuid()
    return EX_OK;
}


void    read_config(mount_point_t mount_points[])

{
    // Make sure config file is only writable by root
    // exit() EX_NOINPUT, EX_CONFIG
}


void    usage(char *argv[])

{
    fprintf(stderr, "Usage: %s\n", argv[0]);
    exit(EX_USAGE);
}
