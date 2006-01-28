/*
 * save and restore routines
 *
 * @(#)save.c	4.33 (Berkeley) 06/01/83
 */

#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "rogue.h"
#include "score.h"

#ifndef NSIG
#define NSIG 32
#endif

typedef struct stat STAT;

extern char version[], encstr[];

#ifdef	attron
# define	CE	clr_eol
#else	attron
extern bool _endwin;
#endif	attron

static char Frob;

static STAT Sbuf;

/*
 * save_game:
 *	Implement the "save game" command
 */
void
save_game(void)
{
    FILE *savef;
    int c;
    auto char buf[MAXSTR];

    /*
     * get file name
     */
    Mpos = 0;
over:
    if (File_name[0] != '\0')
    {
	for (;;)
	{
	    msg("save file (%s)? ", File_name);
	    c = readchar();
	    Mpos = 0;
	    if (c == ESCAPE)
	    {
		msg("");
		return;
	    }
	    else if (c == 'n' || c == 'N' || c == 'y' || c == 'Y')
		break;
	    else
		msg("please answer Y or N");
	}
	if (c == 'y' || c == 'Y')
	{
	    addstr("Yes\n");
	    refresh();
	    strcpy(buf, File_name);
	    goto gotfile;
	}
    }

    do
    {
	Mpos = 0;
	msg("file name: ");
	buf[0] = '\0';
	if (get_str(buf, stdscr) == QUIT)
	{
quit_it:
	    msg("");
	    return;
	}
	Mpos = 0;
gotfile:
	/*
	 * test to see if the file exists
	 */
	if (stat(buf, &Sbuf) >= 0)
	{
	    for (;;)
	    {
		msg("File exists.  Do you wish to overwrite it?");
		Mpos = 0;
		if ((c = readchar()) == ESCAPE)
		    goto quit_it;
		if (c == 'y' || c == 'Y')
		    break;
		else if (c == 'n' || c == 'N')
		    goto over;
		else
		    msg("Please answer Y or N");
	    }
	    msg("file name: %s", buf);
	    md_unlink(File_name);
	}
	strcpy(File_name, buf);
	if ((savef = fopen(File_name, "w")) == NULL)
	    msg(strerror(errno));
    } while (savef == NULL);

    save_file(savef);
    /* NOTREACHED */
}

/*
 * auto_save:
 *	Automatically save a file.  This is used if a HUP signal is
 *	recieved
 */
void
auto_save(int sig)
{
    FILE *savef;
    int i;

    for (i = 0; i < NSIG; i++)
	signal(i, SIG_IGN);
    if (File_name[0] != '\0' && ((savef = fopen(File_name, "w")) != NULL ||
	(md_unlink_open_file(File_name, fileno(savef)) >= 0 && (savef = fopen(File_name, "w")) != NULL)))
	    save_file(savef);
    exit(0);
}

/*
 * save_file:
 *	Write the saved game on the file
 */
void
save_file(FILE *savef)
{
    char buf[80];
    mvcur(0, COLS - 1, LINES - 1, 0); 
    putchar('\n');
    endwin();
    resetltchars();
    chmod(File_name, 0400);
    /*
     * DO NOT DELETE.  This forces stdio to allocate the output buffer
     * so that malloc doesn't get confused on restart
     */
    Frob = 0;
    fwrite(&Frob, sizeof Frob, 1, savef);

#ifndef	attron
    _endwin = TRUE;
#endif	/* attron */
    fstat(fileno(savef), &Sbuf);
    encwrite(version, strlen(version)+1, savef);
    sprintf(buf,"%d x %d\n", LINES, COLS);
    encwrite(buf,80,savef);
    rs_save_file(savef);
    fflush(savef);
    fstat(fileno(savef), &Sbuf);
    fclose(savef);
    exit(0);
}

/*
 * restore:
 *	Restore a saved game from a file with elaborate checks for file
 *	integrity from cheaters
 */
