# gestures

A Zephyr gesture recognition library. Connects to an lsm6ds3tr accelerometer to recognise gestures.

### Usage

Requires the accelerometer to be setup in the device tree with alias 'accelerometer'.

A callback function that accepts a 'gesture' structure should be passed to the initialisation function.

```C
struct gesture {
  gesture_type type;
  uint64_t timestamp;
};
```

```C
int gesture_init(void (*callback)(struct gesture));
```

The gesture 'mode' selects which type of gestures are detected as follows:

- MODE_6D
    - Orientation change
    - Double twist
- MODE_TAP
    - Double Tap


```C
void gesture_select_mode(gesture_mode new_mode);
```

The i2c bus that the accelerometer is on is not checked at startup, this must be done independently.

### Installation Notes

lsm6ds3tr.h and lsm6ds3tr.c must be included in the build.

### Todo:

Add lsm6ds3tr library as a submodule.<br>
Support for other accelerometers.<br>
