#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#define IOCTL_PRINT 1
#define LED_MAX 192

void set_pixel(int x, int y, int red, char green, char blue, char* frame_buffer){
	int r_addr, g_addr, b_addr;
	//max, min값을 찾기보다 필터 적용
	x = x & 0x7;
	y = y & 0x7;
	r_addr = (y * 24) + x;
	g_addr = r_addr + 8;
	b_addr = g_addr + 8;
	//밝기 최대 63
	frame_buffer[r_addr] = (red & 63);
	frame_buffer[g_addr] = (green & 63);
	frame_buffer[b_addr] = (blue & 63);
	//printf("frame_buffer: %s\n",frame_buffer);
}

void sp_default(char* frame_buffer)
{
	int i,j;
	for (i=0; i<8; i++){
		for (j=0; j<8; j++){
			set_pixel(i,j,0,0,0, frame_buffer);
		}
	}


}

//setPixel
void sp_1(int y, int color, char* frame_buffer){
	int i;
	//y=6, digit = 2
	//y=1, digit = 1
	if (y>7) y=5;
	if (y<0) y=0;

	color = color & 63;
	for (i=1;i<7;i++){
		set_pixel(i,y,color,0,0,frame_buffer);
	}

}

void sp_2(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	if (y>7) y=5;
	if (y<0) y=0;
	color = color & 63;

	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y+1,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(2,y,color,0,0,frame_buffer);
	set_pixel(4,y+2,color,0,0,frame_buffer);

}

void sp_3(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;
	color = color & 63;


	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y+1,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(2,y,color,0,0,frame_buffer);
	set_pixel(4,y,color,0,0,frame_buffer);
}

void sp_4(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;
	int x;
	x=1;
	color = color & 63;

	set_pixel(x,y,color,0,0,frame_buffer);
	set_pixel(x+1,y+1,color,0,0,frame_buffer);
	set_pixel(x+2,y+2,color,0,0,frame_buffer);
	set_pixel(x+3,y+2,color,0,0,frame_buffer);


	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(x+1,y,color,0,0,frame_buffer);
	set_pixel(x+2,y,color,0,0,frame_buffer);
	set_pixel(x+3,y,color,0,0,frame_buffer);
	set_pixel(x+4,y,color,0,0,frame_buffer);
	set_pixel(x+5,y,color,0,0,frame_buffer);
	set_pixel(x+6,y,color,0,0,frame_buffer);

}


void sp_5(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;

	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y+1,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(2,y+2,color,0,0,frame_buffer);
	set_pixel(4,y,color,0,0,frame_buffer);
}

void sp_6(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;

	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y+1,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(4,y,color,0,0,frame_buffer);
	set_pixel(4,y+2,color,0,0,frame_buffer);
	set_pixel(2,y+2,color,0,0,frame_buffer);


}

void sp_7(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	int i;
	for (i=1;i<7;i++){
		set_pixel(i,y,color,0,0,frame_buffer);
	}
	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

}

void sp_8(int y, int color, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y+1,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(2,y,color,0,0,frame_buffer);
	set_pixel(4,y+2,color,0,0,frame_buffer);
	set_pixel(2,y+2,color,0,0,frame_buffer);
	set_pixel(4,y,color,0,0,frame_buffer);

}

void sp_9(int y, int color, char* frame_buffer){
	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y+1,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(2,y,color,0,0,frame_buffer);
	set_pixel(2,y+2,color,0,0,frame_buffer);
	set_pixel(4,y,color,0,0,frame_buffer);
}

void sp_0(int y, int color, char* frame_buffer){
	set_pixel(1,y+2,color,0,0,frame_buffer);
	set_pixel(1,y+1,color,0,0,frame_buffer);
	set_pixel(1,y,color,0,0,frame_buffer);

	set_pixel(3,y+2,color,0,0,frame_buffer);
	set_pixel(3,y,color,0,0,frame_buffer);

	set_pixel(5,y+2,color,0,0,frame_buffer);
	set_pixel(5,y+1,color,0,0,frame_buffer);
	set_pixel(5,y,color,0,0,frame_buffer);

	set_pixel(2,y,color,0,0,frame_buffer);
	set_pixel(2,y+2,color,0,0,frame_buffer);
	set_pixel(4,y,color,0,0,frame_buffer);
	set_pixel(4,y+2,color,0,0,frame_buffer);

}


int main(void) {
	int fd;
	char buf[100];
	int number;
	char image[LED_MAX] = "";
	int read_ret, write_ret;
	while (1) {

		fd = open("/dev/rs-tmpre1", O_RDWR);
		if (fd < 0) {
			printf("failed opening device: %s\n", strerror(errno));
			return 0;
		}
		sp_default(image);
		//sp_1(1, 10, image);
		//sp_2(1, 10, image);
		//sp_3(1,10, image);
		//sp_4(5,10, image);
		//sp_5(1,10, image);
		//sp_5(5,10, image);
		//sp_6(1,10, image);
		//sp_7(5,10, image);
		//sp_8(5, 10, image);
		//sp_9(1, 10, image);
		//sp_0(1, 10, image);
		printf("%s\n",image);
		read_ret = read(fd, buf, 4);
		printf("fd = %d, ret write = %d, ret read = %d\n", fd, write_ret, read_ret);
		printf("content = %s\n", buf);
		number = (int)buf[0]- '0';
		printf("number=%d\n", number);
		switch (number%10) {
			case 0:
				sp_0(5, 10, image);
				break;
			case 1:
				sp_1(5, 10, image);
				break;
			case 2:
				sp_2(5, 10, image);
				break;
			case 3:
				sp_3(5, 10, image);
				break;

			case 4:
				sp_4(5, 10, image);
				break;
			case 5:
				sp_5(5, 10, image);
				break;
			case 6:
				sp_6(5, 10, image);
				break;
			case 7:
				sp_7(5, 10, image);
				break;
			case 8:
				sp_8(5, 10, image);
				break;
			case 9:
				sp_9(5, 10, image);
				break;
		}
		number = (int)buf[1]- '0';
		printf("number=%d\n", number);
		switch (number%10) {
			case 0:
				sp_0(1, 10, image);
				break;
			case 1:
				sp_1(1, 10, image);
				break;
			case 2:
				sp_2(1, 10, image);
				break;
			case 3:
				sp_3(1, 10, image);
				break;

			case 4:
				sp_4(1, 10, image);
				break;
			case 5:
				sp_5(1, 10, image);
				break;
			case 6:
				sp_6(1, 10, image);
				break;
			case 7:
				sp_7(1, 10, image);
				break;
			case 8:
				sp_8(1, 10, image);
				break;
			case 9:
				sp_9(1, 10, image);
				break;
		}

		write_ret = write(fd, image, LED_MAX);


		//ioctl(fd, IOCTL_PRINT, NULL);
		close(fd);

		printf("in a loop\n");
		sleep(10);
	}
}

