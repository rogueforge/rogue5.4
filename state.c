/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005 Nicholas J. Kisseberth
    All rights reserved.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_THING        0xABCD0002
#define RSID_THING_NULL   0xDEAD0002
#define RSID_OBJECT       0xABCD0003
#define RSID_MAGICITEMS   0xABCD0004
#define RSID_KNOWS        0xABCD0005
#define RSID_GUESSES      0xABCD0006
#define RSID_OBJECTLIST   0xABCD0007
#define RSID_BAGOBJECT    0xABCD0008
#define RSID_MONSTERLIST  0xABCD0009
#define RSID_MONSTERSTATS 0xABCD000A
#define RSID_MONSTERS     0xABCD000B
#define RSID_TRAP         0xABCD000C
#define RSID_WINDOW       0xABCD000D
#define RSID_DAEMONS      0xABCD000E
#define RSID_IWEAPS       0xABCD000F
#define RSID_IARMOR       0xABCD0010
#define RSID_SPELLS       0xABCD0011
#define RSID_ILIST        0xABCD0012
#define RSID_HLIST        0xABCD0013
#define RSID_DEATHTYPE    0xABCD0014
#define RSID_CTYPES       0XABCD0015
#define RSID_COORDLIST    0XABCD0016
#define RSID_ROOMS        0XABCD0017

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include "rogue.h"

#define READSTAT (format_error || read_error )
#define WRITESTAT (write_error)

static int read_error   = FALSE;
static int write_error  = FALSE;
static int format_error = FALSE;
static int endian = 0x01020304;
#define  big_endian ( *((char *)&endian) == 0x01 )

int
rs_write(FILE *savef, void *ptr, size_t size)
{
    if (write_error)
        return(WRITESTAT);

    if (encwrite(ptr, size, savef) != size)
        write_error = 1;

    return(WRITESTAT);
}

int
rs_read(int inf, void *ptr, size_t size)
{
    if (read_error || format_error)
        return(READSTAT);

    if (encread(ptr, size, inf) != size)
        read_error = 1;
       
    return(READSTAT);
}

int
rs_write_char(FILE *savef, char c)
{
    if (write_error)
        return(WRITESTAT);

    rs_write(savef, &c, 1);

    return(WRITESTAT);
}

int
rs_read_char(int inf, char *c)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, c, 1);

    return(READSTAT);
}

int
rs_write_chars(FILE *savef, char *c, int count)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);
    rs_write(savef, c, count);

    return(WRITESTAT);
}

int
rs_read_chars(int inf, char *i, int count)
{
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &value);
    
    if (value != count)
        format_error = TRUE;

    rs_read(inf, i, count);
    
    return(READSTAT);
}

