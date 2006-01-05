/*
    mdport.c - Machine Dependent Code for Porting Unix/Curses games

    Copyright (C) 2005 Nicholas J. Kisseberth
    All rights reserved.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#if defined(_WIN32)
#include <Windows.h>
#include <Lmcons.h>
#include <process.h>
#include <shlobj.h>
#include <Shlwapi.h>
#include <sys/types.h>
#undef MOUSE_MOVED
#elif defined(__DJGPP__)
#include <process.h>
#else
#include <pwd.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include <curses.h>

#if defined(__INTERIX) || defined(__MSYS__) || defined(__linux)
#include <term.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>
#include "extern.h"

void
md_init()
{
#ifdef __INTERIX
    char *term;

    term = getenv("TERM");

    if (term == NULL)
        setenv("TERM","interix");
#endif
#if defined(__DJGPP__) || defined(_WIN32)
    _fmode = _O_BINARY;
#endif
#if defined(__CYGWIN__) || defined(__MSYS__)
    ESCDELAY=250;
#endif
}

int
md_hasclreol()
{
#ifndef	attron
    return(!CE);
#elif !defined(__PDCURSES__)
    return(clr_eol != NULL);
#else
    return(TRUE);
#endif
}

#ifdef	attron
# define	_puts(s)	tputs(s, 0, md_putchar);
# define	SO		enter_standout_mode
# define	SE		exit_standout_mode
#endif

int
md_putchar(int c)
{
    putchar(c);
}

static int md_standout_mode = 0;

int
md_raw_standout()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    HANDLE hStdout;
    int fgattr,bgattr;

    if (md_standout_mode == 0)
    {
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
        GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
        fgattr = (csbiInfo.wAttributes & 0xF);
        bgattr = (csbiInfo.wAttributes & 0xF0);
        SetConsoleTextAttribute(hStdout,(fgattr << 4) | (bgattr >> 4));
        md_standout_mode = 1;
    }
#elif !defined(__PDCURSES__)
    _puts(SO);
    fflush(stdout);
#endif
}

int
md_raw_standend()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    HANDLE hStdout;
    int fgattr,bgattr;

    if (md_standout_mode == 1)
    {
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
        GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
        fgattr = (csbiInfo.wAttributes & 0xF);
        bgattr = (csbiInfo.wAttributes & 0xF0);
        SetConsoleTextAttribute(hStdout,(fgattr << 4) | (bgattr >> 4));
        md_standout_mode = 0;
    }
#elif !defined(__PDCURSES__)
    _puts(SE);
    fflush(stdout);
#endif
}

int
md_unlink_open_file(char *file, int inf)
{
#ifdef _WIN32
    close(inf);
    chmod(file, 0600);
    return( _unlink(file) );
#else
    return(unlink(file));
#endif
}

int
md_unlink(char *file)
{
#ifdef _WIN32
    chmod(file, 0600);
    return( _unlink(file) );
#else
    return(unlink(file));
#endif
}

int
md_creat(char *file, int mode)
{
    int fd;
#ifdef _WIN32
    mode = _S_IREAD | _S_IWRITE;
#endif
    fd = open(file,O_CREAT | O_EXCL | O_WRONLY, mode);

    return(fd);
}


int
md_normaluser()
{
#ifndef _WIN32
    setuid(getuid());
    setgid(getgid());
#endif
}

int
md_getuid()
{
#ifndef _WIN32
    return( getuid() );
#else
    return(42);
#endif
}

char *
md_getusername()
{
    static char login[80];
    char *l = NULL;
#ifdef _WIN32
    LPSTR mybuffer;
    DWORD size = UNLEN + 1;
    TCHAR buffer[UNLEN + 1];

    mybuffer = buffer;
    GetUserName(mybuffer,&size);
    l = mybuffer;
#endif
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pw;

    pw = getpwuid(getuid());

    l = pw->pw_name;
#endif

    if ((l == NULL) || (*l == '\0'))
        if ( (l = getenv("USERNAME")) == NULL )
            if ( (l = getenv("LOGNAME")) == NULL )
                if ( (l = getenv("USER")) == NULL )
                    l = "nobody";

    strncpy(login,l,80);
    login[79] = 0;

    return(login);
}

char *
md_gethomedir()
{
    static char homedir[PATH_MAX];
    char *h = NULL;
    size_t len;
#if defined(_WIN32)
    TCHAR szPath[PATH_MAX];
#endif
#if defined(_WIN32) || defined(DJGPP)
        char slash = '\\';
#else
    char slash = '/';
    struct passwd *pw;
    pw = getpwuid(getuid());

    h = pw->pw_dir;

    if (strcmp(h,"/") == 0)
        h = NULL;
#endif
    homedir[0] = 0;
#ifdef _WIN32
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath)))
        h = szPath;
#endif

    if ( (h == NULL) || (*h == '\0') )
        if ( (h = getenv("HOME")) == NULL )
            if ( (h = getenv("HOMEDRIVE")) == NULL)
                h = "";
            else
            {
                strncpy(homedir,h,PATH_MAX-1);
                homedir[PATH_MAX-1] = 0;

                if ( (h = getenv("HOMEPATH")) == NULL)
                    h = "";
            }


    len = strlen(homedir);
    strncat(homedir,h,PATH_MAX-len-1);
    len = strlen(homedir);

    if ((len > 0) && (homedir[len-1] != slash)) {
        homedir[len] = slash;
        homedir[len+1] = 0;
    }

    return(homedir);
}

int
md_sleep(int s)
{
#ifdef _WIN32
    _sleep(s);
#else
    sleep(s);
#endif
}

char *
md_getshell()
{
    static char shell[PATH_MAX];
    char *s = NULL;
#ifdef _WIN32
    char *def = "C:\\WINDOWS\\SYSTEM32\\CMD.EXE";
#elif defined(__DJGPP__)
    char *def = "C:\\COMMAND.COM";
#else
    char *def = "/bin/sh";
    struct passwd *pw;
    pw = getpwuid(getuid());
    s = pw->pw_shell;
#endif
    if ((s == NULL) || (*s == '\0'))
        if ( (s = getenv("COMSPEC")) == NULL)
            if ( (s = getenv("SHELL")) == NULL)
                if ( (s = getenv("SystemRoot")) == NULL)
                    s = def;

    strncpy(shell,s,PATH_MAX);
    shell[PATH_MAX-1] = 0;

    return(shell);
}

int
md_shellescape()
{
#if (!defined(_WIN32) && !defined(__DJGPP__))
    int ret_status;
    int pid;
    void (*myquit)(int);
    void (*myend)(int);
#endif
    char *sh;

    sh = md_getshell();

#if defined(_WIN32)
    return(_spawnl(_P_WAIT,sh,"shell",NULL,0));
#elif defined(__DJGPP__)
    return ( spawnl(P_WAIT,sh,"shell",NULL,0) );
#else
    while((pid = fork()) < 0)
        sleep(1);

    if (pid == 0) /* Shell Process */
    {
        /*
         * Set back to original user, just in case
         */
        setuid(getuid());
        setgid(getgid());
        execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
        perror("No shelly");
        _exit(-1);
    }
    else /* Application */
    {
	myend = signal(SIGINT, SIG_IGN);
#ifdef SIGQUIT
        myquit = signal(SIGQUIT, SIG_IGN);
#endif  
        while (wait(&ret_status) != pid)
            continue;
	    
        signal(SIGINT, myquit);
#ifdef SIGQUIT
        signal(SIGQUIT, myend);
#endif
    }

    return(ret_status);
