#!/bin/bash

if [ "$1" == "-flash" ]
then
	esptool.py -p /dev/ttyUSB0 write_flash --flash_mode qio --flash_size 4MB 0x00000 ../bin/eagle.flash.bin 0x10000 ../bin/eagle.irom0text.bin 0x3FB000 ../bin/blank.bin 0x3FC000 ../bin/esp_init_data_default_v08.bin 0x3FE000 ../bin/blank.bin	
fi

if [ "$1" == "-erase" ]
then
	esptool.py -p /dev/ttyUSB0 erase_flash
fi
