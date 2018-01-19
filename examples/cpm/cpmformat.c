#include <stdlib.h>
#include <stdio.h>

#define SECTOR_SIZE 128
#define SECTOR_COUNT 26
#define TRACK_SIZE (SECTOR_SIZE * SECTOR_COUNT)
#define TRACK_COUNT 77
#define DISK_SIZE (SECTOR_SIZE * SECTOR_COUNT* TRACK_COUNT)

int main(void) {
    FILE *disk = fopen("cpmdisk.img", "w");
    if(!disk) {
        fprintf(stderr, "Could not create file.");
        exit(EXIT_FAILURE);
    }

    for(long i = 0 ; i < 2 * TRACK_SIZE; ++i) {
        fputc('\0', disk);
    }

    for(long i = 0; i < (TRACK_COUNT - 2) * TRACK_SIZE; ++i) {
        fputc('\xe5', disk);
    }

    fclose(disk);
    return 0;
}

