@echo off

set VERSION=%1
set CURDIR=%CD%
echo ***************************************
echo *                                     *
echo Building BIN files for version: %VERSION%
echo *                                     *
echo ***************************************
echo `

echo Current DIR: %CURDIR%
if exist ".\bin" GOTO steptwo
echo `
echo -----------------------------------------
echo --------Creating bin files folder--------
echo -----------------------------------------
mkdir %CURDIR%\bin

:steptwo
echo `
echo -----------------------------------
echo --------Building config.bin--------
echo -----------------------------------
echo `
echo Building config.bin for board Zyber8S
python.exe D:\espidf\v5.4\esp-idf\components\nvs_flash\nvs_partition_generator\nvs_partition_gen.py generate .\config-zyber8s.cvs .\build\config-zyber8s.bin 0x6000




echo Building config.bin for board Zyber8S-office
python.exe D:\espidf\v5.4\esp-idf\components\nvs_flash\nvs_partition_generator\nvs_partition_gen.py generate .\config-zyber8s-office.cvs .\build\config-zyber8s-office.bin 0x6000



echo `
echo -----------------------------------
echo --------Building factory.bin-------
echo -----------------------------------
echo `
echo Building factory.bin for board zyber8s
python -m esptool --chip esp32s3 merge_bin --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x9000 build/config-zyber8s.bin 0x10000 build/esp-miner.bin 0x410000 build/www.bin 0xf10000 build/ota_data_initial.bin -o "bin\esp-miner-zyber8s-factory-%VERSION%.bin"

echo Building factory.bin for board zyber8s-office
python -m esptool --chip esp32s3 merge_bin --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x9000 build/config-zyber8s-office.bin 0x10000 build/esp-miner.bin 0x410000 build/www.bin 0xf10000 build/ota_data_initial.bin -o "bin\esp-miner-zyber8s-office-factory-%VERSION%.bin"


xcopy %CURDIR%\build\esp-miner.bin %CURDIR%\bin /Y
xcopy %CURDIR%\build\www.bin %CURDIR%\bin /Y