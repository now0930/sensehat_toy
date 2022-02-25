#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define IOCTL_PRINT 1
#define LED_MAX 192
float get_temper(char* buf);
float get_humidity(char* buf);

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
	int ret;
	/*
	int i,j;
	for (i=0; i<8; i++){
		for (j=0; j<8; j++){
			set_pixel(i,j,0,0,0, frame_buffer);
		}
	}
	*/
	memset(frame_buffer, 0 , LED_MAX);
	//printf("%s\n", *frame_buffer);

}

//setPixel
void sp_1(int y, int red, int green, int blue, char* frame_buffer){
	int i;
	//y=6, digit = 2
	//y=1, digit = 1
	if (y>7) y=5;
	if (y<0) y=0;

	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	for (i=1;i<6;i++){
		set_pixel(i,y,red, green, blue, frame_buffer);
	}

}

void sp_2(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	if (y>7) y=5;
	if (y<0) y=0;
	red = red & 63;
	green = green & 63;
	blue = blue & 63;


	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y+1,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(2,y,red, green, blue, frame_buffer);
	set_pixel(4,y+2,red, green, blue, frame_buffer);

}

void sp_3(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;
	red = red & 63;
	green = green & 63;
	blue = blue & 63;


	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y+1,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(2,y,red, green, blue, frame_buffer);
	set_pixel(4,y,red, green, blue, frame_buffer);
}

void sp_4(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;
	int x;
	x=1;
	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	set_pixel(x,y,red, green, blue, frame_buffer);
	set_pixel(x+1,y+1,red, green, blue, frame_buffer);
	set_pixel(x+2,y+2,red, green, blue, frame_buffer);
	set_pixel(x+3,y+2,red, green, blue, frame_buffer);


	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(x+1,y,red, green, blue, frame_buffer);
	set_pixel(x+2,y,red, green, blue, frame_buffer);
	set_pixel(x+3,y,red, green, blue, frame_buffer);
	set_pixel(x+4,y,red, green, blue, frame_buffer);
	set_pixel(x+5,y,red, green, blue, frame_buffer);
	set_pixel(x+6,y,red, green, blue, frame_buffer);

}


void sp_5(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1

	if (y>7) y=5;
	if (y<0) y=0;

	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y+1,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(2,y+2,red, green, blue, frame_buffer);
	set_pixel(4,y,red, green, blue, frame_buffer);
}

void sp_6(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	if (y>7) y=5;
	if (y<0) y=0;

	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y+1,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(4,y,red, green, blue, frame_buffer);
	set_pixel(4,y+2,red, green, blue, frame_buffer);
	set_pixel(2,y+2,red, green, blue, frame_buffer);


}

void sp_7(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	int i;
	for (i=1;i<6;i++){
		set_pixel(i,y,red, green, blue, frame_buffer);
	}
	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

}

void sp_8(int y, int red, int green, int blue, char* frame_buffer){
	//y=5, digit = 2
	//y=1, digit = 1
	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y+1,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(2,y,red, green, blue, frame_buffer);
	set_pixel(4,y+2,red, green, blue, frame_buffer);
	set_pixel(2,y+2,red, green, blue, frame_buffer);
	set_pixel(4,y,red, green, blue, frame_buffer);

}

void sp_9(int y, int red, int green, int blue, char* frame_buffer){
	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y+1,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(2,y,red, green, blue, frame_buffer);
	set_pixel(2,y+2,red, green, blue, frame_buffer);
	set_pixel(4,y,red, green, blue, frame_buffer);
}

