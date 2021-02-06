# matrixscan - Key Matrix Scanner prototype for RaspberryPi Pico

How to build:

```console
$ cmake -B build
$ make -j4 -C build
```

Output: `build\matrixscan.uf2`

## Pin assign

* COLUMNS: 11, 12, 14, 15, 16, 17, 19, 20, 21, 22
  (GP8, 9, 10, 11, 12, 13, 14, 15, 16, 17)
* ROWS: 24, 25, 26, 27, 29
  (GP18, 19, 20, 21, 22)
* Scan direction: column to row
* LED: 34 (GP28)
* OLED: 4(I2C1 SDA), 5(I2C1 SCL)
