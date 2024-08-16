#include <esp_system.h>
#include <string.h>
#include "esp_mac.h"
#include "esp_gap_bt_api.h"
#include "ps4_control.h"
#include "ps4_spp.h"
#include "ps4_l2cap.h"

static const uint8_t hid_cmd_payload_ps4_enable[] = { 0x43, 0x02 };

static ps4_cmd_t ps4_control_cmd = { 0 };

ds4_ctrl_t * g_ds4_ctrt = NULL;
/**
  * @brief  初始化PS4手柄
  *
  * @param  mac ps4 MAC address, length: 6 bytes.
  *
  * @return null
  */
void ps4_control_init(ds4_ctrl_t * ds4_ctrt) 
{
    if(ds4_ctrt == NULL) return;

    g_ds4_ctrt = ds4_ctrt;

    ps4SetBluetoothMacAddress(g_ds4_ctrt->da4_mac);
    sppInit();
    ps4_l2cap_init_services();

    int count = esp_bt_gap_get_bond_device_num();
    uint8_t pairedDeviceBtAddr[count][6];
    esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);

    for (int i = 0; i < count; i++) {
        esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
    }
}

/*******************************************************************************
**
** Function         ps4Deinit
**
** Description      This deinitializes the bluetooth services to stop
**                  listening for incoming connections.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Deinit() {
    ps4_l2cap_deinit_services();
    //spp_deinit();
}

/*******************************************************************************
**
** Function         ps4Enable
**
** Description      This triggers the PS4 controller to start continually
**                  sending its data.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Enable() {
  uint16_t length = sizeof(hid_cmd_payload_ps4_enable);
  hid_cmd_t hidCommand;

  hidCommand.code = hid_cmd_code_set_report | hid_cmd_code_type_feature;
  hidCommand.identifier = hid_cmd_identifier_ps4_enable;

  memcpy(hidCommand.data, hid_cmd_payload_ps4_enable, length);

  ps4_l2cap_send_hid(&hidCommand, length);
  ps4_set_led(32, 32, 200);
}

/*******************************************************************************
**
** Function         ps4Cmd
**
** Description      Send a command to the PS4 controller.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Cmd(ps4_cmd_t cmd) {
  hid_cmd_t hidCommand = { .data = { 0x80, 0x00, 0xFF } };
  uint16_t length = sizeof(hidCommand.data);

  hidCommand.code = hid_cmd_code_set_report | hid_cmd_code_type_output;
  hidCommand.identifier = hid_cmd_identifier_ps4_control;

  hidCommand.data[ps4_control_packet_index_small_rumble] = cmd.smallRumble;  // Small Rumble
  hidCommand.data[ps4_control_packet_index_large_rumble] = cmd.largeRumble;  // Big rumble

  hidCommand.data[ps4_control_packet_index_red] = cmd.r;    // Red
  hidCommand.data[ps4_control_packet_index_green] = cmd.g;  // Green
  hidCommand.data[ps4_control_packet_index_blue] = cmd.b;   // Blue

  // Time to flash bright (255 = 2.5 seconds)
  hidCommand.data[ps4_control_packet_index_flash_on_time] = cmd.flashOn;
  // Time to flash dark (255 = 2.5 seconds)
  hidCommand.data[ps4_control_packet_index_flash_off_time] = cmd.flashOff;

  ps4_l2cap_send_hid(&hidCommand, length);
}

/**
  * @brief  设置PS4手柄状态rgb灯
  *
  * @param  r,g,b ps4状态rgb灯亮度.
  *
  * @return null
  */
void ps4_set_led(uint8_t r, uint8_t g, uint8_t b) 
{
  ps4_control_cmd.r = r;
  ps4_control_cmd.g = g;
  ps4_control_cmd.b = b;

  ps4Cmd(ps4_control_cmd);
}

/**
  * @brief  设置PS4手柄震动马达
  *
  * @param  large 大马达强度
  * @param  small 小马达强度
  *
  * @return null
  */
void ps4_set_rumble(uint8_t large, uint8_t small) 
{
  ps4_control_cmd.largeRumble = large;
  ps4_control_cmd.smallRumble = small;
  ps4_control_cmd.flashOn = 0;
  ps4_control_cmd.flashOff = 0;

  ps4Cmd(ps4_control_cmd);
}