int
rs_write_int(FILE *savef, int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_read_int(int inf, int *i)
{
    unsigned char bytes[4];
    int input = 0;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((int *) buf);

    return(READSTAT);
}

int
rs_write_ints(FILE *savef, int *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if( rs_write_int(savef,c[n]) != 0)
            break;

    return(WRITESTAT);
}

int
rs_read_ints(int inf, int *i, int count)
{
    int n, value;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_int(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_boolean(FILE *savef, bool c)
{
    unsigned char buf = (c == 0) ? 0 : 1;
    
    if (write_error)
        return(WRITESTAT);

    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_read_boolean(int inf, bool *i)
{
    unsigned char buf = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &buf, 1);

    *i = (buf != 0);
    
    return(READSTAT);
}

int
rs_write_booleans(FILE *savef, bool *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if (rs_write_boolean(savef, c[n]) != 0)
            break;

    return(WRITESTAT);
}

int
rs_read_booleans(int inf, bool *i, int count)
{
    int n = 0, value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_boolean(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_short(FILE *savef, short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_read_short(int inf, short *i)
{
    unsigned char bytes[2];
    short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((short *) buf);

    return(READSTAT);
} 

int
rs_write_shorts(FILE *savef, short *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if (rs_write_short(savef, c[n]) != 0)
            break; 

    return(WRITESTAT);
}

int
rs_read_shorts(int inf, short *i, int count)
{
    int n = 0, value = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < value; n++)
        if (rs_read_short(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_ushort(FILE *savef, unsigned short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_read_ushort(int inf, unsigned short *i)
{
    unsigned char bytes[2];
    unsigned short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((unsigned short *) buf);

    return(READSTAT);
} 

int
rs_write_uint(FILE *savef, unsigned int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_read_uint(int inf, unsigned int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned int *) buf);

    return(READSTAT);
}

int
rs_write_long(FILE *savef, long c)
{
    int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if (write_error)
        return(WRITESTAT);

    c2 = c;
    buf = (unsigned char *) &c2;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_read_long(int inf, long *i)
{
    unsigned char bytes[4];
    long input;
    unsigned char *buf = (unsigned char *) &input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((long *) buf);

    return(READSTAT);
}

int
rs_write_longs(FILE *savef, long *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_long(savef, c[n]);

    return(WRITESTAT);
}

int
rs_read_longs(int inf, long *i, int count)
{
    int n = 0, value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < value; n++)
        if (rs_read_long(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_ulong(FILE *savef, unsigned long c)
{
    unsigned int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if (write_error)
        return(WRITESTAT);

    c2 = c;
    buf = (unsigned char *) &c2;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_read_ulong(int inf, unsigned long *i)
{
    unsigned char bytes[4];
    unsigned long input;
    unsigned char *buf = (unsigned char *) &input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned long *) buf);

    return(READSTAT);
}

int
rs_write_ulongs(FILE *savef, unsigned long *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef,count);

    for(n = 0; n < count; n++)
        if (rs_write_ulong(savef,c[n]) != 0)
            break;

    return(WRITESTAT);
}

int
rs_read_ulongs(int inf, unsigned long *i, int count)
{
    int n = 0, value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_ulong(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_marker(FILE *savef, int id)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, id);

    return(WRITESTAT);
}

int 
rs_read_marker(int inf, int id)
{
    int nid;

    if (read_error || format_error)
        return(READSTAT);

    if (rs_read_int(inf, &nid) == 0)
        if (id != nid)
            format_error = 1;
    
    return(READSTAT);
}



/******************************************************************************/

int
rs_write_string(FILE *savef, char *s)
{
    int len = 0;

    if (write_error)
        return(WRITESTAT);

    len = (s == NULL) ? 0 : (int) strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write_chars(savef, s, len);
            
    return(WRITESTAT);
}

int
rs_read_string(int inf, char *s, int max)
{
    int len = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &len);

    if (len > max)
        format_error = TRUE;

    rs_read_chars(inf, s, len);
    
    return(READSTAT);
}

int
rs_read_new_string(int inf, char **s)
{
    int len=0;
    char *buf=0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &len);

    if (len == 0)
        buf = NULL;
    else
    { 
        buf = malloc(len);

        if (buf == NULL)            
            read_error = TRUE;
    }

    rs_read_chars(inf, buf, len);

    *s = buf;

    return(READSTAT);
}

int
rs_write_strings(FILE *savef, char *s[], int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if (rs_write_string(savef, s[n]) != 0)
            break;
    
    return(WRITESTAT);
}

int
rs_read_strings(int inf, char **s, int count, int max)
{
    int n     = 0;
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_string(inf, s[n], max) != 0)
            break;
    
    return(READSTAT);
}

int
rs_read_new_strings(int inf, char **s, int count)
{
    int n     = 0;
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_new_string(inf, &s[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_string_index(FILE *savef, char *master[], int max, const char *str)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < max; i++)
        if (str == master[i])
            return( rs_write_int(savef, i) );

    return( rs_write_int(savef,-1) );
}

int
rs_read_string_index(int inf, char *master[], int maxindex, char **str)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    if (i > maxindex)
        format_error = TRUE;
    else if (i >= 0)
        *str = master[i];
    else
        *str = NULL;

    return(READSTAT);
}

int
rs_write_str_t(FILE *savef, str_t st)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_uint(savef, st);

    return( WRITESTAT );
}

int
rs_read_str_t(int inf, str_t *st)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_uint(inf, st);

    return(READSTAT);
}

int
rs_write_coord(FILE *savef, coord c)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, c.x);
    rs_write_int(savef, c.y);
    
    return(WRITESTAT);
}

int
rs_read_coord(int inf, coord *c)
{
    coord in;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&in.x);
    rs_read_int(inf,&in.y);

    if (READSTAT == 0) 
    {
        c->x = in.x;
        c->y = in.y;
    }

    return(READSTAT);
}

int
rs_write_window(FILE *savef, WINDOW *win)
{
    int row,col,height,width;

    if (write_error)
        return(WRITESTAT);

    width  = getmaxx(win);
    height = getmaxy(win);

    rs_write_marker(savef,RSID_WINDOW);
    rs_write_int(savef,height);
    rs_write_int(savef,width);

    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            if (rs_write_int(savef, mvwinch(win,row,col)) != 0)
                return(WRITESTAT);

    return(WRITESTAT);
}

int
rs_read_window(int inf, WINDOW *win)
{
    int row,col,maxlines,maxcols,value,width,height;
    
    if (read_error || format_error)
        return(READSTAT);

    width  = getmaxx(win);
    height = getmaxy(win);

    rs_read_marker(inf, RSID_WINDOW);

    rs_read_int(inf, &maxlines);
    rs_read_int(inf, &maxcols);

    for(row = 0; row < maxlines; row++)
        for(col = 0; col < maxcols; col++)
        {
            if (rs_read_int(inf, &value) != 0)
                return(READSTAT);

            if ((row < height) && (col < width))
                mvwaddch(win,row,col,value);
        }
        
    return(READSTAT);
}

/******************************************************************************/

void *
get_list_item(THING *l, int i)
{
    int count;

    for(count = 0; l != NULL; count++, l = l->l_next)
        if (count == i)
            return(l);
    
    return(NULL);
}

int
find_list_ptr(THING *l, void *ptr)
{
    int count;

    for(count = 0; l != NULL; count++, l = l->l_next)
        if (l == ptr)
            return(count);
    
    return(-1);
}

int
list_size(THING *l)
{
    int count;
    
    for(count = 0; l != NULL; count++, l = l->l_next)
        ;
    
    return(count);
}

/******************************************************************************/

int
rs_write_stats(FILE *savef, struct stats *s)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_STATS);
    rs_write_str_t(savef, s->s_str);
    rs_write_long(savef, s->s_exp);
    rs_write_int(savef, s->s_lvl);
    rs_write_int(savef, s->s_arm);
    rs_write_int(savef, s->s_hpt);
    rs_write_chars(savef, s->s_dmg, sizeof(s->s_dmg));
    rs_write_int(savef,s->s_maxhp);

    return(WRITESTAT);
}

int
rs_read_stats(int inf, struct stats *s)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_STATS);
    rs_read_str_t(inf,&s->s_str);
    rs_read_long(inf,&s->s_exp);
    rs_read_int(inf,&s->s_lvl);
    rs_read_int(inf,&s->s_arm);
    rs_read_int(inf,&s->s_hpt);
    rs_read_chars(inf,s->s_dmg,sizeof(s->s_dmg));
    rs_read_int(inf,&s->s_maxhp);

    return(READSTAT);
}