void sp_0(int y, int red, int green, int blue, char* frame_buffer){
	red = red & 63;
	green = green & 63;
	blue = blue & 63;

	set_pixel(1,y+2,red, green, blue, frame_buffer);
	set_pixel(1,y+1,red, green, blue, frame_buffer);
	set_pixel(1,y,red, green, blue, frame_buffer);

	set_pixel(3,y+2,red, green, blue, frame_buffer);
	set_pixel(3,y,red, green, blue, frame_buffer);

	set_pixel(5,y+2,red, green, blue, frame_buffer);
	set_pixel(5,y+1,red, green, blue, frame_buffer);
	set_pixel(5,y,red, green, blue, frame_buffer);

	set_pixel(2,y,red, green, blue, frame_buffer);
	set_pixel(2,y+2,red, green, blue, frame_buffer);
	set_pixel(4,y,red, green, blue, frame_buffer);
	set_pixel(4,y+2,red, green, blue, frame_buffer);

}

float get_temper(char* buf){
	float temp = 0.0;
	int T0_degC_x8;
	int T1_degC_x8;
	int T0_OUT;
	int T1_OUT;
	int T_OUT;
	int T1T0MSB;
	int temp_int[7];
	int temperature, i;
	i=0;
	char *token;
	//https://blockdmask.tistory.com/382
	token = strtok(buf, ",");
	while (token != NULL){
		temp_int[i] = atoi(token);
		//printf("%s, %d, %d\n", token, temp_int[i], i);
		token = strtok(NULL, ",");
		i++;
	}
	T0_degC_x8 = temp_int[1];
	T1_degC_x8 = temp_int[2];
	T0_OUT = temp_int[3];
	T1_OUT = temp_int[4];
	T_OUT = temp_int[5];
	T1T0MSB = temp_int[6];

	temp = T0_degC_x8 + ((T_OUT - T0_OUT) * (T1_degC_x8 - T0_degC_x8))/(T1_OUT - T0_OUT);
	//printf("temp: %0.1f\n", temp);
	return temp;
}

float get_humidity(char* buf){
	float humidity = 0.0;
	int H0_rH_x2;
	int H1_rH_x2;
	int H0_T0_OUT;
	int H1_T0_OUT;
	int H_OUT;
	char *token;
	int temp_int[7];
	int i=0, dividor=1;
	token = strtok(buf, ",");
	while (token != NULL){
		temp_int[i] = atoi(token);
		//printf("%s, %d, %d\n", token, temp_int[i], i);
		token = strtok(NULL, ",");
		i++;
	}

	H0_rH_x2 = temp_int[0];
	H1_rH_x2 = temp_int[1];
	H0_T0_OUT = temp_int[2];
	H1_T0_OUT = temp_int[3];
	H_OUT = temp_int[4];
	
	dividor = (H1_T0_OUT - H0_T0_OUT);
	if (dividor == 0)
		dividor = 1;


	humidity = H0_rH_x2 + (H_OUT - H0_T0_OUT) * (H1_rH_x2 - H0_rH_x2)/\
		   dividor;
	//printf("check: %0.1f\n", humidity);
	return humidity;
}



