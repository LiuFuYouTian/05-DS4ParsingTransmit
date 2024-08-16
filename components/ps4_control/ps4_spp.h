#ifndef PS4_SPP_H
#define PS4_SPP_H

enum hid_cmd_code {
  hid_cmd_code_set_report = 0x50,
  hid_cmd_code_type_output = 0x02,
  hid_cmd_code_type_feature = 0x03
};

enum hid_cmd_identifier {
  hid_cmd_identifier_ps4_enable = 0xF4,
  hid_cmd_identifier_ps4_control = 0x11
};



void sppInit();

#endif