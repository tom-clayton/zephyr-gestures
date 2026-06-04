/*
 * gestures.h: Zephyr gesture recognition library.
 *
 * Tom Clayton 2025
 */
#ifndef GESTURES_H
#define GESTURES_H

#include <stdint.h>

/*
 * gesture_type: The gesture that has been detected.
 */
typedef enum {
  GESTURE_ORIENTATION_CHANGE,
  GESTURE_DOUBLE_TWIST,
  GESTURE_TAP,
  N_GESTURES
} gesture_type;

/*
 * gesture_mode: The type of gesture to detect.
 */
typedef enum {
  MODE_6D,
  MODE_TAP,
  N_MODES
} gesture_mode;

/*
 * gesture: A structure representing a detected gesture.
 */
struct gesture {
  gesture_type type;
  uint64_t timestamp;
};

//struct k_msgq;

/*
 * Initialise the gesture recognition system.
 *
 * @param callback: A function called when a gesture is detected.
 * 
 * @return 0 on success, -1 on failure
 */
int gesture_init(void (*callback)(struct gesture));

/*
 * Select the mode of gesture detection.
 *
 * @param new_mode: The new mode of gesture detection.
 */
void gesture_select_mode(gesture_mode new_mode);

#endif /* GESTURES_H */