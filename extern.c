/*
 * global variable initializaton
 *
 * @(#)extern.c	4.82 (Berkeley) 02/05/99
 */

#include <curses.h>
#include "rogue.h"

bool After;				/* True if we want after daemons */
bool Again;				/* Repeating the last command */
bool Noscore;				/* Was a wizard sometime */
bool Seenstairs;			/* Have seen the stairs (for lsd) */
bool Amulet = FALSE;			/* He found the amulet */
bool Door_stop = FALSE;			/* Stop running when we pass a door */
bool Fight_flush = FALSE;		/* True if toilet input */
bool Firstmove = FALSE;			/* First move after setting Door_stop */
bool Got_ltc = FALSE;			/* We have gotten the local tty chars */
bool Has_hit = FALSE;			/* Has a "hit" message pending in msg */
bool In_shell = FALSE;			/* True if executing a shell */
bool Inv_describe = TRUE;		/* Say which way items are being used */
bool Jump = FALSE;			/* Show running as series of jumps */
bool Kamikaze = FALSE;			/* To_death really to DEATH */
bool Lower_msg = FALSE;			/* Messages should start w/lower case */
bool Move_on = FALSE;			/* Next move shouldn't pick up items */
bool Msg_esc = FALSE;			/* Check for ESC from msg's --More-- */
bool Passgo = FALSE;			/* Follow passages */
bool Playing = TRUE;			/* True until he quits */
bool Q_comm = FALSE;			/* Are we executing a 'Q' command? */
bool Running = FALSE;			/* True if player is running */
bool Save_msg = TRUE;			/* Remember last msg */
bool See_floor = TRUE;			/* Show the lamp illuminated floor */
bool Stat_msg = FALSE;			/* Should status() print as a msg() */
bool Terse = FALSE;			/* True if we should be short */
bool To_death = FALSE;			/* Fighting is to the death! */
bool Tombstone = TRUE;			/* Print out tombstone at end */
#ifdef MASTER
bool Wizard = FALSE;			/* True if allows wizard commands */
#endif
bool Pack_used[26] = {			/* Is the character used in the pack? */
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
};

char Dir_ch;				/* Direction from last get_dir() call */
char File_name[MAXSTR];			/* Save file name */
char Huh[MAXSTR];			/* The last message printed */
char *P_colors[MAXPOTIONS];		/* Colors of the potions */
char Prbuf[2*MAXSTR];			/* Buffer for sprintfs */
char *R_stones[MAXRINGS];		/* Stone settings of the rings */
char *Release;				/* Release number of program */
char Runch;				/* Direction player is Running */
char *S_names[MAXSCROLLS];		/* Names of the scrolls */
char Take;				/* Thing she is taking */
char Whoami[MAXSTR];			/* Name of player */
char *Ws_made[MAXSTICKS];		/* What sticks are made of */
char *Ws_type[MAXSTICKS];		/* Is it a wand or a staff */
int  Orig_dsusp;			/* Original dsusp char */
char Fruit[MAXSTR] =			/* Favorite fruit */
		{ 's', 'l', 'i', 'm', 'e', '-', 'm', 'o', 'l', 'd', '\0' };
char Home[MAXSTR] = { '\0' };		/* User's home directory */
char *Inv_t_name[] = {
	"Overwrite",
	"Slow",
	"Clear"
};
char L_last_comm = '\0';		/* Last Last_comm */
char L_last_dir = '\0';			/* Last Last_dir */
char Last_comm = '\0';			/* Last command typed */
char Last_dir = '\0';			/* Last direction given */
char *Tr_name[] = {			/* Names of the traps */
	"a trapdoor",
	"an arrow trap",
	"a sleeping gas trap",
	"a beartrap",
	"a teleport trap",
	"a poison dart trap",
	"a rust trap",
        "a mysterious trap"
};


