# sensehat_toy


raspberry pi4에 sensehat을 붙여 테스트하기.<br>
간단한 온도, 습도 측정하여<br>
LED matrix에 표시<br>


자동화 하기 위해서는 UDEV, systemctl을 사용해야 함.<br>


1. module 설치
pi@raspberrypi:~/rasp/sensehat2 $ find /usr/lib/modules/$(uname -r)/kernel -type f -iname "sense*"
/usr/lib/modules/5.10.92-v7l+/kernel/drivers/i2c/sensehat.ko

2. depmod 실행..
pi@raspberrypi:~/rasp/sensehat2 $ sudo depmod 

3. module auto loading 설정.
pi@raspberrypi:/etc/modules-load.d $ pwd
/etc/modules-load.d
pi@raspberrypi:/etc/modules-load.d $ cat modules.conf 
# /etc/modules: kernel modules to load at boot time.
#
# This file contains the names of kernel modules that should be loaded
# at boot time, one per line. Lines beginning with "#" are ignored.

i2c-dev
sensehat.ko

4. blacklist 설정
pi@raspberrypi:/etc/modprobe.d $ pwd
/etc/modprobe.d
pi@raspberrypi:/etc/modprobe.d $ cat blacklist-sense.conf
blacklist rpisense_core
blacklist rpisense_fb
blacklist rpisense_js

Raspberry pi4에 기본으로 설정되어 있는 모듈
rpisense_fb가 LED Matrix를 사용하고 있어 먼저 내려야 됨

5. /bin/에 sensehat_display 파일 복사.

6. systemctl 설정 가능하도록 정리.
https://baykara.medium.com/how-to-daemonize-a-process-or-service-with-systemd-c34501e646c9
pi@raspberrypi:/etc/systemd/system $ cat sensehat_display.service 
[Unit]
Description=SenseHat Temperatur & Humidity Monitor

[Service]
Type=simple
ExecStart=/usr/bin/sensehat_display

[Install]

7. udev rules 설정.
pi@raspberrypi:~/rasp/sensehat2 $ cat /etc/udev/rules.d/100-sensehat.rules
ACTION=="add", SUBSYSTEM=="rpi-sense-tmpre", KERNEL=="rs-tmpre2", RUN+="/bin/systemctl start sensehat_display'"

관련 정보는 다음에서 확인 가능.
pi@raspberrypi:~ $ udevadm info /dev/rs-tmpre1
P: /devices/virtual/rpi-sensehat/rs-tmpre1
N: rs-tmpre1
L: 0
E: DEVPATH=/devices/virtual/rpi-sensehat/rs-tmpre1
E: DEVNAME=/dev/rs-tmpre1
E: DEVMODE=0666
E: MAJOR=235
E: MINOR=0
E: SUBSYSTEM=rpi-sensehat

pi@raspberrypi:~ $ udevadm info /dev/rs-tmpre2
P: /devices/virtual/rpi-sensehat/rs-tmpre2
N: rs-tmpre2
L: 0
E: DEVPATH=/devices/virtual/rpi-sensehat/rs-tmpre2
E: DEVNAME=/dev/rs-tmpre2
E: DEVMODE=0666
E: MAJOR=235
E: MINOR=1
E: SUBSYSTEM=rpi-sensehat

pi@raspberrypi:/dev $ udevadm info -a -n /dev/rs-tmpre1

Udevadm info starts with the device specified by the devpath and then
walks up the chain of parent devices. It prints for every device
found, all possible attributes in the udev rules key format.
A rule to match, can be composed by the attributes of the device
and the attributes from one single parent device.

  looking at device '/devices/virtual/rpi-sensehat/rs-tmpre1':
    KERNEL=="rs-tmpre1"
    SUBSYSTEM=="rpi-sensehat"
    DRIVER==""
    ATTR{power/control}=="auto"
    ATTR{power/runtime_active_time}=="0"
    ATTR{power/runtime_status}=="unsupported"
    ATTR{power/runtime_suspended_time}=="0"

