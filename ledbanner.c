#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <time.h>
#include <math.h>
#include <stdint.h>

#define WIDTH 80
#define HEIGHT 8
//#define TAIL 21
#define TAIL 50
#define PIXELS (WIDTH * HEIGHT)
#define BYTES (PIXELS * 3)

// raw RGB values for a theoretical pure white pixel
#define WHITE_R 255
#define WHITE_G 210
#define WHITE_B 200

static uint8_t rgb_table[256][3];

static char out[BYTES + TAIL];

// see http://en.wikipedia.org/wiki/SRGB
static double rgbs(double x)
{
    if (x <= 0.04045) {
        return x / 12.92;
    } else {
        return pow((x + 0.055) / (1 + 0.055), 2.4);
    }
}

static void create_table(uint8_t table[][3], uint8_t white[3])
{
    int i, j;

    for (i = 0; i < 256; i++) {
        double t = i / 255.0;
        for (j = 0; j < 3; j++) {
            table[i][j] = round(rgbs(t) * white[j]);
        }
    }    
}


int main (void) {
    char in[BYTES];
    int i, c;
    uint8_t r,g,b;

    uint8_t white[] = {WHITE_R, WHITE_G, WHITE_B};
    create_table(rgb_table, white);

    int spi = open("/dev/spidev0.0", O_WRONLY);

//    int hz = 8000000;
    int hz = 4000000;
    ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &hz);

    write(spi, "\0\0\0\0\0\0\0\0\0\0", 10);

    while ((c = read(0, in, BYTES))) {
        for (i = 0; i < (c/3); i++) {
            int xx = (i % WIDTH);
            int yy = (i - xx) / WIDTH;
            int j = (yy % 2) ? (yy * WIDTH + WIDTH - xx - 1) : i;

            r = in[i * 3 + 0];
            g = in[i * 3 + 1];
            b = in[i * 3 + 2];
            
            out[j * 3 + 1] = (rgb_table[r][0] >> 1) | 0x80;
            out[j * 3 + 0] = (rgb_table[g][1] >> 1) | 0x80;
            out[j * 3 + 2] = (rgb_table[b][2] >> 1) | 0x80;
        }
	    write(spi, out, sizeof(out));
    }
    close(spi);
    return 0;
}
