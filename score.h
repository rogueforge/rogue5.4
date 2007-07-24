/*
 * Score file structure
 *
 * @(#)score.h	4.6 (Berkeley) 02/05/99
 */

struct sc_ent {
    unsigned int sc_uid;
    int sc_score;
    unsigned int sc_flags;
    unsigned short sc_monster;
    char sc_name[MAXSTR];
    int sc_level;
    int sc_time;
};

typedef struct sc_ent SCORE;

void	rd_score(SCORE *top_ten);
void	wr_score(SCORE *top_ten);
