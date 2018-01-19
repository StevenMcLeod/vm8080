        DEFC    buf = 0x8000
        DEFC    buf2 = 0x8100

        EXTERN  strcpy
        EXTERN  strcat
        EXTERN  strcmp
        EXTERN  gets
        EXTERN  puts

main:
        ; static char buf[0x100], buf2[0x100];
        ; strcpy(buf, "Hello, ");
        ld      hl, str0
        push    hl
        ld      hl, buf
        push    hl
        call    strcpy
        pop     af
        pop     af

        ; gets(buf2);
        ld      hl, buf2
        push    hl
        call    gets
        pop     af

        ; if(strcmp(buf2, "Steven") == 0)
        ld      hl, str1
        push    hl
        ld      hl, buf2
        push    hl
        call    strcmp
        pop     af
        pop     af

        ld      a, l
        or      a, h
        jp      nz, main1

        ; strcat(buf, buf2);
        ld      hl, buf2
        push    hl
        ld      hl, buf
        push    hl
        call    strcat
        pop     af
        pop     af

        ; puts(buf);
        push    hl
        call    puts
        pop     af
        jp      main2

main1:
        ; } else {
        ; puts("Invalid Name")
        ld      hl, str2
        push    hl
        call    puts
        pop     af

        ; }
main2:
        halt

str0:
        DEFM "Hello, ", 0

str1:
        DEFM "Steven", 0

str2:
        DEFM "Invalid Name", 0

        ;INCLUDE "string/strcpy.asm"
        ;INCLUDE "string/strcat.asm"
        ;INCLUDE "string/strcmp.asm"
        ;INCLUDE "stdio/gets.asm"
        ;INCLUDE "stdio/puts.asm"