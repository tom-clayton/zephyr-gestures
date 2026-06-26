/*
 * gestures.c: Zephyr gesture recognition library.
 *
 * Tom Clayton 2025
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "gestures.h"
#include "lsm6ds3tr.h"
#include "i2c_wrapper.h"

#define TILT_TIMEOUT 1000
#define N_TILTS 3
#define DEFAULT_MODE MODE_6D

#define I2C_NODE DT_ALIAS(accelerometer)
static const struct gpio_dt_spec accel = GPIO_DT_SPEC_GET(I2C_NODE, irq_gpios);
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

static struct gpio_callback gesture_cb_data;
static gesture_mode mode;
static void (*gesture_callback)(struct gesture) = NULL;

/*
 *  I2C write function.
 */
int zephyr_i2c_write(const uint8_t *buf, size_t len)
{
    return i2c_write(dev_i2c.bus, buf, len, dev_i2c.addr);
}

/*
 * I2C write-read function.
 */
int zephyr_i2c_write_read(uint8_t reg, uint8_t *out, size_t out_len)
{
    return i2c_write_read(dev_i2c.bus, dev_i2c.addr, &reg, 1, out, out_len); 
}

/*
 * I2C wrapper struct.
 */
static const i2c_bus_t i2c_bus = {
  .i2c_write = zephyr_i2c_write,
  .i2c_write_read = zephyr_i2c_write_read,
};

/*
 * Detect a double twist gesture by checking times of previous twists. 
 */
uint64_t detect_double_twist(uint64_t timestamp)
{
  static uint64_t previous_timestamp;
  static uint8_t tilt_count = 0;

  if(timestamp - previous_timestamp < TILT_TIMEOUT) {
    tilt_count++;
  } else {
    tilt_count = 0;
  }
  
  if (tilt_count >= N_TILTS) {
    tilt_count = 0;
    return 1; 
  }

  previous_timestamp = timestamp;
  return 0;
}

/*
 * Interrupt service routine. 
 * 
 * Determine gesture type and activate callback. 
 */ 
static void gesture_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) 
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    uint64_t timestamp = k_uptime_get();
    
    struct gesture gesture_event = {
      .timestamp = timestamp,
      .type = GESTURE_TAP
    };
    
    if (mode != MODE_TAP) {
      gesture_event.type = detect_double_twist(timestamp) ? GESTURE_DOUBLE_TWIST : GESTURE_ORIENTATION_CHANGE;
    }
    
    if (gesture_callback) {
      gesture_callback(gesture_event);
    }
}
/*
 * Initialise accelerometer interrupt.
 */
int gesture_init(void (*callback)(struct gesture))
{
  if (callback == NULL) {
    return -1;
  }
  gesture_callback = callback;

  if (lsm6ds_init(&i2c_bus) != 0) {
      return -1;
  }

  if (gpio_pin_configure_dt(&accel, GPIO_INPUT)) {
      return -1;
  }

  if (gpio_pin_interrupt_configure_dt(&accel, GPIO_INT_EDGE_TO_ACTIVE)) {
      return -1;
  }

  gpio_init_callback(&gesture_cb_data, gesture_isr, BIT(accel.pin));

  if(gpio_add_callback(accel.port, &gesture_cb_data)){
      return -1;
  }

  gesture_select_mode(DEFAULT_MODE);
  return 0;
}

/*
 * Select accelerometer mode.
 */
void gesture_select_mode(gesture_mode new_mode)
{
  mode = new_mode;
  switch(mode){
    case MODE_6D:
      lsm6ds_setup_6D();
      break;
    case MODE_TAP:
      lsm6ds_setup_tap();
      break;
    default:
      break;
  }
}

