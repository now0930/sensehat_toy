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

int   main(void) {
  int fd;
  char buf[1000];
  char image[LED_MAX] = "";
  int read_ret, write_ret;
  int i,j;
  for (i=0; i<8; i++){
	  for (j=0; j<8; j++){
		  set_pixel(i,j,5,2,0, image);
	  }
  }

  fd = open("/dev/rs-tmpre1", O_RDWR);
  if (fd < 0) {
    printf("failed opening device: %s\n", strerror(errno));
    return 0;
  }

  printf("%s\n",image);
  write_ret = write(fd, image, LED_MAX);
  read_ret = read(fd, buf, 4);
  printf("fd = %d, ret write = %d, ret read = %d\n", fd, write_ret, read_ret);
  printf("content = %s\n", buf);

  //ioctl(fd, IOCTL_PRINT, NULL);
  close(fd);
}