int main(void) {
	int fd, fd2;
	int i;
	char buf[100], temp[4], buf2[100];
	int number;
	char image[LED_MAX] = {"0x0"}, image2[LED_MAX] = {"0x0"};
	int read_ret, write_ret;
	sleep(1);
	while (1) {

		fd = open("/dev/rs-tmpre1", O_RDWR);
		if (fd < 0) {
			printf("failed opening device: %s\n", strerror(errno));
			return 0;
		}
		sp_default(image);
		read_ret = read(fd, buf, 100);
		//printf("fd = %d, ret write = %d, ret read = %d\n", fd, write_ret, read_ret);
		//printf("temp content = %s\n", buf);
		sprintf(temp, "%0.1f", get_temper(buf));
		//printf("temp is: %s\n", temp);
		number = (int)temp[0]- '0';
		//printf("number=%d\n", number);
		switch (number%10) {
			case 0:
				sp_0(5, 10, 0, 0, image);
				break;
			case 1:
				sp_1(5, 10, 0, 0, image);
				break;
			case 2:
				sp_2(5, 10, 0, 0,image);
				break;
			case 3:
				sp_3(5, 10, 0, 0,image);
				break;
			case 4:
				sp_4(5, 10, 0, 0,image);
				break;
			case 5:
				sp_5(5, 10, 0, 0,image);
				break;
			case 6:
				sp_6(5, 10, 0, 0,image);
				break;
			case 7:
				sp_7(5, 10, 0, 0,image);
				break;
			case 8:
				sp_8(5, 10, 0, 0,image);
				break;
			case 9:
				sp_9(5, 10, 0, 0,image);
				break;
		}
		number = (int)temp[1]- '0';
		//printf("number=%d\n", number);
		switch (number%10) {
			case 0:
				sp_0(1, 10, 0, 0,image);
				break;
			case 1:
				sp_1(1, 10, 0, 0,image);
				break;
			case 2:
				sp_2(1, 10, 0, 0,image);
				break;
			case 3:
				sp_3(1, 10, 0, 0,image);
				break;
			case 4:
				sp_4(1, 10, 0, 0,image);
				break;
			case 5:
				sp_5(1, 10, 0, 0,image);
				break;
			case 6:
				sp_6(1, 10, 0, 0,image);
				break;
			case 7:
				sp_7(1, 10, 0, 0,image);
				break;
			case 8:
				sp_8(1, 10, 0, 0,image);
				break;
			case 9:
				sp_9(1, 10, 0, 0,image);
				break;
		}

		write_ret = write(fd, image, LED_MAX);

		//ioctl(fd, IOCTL_PRINT, NULL);
		close(fd);
		//printf("in a loop\n");
		sleep(5);
		fd2 = open("/dev/rs-tmpre2", O_RDWR);
		if (fd2 < 0) {
			printf("failed opening device: %s\n", strerror(errno));
			return 0;
		}
		sp_default(image2);
		read_ret = read(fd2, buf2, 100);
		//printf("humidity contents: %s\n", buf2);
		sprintf(temp, "%0.1f", get_humidity(buf2));
		//printf("humidity: %0.1f\n", temp);
		//printf("temp is: %s\n", temp);
		number = (int)temp[0]- '0';
		switch (number%10) {
			case 0:
				sp_0(5, 0, 10, 0, image2);
				break;
			case 1:
				sp_1(5, 0, 10, 0, image2);
				break;
			case 2:
				sp_2(5, 0, 10, 0, image2);
				break;
			case 3:
				sp_3(5, 0, 10, 0, image2);
				break;
			case 4:
				sp_4(5, 0, 10, 0, image2);
				break;
			case 5:
				sp_5(5, 0, 10, 0, image2);
				break;
			case 6:
				sp_6(5, 0, 10, 0, image2);
				break;
			case 7:
				sp_7(5, 0, 10, 0, image2);
				break;
			case 8:
				sp_8(5, 0, 10, 0, image2);
				break;
			case 9:
				sp_9(5, 0, 10, 0, image2);
				break;
		}
		number = (int)temp[1]- '0';
		//printf("number=%d\n", number);
		switch (number%10) {
			case 0:
				sp_0(1, 0, 10, 0, image2);
				break;
			case 1:
				sp_1(1, 0, 10, 0, image2);
				break;
			case 2:
				sp_2(1, 0, 10, 0, image2);
				break;
			case 3:
				sp_3(1, 0, 10, 0, image2);
				break;
			case 4:
				sp_4(1, 0, 10, 0, image2);
				break;
			case 5:
				sp_5(1, 0, 10, 0, image2);
				break;
			case 6:
				sp_6(1, 0, 10, 0, image2);
				break;
			case 7:
				sp_7(1, 0, 10, 0, image2);
				break;
			case 8:
				sp_8(1, 0, 10, 0, image2);
				break;
			case 9:
				sp_9(1, 0, 10, 0, image2);
				break;
		}


		write_ret = write(fd2, image2, LED_MAX);
		close(fd2);
		sleep(5);

	}
}

