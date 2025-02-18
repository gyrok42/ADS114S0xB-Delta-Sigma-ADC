# ADS114S0xB-Delta-Sigma-ADC
Exercise on TI-ADS114S0XB ADC linux device driver

`sudo modprobe industrialio`
`sudo modprobe industrialio-buffer-cb`
`sudo modprobe industrialio-triggered-buffer`
`sudo modprobe iio-trig-sysfs`


########################################
sudo modprobe industrialio
sudo modprobe industrialio-buffer-cb
sudo modprobe industrialio-triggered-buffer
sudo modprobe iio-trig-sysfs
sudo modprobe industrialio-buf-kfifo

modprobe iio_trig_sysfs
modprobe iio_dummy

modprobe iio_trig_sysfs
modprobe iio_dummy (make sure CONFIG_IIO_SIMPLE_DUMMY_BUFFER is #defined)
cd /sys/bus/iio/devices/iio_sysfs_trigger
echo 123 > add_trigger
cd /sys/bus/iio/devices/trigger0
cat name # should give sysfstrig123
cd /sys/bus/iio/devices/iio:device0
echo sysfstrig123 > trigger/current_trigger
echo 1 > scan_elements/in_voltage0_en
echo 1 > buffer/enable
########################################

cd /sys/bus/iio/devices/iio_sysfs_trigger
echo 123 > add_trigger
cd /sys/bus/iio/devices/trigger0
cat name # should give sysfstrig123
cd /sys/bus/iio/devices/iio:device0
echo sysfstrig123 > trigger/current_trigger
echo 1 > scan_elements/in_voltage0_en
echo 1 > buffer/enable




/sys/bus/iio/devices/iio_sysfs_trigger
echo 0 > /sys/bus/iio/devices/iio_sysfs_trigger/add_trigger

root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger# echo 0 > /sys/bus/iio/devices/iio_sysfs_trigger/add_trigger
root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger# ls -l
total 0
--w------- 1 root root 16384 Feb 17 23:39 add_trigger
drwxr-xr-x 2 root root     0 Feb 17 23:36 power
--w------- 1 root root 16384 Feb 17 23:36 remove_trigger
lrwxrwxrwx 1 root root     0 Feb 17 23:36 subsystem -> ../../bus/iio
drwxr-xr-x 3 root root     0 Feb 17 23:39 trigger0
-rw-r--r-- 1 root root 16384 Feb 17 23:33 uevent
root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger# 

root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger# cd trigger0/
root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger/trigger0# ls
name  power  subsystem	trigger_now  uevent

root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger/trigger0# echo 1 > /sys/bus/iio/devices/iio:device0/buffer/enable
-bash: echo: write error: Invalid argument
root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger/trigger0# ls /sys/bus/iio/devices/iio:device0/trigger/current_trigger
/sys/bus/iio/devices/iio:device0/trigger/current_trigger
root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger/trigger0# echo 1 > /sys/bus/iio/devices/iio:device0/buffer/enable
-bash: echo: write error: Invalid argument
root@raspberrypi:/sys/bus/iio/devices/iio_sysfs_trigger/trigger0# echo 1 > /sys/bus/iio/devices/trigger0/trigger_now



root@raspberrypi:/sys/bus/iio/devices/iio:device0# echo 1 > /sys/bus/iio/devices/iio:device0/scan_elements/in_voltage0_en
root@raspberrypi:/sys/bus/iio/devices/iio:device0# echo 256 > /sys/bus/iio/devices/iio:device0/buffer/length
root@raspberrypi:/sys/bus/iio/devices/iio:device0# echo trigger0 > /sys/bus/iio/devices/iio:device0/trigger/current_trigger
root@raspberrypi:/sys/bus/iio/devices/iio:device0# echo 1 > /sys/bus/iio/devices/iio:device0/buffer/enable

jairo@raspberrypi:~/rpi-linux/drivers/iio/adc $