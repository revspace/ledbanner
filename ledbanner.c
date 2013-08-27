#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <time.h>

#define width 80
#define height 8
//#define tail 21
#define tail 50
#define pixels (width * height)
#define bytes (pixels * 3)

// maximum *average* brightness of input
#define max_bright 0xff
#define max_bright_total (bytes * max_bright)


//#define brightness 0.5

char cielab[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
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

char out[bytes + tail];

int pixelchase (spi) {
    int i = 0;
    for (i; i < pixels; i++) {
        int pos = 0;
        for (; pos < i; pos++) {
            out[pos * 3 + 0] = 0x80;
            out[pos * 3 + 1] = 0x80;
            out[pos * 3 + 2] = 0x80;
        }
        out[pos * 3 + 0] = 0xff;
        out[pos * 3 + 1] = 0xff;
        out[pos * 3 + 2] = 0xff;
        for (pos++; pos < pixels; pos++) {
            out[pos * 3 + 0] = 0x80;
            out[pos * 3 + 1] = 0x80;
            out[pos * 3 + 2] = 0x80;
        }
        write(spi, out, sizeof(out));
    }
}

int main (void) {
    char r[pixels], g[pixels], b[pixels];
    char in[bytes];
    char triplet[3];
    int i, k, c;

    long sum;

    int spi = open("/dev/spidev0.0", O_WRONLY);

//    int hz = 8000000;
    int hz = 4000000;
    ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &hz);

    // pixelchase(spi); return 0;
    
    write(spi, "\0\0\0\0\0\0\0\0\0\0", 10);

    while (c = read(0, in, bytes)) {
        sum = 0;
        for (i = 0; i < bytes; i++)
            sum += in[i];

        if (sum > max_bright_total)
            for (i = 0; i < bytes; i++)
                in[i] = in[i] / max_bright * (sum / bytes);

        for (i = 0; i < (c/3); i++) {
            int xx = (i % width);
            int yy = (i - xx) / width;
            int j = yy % 2 ? (yy * width + width - xx - 1) : i;
#ifdef brightness
	    in[i * 3 + 0] *= brightness;
	    in[i * 3 + 1] *= brightness;
	    in[i * 3 + 2] *= brightness;
#endif
            in[i * 3 + 1] *= .9;
            in[i * 3 + 1] *= .95;

            out[j * 3 + 1] = (cielab[in[i * 3 + 0]] >> 1) | 0x80;
            out[j * 3 + 0] = (cielab[in[i * 3 + 1]] >> 1) | 0x80;
            out[j * 3 + 2] = (cielab[in[i * 3 + 2]] >> 1) | 0x80;
        }
	write(spi, out, sizeof(out));
    }
    return 0;
}