int N_objs;				/* # items listed in inventory() call */
int Ntraps;				/* Number of traps on this level */
int Hungry_state = 0;			/* How hungry is he */
int Inpack = 0;				/* Number of things in pack */
int Inv_type = 0;			/* Type of inventory to use */
int Level = 1;				/* What level she is on */
int Max_hit;				/* Max damage done to her in To_death */
int Max_level;				/* Deepest player has gone */
int Mpos = 0;				/* Where cursor is on top line */
int No_food = 0;			/* Number of levels without food */
int A_class[MAXARMORS] = {		/* Armor class for each armor type */
	8,	/* LEATHER */
	7,	/* RING_MAIL */
	7,	/* STUDDED_LEATHER */
	6,	/* SCALE_MAIL */
	5,	/* CHAIN_MAIL */
	4,	/* SPLINT_MAIL */
	4,	/* BANDED_MAIL */
	3,	/* PLATE_MAIL */
};

int Count = 0;				/* Number of times to repeat command */
int Fd;					/* File descriptor for score file */
int Food_left;				/* Amount of food in hero's stomach */
int Lastscore = -1;			/* Score before this turn */
int No_command = 0;			/* Number of turns asleep */
int No_move = 0;			/* Number of turns held in place */
int Purse = 0;				/* How much gold he has */
int Quiet = 0;				/* Number of quiet turns */
int Vf_hit = 0;				/* Number of time flytrap has hit */

long Dnum;				/* Dungeon number */
long Seed;				/* Random number seed */
long E_levels[] = {
        10L,
	20L,
	40L,
	80L,
       160L,
       320L,
       640L,
      1300L,
      2600L,
      5200L,
     13000L,
     26000L,
     50000L,
    100000L,
    200000L,
    400000L,
    800000L,
   2000000L,
   4000000L,
   8000000L,
	 0L
};

coord Delta;				/* Change indicated to get_dir() */
coord Oldpos;				/* Position before last look() call */
coord Stairs;				/* Location of staircase */

PLACE Places[MAXLINES*MAXCOLS];		/* Level map */

THING *Cur_armor;			/* What he is wearing */
THING *Cur_ring[2];			/* Which rings are being worn */
THING *Cur_weapon;			/* Which weapon he is weilding */
THING *L_last_pick = NULL;		/* Last Last_pick */
THING *Last_pick = NULL;		/* Last object picked in get_item() */
THING *Lvl_obj = NULL;			/* List of objects on this level */
THING *Mlist = NULL;			/* List of monsters on the level */
THING Player;				/* His stats */
					/* restart of game */

WINDOW *Hw = NULL;			/* Used as a scratch window */

#define INIT_STATS { 16, 0, 1, 10, 12, "1x4", 12 }

struct stats Max_stats = INIT_STATS;	/* The maximum for the player */

struct room *Oldrp;			/* Roomin(&Oldpos) */
struct room Rooms[MAXROOMS];		/* One for each room -- A level */
struct room Passages[MAXPASS] =		/* One for each passage */
{
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE|ISDARK, 0, 0 }
};

