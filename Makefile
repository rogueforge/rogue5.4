#
# Makefile for rogue
# @(#)Makefile	4.21 (Berkeley) 02/04/99
#
HDRS=	rogue.h extern.h score.h
DOBJS=	vers.o extern.o armor.o chase.o command.o daemon.o \
	daemons.o fight.o init.o io.o list.o main.o misc.o \
	monsters.o move.o new_level.o options.o pack.o passages.o potions.o \
	rings.o rooms.o save.o scrolls.o sticks.o things.o \
	weapons.o wizard.o
OBJS=	$(DOBJS) mach_dep.o rip.o
CFILES=	vers.c extern.c armor.c chase.c command.c daemon.c \
	daemons.c fight.c init.c io.c list.c main.c misc.c \
	monsters.c move.c new_level.c options.c pack.c passages.c potions.c \
	rings.c rip.c rooms.c save.c scrolls.c sticks.c things.c \
	weapons.c wizard.c mach_dep.c
MISC_C=	findpw.c score.c smisc.c
MISC=	Makefile $(MISC_C)

DEFS=		-DMASTER -DDUMP -DALLSCORES -DUSE_OLD_TTY
CFLAGS=		-g $(DEFS)
#CFLAGS=		-Bcpp/ -tp -O $(DEFS)
PROFLAGS=	-pg -O
#LDFLAGS=	-i	# For PDP-11's
LDFLAGS=	-g	# For VAXen
CRLIB=		-lncurses
#CRLIB=		-lcurses
#CRLIB=		libcurses.a

#SCOREFILE=	/usr/public/n_rogue_roll
SCOREFILE=	/usr/games/rogue.scores
#SCOREFILE=	./net_hist
SF=		-DSCOREFILE='"$(SCOREFILE)"'
NAMELIST=	/vmunix
NL=		-DNAMELIST='"$(NAMELIST)"'
#MACHDEP=	-DMAXLOAD=40 -DLOADAV -DCHECKTIME=4

LD=	ld
VGRIND=	/usr/ucb/vgrind
# for sites without sccs front end, GET= get
GET=	sccs get

# Use ansi flag to gcc
#CC = gcc -ansi
CC = cc

.DEFAULT:
	$(GET) $@

a.out: $(HDRS) $(OBJS)
	-rm -f a.out
	@rm -f x.c
	-$(CC) $(LDFLAGS) $(OBJS) $(CRLIB)
#	-$(CC) $(LDFLAGS) $(OBJS) $(CRLIB) -ltermlib
#	-$(CC) $(LDFLAGS) $(OBJS) $(CRLIB) -lcrypt
	size a.out
#	@echo -n 'You still have to remove '		# 11/70's
#	@size a.out | sed 's/+.*/ 1024 63 * - p/' | dc	# 11/70's

vers.o:
	$(CC) -c $(CFLAGS) vers.c

mach_dep.o: mach_dep.c
	$(CC) -c $(CFLAGS) $(SF) $(NL) $(MACHDEP) mach_dep.c

rip.o: rip.c
	$(CC) -c $(CFLAGS) $(SF) $(NL) $(MACHDEP) rip.c

rogue: newvers a.out
	cp a.out rogue 
	strip rogue

findpw: findpw.c
	$(CC) -s -o findpw findpw.c

score: score.o smisc.o vers.o
	$(CC) -s -o score vers.o score.o smisc.o -lcurses

smisc.o score.o:
	$(CC) -O -c $(SF) $*.c

newvers:
	$(GET) -e vers.c
	sccs delta -y vers.c

flist: $(HDRS) $(CFILES) $(MISC_C)
	-mv flist tags
	ctags -u $?
	ed - tags < :rofix
	sort tags -o flist
	rm -f tags

lint:
	/bin/csh -c "lint -hxbc $(DEFS) $(MACHDEP) $(SF) $(NL) $(CFILES) -lcurses > linterrs"

clean:
	rm -f $(OBJS) core a.out p.out rogue strings ? rogue.tar vgrind.* x.c x.o xs.c linterrs findpw

xtar: $(HDRS) $(CFILES) $(MISC)
	rm -f rogue.tar
	tar cf rogue.tar $? :rofix
	touch xtar

vgrind:
	@csh $(VGRIND) -t -h "Rogue Version 3.7" $(HDRS) *.c > vgrind.out
	@ctags -v $(HDRS) *.c > index
	@csh $(VGRIND) -t -x index > vgrind.out.tbl

wc:
	@echo "  bytes  words  lines  pages file"
	@wc -cwlp $(HDRS) $(CFILES)

cfiles: $(CFILES)
