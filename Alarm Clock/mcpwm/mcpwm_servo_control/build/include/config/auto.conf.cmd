deps_config := \
	/Users/roberto.gapa96/esp/esp-idf/components/app_trace/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/bt/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/driver/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/esp32/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/esp_http_client/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/ethernet/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/fatfs/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/freertos/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/heap/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/http_server/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/libsodium/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/log/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/lwip/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/mdns/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/openssl/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/pthread/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/spiffs/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/vfs/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/components/wear_levelling/Kconfig \
	/Users/roberto.gapa96/esp/esp-idf/Kconfig.compiler \
	/Users/roberto.gapa96/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/roberto.gapa96/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/roberto.gapa96/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/roberto.gapa96/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