#define ___ 1
#define XX 10
struct monster Monsters[26] =
    {
/* Name		 CARRY	FLAG    str, exp, lvl, amr, hpt, dmg */
{ "aquator",	   0,	ISMEAN,	{ XX, 20,   5,   2, ___, "0x0/0x0" } },
{ "bat",	   0,	ISFLY,	{ XX,  1,   1,   3, ___, "1x2" } },
{ "centaur",	  15,	0,	{ XX, 17,   4,   4, ___, "1x2/1x5/1x5" } },
{ "dragon",	 100,	ISMEAN,	{ XX,5000, 10,  -1, ___, "1x8/1x8/3x10" } },
{ "emu",	   0,	ISMEAN,	{ XX,  2,   1,   7, ___, "1x2" } },
{ "venus flytrap", 0,	ISMEAN,	{ XX, 80,   8,   3, ___, "%%%x0" } },
	/* NOTE: the damage is %%% so that xstr won't merge this */
	/* string with others, since it is written on in the program */
{ "griffin",	  20,	ISMEAN|ISFLY|ISREGEN, { XX,2000, 13,   2, ___, "4x3/3x5" } },
{ "hobgoblin",	   0,	ISMEAN,	{ XX,  3,   1,   5, ___, "1x8" } },
{ "ice monster",   0,	0,	{ XX,  5,   1,   9, ___, "0x0" } },
{ "jabberwock",   70,	0,	{ XX,3000, 15,   6, ___, "2x12/2x4" } },
{ "kestrel",	   0,	ISMEAN|ISFLY,	{ XX,  1,   1,   7, ___, "1x4" } },
{ "leprechaun",	   0,	0,	{ XX, 10,   3,   8, ___, "1x1" } },
{ "medusa",	  40,	ISMEAN,	{ XX,200,   8,   2, ___, "3x4/3x4/2x5" } },
{ "nymph",	 100,	0,	{ XX, 37,   3,   9, ___, "0x0" } },
{ "orc",	  15,	ISGREED,{ XX,  5,   1,   6, ___, "1x8" } },
{ "phantom",	   0,	ISINVIS,{ XX,120,   8,   3, ___, "4x4" } },
{ "quagga",	   0,	ISMEAN,	{ XX, 15,   3,   3, ___, "1x5/1x5" } },
{ "rattlesnake",   0,	ISMEAN,	{ XX,  9,   2,   3, ___, "1x6" } },
{ "snake",	   0,	ISMEAN,	{ XX,  2,   1,   5, ___, "1x3" } },
{ "troll",	  50,	ISREGEN|ISMEAN,{ XX, 120, 6, 4, ___, "1x8/1x8/2x6" } },
{ "black unicorn", 0,	ISMEAN,	{ XX,190,   7,  -2, ___, "1x9/1x9/2x9" } },
{ "vampire",	  20,	ISREGEN|ISMEAN,{ XX,350,   8,   1, ___, "1x10" } },
{ "wraith",	   0,	0,	{ XX, 55,   5,   4, ___, "1x6" } },
{ "xeroc",	  30,	0,	{ XX,100,   7,   7, ___, "4x4" } },
{ "yeti",	  30,	0,	{ XX, 50,   4,   6, ___, "1x6/1x6" } },
{ "zombie",	   0,	ISMEAN,	{ XX,  6,   2,   8, ___, "1x8" } }
    };
#undef ___
#undef XX

struct obj_info Things[NUMTHINGS] = {
    { 0,			26 },	/* potion */
    { 0,			36 },	/* scroll */
    { 0,			16 },	/* food */
    { 0,			 7 },	/* weapon */
    { 0,			 7 },	/* armor */
    { 0,			 4 },	/* ring */
    { 0,			 4 },	/* stick */
};

