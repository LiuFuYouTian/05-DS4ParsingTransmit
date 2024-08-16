#ifndef PS4_L2CAP_H
#define PS4_L2CAP_H

void ps4_l2cap_init_services();
void ps4_l2cap_deinit_services();
void ps4_l2cap_send_hid(hid_cmd_t* hid_cmd, uint8_t len);

#endif