/*******************************************************************************
**
** Function         ps4_set_output
**
** Description      Sets feedback on the PS4 controller.
**
**
** Returns          void
**
*******************************************************************************/
void ps4_set_output(ps4_cmd_t prevCommand) {
  ps4Cmd(prevCommand);
}

/*******************************************************************************
**
** Function         ps4SetBluetoothMacAddress
**
** Description      Writes a Registers a callback for receiving PS4 controller
*events
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetBluetoothMacAddress(const uint8_t* mac) {
  uint8_t baseMac[6];
  memcpy(baseMac, mac, 6);
  baseMac[5] -= 2;
  esp_base_mac_addr_set(baseMac);
}

/********************************************************************************/
/*                      L O C A L    F U N C T I O N S */
/********************************************************************************/
void ps4ConnectEvent(uint8_t is_connected) {
  if (is_connected) {
    ps4Enable();
  } else {
    // g_ds4_ctrt->da4_is_active = false;
  }
}

/**
  * @brief  获取PS4手柄指定按钮/传感器当前输出
  *
  * @param  packet 收到的数据
  * @param  len 收到的数据长度
  *
  * @return 指定按钮/传感器输出
  * @note 
    0~2字节为按钮输出1bit
    3~6字节为摇杆XY输出8bit
    7~12字节为陀螺仪XYZ输出16bit
    13~18字节为加速度计XYZ输出16bit
    19字节为遥控器当前电量8bit
*/

