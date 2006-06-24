/*
 * Defines for things used in mach_dep.c
 *
 * @(#)extern.h	4.35 (Berkeley) 02/05/99
 */

/*
 * Don't change the constants, since they are used for sizes in many
 * places in the program.
 */

#ifndef _WIN32
#include <sys/ioctl.h>
#endif
#include <stdlib.h>

#undef SIGTSTP

#if defined(_WIN32) && !defined(__MINGW32__)
#ifdef _PATH_MAX
#define PATH_MAX _PATH_MAX
#endif
#ifdef _MAX_PATH
#define PATH_MAX _MAX_PATH
#endif
#endif
#include <stdlib.h>
#define MAXSTR		1024	/* maximum length of strings */
#define MAXLINES	32	/* maximum number of screen lines used */
#define MAXCOLS		80	/* maximum number of screen columns used */

#define RN		(((Seed = Seed*11109+13849) >> 16) & 0xffff)
#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c)		(c & 037)

/*
 * Now all the global variables
 */

extern bool	Got_ltc, In_shell, Wizard;

extern char	Fruit[], Orig_dsusp, Prbuf[], Whoami[];

extern int	Fd;

#ifdef TIOCGLTC
extern struct ltchars	Ltc;
#endif /* TIOCGLTC */

/*
 * Function types
 */

#include <stdlib.h>

void  auto_save(int sig);
void  come_down(void);
void  doctor(void);
void  end_line(void);
void  endit(int sig);
void  fatal(char *s);
void  getltchars(void);
void  land(void);
void  leave(int sig);
void  my_exit(int st);
void  nohaste(void);
void  playit(void);
void  playltchars(void);
void  print_disc(char type);
void  quit(int sig);
void  resetltchars(void);
void  rollwand(void);
void  runners(void);
void  set_order(short *order, int numthings);
void  sight(void);
void  stomach(void);
void  swander(void);
void  tstp(int ignored);
void  unconfuse(void);
void  unsee(void);
void  visuals(void);

char	add_line(char *fmt, char *arg);

char	*killname(char monst, bool doart);
char	*nothing(char type);
char	*type_name(int type);

#ifdef CHECKTIME
void	checkout(int sig);
#endif

char *md_getusername();
char *md_getroguedir();
char *md_crypt();
char *md_getpass();
char *md_gethomedir();

#ifdef _WIN32
#define fdopen _fdopen
#define fileno _fileno
#endif