###############################################################################
#
# Makefile for rogue
#
# Rogue: Exploring the Dungeons of Doom
# Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
# All rights reserved.
#       
# See the file LICENSE.TXT for full copyright and licensing information.
#
###############################################################################
        
###############################################################################
# Site configuration occurs beneath this comment
# Typically ./configure (autoconf tools) configures this section
# This section could be manually configured if autoconf/configure fails
###############################################################################

DISTNAME=rogue5.4.2

PROGRAM=@PROGRAM@

O=o

#CC=gcc
CC    = @CC@

#CFLAGS=-O2
CFLAGS= @CFLAGS@ 

#LIBS=-lcurses
LIBS =	@LIBS@

#RM=rm -f
RM    = rm -f

#GROFF=groff
GROFF = @GROFF@

#NROFF=nroff
NROFF = @NROFF@

#TBL=tbl
TBL   = @TBL@

#COLCRT=colcrt
COLCRT = @COLCRT@

#SED=sed
SED   = @SED@

#SCOREFILE=rogue54.scr
SCOREFILE = @SCOREFILE@

#LOCKFILE=rogue54.lck
LOCKFILE = @LOCKFILE@

#GROUPOWNER=games
GROUPOWNER = games

#CPPFLAGS=-DHAVE_CONFIG_H
CPPFLAGS =@DEFS@ @CPPFLAGS@

INSTALL=./install-sh

CHGRP=chgrp

MKDIR=mkdir

TOUCH=touch

RMDIR=rmdir

DESTDIR=

prefix=@prefix@
exec_prefix=@exec_prefix@
datarootdir=@datarootdir@
datadir=@datadir@
bindir=@bindir@
mandir=@mandir@
docdir=@docdir@
man6dir = $(mandir)/man6

###############################################################################
# Site configuration occurs above this comment
# It should not be necessary to change anything below this comment
############################################################################### 

HDRS=	rogue.h extern.h score.h

OBJS1 =	vers.$(O) extern.$(O) armor.$(O) chase.$(O) command.$(O) daemon.$(O) \
	daemons.$(O) fight.$(O) init.$(O) io.$(O) list.$(O) mach_dep.$(O) \
	main.$(O) mdport.$(O) misc.$(O) monsters.$(O) move.$(O) new_level.$(O)
OBJS2 = options.$(O) pack.$(O) passages.$(O) potions.$(O) rings.$(O) rip.$(O) \
        rooms.$(O) save.$(O) scrolls.$(O) state.$(O) sticks.$(O) things.$(O) \
        weapons.$(O) wizard.$(O) xcrypt.$(O)
OBJS  =	$(OBJS1) $(OBJS2)

CFILES=	vers.c extern.c armor.c chase.c command.c daemon.c \
	daemons.c fight.c init.c io.c list.c mach_dep.c \
	main.c  mdport.c misc.c monsters.c move.c new_level.c \
	options.c pack.c passages.c potions.c rings.c rip.c \
	rooms.c save.c scrolls.c state.c sticks.c things.c \
	weapons.c wizard.c xcrypt.c
MISC_C=	findpw.c scedit.c scmisc.c
DOCSRC= rogue.me rogue.6
DOCS  = $(PROGRAM).doc $(PROGRAM).html $(PROGRAM).cat readme54.html
MISC  =	Makefile $(MISC_C) LICENSE.TXT $(PROGRAM).sln $(PROGRAM).vcproj $(DOCS)\
        $(DOCSRC)

.SUFFIXES: .obj

.c.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) /c $*.c
    
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $*.c
    
$(PROGRAM): $(HDRS) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
    
clean:
	$(RM) $(OBJS1)
	$(RM) $(OBJS2)
	$(RM) core a.exe a.out a.exe.stackdump $(PROGRAM) $(PROGRAM).exe
	$(RM) $(PROGRAM).tar $(PROGRAM).tar.gz $(PROGRAM).zip 
    
maintainer-clean:
	$(RM) config.h
	$(RM) Makefile
	$(RM) config.status
	$(RM) -r autom4te.cache
	$(RM) config.log
	$(RM) $(PROGRAM).scr $(PROGRAM).lck

dist.src:
	make clean
	tar cf $(DISTNAME)-src.tar $(CFILES) $(HDRS) $(MISC)
	gzip -f $(DISTNAME)-src.tar

findpw: findpw.c xcrypt.o mdport.o xcrypt.o
	$(CC) -s -o findpw findpw.c xcrypt.o mdport.o -lcurses

scedit: scedit.o scmisc.o vers.o mdport.o xcrypt.o
	$(CC) -s -o scedit vers.o scedit.o scmisc.o mdport.o xcrypt.o -lcurses

scmisc.o scedit.o:
	$(CC) -O -c $(SF) $*.c

$(PROGRAM).doc: rogue.me
	if test "x$(GROFF)" != "x" -a "x$(SED)" != "x" ; then \
	$(GROFF) -P-c -t -me -Tascii rogue.me | $(SED) -e 's/.\x08//g' > $(PROGRAM).doc ;\
	elif test "x$(NROFF)" != "x" -a "x$(TBL)" != "x" -a "x$(COLCRT)" != "x" ; then \
        tbl rogue.me | $(NROFF) -me | colcrt - > $(PROGRAM).doc ;\
	fi

$(PROGRAM).cat: rogue.6
	if test "x$(GROFF)" != "x" -a "x$(SED)" != "x" ; then \
	$(GROFF) -Tascii -man rogue.6 | $(SED) -e 's/.\x08//g' > $(PROGRAM).cat ;\
	elif test "x$(NROFF)" != "x" -a "x$(TBL)" != "x" -a "x$(COLCRT)" != "x" ; then \
	$(NROFF) -man rogue.6 | $(COLCRT) - > $(PROGRAM).cat ;\
	fi

dist: clean $(PROGRAM)
	tar cf $(DISTNAME)-`uname`.tar $(PROGRAM) LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-`uname`.tar

install: $(PROGRAM)
	-$(TOUCH) test
	-$(INSTALL) -g $(GROUPOWNER) -m 02755 $(PROGRAM) $(DESTDIR)$(bindir)/$(PROGRAM)
	-$(INSTALL) -m 0644 rogue.6 $(DESTDIR)$(man6dir)/$(PROGRAM).6
	-$(INSTALL) -m 0644 rogue.doc $(DESTDIR)$(docdir)$(PROGRAM)/$(PROGRAM).doc
	-if test ! -f $(SCOREFILE) ; then $(INSTALL) -g $(GROUPOWNER) -m 0664 test $(SCOREFILE) ; fi
	-if test ! -f $(LOCKFILE) ; then $(INSTALL) -m 0666 test $(LOCKFILE) ; $(RM) $(LOCKFILE) ; fi
	-$(RM) test

uninstall:	
	-$(RM) $(DESTDIR)$(bindir)/$(PROGRAM)
	-$(RM) $(DESTDIR)$(man6dir)/$(PROGRAM).6
	-$(RM) $(DESTDIR)$(docdir)$(PROGRAM)/$(PROGRAM).doc
	-$(RM) $(LOCKFILE)
	-$(RMDIR) $(DESTDIR)$(docdir)$(PROGRAM)
    
reinstall: uninstall install
