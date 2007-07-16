bin_PROGRAMS=rogue54
rogue54_SOURCES=vers.c extern.c armor.c chase.c command.c daemon.c \
	daemons.c fight.c init.c io.c list.c mach_dep.c \
	main.c  mdport.c misc.c monsters.c move.c new_level.c \
	options.c pack.c passages.c potions.c rings.c rip.c \
	rooms.c save.c scrolls.c state.c sticks.c things.c \
	weapons.c wizard.c xcrypt.c

install-exec-hook:
	chgrp games $(DESTDIR)$(bindir)/rogue54
	chmod -f 2755 $(DESTDIR)$(bindir)/rogue54
	mkdir -p /var/games/rogue54
	chgrp games /var/games/rogue54
	chmod -f 775 /var/games/rogue54
