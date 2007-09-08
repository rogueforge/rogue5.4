/*
 * score editor
 *
 * @(#)scedit.c	4.6 (Berkeley) 02/05/99
 */

# include   <curses.h>
# include	<stdio.h>
# include	<signal.h>
# include	<ctype.h>

#ifndef TRUE
# define	TRUE	1
#endif
# define	FALSE	0
# define	RN	(((seed = seed*11109+13849) >> 16) & 0xffff)

# define	MAXSTR	80

# include	"score.h"

SCORE	Top_ten[10];

char	Buf[BUFSIZ],
	*reason[] = {
		"killed",
		"quit",
		"A total winner",
		"killed with Amulet",
	};

int	Seed;
FILE	*Inf;

struct passwd	*getpwnam();
int do_comm();
int pr_score(SCORE *, int);

int
main(ac, av)
int	ac;
char	**av;
{
	char	*scorefile;
	FILE	*outf;

	if (ac == 1)
		scorefile = "rogue54.scr";
	else
		scorefile = av[1];
	Seed = md_getpid();

	if ((Inf = fopen(scorefile, "r+")) < 0) {
		perror(scorefile);
		exit(1);
	}
	s_encread((char *) Top_ten, sizeof Top_ten, Inf);

	while (do_comm())
		continue;

	exit(0);
}

/*
 * do_comm:
 *	Get and execute a command
 */
int
do_comm()
{
	char		*sp;
	SCORE		*scp;
	struct passwd	*pp;
	static FILE	*outf = NULL;
	static int written = TRUE;

	printf("\nCommand: ");
	while (isspace(Buf[0] = getchar()) || Buf[0] == '\n')
		continue;
	(void) fgets(&Buf[1], BUFSIZ, stdin);
	Buf[strlen(Buf) - 1] = '\0';
	switch (Buf[0]) {
	  case 'w':
		if (strncmp(Buf, "write", strlen(Buf)))
			goto def;
		lseek(Inf, 0L, 0);
		if (outf == NULL && (outf = fdopen(Inf, "w")) == NULL) {
			perror("fdopen");
			exit(1);
		}
		fseek(outf, 0L, 0);
		if (s_lock_sc())
		{
			void (*fp)(int);

			fp = signal(SIGINT, SIG_IGN);
			s_encwrite((char *) Top_ten, sizeof Top_ten, outf);
			s_unlock_sc();
			signal(SIGINT, fp);
			written = TRUE;
		}
		break;

	  case 'a':
		if (strncmp(Buf, "add", strlen(Buf)))
			goto def;
		add_score();
		written = FALSE;
		break;

	  case 'd':
		if (strncmp(Buf, "delete", strlen(Buf)))
			goto def;
		del_score();
		written = FALSE;
		break;

	  case 'p':
		if (strncmp(Buf, "print", strlen(Buf)))
			goto def;
		printf("\nTop Ten Rogueists:\nRank\tScore\tName\n");
		for (scp = Top_ten; scp < &Top_ten[10]; scp++)
			if (!pr_score(scp, TRUE))
				break;
		break;

	  case 'q':
		if (strncmp(Buf, "quit", strlen(Buf)))
			goto def;
		if (!written) {
			printf("No write\n");
			written = TRUE;
		}
		else
			return FALSE;
		break;

	  default:
def:
		printf("Unkown command: \"%s\"\n", Buf);
	}
	return TRUE;
}

/*
 * add_score:
 *	Add a score to the score file
 */
void
add_score(void)
{
	SCORE		*scp;
    int id = -1;
	int		i;
	static SCORE	new;

	printf("Name: ");
	(void) fgets(new.sc_name, MAXSTR, stdin);
	new.sc_name[strlen(new.sc_name) - 1] = '\0';
	do {
		printf("Reason: ");
		if ((new.sc_flags = getchar()) < '0' || new.sc_flags > '2') {
			for (i = 0; i < 3; i++)
				printf("%d: %s\n", i, Reason[i]);
			ungetc(new.sc_flags, stdin);
		}
		while (getchar() != '\n')
			continue;
	} while (new.sc_flags < '0' || new.sc_flags > '2');
	new.sc_flags -= '0';
	do {
		printf("User Id: ");
		(void) fgets(Buf, BUFSIZ, stdin);
		Buf[strlen(Buf) - 1] = '\0';
        id = atoi(Buf);
	} while (id == -1);
	new.sc_uid = id;
	do {
		printf("Monster: ");
		if (!isalpha(new.sc_monster = getchar())) {
			printf("type A-Za-z [%s]\n", unctrl(new.sc_monster));
			ungetc(new.sc_monster, stdin);
		}
		while (getchar() != '\n')
			continue;
	} while (!isalpha(new.sc_monster));
	printf("Score: ");
	scanf("%d", &new.sc_score);
	printf("Level: ");
	scanf("%d", &new.sc_level);
	while (getchar() != '\n')
		continue;
	pr_score(&new, FALSE);
	printf("Really add it? ");
	if (getchar() != 'y')
		return;
	while (getchar() != '\n')
		continue;
	insert_score(&new);
}

/*
 * pr_score:
 *	Print out a score entry.  Return FALSE if last entry.
 */
pr_score(SCORE *scp, int num)
{
	if (scp->sc_score) {
		if (num)
			printf("%d", scp - Top_ten + 1);
		printf("\t%d\t%s: %s on level %d", scp->sc_score, scp->sc_name,
		    Reason[scp->sc_flags], scp->sc_level);
		if (scp->sc_flags == 0)
		    printf(" by %s", s_killname(scp->sc_monster, TRUE));
		
        printf(" (%s)", md_getrealname(scp->sc_uid));
		putchar('\n');
	}
	return scp->sc_score;
}

/*
 * insert_score:
 *	Insert a score into the top ten list
 */
void
insert_score(SCORE *new)
{
	SCORE	*scp, *sc2;
	int	flags, uid, amount;

	flags = new->sc_flags;
	uid = new->sc_uid;
	amount = new->sc_score;

	for (scp = Top_ten; scp < &Top_ten[10]; scp++)
		if (amount > scp->sc_score)
			break;
		else if (flags != 2 && scp->sc_uid == uid && scp->sc_flags != 2)
			scp = &Top_ten[10];
	if (scp < &Top_ten[10]) {
		if (flags != 2)
			for (sc2 = scp; sc2 < &Top_ten[10]; sc2++) {
			    if (sc2->sc_uid == uid && sc2->sc_flags != 2)
				break;
			}
		else
			sc2 = &Top_ten[9];
		while (sc2 > scp) {
			*sc2 = sc2[-1];
			sc2--;
		}
		*scp = *new;
		sc2 = scp;
	}
}

/*
 * del_score:
 *	Delete a score from the score list.
 */
void
del_score(void)
{
	SCORE	*scp;
	int	i;
	int	num;

	for (;;) {
		printf("Which score? ");
		(void) fgets(Buf, BUFSIZ, stdin);
		if (Buf[0] == '\n')
			return;
		sscanf(Buf, "%d", &num);
		if (num < 1 || num > 10)
			printf("range is 1-10\n");
		else
			break;
	}
	num--;
	for (scp = &Top_ten[num]; scp < &Top_ten[9]; scp++)
		*scp = scp[1];
	Top_ten[9].sc_score = 0;
	for (i = 0; i < MAXSTR; i++)
	    Top_ten[9].sc_name[i] = RN;
	Top_ten[9].sc_flags = RN;
	Top_ten[9].sc_level = RN;
	Top_ten[9].sc_monster = RN;
	Top_ten[9].sc_uid = RN;
}