int
rs_write_stone_index(FILE *savef, STONE master[], int max, const char *str)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < max; i++)
        if (str == master[i].st_name)
        {
            rs_write_int(savef,i);
            return(WRITESTAT);
        }

    rs_write_int(savef,-1);

    return(WRITESTAT);
}

int
rs_read_stone_index(int inf, STONE master[], int maxindex, char **str)
{
    int i = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&i);

    if (i > maxindex)
        format_error = TRUE;
    else if (i >= 0)
        *str = master[i].st_name;
    else
        *str = NULL;

    return(READSTAT);
}

int
rs_write_scrolls(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < MAXSCROLLS; i++)
        rs_write_string(savef, S_names[i]);

    return(READSTAT);
}

int
rs_read_scrolls(int inf)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXSCROLLS; i++)
        rs_read_new_string(inf, &S_names[i]);

    return(READSTAT);
}

int
rs_write_potions(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < MAXPOTIONS; i++)
        rs_write_string_index(savef, Rainbow, cNCOLORS, P_colors[i]);

    return(WRITESTAT);
}

int
rs_read_potions(int inf)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXPOTIONS; i++)
        rs_read_string_index(inf, Rainbow, cNCOLORS, &P_colors[i]);

    return(READSTAT);
}

int
rs_write_rings(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < MAXRINGS; i++)
        rs_write_stone_index(savef, Stones, cNSTONES, R_stones[i]);

    return(WRITESTAT);
}

int
rs_read_rings(int inf)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXRINGS; i++)
        rs_read_stone_index(inf, Stones, cNSTONES, &R_stones[i]);

    return(READSTAT);
}

int
rs_write_sticks(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(Ws_type[i],"staff") == 0)
        {
            rs_write_int(savef,0);
            rs_write_string_index(savef, Wood, cNWOOD, Ws_made[i]);
        }
        else
        {
            rs_write_int(savef,1);
            rs_write_string_index(savef, Metal, cNMETAL, Ws_made[i]);
        }
    }
 
    return(WRITESTAT);
}
        
int
rs_read_sticks(int inf)
{
    int i = 0, list = 0;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXSTICKS; i++)
    { 
        rs_read_int(inf,&list);

        if (list == 0)
        {
            rs_read_string_index(inf, Wood, cNWOOD, &Ws_made[i]);
            Ws_type[i] = "staff";
        }
        else 
        {
            rs_read_string_index(inf, Metal, cNMETAL, &Ws_made[i]);
            Ws_type[i] = "wand";
        }
    }

    return(READSTAT);
}

int
rs_write_daemons(FILE *savef, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
        
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_DAEMONS);
    rs_write_int(savef, count);
        
    for(i = 0; i < count; i++)
    {
        if (d_list[i].d_func == rollwand)
            func = 1;
        else if (d_list[i].d_func == doctor)
            func = 2;
        else if (d_list[i].d_func == stomach)
            func = 3;
        else if (d_list[i].d_func == runners)
            func = 4;
        else if (d_list[i].d_func == swander)
            func = 5;
        else if (d_list[i].d_func == nohaste)
            func = 6;
        else if (d_list[i].d_func == unconfuse)
            func = 7;
        else if (d_list[i].d_func == unsee)
            func = 8;
        else if (d_list[i].d_func == sight)
            func = 9;
        else if (d_list[i].d_func == NULL)
            func = 0;
        else
            func = -1;

        rs_write_int(savef, d_list[i].d_type);
        rs_write_int(savef, func);
        rs_write_int(savef, d_list[i].d_arg);
        rs_write_int(savef, d_list[i].d_time);
    }
    
    return(WRITESTAT);
}       

int
rs_read_daemons(int inf, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_DAEMONS);
    rs_read_int(inf, &value);

    if (value > count)
        format_error = TRUE;

    for(i=0; i < count; i++)
    {
        func = 0;
        rs_read_int(inf, &d_list[i].d_type);
        rs_read_int(inf, &func);
        rs_read_int(inf, &d_list[i].d_arg);
        rs_read_int(inf, &d_list[i].d_time);
                    
        switch(func)
        {
            case 1: d_list[i].d_func = rollwand;
                    break;
            case 2: d_list[i].d_func = doctor;
                    break;
            case 3: d_list[i].d_func = stomach;
                    break;
            case 4: d_list[i].d_func = runners;
                    break;
            case 5: d_list[i].d_func = swander;
                    break;
            case 6: d_list[i].d_func = nohaste;
                    break;
            case 7: d_list[i].d_func = unconfuse;
                    break;
            case 8: d_list[i].d_func = unsee;
                    break;
            case 9: d_list[i].d_func = sight;
                    break;
            default:d_list[i].d_func = NULL;
                    break;
        }
    }

    if (d_list[i].d_func == NULL)
    {
        d_list[i].d_type = 0;
        d_list[i].d_arg = 0;
        d_list[i].d_time = 0;
    }
    
    return(READSTAT);
}       
        
int
rs_write_obj_info(FILE *savef, struct obj_info *i, int count)
{
    int n;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_MAGICITEMS);
    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
    {
        /* mi_name is constant, defined at compile time in all cases */
        rs_write_int(savef,i[n].oi_prob);
        rs_write_int(savef,i[n].oi_worth);
        rs_write_string(savef,i[n].oi_guess);
        rs_write_boolean(savef,i[n].oi_know);
    }
    
    return(WRITESTAT);
}

