/*
 * Loader for Wine installed in the bin directory
 *
 * Copyright 2025 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include "../tools.h"

#include <dlfcn.h>
#include <strings.h>

static const char *bindir;
static const char *libdir;

#define WINE_RESTART_EXIT_CODE 0x69

static int restart_debug_enabled(void)
{
    static int initialized;
    static int enabled;
    const char *value;

    if (!initialized)
    {
        initialized = 1;
        value = getenv( "WINE_EXIT69_DEBUG" );
        enabled = value && *value && strcmp( value, "0" );
    }
    return enabled;
}

static void restart_debug( const char *format, ... )
{
    va_list args;

    if (!restart_debug_enabled()) return;
    fprintf( stderr, "wine[exit69]: " );
    va_start( args, format );
    vfprintf( stderr, format, args );
    va_end( args );
    fputc( '\n', stderr );
}

static int wait_wineserver_path( const char *path, int search_path )
{
    pid_t pid;
    int status;

    restart_debug( "trying wineserver wait via '%s'%s", path, search_path ? " (PATH)" : "" );

    if (!(pid = fork()))
    {
        if (search_path) execlp( path, path, "-w", NULL );
        else execl( path, path, "-w", NULL );
        _exit(1);
    }
    if (pid == -1) return -1;

    while (waitpid( pid, &status, 0 ) == -1)
    {
        if (errno != EINTR) return -1;
    }
    if (!WIFEXITED( status ))
    {
        restart_debug( "wineserver wait '%s' did not exit normally", path );
        return -1;
    }

    restart_debug( "wineserver wait '%s' exited with code %d", path, WEXITSTATUS( status ) );
    return WEXITSTATUS( status );
}

static void wait_wineserver(void)
{
    const char *path;
    char *candidate;

    if ((path = getenv( "WINESERVER" )) && !wait_wineserver_path( path, 0 )) return;

    if (bindir && strendswith( bindir, "/tools/wine" ))
    {
        candidate = strmake( "%s/../../server/wineserver", bindir );
        if (!wait_wineserver_path( candidate, 0 ))
        {
            free( candidate );
            return;
        }
        free( candidate );
    }

    if (bindir)
    {
        candidate = strmake( "%s/wineserver", bindir );
        if (!wait_wineserver_path( candidate, 0 ))
        {
            free( candidate );
            return;
        }
        free( candidate );
    }

    wait_wineserver_path( "wineserver", 1 );
}

static int is_fivem_target( int argc, char *argv[] )
{
    const char *target;

    if (argc < 2 || !argv[1]) return 0;
    target = strrchr( argv[1], '/' );
    if (target) target++;
    else target = argv[1];
    return !strcasecmp( target, "FiveM.exe" );
}

static void *load_ntdll(void)
{
    const char *arch_dir = get_arch_dir( get_default_target() );
    struct strarray dllpath;
    void *handle;

    if (bindir && strendswith( bindir, "/tools/wine" ) &&
        ((handle = dlopen( strmake( "%s/../../dlls/ntdll/ntdll.so", bindir ), RTLD_NOW ))))
        return handle;

    if ((handle = dlopen( strmake( "%s/wine%s/ntdll.so", libdir, arch_dir ), RTLD_NOW )))
        return handle;

    dllpath = strarray_frompath( getenv( "WINEDLLPATH" ));
    STRARRAY_FOR_EACH( dir, &dllpath )
    {
        if ((handle = dlopen( strmake( "%s%s/ntdll.so", dir, arch_dir ), RTLD_NOW )))
            return handle;
        if ((handle = dlopen( strmake( "%s/ntdll.so", dir ), RTLD_NOW )))
            return handle;
    }
    fprintf( stderr, "wine: could not load ntdll.so: %s\n", dlerror() );
    exit(1);
}

int main( int argc, char *argv[] )
{
    void (*init_func)(int, char **);
    pid_t pid;
    int status;
    int code;
    int fivem_target;

    bindir = get_bindir( argv[0] );
    libdir = get_libdir( bindir );
    fivem_target = is_fivem_target( argc, argv );
    init_func = dlsym( load_ntdll(), "__wine_main" );
    if (init_func)
    {
        restart_debug( "launcher start pid=%d, argc=%d, fivem_target=%d", getpid(), argc, fivem_target );

        if ((pid = fork()) == -1)
        {
            perror( "wine: fork" );
            exit(1);
        }
        if (!pid)
        {
            init_func( argc, argv );
            exit(1);
        }

        restart_debug( "spawned __wine_main child pid=%d", pid );

        while (waitpid( pid, &status, 0 ) == -1)
        {
            if (errno != EINTR)
            {
                perror( "wine: waitpid" );
                exit(1);
            }
        }

        if (WIFSIGNALED( status ))
        {
            restart_debug( "__wine_main child signaled: sig=%d", WTERMSIG( status ) );
            return 128 + WTERMSIG( status );
        }
        if (!WIFEXITED( status ))
        {
            restart_debug( "__wine_main child ended unexpectedly" );
            return 1;
        }

        code = WEXITSTATUS( status );
        restart_debug( "__wine_main child exited with code=%d", code );

        if (code == WINE_RESTART_EXIT_CODE && !getenv( "WINE_DISABLE_EXIT69_WAIT" ))
        {
            restart_debug( "detected exit 0x69, waiting for wineserver" );
            wait_wineserver();
            restart_debug( "exit 0x69 handling completed, returning 0" );
            return 0;
        }
        if (code == WINE_RESTART_EXIT_CODE)
            restart_debug( "exit 0x69 wait disabled by WINE_DISABLE_EXIT69_WAIT" );

        if (code == 0 && fivem_target && !getenv( "WINE_DISABLE_FIVEM_EXIT0_WAIT" ))
        {
            restart_debug( "detected FiveM exit 0, waiting for wineserver handoff" );
            wait_wineserver();
            restart_debug( "FiveM exit 0 wait completed, returning 0" );
            return 0;
        }
        if (code == 0 && fivem_target)
            restart_debug( "FiveM exit 0 wait disabled by WINE_DISABLE_FIVEM_EXIT0_WAIT" );

        return code;
    }

    fprintf( stderr, "wine: __wine_main function not found in ntdll.so\n" );
    exit(1);
}
