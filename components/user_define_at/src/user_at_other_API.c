/**
 * @file user_at_other_API.c
 * @author your name (you@domain.com)
 * @brief  用户其他API
 * @version 0.1
 * @date 2021-07-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "cJSON.h"
#include "user_at_commands.h"

/**
 * @brief 数据创建
 * 
 * @param out_data 
 * @param tcp_data 
 */
void tcp_data_handle(unsigned char *out_data,unsigned char tcp_data[1024])
{
    unsigned char *data_temp=out_data;
    
    *(data_temp++)=0x0d;
    *(data_temp++)=0x0a;
    *(data_temp++)=0x0d;
    *(data_temp++)=0x0a;
    if (strlen((const char *)tcp_data)<256)
    {
         *(data_temp++)=0x00;
         *(data_temp++)=strlen((const char *)tcp_data); 
    } else {
        *(data_temp++)=strlen((const char *)tcp_data); 
        data_temp++;
    }
    //printf("str %d\n",data_len);
    for (int i = 0; i < strlen((const char *)tcp_data); i++)
        data_temp[i]=tcp_data[i];    
}
/**
 * @brief 数据解析 函数
 * 
 * @param out_data 
 * @param tcp_data 
 */
void tcp_data_analysis(unsigned char *out_data,unsigned char tcp_data[1024])
{
    unsigned char *buffer=out_data;
    cJSON *cJson=NULL;
    cJSON *json_msgType=NULL;
    cJSON *json_data=NULL;
     for (size_t i = 0; i < 256; i++)
        buffer[i]=tcp_data[i+6];
    cJSON_Parse(cJson);
        
    json_msgType =cJSON_GetObjectItem(cJson,"msgType");
    json_data    =cJSON_GetObjectItem(cJson,"data");
   // json_data->string
}