int
rs_read_obj_info(int inf, struct obj_info *mi, int count)
{
    int n;
    int value;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_MAGICITEMS);

    rs_read_int(inf, &value);

    if (value > count)
        format_error = TRUE;

    for(n = 0; n < value; n++)
    {
        /* mi_name is const, defined at compile time in all cases */
        rs_read_int(inf,&mi[n].oi_prob);
        rs_read_int(inf,&mi[n].oi_worth);
        rs_read_new_string(inf,&mi[n].oi_guess);
        rs_read_boolean(inf,&mi[n].oi_know);
    }
    
    return(READSTAT);
}

int
rs_write_room(FILE *savef, struct room *r)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_coord(savef, r->r_pos);
    rs_write_coord(savef, r->r_max);
    rs_write_coord(savef, r->r_gold);
    rs_write_int(savef,   r->r_goldval);
    rs_write_short(savef, r->r_flags);
    rs_write_int(savef, r->r_nexits);
    rs_write_coord(savef, r->r_exit[0]);
    rs_write_coord(savef, r->r_exit[1]);
    rs_write_coord(savef, r->r_exit[2]);
    rs_write_coord(savef, r->r_exit[3]);
    rs_write_coord(savef, r->r_exit[4]);
    rs_write_coord(savef, r->r_exit[5]);
    rs_write_coord(savef, r->r_exit[6]);
    rs_write_coord(savef, r->r_exit[7]);
    rs_write_coord(savef, r->r_exit[8]);
    rs_write_coord(savef, r->r_exit[9]);
    rs_write_coord(savef, r->r_exit[10]);
    rs_write_coord(savef, r->r_exit[11]);
    
    return(WRITESTAT);
}

int
rs_read_room(int inf, struct room *r)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_coord(inf,&r->r_pos);
    rs_read_coord(inf,&r->r_max);
    rs_read_coord(inf,&r->r_gold);
    rs_read_int(inf,&r->r_goldval);
    rs_read_short(inf,&r->r_flags);
    rs_read_int(inf,&r->r_nexits);
    rs_read_coord(inf,&r->r_exit[0]);
    rs_read_coord(inf,&r->r_exit[1]);
    rs_read_coord(inf,&r->r_exit[2]);
    rs_read_coord(inf,&r->r_exit[3]);
    rs_read_coord(inf,&r->r_exit[4]);
    rs_read_coord(inf,&r->r_exit[5]);
    rs_read_coord(inf,&r->r_exit[6]);
    rs_read_coord(inf,&r->r_exit[7]);
    rs_read_coord(inf,&r->r_exit[8]);
    rs_read_coord(inf,&r->r_exit[9]);
    rs_read_coord(inf,&r->r_exit[10]);
    rs_read_coord(inf,&r->r_exit[11]);

    return(READSTAT);
}

int
rs_write_rooms(FILE *savef, struct room r[], int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);
    
    for(n = 0; n < count; n++)
        rs_write_room(savef, &r[n]);
    
    return(WRITESTAT);
}

int
rs_read_rooms(int inf, struct room *r, int count)
{
    int value = 0, n = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value > count)
        format_error = TRUE;

    for(n = 0; n < value; n++)
        rs_read_room(inf,&r[n]);

    return(READSTAT);
}

int
rs_write_room_reference(FILE *savef, struct room *rp)
{
    int i, room = -1;
    
    if (write_error)
        return(WRITESTAT);

    for (i = 0; i < MAXROOMS; i++)
        if (&Rooms[i] == rp)
            room = i;

    rs_write_int(savef, room);

    return(WRITESTAT);
}

int
rs_read_room_reference(int inf, struct room **rp)
{
    int i;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    *rp = &Rooms[i];
            
    return(READSTAT);
}

int
rs_write_monsters(FILE *savef, struct monster *m, int count)
{
    int n;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_MONSTERS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
        rs_write_stats(savef, &m[n].m_stats);
    
    return(WRITESTAT);
}

int
rs_read_monsters(int inf, struct monster *m, int count)
{
    int value = 0, n = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_MONSTERS);

    rs_read_int(inf, &value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        rs_read_stats(inf, &m[n].m_stats);
    
    return(READSTAT);
}

int
rs_write_object(FILE *savef, THING *o)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_OBJECT);
    rs_write_int(savef, o->_o._o_type); 
    rs_write_coord(savef, o->_o._o_pos); 
    rs_write_int(savef, o->_o._o_launch);
    rs_write_char(savef, o->_o._o_packch);
    rs_write_chars(savef, o->_o._o_damage, sizeof(o->_o._o_damage));
    rs_write_chars(savef, o->_o._o_hurldmg, sizeof(o->_o._o_hurldmg));
    rs_write_int(savef, o->_o._o_count);
    rs_write_int(savef, o->_o._o_which);
    rs_write_int(savef, o->_o._o_hplus);
    rs_write_int(savef, o->_o._o_dplus);
    rs_write_int(savef, o->_o._o_arm);
    rs_write_int(savef, o->_o._o_flags);
    rs_write_int(savef, o->_o._o_group);
    rs_write_string(savef, o->_o._o_label);
    return(WRITESTAT);
}

