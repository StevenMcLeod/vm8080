        ORG     0x100

main:
        ld      c, 9
        ld      de, msg
        call    5
        ret

msg:
        defm    "Hello, World!", 0xa, 0xd, "$"
