#ifndef _USER_AT_COMMANDES_H
#define _USER_AT_COMMANDES_H

#include "esp_at.h"

#define esp_debug_enable 1

//配网指令 API
uint8_t at_test_cmd_SCWJAP(uint8_t *cmd_name);
uint8_t at_query_cmd_SCWJAP(uint8_t *cmd_name);
 uint8_t at_setup_cmd_SCWJAP(uint8_t para_num);
void user_at_cmd_init(void);

void tcp_data_handle(unsigned char *out_data,unsigned char tcp_data[1024]);
void tcp_data_analysis(unsigned char *out_data,unsigned char tcp_data[1024]);
#endif