int
rs_read_object(int inf, THING *o)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_OBJECT);
    rs_read_int(inf, &o->_o._o_type);
    rs_read_coord(inf, &o->_o._o_pos);
    rs_read_int(inf, &o->_o._o_launch);
    rs_read_char(inf, &o->_o._o_packch);
    rs_read_chars(inf, o->_o._o_damage, sizeof(o->_o._o_damage));
    rs_read_chars(inf, o->_o._o_hurldmg, sizeof(o->_o._o_hurldmg));
    rs_read_int(inf, &o->_o._o_count);
    rs_read_int(inf, &o->_o._o_which);
    rs_read_int(inf, &o->_o._o_hplus);
    rs_read_int(inf, &o->_o._o_dplus);
    rs_read_int(inf, &o->_o._o_arm);
    rs_read_int(inf, &o->_o._o_flags);
    rs_read_int(inf, &o->_o._o_group);
    rs_read_new_string(inf, &o->_o._o_label);
    
    return(READSTAT);
}

int
rs_write_object_list(FILE *savef, THING *l)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list_size(l));

    for( ;l != NULL; l = l->l_next)
        rs_write_object(savef, l);
    
    return(WRITESTAT);
}

int
rs_read_object_list(int inf, THING **list)
{
    int i, cnt;
    THING *l = NULL, *previous = NULL, *head = NULL;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_OBJECTLIST);
    rs_read_int(inf, &cnt);

    for (i = 0; i < cnt; i++) 
    {
        l = new_item();

        l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;

        rs_read_object(inf,l);

        if (previous == NULL)
            head = l;

        previous = l;
    }
            
    if (l != NULL)
        l->l_next = NULL;
    
    *list = head;

    return(READSTAT);
}

int
rs_write_object_reference(FILE *savef, THING *list, THING *item)
{
    int i;
    
    if (write_error)
        return(WRITESTAT);

    i = find_list_ptr(list, item);

    rs_write_int(savef, i);

    return(WRITESTAT);
}

int
rs_read_object_reference(int inf, THING *list, THING **item)
{
    int i;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    *item = get_list_item(list,i);
            
    return(READSTAT);
}

int
find_room_coord(struct room *rmlist, coord *c, int n)
{
    int i = 0;
    
    for(i = 0; i < n; i++)
        if(&rmlist[i].r_gold == c)
            return(i);
    
    return(-1);
}

int
find_thing_coord(THING *monlist, coord *c)
{
    THING *mitem;
    THING *tp;
    int i = 0;

    for(mitem = monlist; mitem != NULL; mitem = mitem->l_next)
    {
        tp = mitem;

        if (c == &tp->t_pos)
            return(i);

        i++;
    }

    return(-1);
}

int
find_object_coord(THING *objlist, coord *c)
{
    THING *oitem;
    THING *obj;
    int i = 0;

    for(oitem = objlist; oitem != NULL; oitem = oitem->l_next)
    {
        obj = oitem;

        if (c == &obj->o_pos)
            return(i);

        i++;
    }

    return(-1);
}

int
rs_write_thing(FILE *savef, THING *t)
{
    int i = -1;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_THING);

    if (t == NULL)
    {
        rs_write_int(savef, 0);
        return(WRITESTAT);
    }
    
    rs_write_int(savef, 1);
    rs_write_coord(savef, t->_t._t_pos);
    rs_write_boolean(savef, t->_t._t_turn);
    rs_write_char(savef, t->_t._t_type);
    rs_write_char(savef, t->_t._t_disguise);
    rs_write_char(savef, t->_t._t_oldch);

    /* 
        t_dest can be:
        0,0: NULL
        0,1: location of hero
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */

    if (t->t_dest == &Hero)
    {
        rs_write_int(savef,0);
        rs_write_int(savef,1);
    }
    else if (t->t_dest != NULL)
    {
        i = find_thing_coord(Mlist, t->t_dest);
            
        if (i >=0 )
        {
            rs_write_int(savef,1);
            rs_write_int(savef,i);
        }
        else
        {
            i = find_object_coord(Lvl_obj, t->t_dest);
            
            if (i >= 0)
            {
                rs_write_int(savef,2);
                rs_write_int(savef,i);
            }
            else
            {
                i = find_room_coord(Rooms, t->t_dest, MAXROOMS);
        
                if (i >= 0) 
                {
                    rs_write_int(savef,3);
                    rs_write_int(savef,i);
                }
                else 
                {
                    rs_write_int(savef, 0);
                    rs_write_int(savef,1); /* chase the hero anyway */
                }
            }
        }
    }
    else
    {
        rs_write_int(savef,0);
        rs_write_int(savef,0);
    }
    
    rs_write_short(savef, t->_t._t_flags);
    rs_write_stats(savef, &t->_t._t_stats);
    rs_write_room_reference(savef, t->_t._t_room);
    rs_write_object_list(savef, t->_t._t_pack);
    
    return(WRITESTAT);
}

int
rs_read_thing(int inf, THING *t)
{
    int listid = 0, index = -1;
    THING *item;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_THING);

    rs_read_int(inf, &index);

    if (index == 0)
        return(READSTAT);

    rs_read_coord(inf,&t->_t._t_pos);
    rs_read_boolean(inf,&t->_t._t_turn);
    rs_read_char(inf,&t->_t._t_type);
    rs_read_char(inf,&t->_t._t_disguise);
    rs_read_char(inf,&t->_t._t_oldch);
            
    /* 
        t_dest can be (listid,index):
        0,0: NULL
        0,1: location of hero
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */
            
    rs_read_int(inf, &listid);
    rs_read_int(inf, &index);
    t->_t._t_reserved = -1;

    if (listid == 0) /* hero or NULL */
    {
        if (index == 1)
            t->_t._t_dest = &Hero;
        else
            t->_t._t_dest = NULL;
    }
    else if (listid == 1) /* monster/thing */
    {
        t->_t._t_dest     = NULL;
        t->_t._t_reserved = index;
    }
    else if (listid == 2) /* object */
    {
        THING *obj;

        item = get_list_item(Lvl_obj, index);

        if (item != NULL)
        {
            obj = item;
            t->_t._t_dest = &obj->o_pos;
        }
    }
    else if (listid == 3) /* gold */
    {
        t->_t._t_dest = &Rooms[index].r_gold;
    }
    else
        t->_t._t_dest = NULL;
            
    rs_read_short(inf,&t->_t._t_flags);
    rs_read_stats(inf,&t->_t._t_stats);
    rs_read_room_reference(inf, &t->_t._t_room);
    rs_read_object_list(inf,&t->_t._t_pack);
    
    return(READSTAT);
}

