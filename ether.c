/*
 * Function(s) for dealing with potions
 *
 * @(#)ether.c	4.46 (Berkeley) 06/07/83
 */

#include <curses.h>
#include <ctype.h>
#include "netprot.h"

typedef struct
{
    int pa_flags;
    void (*pa_daemon)(void);
    int pa_time;
    char *pa_high, *pa_straight;
} PACT;

static PACT P_actions[] =
{
	{ ISHUH,	unconfuse,	HUHDURATION,	/* P_CONFUSE */
		"what a tripy feeling!",
		"wait, what's going on here. Huh? What? Who?" },
	{ ISHALU,	come_down,	SEEDURATION,	/* P_LSD */
		"Oh, wow!  Everything seems so cosmic!",
		"Oh, wow!  Everything seems so cosmic!" },
	{ 0,		NULL,	0 },			/* P_POISON */
	{ 0,		NULL,	0 },			/* P_STRENGTH */
	{ CANSEE,	unsee,	SEEDURATION,		/* P_SEEINVIS */
		Prbuf,
		Prbuf },
	{ 0,		NULL,	0 },			/* P_HEALING */
	{ 0,		NULL,	0 },			/* P_MFIND */
	{ 0,		NULL,	0 },			/* P_TFIND  */
	{ 0,		NULL,	0 },			/* P_RAISE */
	{ 0,		NULL,	0 },			/* P_XHEAL */
	{ 0,		NULL,	0 },			/* P_HASTE */
	{ 0,		NULL,	0 },			/* P_RESTORE */
	{ ISBLIND,	sight,	SEEDURATION,		/* P_BLIND */
		"oh, bummer!  Everything is dark!  Help!",
		"a cloak of darkness falls around you" },
	{ ISLEVIT,	land,	HEALTIME,		/* P_LEVIT */
		"oh, wow!  You're floating in the air!",
		"you start to float in the air" }
};

/*
 * quaff:
 *	Quaff a potion from the pack
 */
