/*
 * Various installation dependent routines
 *
 * @(#)mach_dep.c	4.37 (Berkeley) 05/23/83
 */

/*
 * The various tuneable defines are:
 *
 *	SCOREFILE	Where/if the score file should live.
 *	ALLSCORES	Score file is top ten scores, not top ten
 *			players.  This is only useful when only a few
 *			people will be playing; otherwise the score file
 *			gets hogged by just a few people.
 *	NUMSCORES	Number of scores in the score file (default 10).
 *	NUMNAME		String version of NUMSCORES (first character
 *			should be capitalized) (default "Ten").
 *	MAXLOAD		What (if any) the maximum load average should be
 *			when people are playing.  Since it is divided
 *			by 10, to specify a load limit of 4.0, MAXLOAD
 *			should be "40".	 If defined, then
 *      LOADAV		Should it use it's own routine to get
 *		        the load average?
 *      NAMELIST	If so, where does the system namelist
 *		        hide?
 *	MAXUSERS	What (if any) the maximum user count should be
 *	                when people are playing.  If defined, then
 *      UCOUNT		Should it use it's own routine to count
 *		        users?
 *      UTMP		If so, where does the user list hide?
 *	CHECKTIME	How often/if it should check during the game
 *			for high load average.
 */

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <curses.h>
#include "extern.h"

#define NOOP(x) (x += 0)

# ifndef NUMSCORES
#	define	NUMSCORES	10
#	define	NUMNAME		"Ten"
# endif

unsigned int numscores = NUMSCORES;
char *Numname = NUMNAME;

# ifdef ALLSCORES
bool Allscore = TRUE;
# else  /* ALLSCORES */
bool Allscore = FALSE;
# endif /* ALLSCORES */

#ifdef CHECKTIME
static int Num_checks;		/* times we've gone over in checkout() */
#endif /* CHECKTIME */

/*
 * init_check:
 *	Check out too see if it is proper to play the game now
 */

void
init_check()
{
#if defined(MAXLOAD) || defined(MAXUSERS)
    if (too_much())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", Whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(Fruit), Fruit);
	if (author())
	    printf("However, since you're a good guy, it's up to you\n");
	else
	    exit(1);
    }
#endif
}

/*
 * open_score:
 *	Open up the score file for future use
 */

void
open_score()
{
#ifdef SCOREFILE
    char *Scorefile = SCOREFILE;
     /* 
      * We drop setgid privileges after opening the score file, so subsequent 
      * open()'s will fail.  Just reuse the earlier filehandle. 
      */

    if (scoreboard != NULL) { 
        rewind(scoreboard); 
        return; 
    } 

    scoreboard = fopen(Scorefile, "r+");

    if ((scoreboard == NULL) && (errno == ENOENT))
    {
    	scoreboard = fopen(Scorefile, "w+");
        md_chmod(Scorefile,0464);
    }

    if (scoreboard == NULL) { 
         fprintf(stderr, "Could not open %s for writing: %s\n", Scorefile, strerror(errno)); 
         fflush(stderr); 
    } 
#else
    scoreboard = NULL;
#endif
}

/*
 * setup:
 *	Get starting setup for all games
 */

void
setup()
{
#ifdef CHECKTIME
    int  checkout();
#endif

#ifdef DUMP
    md_onsignal_autosave();
#else
    md_onsignal_default();
#endif

#ifdef CHECKTIME
    md_start_checkout_timer(CHECKTIME*60);
    num_checks = 0;
#endif

    raw();				/* Raw mode */
    noecho();				/* Echo off */
    keypad(stdscr,1);
    getltchars();			/* get the local tty chars */
}

/*
 * getltchars:
 *	Get the local tty chars for later use
 */

void
getltchars()
{
    Got_ltc = TRUE;
    Orig_dsusp = md_dsuspchar();
    md_setdsuspchar( md_suspchar() );
}

/* 
 * resetltchars: 
 *      Reset the local tty chars to original values. 
 */ 
void 
resetltchars(void) 
{ 
    if (Got_ltc) {
        md_setdsuspchar(Orig_dsusp);
    } 
} 
  
/* 
 * playltchars: 
 *      Set local tty chars to the values we use when playing. 
 */ 
void 
playltchars(void) 
{ 
    if (Got_ltc) { 
        md_setdsuspchar( md_suspchar() );
    } 
} 

/*
 * start_score:
 *	Start the scoring sequence
 */

void
start_score()
{
#ifdef CHECKTIME
    md_stop_checkout_timer();
#endif
}

/* 	 	 
 * is_symlink: 	 	 
 *      See if the file has a symbolic link 	 	 
  */ 	 	 
bool 	 	 
is_symlink(char *sp) 	 	 
{ 	 	 
#ifdef S_IFLNK 	 	 
    struct stat sbuf2; 	 	 
 	 	 
    if (lstat(sp, &sbuf2) < 0) 	 	 
        return FALSE; 	 	 
    else 	 	 
        return ((sbuf2.st_mode & S_IFMT) != S_IFREG); 	 	 
#else
	NOOP(sp);
    return FALSE; 	 	 
#endif 
} 