#endif
}

int
directory_exists(char *dirname)
{
    struct stat sb;

    if (stat(dirname, &sb) == 0) /* path exists */
        return (sb.st_mode & S_IFDIR);

    return(0);
}

char *
md_getroguedir()
{
    static char path[1024];
    char *end,*home;

    if ( (home = getenv("ROGUEHOME")) != NULL)
    {
        if (*home)
        {
            strncpy(path, home, PATH_MAX - 20);

            end = &path[strlen(path)-1];

            while( (end >= path) && ((*end == '/') || (*end == '\\')))
                *end-- = '\0';

            if (directory_exists(path))
                return(path);
        }
    }

    if (directory_exists("/var/games/roguelike"))
        return("/var/games/roguelike");
    if (directory_exists("/var/lib/roguelike"))
        return("/var/lib/roguelike");
    if (directory_exists("/var/roguelike"))
        return("/var/roguelike");
    if (directory_exists("/usr/games/lib"))
        return("/usr/games/lib");
    if (directory_exists("/games/roguelik"))
        return("/games/roguelik");

    return("");
}

char *
md_getrealname(int uid)
{
    static char uidstr[20];
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pp;

	if ((pp = getpwuid(uid)) == NULL)
    {
        sprintf(uidstr,"%d", uid);
        return(uidstr);
    }
	else
	    return(pp->pw_name);
#else
   sprintf(uidstr,"%d", uid);
   return(uidstr);
#endif
}

extern char *xcrypt(char *key, char *salt);

char *
md_crypt(char *key, char *salt)
{
    return( xcrypt(key,salt) );
}

char *
md_getpass(prompt)
char *prompt;
{
#ifdef _WIN32
    static char password_buffer[9];
    char *p = password_buffer;
    int c, count = 0;
    int max_length = 9;

    fflush(stdout);
    /* If we can't prompt, abort */
    if (fputs(prompt, stderr) < 0)
    {
        *p = '\0';
        return NULL;
    }

    for(;;)
    {
        /* Get a character with no echo */
        c = _getch();

        /* Exit on interrupt (^c or ^break) */
        if (c == '\003' || c == 0x100)
            exit(1);

        /* Terminate on end of line or file (^j, ^m, ^d, ^z) */
        if (c == '\r' || c == '\n' || c == '\004' || c == '\032')
            break;

        /* Back up on backspace */
        if (c == '\b')
        {
            if (count)
                count--;
            else if (p > password_buffer)
                p--;
            continue;
        }

        /* Ignore DOS extended characters */
        if ((c & 0xff) != c)
            continue;

        /* Add to password if it isn't full */
        if (p < password_buffer + max_length - 1)
            *p++ = c;
        else
            count++;
    }
   *p = '\0';

   fputc('\n', stderr);

   return password_buffer;
#else
   return( (char *) getpass(prompt) );
#endif
}


int md_endian = 0x01020304;

unsigned long int
md_ntohl(unsigned long int x)
{
#ifdef _WIN32
    if ( *((char *)&md_endian) == 0x01 )
        return(x);
    else
        return( ((x & 0x000000ffU) << 24) |
                ((x & 0x0000ff00U) <<  8) |
                ((x & 0x00ff0000U) >>  8) |
                ((x & 0xff000000U) >> 24) );
#else
    return( ntohl(x) );
#endif
}

unsigned long int
md_htonl(unsigned long int x)
{
#ifdef _WIN32
    if ( *((char *)&md_endian) == 0x01 )
        return(x);
    else
        return( ((x & 0x000000ffU) << 24) |
                ((x & 0x0000ff00U) <<  8) |
                ((x & 0x00ff0000U) >>  8) |
                ((x & 0xff000000U) >> 24) );
#else
    return( htonl(x) );
#endif
}