struct obj_info Arm_info[MAXARMORS] = {
    { "leather armor",		 20,	 20, NULL, FALSE },
    { "ring mail",		 15,	 25, NULL, FALSE },
    { "studded leather armor",	 15,	 20, NULL, FALSE },
    { "scale mail",		 13,	 30, NULL, FALSE },
    { "chain mail",		 12,	 75, NULL, FALSE },
    { "splint mail",		 10,	 80, NULL, FALSE },
    { "banded mail",		 10,	 90, NULL, FALSE },
    { "plate mail",		  5,	150, NULL, FALSE },
};
struct obj_info Pot_info[MAXPOTIONS] = {
    { "confusion",		 7,   5, NULL, FALSE },
    { "hallucination",		 8,   5, NULL, FALSE },
    { "poison",			 8,   5, NULL, FALSE },
    { "gain strength",		13, 150, NULL, FALSE },
    { "see invisible",		 3, 100, NULL, FALSE },
    { "healing",		13, 130, NULL, FALSE },
    { "monster detection",	 6, 130, NULL, FALSE },
    { "magic detection",	 6, 105, NULL, FALSE },
    { "raise level",		 2, 250, NULL, FALSE },
    { "extra healing",		 5, 200, NULL, FALSE },
    { "haste self",		 5, 190, NULL, FALSE },
    { "restore strength",	13, 130, NULL, FALSE },
    { "blindness",		 5,   5, NULL, FALSE },
    { "levitation",		 6,  75, NULL, FALSE },
};
struct obj_info Ring_info[MAXRINGS] = {
    { "protection",		 9, 400, NULL, FALSE },
    { "add strength",		 9, 400, NULL, FALSE },
    { "sustain strength",	 5, 280, NULL, FALSE },
    { "searching",		10, 420, NULL, FALSE },
    { "see invisible",		10, 310, NULL, FALSE },
    { "adornment",		 1,  10, NULL, FALSE },
    { "aggravate monster",	10,  10, NULL, FALSE },
    { "dexterity",		 8, 440, NULL, FALSE },
    { "increase damage",	 8, 400, NULL, FALSE },
    { "regeneration",		 4, 460, NULL, FALSE },
    { "slow digestion",		 9, 240, NULL, FALSE },
    { "teleportation",		 5,  30, NULL, FALSE },
    { "stealth",		 7, 470, NULL, FALSE },
    { "maintain armor",		 5, 380, NULL, FALSE },
};
struct obj_info Scr_info[MAXSCROLLS] = {
    { "monster confusion",		 7, 140, NULL, FALSE },
    { "magic mapping",			 4, 150, NULL, FALSE },
    { "hold monster",			 2, 180, NULL, FALSE },
    { "sleep",				 3,   5, NULL, FALSE },
    { "enchant armor",			 7, 160, NULL, FALSE },
    { "identify potion",		10,  80, NULL, FALSE },
    { "identify scroll",		10,  80, NULL, FALSE },
    { "identify weapon",		 6,  80, NULL, FALSE },
    { "identify armor",		 	 7, 100, NULL, FALSE },
    { "identify ring, wand or staff",	10, 115, NULL, FALSE },
    { "scare monster",			 3, 200, NULL, FALSE },
    { "food detection",			 2,  60, NULL, FALSE },
    { "teleportation",			 5, 165, NULL, FALSE },
    { "enchant weapon",			 8, 150, NULL, FALSE },
    { "create monster",			 4,  75, NULL, FALSE },
    { "remove curse",			 7, 105, NULL, FALSE },
    { "aggravate monsters",		 3,  20, NULL, FALSE },
    { "protect armor",			 2, 250, NULL, FALSE },
};
struct obj_info Weap_info[MAXWEAPONS + 1] = {
    { "mace",				11,   8, NULL, FALSE },
    { "long sword",			11,  15, NULL, FALSE },
    { "short bow",			12,  15, NULL, FALSE },
    { "arrow",				12,   1, NULL, FALSE },
    { "dagger",				 8,   3, NULL, FALSE },
    { "two handed sword",		10,  75, NULL, FALSE },
    { "dart",				12,   2, NULL, FALSE },
    { "shuriken",			12,   5, NULL, FALSE },
    { "spear",				12,   5, NULL, FALSE },
    { NULL, 0 },	/* DO NOT REMOVE: fake entry for dragon's breath */
};
struct obj_info Ws_info[MAXSTICKS] = {
    { "light",			12, 250, NULL, FALSE },
    { "invisibility",		 6,   5, NULL, FALSE },
    { "lightning",		 3, 330, NULL, FALSE },
    { "fire",			 3, 330, NULL, FALSE },
    { "cold",			 3, 330, NULL, FALSE },
    { "polymorph",		15, 310, NULL, FALSE },
    { "magic missile",		10, 170, NULL, FALSE },
    { "haste monster",		10,   5, NULL, FALSE },
    { "slow monster",		11, 350, NULL, FALSE },
    { "drain life",		 9, 300, NULL, FALSE },
    { "nothing",		 1,   5, NULL, FALSE },
    { "teleport away",		 6, 340, NULL, FALSE },
    { "teleport to",		 6,  50, NULL, FALSE },
    { "cancellation",		 5, 280, NULL, FALSE },
};

