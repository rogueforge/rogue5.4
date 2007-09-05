/*
 * This file has all the code for the option command.  I would rather
 * this command were not necessary, but it is the only way to keep the
 * wolves off of my back.
 *
 * @(#)options.c	4.24 (Berkeley) 05/10/83
 */

#include <stdlib.h>
#include <curses.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    void 	*o_opt;		/* pointer to thing to set */
				/* function to print value */
    void 	(*o_putfunc)(void *opt);
				/* function to get value interactively */
    int		(*o_getfunc)(void *opt, WINDOW *win);
};

typedef struct optstruct	OPTION;

void	pr_optname(const OPTION *op);

static const OPTION	optlist[] = {
    {"terse",	 "Terse output",
		 &Terse,	put_bool,	get_bool	},
    {"flush",	 "Flush typeahead during battle",
		 &Fight_flush,	put_bool,	get_bool	},
    {"jump",	 "Show position only at end of run",
		 &Jump,		put_bool,	get_bool	},
    {"seefloor", "Show the lamp-illuminated floor",
		 &See_floor,	put_bool,	get_sf		},
    {"passgo",	"Follow turnings in passageways",
		 &Passgo,	put_bool,	get_bool	},
    {"tombstone", "Print out tombstone when killed",
		 &Tombstone,	put_bool,	get_bool	},
    {"inven",	"Inventory style",
		 &Inv_type,	put_inv_t,	get_inv_t	},
    {"name",	 "Name",
		 Whoami,	put_str,	get_str		},
    {"fruit",	 "Fruit",
		 Fruit,		put_str,	get_str		},
    {"file",	 "Save file",
		 File_name,	put_str,	get_str		}
};

/*
 * option:
 *	Print and then set options from the terminal
 */

void
option(void)
{
    const OPTION *op;
    int		retval;

    wclear(Hw);
    /*
     * Display current values of options
     */
    for (op = optlist; op <= &optlist[NUM_OPTS-1]; op++)
    {
	pr_optname(op);
	(*op->o_putfunc)(op->o_opt);
	waddch(Hw, '\n');
    }
    /*
     * Set values
     */
    wmove(Hw, 0, 0);
    for (op = optlist; op <= &optlist[NUM_OPTS-1]; op++)
    {
	pr_optname(op);
	retval = (*op->o_getfunc)(op->o_opt, Hw);
	if (retval)
	{
	    if (retval == QUIT)
		break;
	    else if (op > optlist) {	/* MINUS */
		wmove(Hw, (int)(op - optlist) - 1, 0);
		op -= 2;
	    }
	    else	/* trying to back up beyond the top */
	    {
		putchar('\007');
		wmove(Hw, 0, 0);
		op--;
	    }
	}
    }
    /*
     * Switch back to original screen
     */
    wmove(Hw, LINES - 1, 0);
    waddstr(Hw, "--Press space to continue--");
    wrefresh(Hw);
    wait_for(Hw, ' ');
    clearok(curscr, TRUE);
    touchwin(stdscr);
    After = FALSE;
}

/*
 * pr_optname:
 *	Print out the option name prompt
 */

void
pr_optname(const OPTION *op)
{
    wprintw(Hw, "%s (\"%s\"): ", op->o_prompt, op->o_name);
}

/*
 * put_bool
 *	Put out a boolean
 */
void
put_bool(void *b)
{
    waddstr(Hw, *(int *) b ? "True" : "False");
}

/*
 * put_str:
 *	Put out a string
 */
void
put_str(const void *str)
{
    waddstr(Hw, (char *) str);
}

/*
 * put_inv_t:
 *	Put out an inventory type
 */
void
put_inv_t(void *ip)
{
    waddstr(Hw, Inv_t_name[*(int *) ip]);
}

/*
 * get_bool:
 *	Allow changing a boolean option and print it out
 */