int
rs_fix_thing(THING *t)
{
    THING *item;
    THING *tp;

    if (t->t_reserved < 0)
        return;

    item = get_list_item(Mlist,t->t_reserved);

    if (item != NULL)
    {
        tp = item;
        t->t_dest = &tp->t_pos;
    }
}

int
rs_write_thing_list(FILE *savef, THING *l)
{
    int cnt = 0;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return(WRITESTAT);

    while (l != NULL) {
        rs_write_thing(savef, l);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_read_thing_list(int inf, THING **list)
{
    int i, cnt;
    THING *l = NULL, *previous = NULL, *head = NULL;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_MONSTERLIST);

    rs_read_int(inf, &cnt);

    for (i = 0; i < cnt; i++) 
    {
        l = new_item();

        l->l_prev = previous;
            
        if (previous != NULL)
            previous->l_next = l;

        rs_read_thing(inf,l);

        if (previous == NULL)
            head = l;

        previous = l;
    }
        
    if (l != NULL)
        l->l_next = NULL;

    *list = head;
    
    return(READSTAT);
}

int
rs_fix_thing_list(THING *list)
{
    THING *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(item);
}

int
rs_write_thing_reference(FILE *savef, THING *list, THING *item)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    if (item == NULL)
        rs_write_int(savef,-1);
    else
    {
        i = find_list_ptr(list, item);

        rs_write_int(savef, i);
    }

    return(WRITESTAT);
}

int
rs_read_thing_reference(int inf, THING *list, THING **item)
{
    int i;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    if (i == -1)
        *item = NULL;
    else
        *item = get_list_item(list,i);

    return(READSTAT);
}

int
rs_write_thing_references(FILE *savef, THING *list, THING *items[], int count)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < count; i++)
        rs_write_thing_reference(savef,list,items[i]);

    return(WRITESTAT);
}

int
rs_read_thing_references(int inf, THING *list, THING *items[], int count)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < count; i++)
        rs_read_thing_reference(inf,list,&items[i]);

    return(WRITESTAT);
}

int 
rs_write_places(FILE *savef, PLACE *places, int count)
{
    int i = 0;
    
    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < count; i++) 
    {
        rs_write_char(savef, Places[i].p_ch);
        rs_write_char(savef, Places[i].p_flags);
        rs_write_thing_reference(savef, Mlist, Places[i].p_monst);
    }

    return(WRITESTAT);
}

int 
rs_read_places(int inf, PLACE *places, int count)
{
    int i = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < count; i++) 
    {
        rs_read_char(inf,&Places[i].p_ch);
        rs_read_char(inf,&Places[i].p_flags);
        rs_read_thing_reference(inf, Mlist, &Places[i].p_monst);
    }

    return(READSTAT);
}

int
rs_save_file(FILE *savef)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_boolean(savef, After);                 /* 1  */    /* extern.c */
    rs_write_boolean(savef, Again);                 /* 2  */
    rs_write_boolean(savef, Noscore);               /* 3  */
    rs_write_boolean(savef, Seenstairs);            /* 4  */
    rs_write_boolean(savef, Amulet);                /* 5  */
    rs_write_boolean(savef, Door_stop);             /* 6  */
    rs_write_boolean(savef, Fight_flush);           /* 7  */
    rs_write_boolean(savef, Firstmove);             /* 8  */
#ifdef TIOCGLTC
    rs_write_boolean(savef, Got_ltc);               /* 9  */
#else
    rs_write_boolean(savef, 0);                     /* 9  */
#endif
    rs_write_boolean(savef, Has_hit);               /* 10 */
    rs_write_boolean(savef, In_shell);              /* 11 */
    rs_write_boolean(savef, Inv_describe);          /* 12 */
    rs_write_boolean(savef, Jump);                  /* 13 */
    rs_write_boolean(savef, Kamikaze);              /* 14 */
    rs_write_boolean(savef, Lower_msg);             /* 15 */
    rs_write_boolean(savef, Move_on);               /* 16 */
    rs_write_boolean(savef, Msg_esc);               /* 17 */
    rs_write_boolean(savef, Passgo);                /* 18 */
    rs_write_boolean(savef, Playing);               /* 19 */
    rs_write_boolean(savef, Q_comm);                /* 20 */
    rs_write_boolean(savef, Running);               /* 21 */
    rs_write_boolean(savef, Save_msg);              /* 22 */
    rs_write_boolean(savef, See_floor);             /* 23 */
    rs_write_boolean(savef, Stat_msg);              /* 24 */
    rs_write_boolean(savef, Terse);                 /* 25 */
    rs_write_boolean(savef, To_death);              /* 26 */
    rs_write_boolean(savef, Tombstone);             /* 27 */
#ifdef MASTER
    rs_write_boolean(savef, Wizard);                /* 28 */
