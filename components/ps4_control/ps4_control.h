#ifndef PS4_H
#define PS4_H

#include <stdbool.h>
#include <stdint.h>

#define PS4_RECV_BUFFER_SIZE 79
#define PS4_SEND_BUFFER_SIZE 77
#define PS4_HID_BUFFER_SIZE 50

#define PS4_OUT_DATA_OFFSET 13 //有效数据相对原始数据偏移

/*PS4手柄输出数据索引定义*/
typedef enum {
  PS4_OUTID_BUTTON_L1,
  PS4_OUTID_BUTTON_L2,
  PS4_OUTID_BUTTON_L3,
  PS4_OUTID_BUTTON_R1,
  PS4_OUTID_BUTTON_R2,
  PS4_OUTID_BUTTON_R3,
  PS4_OUTID_BUTTON_DIR_UP,
  PS4_OUTID_BUTTON_DIR_DOWN,
  PS4_OUTID_BUTTON_DIR_LEFT,
  PS4_OUTID_BUTTON_DIR_RIGHR,
  PS4_OUTID_BUTTON_DIR_UP_RIGHT,
  PS4_OUTID_BUTTON_DIR_UP_LEFT,
  PS4_OUTID_BUTTON_DIR_DOWN_LEFT,
  PS4_OUTID_BUTTON_DIR_DOWN_RIGHR,
  PS4_OUTID_BUTTON_SHAPE_TRIANGLE,
  PS4_OUTID_BUTTON_SHAPE_CIRCLE,
  PS4_OUTID_BUTTON_SHAPE_CROSS,
  PS4_OUTID_BUTTON_SHAPE_SQUARE,
  PS4_OUTID_BUTTON_PS,
  PS4_OUTID_BUTTON_SHARE,
  PS4_OUTID_BUTTON_OPTIONS,
  PS4_OUTID_BUTTON_TOUCH,
  PS4_OUTID_ANALOG_ROCKER_LX,
  PS4_OUTID_ANALOG_ROCKER_LY,
  PS4_OUTID_ANALOG_ROCKER_L2,
  PS4_OUTID_ANALOG_ROCKER_RX,
  PS4_OUTID_ANALOG_ROCKER_RY,
  PS4_OUTID_ANALOG_ROCKER_R2,
  PS4_OUTID_ANALOG_TOUCH_X,
  PS4_OUTID_ANALOG_TOUCH_Y,
  PS4_OUTID_ANALOG_GRY_X,
  PS4_OUTID_ANALOG_GRY_Y,
  PS4_OUTID_ANALOG_GRY_Z,
  PS4_OUTID_ANALOG_ACC_X,
  PS4_OUTID_ANALOG_ACC_Y,
  PS4_OUTID_ANALOG_ACC_Z,
  PS4_OUTID_ANALOG_BAT,
  PS4_OUTID_MAX,
} ps4_out_datat_tpye;

enum ps4_packet_index {
  packet_index_analog_stick_lx = 13,
  packet_index_analog_stick_ly = 14,
  packet_index_analog_stick_rx = 15,
  packet_index_analog_stick_ry = 16,

  packet_index_button_standard = 17,
  packet_index_button_extra = 18,
  packet_index_button_ps = 19,

  packet_index_analog_l2 = 20,
  packet_index_analog_r2 = 21,

  packet_index_sensor_gyroscope_x = 25,
  packet_index_sensor_gyroscope_y = 27,
  packet_index_sensor_gyroscope_z = 29,

  packet_index_sensor_accelerometer_x = 31,
  packet_index_sensor_accelerometer_y = 33,
  packet_index_sensor_accelerometer_z = 35,

  packet_index_status = 42
};


enum ps4_button_mask {
  button_mask_up = 0,
  button_mask_right = 0b00000010,
  button_mask_down = 0b00000100,
  button_mask_left = 0b00000110,

  button_mask_upright = 0b00000001,
  button_mask_downright = 0b00000011,
  button_mask_upleft = 0b00000111,
  button_mask_downleft = 0b00000101,

  button_mask_direction = 0b00001111,

  button_mask_square = 0b00010000,
  button_mask_cross = 0b00100000,
  button_mask_circle = 0b01000000,
  button_mask_triangle = 0b10000000,

  button_mask_l1 = 0b00000001,
  button_mask_r1 = 0b00000010,
  button_mask_l2 = 0b00000100,
  button_mask_r2 = 0b00001000,

  button_mask_share = 0b00010000,
  button_mask_options = 0b00100000,

  button_mask_l3 = 0b01000000,
  button_mask_r3 = 0b10000000,

  button_mask_ps = 0b01,
  button_mask_touchpad = 0b10
};

enum ps4_status_mask {
  ps4_status_mask_battery = 0b00001111,
  ps4_status_mask_charging = 0b00010000,
  ps4_status_mask_audio = 0b00100000,
  ps4_status_mask_mic = 0b01000000,
};

/**
  * @brief  获取PS4手柄指定按钮/传感器当前输出
  *
  * @param  packet 收到的数据
  * @param  len 收到的数据长度
  *
  * @return 指定按钮/传感器输出
  */
int16_t ps4_control_parsing(uint8_t *ps4_date,ps4_out_datat_tpye out_type);

typedef struct {
  uint8_t smallRumble;
  uint8_t largeRumble;
  uint8_t r, g, b;
  uint8_t flashOn;
  uint8_t flashOff;  // Time to flash bright/dark (255 = 2.5 seconds)
} ps4_cmd_t;


enum ps4_control_packet_index {
  ps4_control_packet_index_small_rumble = 5,
  ps4_control_packet_index_large_rumble = 6,

  ps4_control_packet_index_red = 7,
  ps4_control_packet_index_green = 8,
  ps4_control_packet_index_blue = 9,

  ps4_control_packet_index_flash_on_time = 10,
  ps4_control_packet_index_flash_off_time = 11
};

typedef struct {
  uint8_t code;
  uint8_t identifier;
  uint8_t data[PS4_SEND_BUFFER_SIZE];
} hid_cmd_t;

/**
  * @brief     Callback function of sending esp_sbus data
  * @param     byte sbus data
  * @param     len 一版情况下SBUS数据长度固定是25
  */
typedef void (*ds4_ctrl_recv_cb_t)(uint8_t *byte, uint8_t len);


typedef struct {
    uint8_t da4_mac[6];   //MAC address of destination device.
    ds4_ctrl_recv_cb_t ds4_ctrl_recv_cb;
} ds4_ctrl_t;

extern ds4_ctrl_t *g_ds4_ctrt;

void ps4ConnectEvent(uint8_t is_connected);
bool ps4IsConnected();
void ps4_control_init(ds4_ctrl_t * ds4_ctrt);
void ps4Deinit();
void ps4Enable();
void ps4Cmd(ps4_cmd_t ps4_cmd);
void ps4SetOutput(ps4_cmd_t prev_cmd);
void ps4SetBluetoothMacAddress(const uint8_t* mac);

/**
  * @brief  获取PS4手柄当前连接状态
  *
  * @param  null
  *
  * @return true/false
  */
bool ps4_get_connected_state();

/**
  * @brief  设置PS4手柄状态rgb灯
  *
  * @param  r,g,b ps4状态rgb灯亮度.
  *
  * @return null
  */
void ps4_set_led(uint8_t r, uint8_t g, uint8_t b) ;

/**
  * @brief  设置PS4手柄震动马达
  *
  * @param  large 大马达强度
  * @param  small 小马达强度
  *
  * @return null
  */
void ps4_set_rumble(uint8_t large, uint8_t small);

#endif