#if defined(MAXLOAD) || defined(MAXUSERS)
/*
 * too_much:
 *	See if the system is being used too much for this game
 */
bool
too_much(void)
{
#ifdef MAXLOAD
    double avec[3];
#else
    int cnt;
#endif

#ifdef MAXLOAD
    md_loadav(avec);
    if (avec[1] > (MAXLOAD / 10.0))
	return TRUE;
#endif
#ifdef MAXUSERS
    if (ucount() > MAXUSERS)
	return TRUE;
#endif
    return FALSE;
}

/*
 * author:
 *	See if a user is an author of the program
 */
bool
author(void)
{
#ifdef MASTER
    if (Wizard)
	return TRUE;
#endif
    switch (md_getuid())
    {
	case -1:
	    return TRUE;
	default:
	    return FALSE;
    }
}
#endif

#ifdef CHECKTIME
/*
 * checkout:
 *	Check each CHECKTIME seconds to see if the load is too high
 */
void
checkout(int sig)
{
    static char *msgs[] = {
	"The load is too high to be playing.  Please leave in %0.1f minutes",
	"Please save your game.  You have %0.1f minutes",
	"Last warning.  You have %0.1f minutes to leave",
    };
    int checktime;

    if (too_much())
    {
	if (author())
	{
	    Num_checks = 1;
	    chmsg("The load is rather high, O exaulted one");
	}
	else if (Num_checks++ == 3)
	    fatal("Sorry.  You took too long.  You are dead\n");
	checktime = (CHECKTIME * 60) / Num_checks;
	chmsg(msgs[Num_checks - 1], ((double) checktime / 60.0));
    }
    else
    {
	if (Num_checks)
	{
	    Num_checks = 0;
	    chmsg("The load has dropped back down.  You have a reprieve");
	}
	checktime = (CHECKTIME * 60);
    }

	md_start_checkout_timer(checktime);
}

/*
 * chmsg:
 *	checkout()'s version of msg.  If we are in the middle of a
 *	shell, do a printf instead of a msg to avoid the refresh.
 */
/* VARARGS1 */
void
chmsg(char *fmt, int arg)
{
    if (!In_shell)
	msg(fmt, arg);
    else
    {
	printf(fmt, arg);
	putchar('\n');
	fflush(stdout);
    }
}
#endif

#ifdef UCOUNT
/*
 * ucount:
 *	Count number of users on the system
 */
#include <utmp.h>

struct utmp buf;

int
ucount(void)
{
    struct utmp *up;
    FILE *utmp;
    int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread(up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif

/*
 * lock_sc:
 *	lock the score file.  If it takes too long, ask the user if
 *	they care to wait.  Return TRUE if the lock is successful.
 */
static FILE *lfd = NULL;
bool
lock_sc(void)
{
#if defined(SCOREFILE) && defined(LOCKFILE)
    int cnt;
    static struct stat sbuf;
    char *Lockfile = LOCKFILE;

over:
    if ((lfd=fopen(Lockfile, "w+")) != NULL)
	return TRUE;
    for (cnt = 0; cnt < 5; cnt++)
    {
	md_sleep(1);
	if ((lfd=fopen(Lockfile, "w+")) != NULL)
	    return TRUE;
    }
    if (stat(Lockfile, &sbuf) < 0)
    {
	lfd=fopen(Lockfile, "w+");
	return TRUE;
    }
    if (time(NULL) - sbuf.st_mtime > 10)
    {
	if (md_unlink(Lockfile) < 0)
	    return FALSE;
	goto over;
    }
    else
    {
	printf("The score file is very busy.  Do you want to wait longer\n");
	printf("for it to become free so your score can get posted?\n");
	printf("If so, type \"y\"\n");
	fgets(Prbuf, MAXSTR, stdin);
	if (Prbuf[0] == 'y')
	    for (;;)
	    {
		if ((lfd=fopen(Lockfile, "w+")) != 0)
		    return TRUE;
		if (stat(Lockfile, &sbuf) < 0)
		{
		    lfd=fopen(Lockfile, "w+");
		    return TRUE;
		}
		if (time(NULL) - sbuf.st_mtime > 10)
		{
		    if (md_unlink(Lockfile) < 0)
			return FALSE;
		}
		md_sleep(1);
	    }
	else
	    return FALSE;
    }
#else
    return TRUE;
#endif
}

/*
 * unlock_sc:
 *	Unlock the score file
 */

void
unlock_sc()
{
#if defined(SCOREFILE) && defined(LOCKFILE)
    if (lfd != NULL)
        fclose(lfd);
    lfd = NULL;
    md_unlink(LOCKFILE);
#endif
}

/*
 * flush_type:
 *	Flush typeahead for traps, etc.
 */

void
flush_type()
{
    flushinp();
}