#else
    rs_write_boolean(savef, 0);                     /* 28 */
#endif
    rs_write_booleans(savef, Pack_used, 26);        /* 29 */
    rs_write_char(savef, Dir_ch);
    rs_write_chars(savef, File_name, MAXSTR);
    rs_write_chars(savef, Huh, MAXSTR);
    rs_write_potions(savef);
    rs_write_chars(savef,Prbuf,2*MAXSTR);
    rs_write_rings(savef);
    rs_write_string(savef,Release);
    rs_write_char(savef, Runch);
    rs_write_scrolls(savef);
    rs_write_char(savef, Take);
    rs_write_chars(savef, Whoami, MAXSTR);
    rs_write_sticks(savef);
    rs_write_int(savef,Orig_dsusp);
    rs_write_chars(savef, Fruit, MAXSTR);
    rs_write_chars(savef, Home, MAXSTR);
    rs_write_strings(savef,Inv_t_name,3);
    rs_write_char(savef,L_last_comm);
    rs_write_char(savef,L_last_dir);
    rs_write_char(savef,Last_comm);
    rs_write_char(savef,Last_dir);
    rs_write_strings(savef,Tr_name,8);
    rs_write_int(savef, N_objs);
    rs_write_int(savef, Ntraps);
    rs_write_int(savef, Hungry_state);
    rs_write_int(savef, Inpack);
    rs_write_int(savef, Inv_type);
    rs_write_int(savef, Level);
    rs_write_int(savef, Max_level);
    rs_write_int(savef, Mpos);
    rs_write_int(savef, No_food);
    rs_write_ints(savef,A_class,MAXARMORS);
    rs_write_int(savef, Count);
    rs_write_int(savef, Fd);
    rs_write_int(savef, Food_left);
    rs_write_int(savef, Lastscore);
    rs_write_int(savef, No_command);
    rs_write_int(savef, No_move);
    rs_write_int(savef, Purse);
    rs_write_int(savef, Quiet);
    rs_write_int(savef, Vf_hit);
    rs_write_long(savef, Dnum);
    rs_write_long(savef, Seed);
    rs_write_longs(savef, E_levels, 21);
    rs_write_coord(savef, Delta);
    rs_write_coord(savef, Oldpos);
    rs_write_coord(savef, Stairs);

    rs_write_thing(savef, &Player);                     
    rs_write_object_reference(savef, Player.t_pack, Cur_armor);
    rs_write_object_reference(savef, Player.t_pack, Cur_ring[0]);
    rs_write_object_reference(savef, Player.t_pack, Cur_ring[1]); 
    rs_write_object_reference(savef, Player.t_pack, Cur_weapon); 
    rs_write_object_reference(savef, Player.t_pack, L_last_pick); 
    rs_write_object_reference(savef, Player.t_pack, Last_pick); 
    
    rs_write_object_list(savef, Lvl_obj);               
    rs_write_thing_list(savef, Mlist);                

    rs_write_places(savef,Places,MAXLINES*MAXCOLS);

    rs_write_stats(savef,&Max_stats); 
    rs_write_rooms(savef, Rooms, MAXROOMS);             
    rs_write_room_reference(savef, Oldrp);              
    rs_write_rooms(savef, Passages, MAXPASS);

    rs_write_monsters(savef, Monsters,26);               
    rs_write_obj_info(savef, Things,   NUMTHINGS);   
    rs_write_obj_info(savef, Arm_info,  MAXARMORS);  
    rs_write_obj_info(savef, Pot_info,  MAXPOTIONS);  
    rs_write_obj_info(savef, Ring_info,  MAXRINGS);    
    rs_write_obj_info(savef, Scr_info,  MAXSCROLLS);  
    rs_write_obj_info(savef, Weap_info,  MAXWEAPONS+1);  
    rs_write_obj_info(savef, Ws_info, MAXSTICKS);      
    
    rs_write_daemons(savef, &d_list[0], 20);            /* 5.4-daemon.c */
#ifdef MASTER
    rs_write_int(savef,Total);                          /* 5.4-list.c   */
#else
    rs_write_int(savef,0);
#endif
    rs_write_int(savef,Between);                        /* 5.4-daemons.c*/
    rs_write_coord(savef, nh);                          /* 5.4-move.c    */
    rs_write_int(savef, Group);                         /* 5.4-weapons.c */

    rs_write_window(savef,stdscr);

    return(WRITESTAT);
}

