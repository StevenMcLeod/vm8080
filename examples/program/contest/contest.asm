        ORG 0h
start:
        in      a, (1)
        or      a, a
        jp      nz, blk
        in      a, (0)
        ld      a, '!'      ; Not blocking
        out     (0), a
        jp      main

blk:
        ld      a, '@'      ; Was blocking
        out     (0), a

main:
        in      a, (1)
        or      a, a
        jp      z, main

        in      a, (0)
        cp      a, '\r'
        jp      nz, check1
        add     a, '\n' - '\r'

check1:
        cp      a, 7fh
        jp      nz, output
        add     a, '\b' - 7fh

output:
        out     (0), a
        jp      main