void
quaff(void)
{
    THING *obj, *tp, *mp;
    bool discardit = FALSE;
    bool show, trip;

    obj = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (obj == NULL)
	return;
    if (obj->o_type != POTION)
    {
	if (!Terse)
	    msg("yuk! Why would you want to drink that?");
	else
	    msg("that's undrinkable");
	return;
    }
    if (obj == Cur_weapon)
	Cur_weapon = NULL;

    /*
     * Calculate the effect it has on the poor guy.
     */
    trip = on(Player, ISHALU);
    discardit = (obj->o_count == 1);
    leave_pack(obj, FALSE, FALSE);
    switch (obj->o_which)
    {
	when P_CONFUSE:
	    do_pot(P_CONFUSE, !trip);
	when P_POISON:
	    Pot_info[P_POISON].oi_know = TRUE;
	    if (ISWEARING(R_SUSTSTR))
		msg("you feel momentarily sick");
	    else
	    {
		chg_str(-(rnd(3) + 1));
		msg("you feel very sick now");
		come_down();
	    }
	when P_HEALING:
	    Pot_info[P_HEALING].oi_know = TRUE;
	    if ((Pstats.s_hpt += roll(Pstats.s_lvl, 4)) > Max_hp)
		Pstats.s_hpt = ++Max_hp;
	    sight();
	    msg("you begin to feel better");
	when P_STRENGTH:
	    Pot_info[P_STRENGTH].oi_know = TRUE;
	    chg_str(1);
	    msg("you feel stronger, now.  What bulging muscles!");
	when P_MFIND:
	    Player.t_flags |= SEEMONST;
	    fuse(turn_see, TRUE, HUHDURATION, AFTER);
	    if (!turn_see(FALSE))
		msg("you have a %s feeling for a moment, then it passes",
		    choose_str("normal", "strange"));
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    show = FALSE;
	    if (Lvl_obj != NULL)
	    {
		wclear(Hw);
		for (tp = Lvl_obj; tp != NULL; tp = next(tp))
		{
		    if (is_magic(tp))
		    {
			show = TRUE;
			wmove(Hw, tp->o_pos.y, tp->o_pos.x);
			waddch(Hw, MAGIC);
			Pot_info[P_TFIND].oi_know = TRUE;
		    }
		}
		for (mp = Mlist; mp != NULL; mp = next(mp))
		{
		    for (tp = mp->t_pack; tp != NULL; tp = next(tp))
		    {
			if (is_magic(tp))
			{
			    show = TRUE;
			    wmove(Hw, mp->t_pos.y, mp->t_pos.x);
			    waddch(Hw, MAGIC);
			}
		    }
		}
	    }
	    if (show)
	    {
		Pot_info[P_TFIND].oi_know = TRUE;
		show_win("You sense the presence of magic on this level.--More--");
	    }
	    else
		msg("you have a %s feeling for a moment, then it passes",
		    choose_str("normal", "strange"));
	when P_LSD:
	    if (!trip)
	    {
		if (on(Player, SEEMONST))
		    turn_see(FALSE);
		daemon(visuals, 0, BEFORE);
		Seenstairs = seen_stairs();
	    }
	    do_pot(P_LSD, TRUE);
	when P_SEEINVIS:
	    sprintf(Prbuf, "this potion tastes like %s juice", Fruit);
	    show = on(Player, CANSEE);
	    do_pot(P_SEEINVIS, FALSE);
	    if (!show)
		invis_on();
	    sight();
	when P_RAISE:
	    Pot_info[P_RAISE].oi_know = TRUE;
	    msg("you suddenly feel much more skillful");
	    raise_level();
	when P_XHEAL:
	    Pot_info[P_XHEAL].oi_know = TRUE;
	    if ((Pstats.s_hpt += roll(Pstats.s_lvl, 8)) > Max_hp)
	    {
		if (Pstats.s_hpt > Max_hp + Pstats.s_lvl + 1)
		    ++Max_hp;
		Pstats.s_hpt = ++Max_hp;
	    }
	    sight();
	    come_down();
	    msg("you begin to feel much better");
	when P_HASTE:
	    Pot_info[P_HASTE].oi_know = TRUE;
	    After = FALSE;
	    if (add_haste(TRUE))
		msg("you feel yourself moving much faster");
	when P_RESTORE:
	    if (ISRING(LEFT, R_ADDSTR))
		add_str(&Pstats.s_str, -Cur_ring[LEFT]->o_arm);
	    if (ISRING(RIGHT, R_ADDSTR))
		add_str(&Pstats.s_str, -Cur_ring[RIGHT]->o_arm);
	    if (Pstats.s_str < Max_stats.s_str)
		Pstats.s_str = Max_stats.s_str;
	    if (ISRING(LEFT, R_ADDSTR))
		add_str(&Pstats.s_str, Cur_ring[LEFT]->o_arm);
	    if (ISRING(RIGHT, R_ADDSTR))
		add_str(&Pstats.s_str, Cur_ring[RIGHT]->o_arm);
	    msg("hey, this tastes great.  It make you feel warm all over");
	when P_BLIND:
	    do_pot(P_BLIND, TRUE);
	when P_LEVIT:
	    do_pot(P_LEVIT, TRUE);
#ifdef MASTER
	otherwise:
	    msg("what an odd tasting potion!");
	    return;
#endif
    }
    status();
    /*
     * Throw the item away
     */

    call_it(&Pot_info[obj->o_which]);

    if (discardit)
	discard(obj);
    return;
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */
bool
is_magic(THING *obj)
{
    switch (obj->o_type)
    {
	case ARMOR:
	    return (obj->o_flags&ISPROT) || obj->o_arm != A_class[obj->o_which];
	case WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	case POTION:
	case SCROLL:
	case STICK:
	case RING:
	case AMULET:
	    return TRUE;
    }
    return FALSE;
}

/*
 * invis_on:
 *	Turn on the ability to see invisible
 */
void
invis_on(void)
{
    THING *mp;

    Player.t_flags |= CANSEE;
    for (mp = Mlist; mp != NULL; mp = next(mp))
	if (on(*mp, ISINVIS) && see_monst(mp) && !on(Player, ISHALU))
	    mvaddch(mp->t_pos.y, mp->t_pos.x, mp->t_disguise);
}

/*
 * turn_see:
 *	Put on or off seeing monsters on this level
 */
bool
turn_see(bool turn_off)
{
    THING *mp;
    bool can_see, add_new;

    add_new = FALSE;
    for (mp = Mlist; mp != NULL; mp = next(mp))
    {
	move(mp->t_pos.y, mp->t_pos.x);
	can_see = see_monst(mp);
	if (turn_off)
	{
	    if (!can_see)
		addch(mp->t_oldch);
	}
	else
	{
	    if (!can_see)
		standout();
	    if (!on(Player, ISHALU))
		addch(mp->t_type);
	    else
		addch(rnd(26) + 'A');
	    if (!can_see)
	    {
		standend();
		add_new++;
	    }
	}
    }
    if (turn_off)
	Player.t_flags &= ~SEEMONST;
    else
	Player.t_flags |= SEEMONST;
    return add_new;
}

/*
 * seen_stairs:
 *	Return TRUE if the player has seen the stairs
 */
bool
seen_stairs(void)
{
    THING	*tp;

    move(Stairs.y, Stairs.x);
    if (inch() == STAIRS)			/* it's on the map */
	return TRUE;
    if (ce(Hero, Stairs))			/* It's under him */
	return TRUE;

    /*
     * if a monster is on the stairs, this gets hairy
     */
    if ((tp = moat(Stairs.y, Stairs.x)) != NULL)
    {
	if (see_monst(tp) && on(*tp, ISRUN))	/* if it's visible and awake */
	    return TRUE;			/* it must have moved there */

	if (on(Player, SEEMONST)		/* if she can detect monster */
	    && tp->t_oldch == STAIRS)		/* and there once were stairs */
		return TRUE;			/* it must have moved there */
    }
    return FALSE;
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */
void
raise_level(void)
{
    Pstats.s_exp = E_levels[Pstats.s_lvl-1] + 1L;
    check_level();
}

/*
 * do_pot:
 *	Do a potion with standard setup.  This means it uses a fuse and
 *	turns on a flag
 */
void
do_pot(int type, bool knowit)
{
    PACT *pp;
    int t;

    pp = &P_actions[type];
    if (!Pot_info[type].oi_know)
	Pot_info[type].oi_know = knowit;
    t = spread(pp->pa_time);
    if (!on(Player, pp->pa_flags))
    {
	Player.t_flags |= pp->pa_flags;
	fuse(pp->pa_daemon, 0, t, AFTER);
	look(FALSE);
    }
    else
	lengthen(pp->pa_daemon, t);
    msg(choose_str(pp->pa_high, pp->pa_straight));
}