bool
restore(char *file, char **envp)
{
    int inf;
    bool syml;
    char fb;
    extern char **environ;
    auto char buf[MAXSTR];
    auto STAT sbuf2;
    int lines, cols;

    if (strcmp(file, "-r") == 0)
	file = File_name;

#ifdef SIGTSTP
    /*
     * If a process can be suspended, this code wouldn't work
     */
# ifdef SIG_HOLD
    signal(SIGTSTP, SIG_HOLD);
# else
    signal(SIGTSTP, SIG_IGN);
# endif
#endif
    if ((inf = open(file, 0)) < 0)
    {
	perror(file);
	return FALSE;
    }
    fstat(inf, &sbuf2);
    syml = is_symlink(file);

    fflush(stdout);
    read(inf, &Frob, sizeof Frob);
    fb = Frob;
    encread(buf, (unsigned) strlen(version) + 1, inf);
    if (strcmp(buf, version) != 0)
    {
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }
    encread(buf,80,inf);
    sscanf(buf,"%d x %d\n", &lines, &cols);

    initscr();                          /* Start up cursor package */

    if (lines > LINES)
    {
        endwin();
        printf("Sorry, original game was played on a screen with %d lines.\n",lines);
        printf("Current screen only has %d lines. Unable to restore game\n",LINES);
        return(FALSE);
    }
    if (cols > COLS)
    {
        endwin();
        printf("Sorry, original game was played on a screen with %d columns.\n",cols);
        printf("Current screen only has %d columns. Unable to restore game\n",COLS);
        return(FALSE);
    }

    Hw = newwin(LINES, COLS, 0, 0);
    setup();

    rs_restore_file(inf);
    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    if (
#ifdef MASTER
	!Wizard &&
#endif
	md_unlink_open_file(file, inf) < 0)
    {
	printf("Cannot unlink file\n");
	return FALSE;
    }
    Mpos = 0;
/*    printw(0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime)); */
/*
    printw("%s: %s", file, ctime(&sbuf2.st_mtime));
*/
    clearok(stdscr,TRUE);
    /*
     * defeat multiple restarting from the same place
     */
#ifdef MASTER
    if (!Wizard)
#endif
	if (sbuf2.st_nlink != 1 || syml)
	{
	    printf("Cannot restore from a linked file\n");
	    return FALSE;
	}

    if (Pstats.s_hpt <= 0)
    {
	printf("\"He's dead, Jim\"\n");
	return FALSE;
    }
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif

    environ = envp;
    strcpy(File_name, file);
    clearok(curscr, TRUE);
    srand(getpid());
    msg("file name: %s", file);
    playit();
    /*NOTREACHED*/
    return(0);
}

/*
 * encwrite:
 *	Perform an encrypted write
 */
size_t
encwrite(char *start, size_t size, FILE *outf)
{
    char *e1, *e2, fb;
    int temp;
    extern char statlist[];
    size_t o_size = size;
    e1 = encstr;
    e2 = statlist;
    fb = Frob;

    while(size)
    {
	if (putc(*start++ ^ *e1 ^ *e2 ^ fb, outf) == EOF)
            break;

	temp = *e1++;
	fb += temp * *e2++;
	if (*e1 == '\0')
	    e1 = encstr;
	if (*e2 == '\0')
	    e2 = statlist;
	size--;
    }

    return(o_size - size);
}

/*
 * encread:
 *	Perform an encrypted read
 */
size_t
encread(char *start, size_t size, int inf)
{
    char *e1, *e2, fb;
    int temp, read_size;
    extern char statlist[];

    fb = Frob;

    if ((read_size = read(inf, start, size)) == 0 || read_size == -1)
	return(read_size);

    e1 = encstr;
    e2 = statlist;

    while (size--)
    {
	*start++ ^= *e1 ^ *e2 ^ fb;
	temp = *e1++;
	fb += temp * *e2++;
	if (*e1 == '\0')
	    e1 = encstr;
	if (*e2 == '\0')
	    e2 = statlist;
    }

    return(read_size);
}

static char scoreline[100];
/*
 * read_scrore
 *	Read in the score file
 */
rd_score(SCORE *top_ten, int fd)
{
    unsigned int i;

    for(i = 0; i < numscores; i++)
    {
        encread(top_ten[i].sc_name, MAXSTR, fd);
        encread(scoreline, 100, fd);
        sscanf(scoreline, " %u %hu %u %hu %hu %lx \n",
            &top_ten[i].sc_uid, &top_ten[i].sc_score,
            &top_ten[i].sc_flags, &top_ten[i].sc_monster,
            &top_ten[i].sc_level, &top_ten[i].sc_time);
    }
}

/*
 * write_scrore
 *	Read in the score file
 */
wr_score(SCORE *top_ten, FILE *outf)
{
    unsigned int i;

    for(i = 0; i < numscores; i++)
    {
          memset(scoreline,0,100);
          encwrite(top_ten[i].sc_name, MAXSTR, outf);
          sprintf(scoreline, " %u %hu %u %hu %hu %lx \n",
              top_ten[i].sc_uid, top_ten[i].sc_score,
              top_ten[i].sc_flags, top_ten[i].sc_monster,
              top_ten[i].sc_level, top_ten[i].sc_time);
          encwrite(scoreline,100,outf);
    }
}
