CC = arm-none-linux-gnueabi-gcc
FILES = jpeg.c main.c yuv2rgb.c yuyv2yuv.c parse.c

all:$(FILES)
	$(CC) $^ -o ov5640 -O3 -L. -ljpeg

clean:
	rm -rf ov5640
