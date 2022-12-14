#include <string.h>
#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	4.40 (Berkeley) 02/05/99
 */

/*
 * update_mdest:
 *      Called after picking up an object, before discarding it.
 *      If this was the object of something's desire, that monster will
 *      get mad and run at the hero
 */
void
update_mdest(obj)
register THING *obj;
{
    register THING *mp;

    for (mp = Mlist; mp != NULL; mp = next(mp))
        if (mp->t_dest == &obj->o_pos)
     mp->t_dest = &Hero;
}

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is
 *	non-null use it as the linked_list pointer instead of gettting
 *	it off the ground.
 */
void
add_pack(THING *obj, int silent)
{
    THING *op, *lp;
    int from_floor;
    int discarded = 0;

    from_floor = FALSE;
    if (obj == NULL)
    {
	if ((obj = find_obj(Hero.y, Hero.x)) == NULL)
	    return;
	from_floor = TRUE;
    }

    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	if (obj->o_flags & ISFOUND)
	{
	    detach(Lvl_obj, obj);
	    mvaddch(Hero.y, Hero.x, floor_ch());
	    chat(Hero.y, Hero.x) = (Proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
            update_mdest(obj);
            discarded = 1;
	    discard(obj);
	    msg("the scroll turns to dust as you pick it up");
	    return;
	}

    if (Pack == NULL)
    {
	Pack = obj;
	obj->o_packch = pack_char();
	Inpack++;
    }
    else
    {
	lp = NULL;
	for (op = Pack; op != NULL; op = next(op))
	{
	    if (op->o_type != obj->o_type)
		lp = op;
	    else
	    {
		while (op->o_type == obj->o_type && op->o_which != obj->o_which)
		{
		    lp = op;
		    if (next(op) == NULL)
			break;
		    else
			op = next(op);
		}
		if (op->o_type == obj->o_type && op->o_which == obj->o_which)
		{
		    if (ISMULT(op->o_type))
		    {
			if (!pack_room(from_floor, obj))
			    return;
			op->o_count++;
dump_it:
			update_mdest(obj);
			discard(obj);
			obj = op;
                        discarded = 1;
			lp = NULL;
			goto out;
		    }
		    else if (obj->o_group)
		    {
			lp = op;
			while (op->o_type == obj->o_type
			    && op->o_which == obj->o_which
			    && op->o_group != obj->o_group)
			{
			    lp = op;
			    if (next(op) == NULL)
				break;
			    else
				op = next(op);
			}
			if (op->o_type == obj->o_type
			    && op->o_which == obj->o_which
			    && op->o_group == obj->o_group)
			{
				op->o_count += obj->o_count;
				Inpack--;
				if (!pack_room(from_floor, obj))
				    return;
				goto dump_it;
			}
		    }
		    else
			lp = op;
		}
out:
		break;
	    }
	}

	if (lp != NULL)
	{
	    if (!pack_room(from_floor, obj))
		return;
	    else
	    {
		obj->o_packch = pack_char();
		next(obj) = next(lp);
		prev(obj) = lp;
		if (next(lp) != NULL)
		    prev(next(lp)) = obj;
		next(lp) = obj;
	    }
	}
    }

    obj->o_flags |= ISFOUND;

    /*
     * If this was the object of something's desire, that monster will
     * get mad and run at the hero.
     */
    if (!discarded)
        update_mdest(obj);

    if (obj->o_type == AMULET)
	Amulet = TRUE;
    /*
     * Notify the user
     */
    if (!silent)
    {
	if (!Terse)
	    addmsg("you now have ");
	msg("%s (%c)", inv_name(obj, !Terse), obj->o_packch);
    }
}

/*
 * pack_room:
 *	See if there's room in the pack.  If not, print out an
 *	appropriate message
 */
int
pack_room(int from_floor, THING *obj)
{
    if (++Inpack > MAXPACK)
    {
	if (!Terse)
	    addmsg("there's ");
	addmsg("no room");
	if (!Terse)
	    addmsg(" in your pack");
	endmsg();
	if (from_floor)
	    move_msg(obj);
	Inpack = MAXPACK;
	return FALSE;
    }

    if (from_floor)
    {
	detach(Lvl_obj, obj);
	mvaddch(Hero.y, Hero.x, floor_ch());
	chat(Hero.y, Hero.x) = (Proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
    }

    return TRUE;
}

/*
 * leave_pack:
 *	Take an item out of the pack
 */
THING *
leave_pack(THING *obj, int newobj, int all)
{
    THING *nobj;

    Inpack--;
    nobj = obj;
    if (obj->o_count > 1 && !all)
    {
	Last_pick = obj;
	obj->o_count--;
	if (obj->o_group)
	    Inpack++;
	if (newobj)
	{
	    nobj = new_item();
	    *nobj = *obj;
	    next(nobj) = NULL;
	    prev(nobj) = NULL;
	    nobj->o_count = 1;
	}
    }
    else
    {
	Last_pick = NULL;
	Pack_used[obj->o_packch - 'a'] = FALSE;
	detach(Pack, obj);
    }
    return nobj;
}

/*
 * pack_char:
 *	Return the next unused pack character.
 */
int
pack_char(void)
{
    int *bp;

    for (bp = Pack_used; *bp; bp++)
	continue;
    *bp = TRUE;
    return ((int)(bp - Pack_used) + 'a');
}

/*
 * inventory:
 *	List what is in the pack.  Return TRUE if there is something of
 *	the given type.
 */
int
inventory(const THING *list, int type)
{
    static char inv_temp[MAXSTR];

    N_objs = 0;
    for (; list != NULL; list = next(list))
    {
	if (type && type != list->o_type && !(type == CALLABLE &&
	    list->o_type != FOOD && list->o_type != AMULET) &&
	    !(type == R_OR_S && (list->o_type == RING || list->o_type == STICK)))
		continue;
	N_objs++;
#ifdef MASTER
	if (!list->o_packch)
	    strcpy(inv_temp, "%s");
	else
#endif
	    sprintf(inv_temp, "%c) %%s", list->o_packch);
	Msg_esc = TRUE;
	if (add_line(inv_temp, inv_name(list, FALSE)) == ESCAPE)
	{
	    Msg_esc = FALSE;
	    msg("");
	    return TRUE;
	}
	Msg_esc = FALSE;
    }
    if (N_objs == 0)
    {
	if (Terse)
	    msg(type == 0 ? "empty handed" :
			    "nothing appropriate");
	else
	    msg(type == 0 ? "you are empty handed" :
			    "you don't have anything appropriate");
	return FALSE;
    }
    end_line();
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
void
pick_up(int ch)
{
    THING *obj, *tmpl;

    if (on(Player, ISLEVIT))
	return;

    obj = find_obj(Hero.y, Hero.x);
    if (Move_on)
	move_msg(obj);
    else
	switch (ch)
	{
	    case GOLD:
		if (obj == NULL)
		    return;
		money(obj->o_goldval);
		Proom->r_goldval = 0;
		for (tmpl = Mlist; tmpl != NULL; tmpl = next(tmpl)) {
		    if (tmpl->t_dest == &obj->o_pos) {
			tmpl->t_dest = &Hero;
		    }
		}
		detach(Lvl_obj, obj);
		update_mdest(obj);
		discard(obj);
		Proom->r_goldval = 0;
		break;
	    default:
#ifdef MASTER
		debug("Where did you pick a '%s' up???", unctrl(ch));
#endif
	    case ARMOR:
	    case POTION:
	    case FOOD:
	    case WEAPON:
	    case SCROLL:	
	    case AMULET:
	    case RING:
	    case STICK:
		add_pack(NULL, FALSE);
		break;
	}
}

/*
 * move_msg:
 *	Print out the message if you are just moving onto an object
 */
void
move_msg(const THING *obj)
{
    if (!Terse)
	addmsg("you ");
    msg("moved onto %s", inv_name(obj, TRUE));
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
void
picky_inven(void)
{
    THING *obj;
    int mch;

    if (Pack == NULL)
	msg("you aren't carrying anything");
    else if (next(Pack) == NULL)
	msg("a) %s", inv_name(Pack, FALSE));
    else
    {
	msg(Terse ? "item: " : "which item do you wish to inventory: ");
	Mpos = 0;
	if ((mch = readchar()) == ESCAPE)
	{
	    msg("");
	    return;
	}
	for (obj = Pack; obj != NULL; obj = next(obj))
	    if (mch == obj->o_packch)
	    {
		msg("%c) %s", mch, inv_name(obj, FALSE));
		return;
	    }
	msg("'%s' not in pack", unctrl(mch));
    }
}

/*
 * get_item:
 *	Pick something out of a pack for a purpose
 */
THING *
get_item(const char *purpose, int type)
{
    THING *obj;
    int ch;

    if (Pack == NULL)
	msg("you aren't carrying anything");
    else if (Again)
	if (Last_pick)
	    return Last_pick;
	else
	    msg("you ran out");
    else
    {
	for (;;)
	{
	    if (!Terse)
		addmsg("which object do you want to ");
	    addmsg(purpose);
	    if (Terse)
		addmsg(" what");
	    msg("? (* for list): ");
	    ch = readchar();
	    Mpos = 0;
	    /*
	     * Give the poor player a chance to abort the command
	     */
	    if (ch == ESCAPE)
	    {
		reset_last();
		After = FALSE;
		msg("");
		return NULL;
	    }
	    N_objs = 1;		/* normal case: person types one char */
	    if (ch == '*')
	    {
		Mpos = 0;
		if (inventory(Pack, type) == 0)
		{
		    After = FALSE;
		    return NULL;
		}
		continue;
	    }
	    for (obj = Pack; obj != NULL; obj = next(obj))
		if (obj->o_packch == ch)
		    break;
	    if (obj == NULL)
	    {
		msg("'%s' is not a valid item",unctrl(ch));
		continue;
	    }
	    else {
		msg("");
		return obj;
	    }
	}
    }
    return NULL;
}

/*
 * money:
 *	Add or subtract gold from the pack
 */
void
money(int value)
{
    Purse += value;
    mvaddch(Hero.y, Hero.x, floor_ch());
    chat(Hero.y, Hero.x) = (Proom->r_flags & ISGONE) ? PASSAGE : FLOOR;
    if (value > 0)
    {
	if (!Terse)
	    addmsg("you found ");
	msg("%d gold pieces", value);
    }
}

/*
 * floor_ch:
 *	Return the appropriate floor character for her room
 */
int
floor_ch(void)
{
    if (Proom->r_flags & ISGONE)
	return PASSAGE;
    return (show_floor() ? FLOOR : ' ');
}

/*
 * floor_at:
 *	Return the character at hero's position, taking See_floor
 *	into account
 */
int
floor_at(void)
{
    int ch;

    ch = chat(Hero.y, Hero.x);
    if (ch == FLOOR)
	ch = floor_ch();
    return ch;
}

/*
 * reset_last:
 *	Reset the last command when the current one is aborted
 */
void
reset_last(void)
{
    Last_comm = L_last_comm;
    Last_dir = L_last_dir;
    Last_pick = L_last_pick;
}