int
rs_restore_file(int inf)
{
    bool junk;
    int junk3;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_boolean(inf, &After);               /* 1  */    /* extern.c */
    rs_read_boolean(inf, &Again);               /* 2  */
    rs_read_boolean(inf, &Noscore);             /* 3  */
    rs_read_boolean(inf, &Seenstairs);          /* 4  */
    rs_read_boolean(inf, &Amulet);              /* 5  */
    rs_read_boolean(inf, &Door_stop);           /* 6  */
    rs_read_boolean(inf, &Fight_flush);         /* 7  */
    rs_read_boolean(inf, &Firstmove);           /* 8  */
#ifdef TIOCGLTC
    rs_read_boolean(inf, &Got_ltc);             /* 9  */
#else
    rs_read_boolean(inf, &junk);                /* 9  */
#endif
    rs_read_boolean(inf, &Has_hit);             /* 10 */
    rs_read_boolean(inf, &In_shell);            /* 11 */
    rs_read_boolean(inf, &Inv_describe);        /* 12 */
    rs_read_boolean(inf, &Jump);                /* 13 */
    rs_read_boolean(inf, &Kamikaze);            /* 14 */
    rs_read_boolean(inf, &Lower_msg);           /* 15 */
    rs_read_boolean(inf, &Move_on);             /* 16 */
    rs_read_boolean(inf, &Msg_esc);             /* 17 */
    rs_read_boolean(inf, &Passgo);              /* 18 */
    rs_read_boolean(inf, &Playing);             /* 19 */
    rs_read_boolean(inf, &Q_comm);              /* 20 */
    rs_read_boolean(inf, &Running);             /* 21 */
    rs_read_boolean(inf, &Save_msg);            /* 22 */
    rs_read_boolean(inf, &See_floor);           /* 23 */
    rs_read_boolean(inf, &Stat_msg);            /* 24 */
    rs_read_boolean(inf, &Terse);               /* 25 */
    rs_read_boolean(inf, &To_death);            /* 26 */
    rs_read_boolean(inf, &Tombstone);           /* 27 */
#ifdef WIZARD
    rs_read_boolean(inf, &Wizard);              /* 28 */
#else
    rs_read_boolean(inf, &junk);                /* 28 */
#endif
    rs_read_booleans(inf, Pack_used, 26);       /* 29 */
    rs_read_char(inf, &Dir_ch);
    rs_read_chars(inf, File_name, MAXSTR);
    rs_read_chars(inf, Huh, MAXSTR);
    rs_read_potions(inf);
    rs_read_chars(inf, Prbuf, 2*MAXSTR);
    rs_read_rings(inf);
    rs_read_new_string(inf,&Release);
    rs_read_char(inf, &Runch);
    rs_read_scrolls(inf);
    rs_read_char(inf, &Take);
    rs_read_chars(inf, Whoami, MAXSTR);
    rs_read_sticks(inf);
    rs_read_int(inf,&Orig_dsusp);
    rs_read_chars(inf, Fruit, MAXSTR);
    rs_read_chars(inf, Home, MAXSTR);
    rs_read_new_strings(inf,Inv_t_name,3);
    rs_read_char(inf, &L_last_comm);
    rs_read_char(inf, &L_last_dir);
    rs_read_char(inf, &Last_comm);
    rs_read_char(inf, &Last_dir);
    rs_read_new_strings(inf,Tr_name,8);
    rs_read_int(inf, &N_objs);
    rs_read_int(inf, &Ntraps);
    rs_read_int(inf, &Hungry_state);
    rs_read_int(inf, &Inpack);
    rs_read_int(inf, &Inv_type);
    rs_read_int(inf, &Level);
    rs_read_int(inf, &Max_level);
    rs_read_int(inf, &Mpos);
    rs_read_int(inf, &No_food);
    rs_read_ints(inf,A_class,MAXARMORS);
    rs_read_int(inf, &Count);
    rs_read_int(inf, &Fd);
    rs_read_int(inf, &Food_left);
    rs_read_int(inf, &Lastscore);
    rs_read_int(inf, &No_command);
    rs_read_int(inf, &No_move);
    rs_read_int(inf, &Purse);
    rs_read_int(inf, &Quiet);
    rs_read_int(inf, &Vf_hit);
    rs_read_long(inf, &Dnum);
    rs_read_long(inf, &Seed);
    rs_read_longs(inf, E_levels,21);
    rs_read_coord(inf, &Delta);
    rs_read_coord(inf, &Oldpos);
    rs_read_coord(inf, &Stairs);

    rs_read_thing(inf, &Player); 
    rs_read_object_reference(inf, Player.t_pack, &Cur_armor);
    rs_read_object_reference(inf, Player.t_pack, &Cur_ring[0]);
    rs_read_object_reference(inf, Player.t_pack, &Cur_ring[1]);
    rs_read_object_reference(inf, Player.t_pack, &Cur_weapon);
    rs_read_object_reference(inf, Player.t_pack, &L_last_pick);
    rs_read_object_reference(inf, Player.t_pack, &Last_pick);

    rs_read_object_list(inf, &Lvl_obj);                 
    rs_read_thing_list(inf, &Mlist);                  
    rs_fix_thing(&Player);
    rs_fix_thing_list(Mlist);

    rs_read_places(inf,Places,MAXLINES*MAXCOLS);

    rs_read_stats(inf, &Max_stats);
    rs_read_rooms(inf, Rooms, MAXROOMS);
    rs_read_room_reference(inf, &Oldrp);
    rs_read_rooms(inf, Passages, MAXPASS);

    rs_read_monsters(inf,Monsters,26);                  
    rs_read_obj_info(inf, Things,   NUMTHINGS);         
    rs_read_obj_info(inf, Arm_info,   MAXARMORS);         
    rs_read_obj_info(inf, Pot_info,  MAXPOTIONS);       
    rs_read_obj_info(inf, Ring_info,  MAXRINGS);         
    rs_read_obj_info(inf, Scr_info,  MAXSCROLLS);       
    rs_read_obj_info(inf, Weap_info, MAXWEAPONS+1);       
    rs_read_obj_info(inf, Ws_info, MAXSTICKS);       

    rs_read_daemons(inf, d_list, 20);                   /* 5.4-daemon.c     */
#ifdef MASTER
    rs_read_int(inf,&Total);                            /* 5.4-list.c    */
#else
    rs_read_int(inf,&junk3);
#endif
    rs_read_int(inf,&Between);                          /* 5.4-daemons.c    */
    rs_read_coord(inf, &nh);                            /* 5.4-move.c       */
    rs_read_int(inf,&Group);                            /* 5.4-weapons.c    */
    
    rs_read_window(inf,stdscr);

    return(READSTAT);
}
