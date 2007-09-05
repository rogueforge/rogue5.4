/*
 * print out an encrypted password on the standard output
 *
 * @(#)findpw.c	1.4 (Berkeley) 02/05/99
 */
#include <stdio.h>

main(int argc, char *argv[])
{
    static char buf[80];

    fprintf(stderr, "Password: ");
    (void) fgets(buf, 80, stdin);
    buf[strlen(buf) - 1] = '\0';
    printf("%s\n", xcrypt(buf, "mT"));
}
