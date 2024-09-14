# Compatible library functions of SD and SdFat

This sketch is based on [libraries/SD/examples/SD_Test][1].

## Functions

- bool SD.begin(SD_CONFIG)

- void listDir(FS_TYPE &fs, const char *dirname, uint8_t levels)

- void createDir(FS_TYPE &fs, const char *path)

- void removeDir(FS_TYPE &fs, const char *path)

- void readFile(FS_TYPE &fs, const char *path)

- void writeFile(FS_TYPE &fs, const char *path, const char *message)

- void appendFile(FS_TYPE &fs, const char *path, const char *message)

- void renameFile(FS_TYPE &fs, const char *path1, const char *path2)

- void deleteFile(FS_TYPE &fs, const char *path)

[1]: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD/examples/SD_Test "arduino-esp32/libraries/SD/examples/SD_Test at master · espressif/arduino-esp32"

[2]: 