int16_t ps4_control_parsing(uint8_t *ps4_date,ps4_out_datat_tpye out_type)
{
  uint8_t frontBtnData = ps4_date[packet_index_button_standard - PS4_OUT_DATA_OFFSET];
  uint8_t extraBtnData = ps4_date[packet_index_button_extra - PS4_OUT_DATA_OFFSET];
  uint8_t psBtnData = ps4_date[packet_index_button_ps - PS4_OUT_DATA_OFFSET];
  uint8_t directionBtnsOnly = button_mask_direction & frontBtnData;
  int16_t buff = 0;

  switch(out_type)
  {
    /*肩键状态*/
    case PS4_OUTID_BUTTON_L1:
        return extraBtnData & button_mask_l1;
    break;
    case PS4_OUTID_BUTTON_L2:
        return extraBtnData & button_mask_l2;
    break;
    case PS4_OUTID_BUTTON_L3:
        return extraBtnData & button_mask_l3;
    break;
    case PS4_OUTID_BUTTON_R1:
        return extraBtnData & button_mask_r1;
    break;
    case PS4_OUTID_BUTTON_R2:
        return extraBtnData & button_mask_r2;
    break;
    case PS4_OUTID_BUTTON_R3:
        return extraBtnData & button_mask_r3;
    break;
    /*方向键状态*/
    case PS4_OUTID_BUTTON_DIR_UP:
        return directionBtnsOnly == button_mask_up;
    break;
    case PS4_OUTID_BUTTON_DIR_DOWN:
        return directionBtnsOnly == button_mask_down;
    break;
    case PS4_OUTID_BUTTON_DIR_LEFT:
        return directionBtnsOnly == button_mask_left;
    break;
    case PS4_OUTID_BUTTON_DIR_RIGHR:
        return directionBtnsOnly == button_mask_right;
    break;
    case PS4_OUTID_BUTTON_DIR_UP_RIGHT:
        return directionBtnsOnly == button_mask_upright;
    break;
    case PS4_OUTID_BUTTON_DIR_UP_LEFT:
        return directionBtnsOnly == button_mask_upleft;
    break;
    case PS4_OUTID_BUTTON_DIR_DOWN_LEFT:
        return directionBtnsOnly == button_mask_downright;
    break;
    case PS4_OUTID_BUTTON_DIR_DOWN_RIGHR:
        return directionBtnsOnly == button_mask_downleft;
    break;
    /*特殊功能键状态*/
    case PS4_OUTID_BUTTON_SHAPE_TRIANGLE:
        return frontBtnData & button_mask_triangle;
    break;
    case PS4_OUTID_BUTTON_SHAPE_CIRCLE:
        return frontBtnData & button_mask_circle;
    break;
    case PS4_OUTID_BUTTON_SHAPE_CROSS:
        return frontBtnData & button_mask_cross;
    break;
    case PS4_OUTID_BUTTON_SHAPE_SQUARE:
        return frontBtnData & button_mask_square;
    break;
    case PS4_OUTID_BUTTON_PS:
        return psBtnData & button_mask_ps;
    break;
    case PS4_OUTID_BUTTON_SHARE:
        return extraBtnData & button_mask_share;
    break;
    case PS4_OUTID_BUTTON_OPTIONS:
        return extraBtnData & button_mask_options;
    break;
    case PS4_OUTID_BUTTON_TOUCH:
        return psBtnData & button_mask_touchpad;
    break;

    /*摇杆模拟量*/
    case PS4_OUTID_ANALOG_ROCKER_LX:
        return ps4_date[packet_index_analog_stick_lx - PS4_OUT_DATA_OFFSET];
    break;
    case PS4_OUTID_ANALOG_ROCKER_LY:
        return ps4_date[packet_index_analog_stick_ly - PS4_OUT_DATA_OFFSET];
    break;
    case PS4_OUTID_ANALOG_ROCKER_L2:
        return ps4_date[packet_index_analog_l2 - PS4_OUT_DATA_OFFSET];
    break;
    case PS4_OUTID_ANALOG_ROCKER_RX:
        return ps4_date[packet_index_analog_stick_rx - PS4_OUT_DATA_OFFSET];
    break;
    case PS4_OUTID_ANALOG_ROCKER_RY:
        return ps4_date[packet_index_analog_stick_ry - PS4_OUT_DATA_OFFSET];
    break;
    case PS4_OUTID_ANALOG_ROCKER_R2:
        return ps4_date[packet_index_analog_r2 - PS4_OUT_DATA_OFFSET];
    break;
    case PS4_OUTID_ANALOG_TOUCH_X:
        buff = (ps4_date[23] << 8) + ps4_date[22];
        return buff;
    break;
    case PS4_OUTID_ANALOG_TOUCH_Y:
        return 100;
    break;
    case PS4_OUTID_ANALOG_GRY_X:
        buff = (ps4_date[packet_index_sensor_gyroscope_x + 1 - PS4_OUT_DATA_OFFSET] << 8) + ps4_date[packet_index_sensor_gyroscope_x - PS4_OUT_DATA_OFFSET];
        return buff;
    break;
    case PS4_OUTID_ANALOG_GRY_Y:
        buff = (ps4_date[packet_index_sensor_gyroscope_y + 1 - PS4_OUT_DATA_OFFSET] << 8) + ps4_date[packet_index_sensor_gyroscope_y - PS4_OUT_DATA_OFFSET];
        return buff;
    break;
    case PS4_OUTID_ANALOG_GRY_Z:
        buff = (ps4_date[packet_index_sensor_gyroscope_z + 1 - PS4_OUT_DATA_OFFSET] << 8) + ps4_date[packet_index_sensor_gyroscope_z - PS4_OUT_DATA_OFFSET];
        return buff;
    break;
    case PS4_OUTID_ANALOG_ACC_X:
        buff = (ps4_date[packet_index_sensor_accelerometer_x + 1 - PS4_OUT_DATA_OFFSET] << 8) + ps4_date[packet_index_sensor_accelerometer_x - PS4_OUT_DATA_OFFSET];
        return buff;
    break;
    case PS4_OUTID_ANALOG_ACC_Y:
        buff = (ps4_date[packet_index_sensor_accelerometer_y + 1 - PS4_OUT_DATA_OFFSET] << 8) + ps4_date[packet_index_sensor_accelerometer_y - PS4_OUT_DATA_OFFSET];
        return buff;
    break;
    case PS4_OUTID_ANALOG_ACC_Z:
        buff = (ps4_date[packet_index_sensor_accelerometer_z + 1 - PS4_OUT_DATA_OFFSET] << 8) + ps4_date[packet_index_sensor_accelerometer_z - PS4_OUT_DATA_OFFSET];
        return buff;
    break;
    case PS4_OUTID_ANALOG_BAT:
        buff = ps4_date[packet_index_status - PS4_OUT_DATA_OFFSET] & ps4_status_mask_battery;
        return buff;
    default:
        return 0;
  };

}
