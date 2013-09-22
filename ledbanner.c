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

// maximum *average* brightness of input
#define MAX_BRIGHT 0xff
#define MAX_BRIGHT_TOTAL (BYTES * MAX_BRIGHT)


//#define brightness 0.5

static const char cielab[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6,
6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12,
12, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20,
21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, 29, 30, 31, 31,
32, 33, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 43, 44, 44, 45, 46,
47, 48, 49, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
66, 67, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78, 79, 80, 82, 83, 84, 85, 87, 88,
89, 90, 92, 93, 94, 96, 97, 99, 100, 101, 103, 104, 106, 107, 108, 110, 111,
113, 114, 116, 118, 119, 121, 122, 124, 125, 127, 129, 130, 132, 134, 135, 137,
139, 141, 142, 144, 146, 148, 149, 151, 153, 155, 157, 159, 161, 162, 164, 166,
168, 170, 172, 174, 176, 178, 180, 182, 185, 187, 189, 191, 193, 195, 197, 200,
202, 204, 206, 208, 211, 213, 215, 218, 220, 222, 225, 227, 230, 232, 234, 237,
239, 242, 244, 247, 249, 252, 255 };

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

    long sum;

    uint8_t white[] = {WHITE_R, WHITE_G, WHITE_B};
    create_table(rgb_table, white);

    int spi = open("/dev/spidev0.0", O_WRONLY);

//    int hz = 8000000;
    int hz = 4000000;
    ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &hz);

    write(spi, "\0\0\0\0\0\0\0\0\0\0", 10);

    while ((c = read(0, in, BYTES))) {
        sum = 0;
        for (i = 0; i < BYTES; i++)
            sum += in[i];

        if (sum > MAX_BRIGHT_TOTAL)
            for (i = 0; i < BYTES; i++)
                in[i] = in[i] / MAX_BRIGHT * (sum / BYTES);

        for (i = 0; i < (c/3); i++) {
            int xx = (i % WIDTH);
            int yy = (i - xx) / WIDTH;
            int j = (yy % 2) ? (yy * WIDTH + WIDTH - xx - 1) : i;
#ifdef brightness
	    in[i * 3 + 0] *= brightness;
	    in[i * 3 + 1] *= brightness;
	    in[i * 3 + 2] *= brightness;
#endif
#if 0
            in[i * 3 + 1] *= .9;
            in[i * 3 + 1] *= .95;
#endif
            r = in[i * 3 + 0];
            g = in[i * 3 + 1];
            b = in[i * 3 + 2];
#if 0
            out[j * 3 + 1] = (cielab[in[i * 3 + 0]] >> 1) | 0x80;
            out[j * 3 + 0] = (cielab[in[i * 3 + 1]] >> 1) | 0x80;
            out[j * 3 + 2] = (cielab[in[i * 3 + 2]] >> 1) | 0x80;
#else
            out[j * 3 + 1] = (rgb_table[r][0] >> 1) | 0x80;
            out[j * 3 + 0] = (rgb_table[g][1] >> 1) | 0x80;
            out[j * 3 + 2] = (rgb_table[b][2] >> 1) | 0x80;
#endif
        }
	    write(spi, out, sizeof(out));
    }
    close(spi);
    return 0;
}