int
get_bool(const void *vp, WINDOW *win)
{
    int *bp = (int *) vp;
    int oy, ox;
    int op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    while (op_bad)	
    {
	wmove(win, oy, ox);
	wrefresh(win);
	switch (wreadchar(win))
	{
	    case 't':
	    case 'T':
		*bp = TRUE;
		op_bad = FALSE;
		break;
	    case 'f':
	    case 'F':
		*bp = FALSE;
		op_bad = FALSE;
		break;
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case ESCAPE:
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		wmove(win, oy, ox + 10);
		waddstr(win, "(T or F)");
	}
    }
    wmove(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    waddch(win, '\n');
    return NORM;
}

/*
 * get_sf:
 *	Change value and handle transition problems from See_floor to
 *	!See_floor.
 */
int
get_sf(const void *vp, WINDOW *win)
{
    int	*bp = (int *) vp;
    int	was_sf;
    int		retval;

    was_sf = See_floor;
    retval = get_bool(bp, win);
    if (retval == QUIT) return(QUIT);
    if (was_sf != See_floor)
    {
	if (!See_floor) {
	    See_floor = TRUE;
	    erase_lamp(&Hero, Proom);
	    See_floor = FALSE;
	}
	else
	    look(FALSE);
    }
    return(NORM);
}

/*
 * get_str:
 *	Set a string option
 */
#define MAXINP	50	/* max string to read from terminal or environment */

int
get_str(const void *vopt, WINDOW *win)
{
    char *opt = (char *) vopt;
    char *sp;
    int oy, ox;
    size_t i;
    int c;
    static char buf[MAXSTR];

    getyx(win, oy, ox);
    wrefresh(win);
    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    for (sp = buf; (c = wreadchar(win)) != '\n' && c != '\r' && c != ESCAPE;
	wclrtoeol(win), wrefresh(win))
    {
	if (c == -1)
	    continue;
	else if (c == erasechar())	/* process erase character */
	{
	    if (sp > buf)
	    {
		sp--;
		for (i = strlen(unctrl(*sp)); i; i--)
		    waddch(win, '\b');
	    }
	    continue;
	}
	else if (c == killchar())	/* process kill character */
	{
	    sp = buf;
	    wmove(win, oy, ox);
	    continue;
	}
	else if (sp == buf)
	{
	    if (c == '-' && win != stdscr)
		break;
	    else if (c == '~')
	    {
		strcpy(buf, Home);
		waddstr(win, Home);
		sp += strlen(Home);
		continue;
	    }
	}
	if (sp >= &buf[MAXINP] || !(isprint(c) || c == ' '))
	    putchar(CTRL('G'));
	else
	{
	    *sp++ = (char) c;
	    waddstr(win, unctrl(c));
	}
    }
    *sp = '\0';
    if (sp > buf)	/* only change option if something has been typed */
	strucpy(opt, buf, strlen(buf));
    mvwprintw(win, oy, ox, "%s\n", opt);
    wrefresh(win);
    if (win == stdscr)
	Mpos += (int)(sp - buf);
    if (c == '-')
	return MINUS;
    else if (c == ESCAPE)
	return QUIT;
    else
	return NORM;
}

/*
 * get_inv_t
 *	Get an inventory type name
 */
int
get_inv_t(const void *vp, WINDOW *win)
{
    int *ip = (int *) vp;
    int oy, ox;
    int op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, Inv_t_name[*ip]);
    while (op_bad)	
    {
	wmove(win, oy, ox);
	wrefresh(win);
	switch (wreadchar(win))
	{
	    case 'o':
	    case 'O':
		*ip = INV_OVER;
		op_bad = FALSE;
		break;
	    case 's':
	    case 'S':
		*ip = INV_SLOW;
		op_bad = FALSE;
		break;
	    case 'c':
	    case 'C':
		*ip = INV_CLEAR;
		op_bad = FALSE;
		break;
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case ESCAPE:
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		wmove(win, oy, ox + 15);
		waddstr(win, "(O, S, or C)");
	}
    }
    mvwprintw(win, oy, ox, "%s\n", Inv_t_name[*ip]);
    return NORM;
}
	

#ifdef MASTER
/*
 * get_num:
 *	Get a numeric option
 */
int
get_num(const void *vp, WINDOW *win)
{
    int *opt = (int *) vp;
    int i;
    static char buf[MAXSTR];

    if ((i = get_str(buf, win)) == NORM)
	*opt = atoi(buf);
    return i;
}
#endif

/*
 * parse_opts:
 *	Parse options from string, usually taken from the environment.
 *	The string is a series of comma seperated values, with booleans
 *	being stated as "name" (true) or "noname" (false), and strings
 *	being "name=....", with the string being defined up to a comma
 *	or the end of the entire option string.
 */
void
parse_opts(char *str)
{
    char *sp;
    const OPTION *op;
    int len;
    const char **i;
    char *start;

    while (*str)
    {
	/*
	 * Get option name
	 */
	for (sp = str; isalpha((int)*sp); sp++)
	    continue;
	len = (int)(sp - str);
	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op <= &optlist[NUM_OPTS-1]; op++)
	    if (EQSTR(str, op->o_name, len))
	    {
		if (op->o_putfunc == put_bool)	/* if option is a boolean */
		    *(int *)op->o_opt = TRUE;	/* NOSTRICT */
		else				/* string option */
		{
		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; *str == '='; str++)
			continue;
		    if (*str == '~')
		    {
			strcpy((char *) op->o_opt, Home);	  /* NOSTRICT */
			start = (char *) op->o_opt + strlen(Home);/* NOSTRICT */
			while (*++str == '/')
			    continue;
		    }
		    else
			start = (char *) op->o_opt;	/* NOSTRICT */
		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ','; sp++)
			continue;
		    /*
		     * check for type of inventory
		     */
		    if (op->o_putfunc == put_inv_t)
		    {
			if (islower((int)*str))
			    *str = (char) toupper(*str);
			for (i = Inv_t_name; i <= &Inv_t_name[INV_CLEAR]; i++)
			    if (strncmp(str, *i, sp - str) == 0)
			    {
				Inv_type = (int)(i - Inv_t_name);
				break;
			    }
		    }
		    else
			strucpy(start, str, (size_t)(sp - str));
		}
		break;
	    }
	    /*
	     * check for "noname" for booleans
	     */
	    else if (op->o_putfunc == put_bool
	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
	    {
		*(int *)op->o_opt = FALSE;	/* NOSTRICT */
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha((int)*sp))
	    sp++;
	str = sp;
    }
}

/*
 * strucpy:
 *	Copy string using unctrl for things
 */
void
strucpy(char *s1, const char *s2, size_t len)
{
    if (len > MAXINP)
	len = MAXINP;
    while (len--)
    {
	if (isprint((int)*s2) || *s2 == ' ')
	    *s1++ = *s2;
	s2++;
    }
    *s1 = '\0';
}
