/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	4.12 (Berkeley) 02/05/99
 */

#include <stdlib.h>
#include <curses.h>
#include "rogue.h"

int Total = 0;			/* total dynamic memory bytes */

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */
void
_detach(THING **list, THING *item)
{
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL)
	item->l_prev->l_next = next(item);
    if (next(item) != NULL)
	item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */
void
_attach(THING **list, THING *item)
{
    if (*list != NULL)
    {
	item->l_next = *list;
	(*list)->l_prev = item;
	item->l_prev = NULL;
    }
    else
    {
	item->l_next = NULL;
	item->l_prev = NULL;
    }
    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */
void
_free_list(THING **ptr)
{
    THING *item;

    while (*ptr != NULL)
    {
	item = *ptr;
	*ptr = next(item);
	discard(item);
    }
}

/*
 * discard:
 *	Free up an item
 */
void
discard(THING *item)
{
    Total--;
    free((char *) item);
}

/*
 * new_item
 *	Get a new item with a specified size
 */
THING *
new_item(void)
{
    THING *item;

    item = calloc(1, sizeof *item);

	if (item == NULL) {
		if (Wizard && debug)
			msg("ran out of memory after %d items", Total);
	}
    else
	{
		Total++;
        item->l_next = NULL;
        item->l_prev = NULL;
	}

    return item;
}
