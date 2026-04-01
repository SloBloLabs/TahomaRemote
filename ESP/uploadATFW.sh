PORT=/dev/cu.usbserial-A505C7BL
FWPATH=ESP-WROOM-02-AT-V2.3.0.0/ESP-WROOM-02-AT-V2.3.0.0

esptool \
 --chip auto \
 --port $PORT \
 --baud 115200 \
 --before default-reset \
 --after hard-reset write-flash \
 -z \
 --flash-mode dio \
 --flash-freq 26m \
 --flash-size 4MB \
 0x0 $FWPATH/factory/factory_WROOM-02.bin


#  -> ./uploadATFW.sh
# esptool v5.0.2
# Connected to ESP8266 on /dev/tty.usbmodem1101:
# Chip type:          ESP8266EX
# Features:           Wi-Fi, 160MHz
# Crystal frequency:  26MHz
# MAC:                ec:fa:bc:cb:1b:0b
# 
# Stub flasher is already running. No upload is necessary.
# 
# Configuring flash size...
# Flash will be erased from 0x00000000 to 0x001fffff...
# Flash parameters set to 0x0241.
# Wrote 2097152 bytes (582385 compressed) at 0x00000000 in 53.9 seconds (311.3 kbit/s).
# Hash of data verified.
# 
# Hard resetting via RTS pin...


# On Log Pin GPIO2:
# (44) boot: ESP-IDF v3.4-112-gc965e03 2nd stage bootloader.[0m
# .[0;32mI (45) boot: compile time 03:16:21.[0m
# .[0;32mI (45) boot: SPI Speed      : 26.7MHz.[0m
# .[0;32mI (48) boot: SPI Mode       : DIO.[0m
# .[0;32mI (52) boot: SPI Flash Size : 4MB.[0m
# .[0;32mI (56) boot: Partition Table:.[0m
# .[0;32mI (60) boot: ## Label            Usage          Type ST Offset   Length.[0m
# .[0;32mI (67) boot:  0 otadata          OTA data         01 00 00009000 00002000.[0m
# .[0;32mI (74) boot:  1 ota_0            OTA app          00 10 00010000 000e0000.[0m
# .[0;32mI (82) boot:  2 at_customize     unknown          40 00 000f0000 00020000.[0m
# .[0;32mI (89) boot:  3 ota_1            OTA app          00 11 00110000 000e0000.[0m
# .[0;32mI (96) boot:  4 nvs              WiFi data        01 02 001f0000 00010000.[0m
# .[0;32mI (104) boot: End of partition table.[0m
# .[0;32mI (108) boot: No factory image, trying OTA 0.[0m
# .[0;32mI (113) esp_image: segment 0: paddr=0x00010010 vaddr=0x40210010 size=0xb90e8 (757992) map.[0m
# .[0;32mI (122) esp_image: segment 1: paddr=0x000c9100 vaddr=0x402c90f8 size=0x1c320 (115488) map.[0m
# .[0;32mI (130) esp_image: segment 2: paddr=0x000e5428 vaddr=0x3ffe8000 size=0x00768 (  1896) load.[0m
# .[0;32mI (139) esp_image: segment 3: paddr=0x000e5b98 vaddr=0x40100000 size=0x00080 (   128) load.[0m
# .[0;32mI (148) esp_image: segment 4: paddr=0x000e5c20 vaddr=0x40100080 size=0x05af0 ( 23280) load.[0m
# .[0;32mI (157) boot: Loaded app from partition at offset 0x10000.[0m
# phy_version: 1163.0, 665d56c, Jun 24 2020, 10:00:08, RTOS new
# module_name:WROOM-02
# 
# max tx power=78,ret=0
# 
# 2.3.0
# 
