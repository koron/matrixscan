# matrixscan - Key Matrix Scanner prototype for RaspberryPi Pico

How to build:

```console
$ cmake -B build
$ make -j4 -C build
```

Output: `build\matrixscan.uf2`

## Pin assign

* COLUMNS: 11, 12, 14, 15, 16, 17, 19, 20, 21, 22
* ROWS: 24, 25, 26, 27, 29
* Scan direction: column to row
* LED: 0 (should be changed)
* OLED: 4(SDA), 5(SCL)
