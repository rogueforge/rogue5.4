/*
 * Defines for things used in mach_dep.c
 *
 * @(#)extern.h	4.35 (Berkeley) 02/05/99
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

extern bool	Got_ltc, In_shell;
extern int	Wizard;
extern char	Fruit[], Prbuf[], Whoami[];
extern int 	Orig_dsusp;
extern FILE	*scoreboard;

/*
 * Function types
 */

#include <stdlib.h>

void    auto_save(int);
void	come_down();
void	doctor();
void	end_line();
void    endit(int sig);
void	fatal();
void	getltchars();
void	land();
void    leave(int);
void	my_exit();
void	nohaste();
void	playit();
void    playltchars(void);
void	print_disc(char);
void    quit(int);
void    resetltchars(void);
void	rollwand();
void	runners();
void	set_order();
void	sight();
void	stomach();
void	swander();
int	tstp();
void	unconfuse();
void	unsee();
void	visuals();

char	add_line(char *fmt, char *arg);

char	*killname(char monst, bool doart);
char	*nothing(char type);
char	*type_name(int type);

#ifdef CHECKTIME
void	checkout(int sig);
#endif

int	md_chmod(char *filename, int mode);
int	md_creat(char *file, int mode);
int	md_open(char *filename, int flag, int mode);
int	md_read(int fd, char *buf, int count);
int	md_close(int fd);
char	*md_crypt(char *key, char *salt);
int	md_dsuspchar();
int	md_erasechar();
char	*md_gethomedir();
char	*md_getusername();
int	md_getuid();
char	*md_getpass(char *prompt);
int	md_getpid();
char	*md_getrealname(int uid);
void	md_init();
int	md_killchar();
void	md_normaluser();
void	md_raw_standout();
void	md_raw_standend();
int	md_readchar();
int	md_setdsuspchar(int c);
int	md_shellescape();
void	md_sleep(int s);
int	md_suspchar();
unsigned long int md_ntohl(unsigned long int x);
unsigned long int md_htonl(unsigned long int x);
int	md_hasclreol();
int	md_unlink(char *file);
int	md_unlink_open_file(char *file, FILE *inf);