struct h_list Helpstr[] = {
    '?',	"	prints help",				TRUE,
    '/',	"	identify object",			TRUE,
    'h',	"	left",					TRUE,
    'j',	"	down",					TRUE,
    'k',	"	up",					TRUE,
    'l',	"	right",					TRUE,
    'y',	"	up & left",				TRUE,
    'u',	"	up & right",				TRUE,
    'b',	"	down & left",				TRUE,
    'n',	"	down & right",				TRUE,
    'H',	"	run left",				FALSE,
    'J',	"	run down",				FALSE,
    'K',	"	run up",				FALSE,
    'L',	"	run right",				FALSE,
    'Y',	"	run up & left",				FALSE,
    'U',	"	run up & right",			FALSE,
    'B',	"	run down & left",			FALSE,
    'N',	"	run down & right",			FALSE,
    CTRL('H'),	"	run left until adjacent",		FALSE,
    CTRL('J'),	"	run down until adjacent",		FALSE,
    CTRL('K'),	"	run up until adjacent",			FALSE,
    CTRL('L'),	"	run right until adjacent",		FALSE,
    CTRL('Y'),	"	run up & left until adjacent",		FALSE,
    CTRL('U'),	"	run up & right until adjacent",		FALSE,
    CTRL('B'),	"	run down & left until adjacent",	FALSE,
    CTRL('N'),	"	run down & right until adjacent",	FALSE,
    '\0',	"	<SHIFT><dir>: run that way",		TRUE,
    '\0',	"	<CTRL><dir>: run till adjacent",	TRUE,
    'f',	"<dir>	fight till death or near death",	TRUE,
    't',	"<dir>	throw something",			TRUE,
    'm',	"<dir>	move onto without picking up",		TRUE,
    'z',	"<dir>	zap a wand in a direction",		TRUE,
    '^',	"<dir>	identify trap type",			TRUE,
    's',	"	search for trap/secret door",		TRUE,
    '>',	"	go down a staircase",			TRUE,
    '<',	"	go up a staircase",			TRUE,
    '.',	"	rest for a turn",			TRUE,
    ',',	"	pick something up",			TRUE,
    'i',	"	inventory",				TRUE,
    'I',	"	inventory single item",			TRUE,
    'q',	"	quaff potion",				TRUE,
    'r',	"	read scroll",				TRUE,
    'e',	"	eat food",				TRUE,
    'w',	"	wield a weapon",			TRUE,
    'W',	"	wear armor",				TRUE,
    'T',	"	take armor off",			TRUE,
    'P',	"	put on ring",				TRUE,
    'R',	"	remove ring",				TRUE,
    'd',	"	drop object",				TRUE,
    'c',	"	call object",				TRUE,
    'a',	"	repeat last command",			TRUE,
    ')',	"	print current weapon",			TRUE,
    ']',	"	print current armor",			TRUE,
    '=',	"	print current rings",			TRUE,
    '@',	"	print current stats",			TRUE,
    'D',	"	recall what's been discovered",		TRUE,
    'o',	"	examine/set options",			TRUE,
    CTRL('R'),	"	redraw screen",				TRUE,
    CTRL('P'),	"	repeat last message",			TRUE,
    ESCAPE,	"	cancel command",			TRUE,
    'S',	"	save game",				TRUE,
    'Q',	"	quit",					TRUE,
    '!',	"	shell escape",				TRUE,
    'F',	"<dir>	fight till either of you dies",		TRUE,
    'v',	"	print version number",			TRUE,
    0,		NULL
};

