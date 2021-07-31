#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbedtls/aes.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "user_at_commands.h"

#define TCP_SERVER_IP "10.10.10.1"
#define TCP_SERVER_PORT 8883

#if esp_debug_enable
    uint8_t SoftAP_ssid[] = "LEAKSENSOR XXXXXX";
#elif 
    uint8_t SoftAP_ssid[] = "HH-SMTWLG01-XXXXXX";
#endif   

//测试命令
/**
 * @brief 测试指令执行函数 AT+SCWJAP=?
 *          
 * @param cmd_name 
 * @return uint8_t 
 */
uint8_t at_test_cmd_SCWJAP(uint8_t *cmd_name)
{
    uint8_t buffer[64] = {0};

    snprintf((char *)buffer, 64, "%s:SSID:12346 PASWD:123456\r\n", cmd_name);

    esp_at_port_write_data(buffer, strlen((char *)buffer));

    return ESP_AT_RESULT_CODE_OK;
}
//查询命令
/**
 * @brief 查询指令执行函数  AT+SCWJAP?
 *        可查询 ssid 及 password
 * @param cmd_name 
 * @return uint8_t 
 */
uint8_t at_query_cmd_SCWJAP(uint8_t *cmd_name)
{
    uint8_t buffer[1024] = {0};
    char *ssid = malloc(33);
    char *password = malloc(65);

    static wifi_config_t wifi_config;
    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

    strcpy(ssid, (const char *)wifi_config.sta.ssid);
    strcpy(password, (const char *)wifi_config.sta.password);
    snprintf((char *)buffer, 64, "%s:SSID:%s PASWD:%s\r\n", cmd_name, ssid, password);
    esp_at_port_write_data(buffer, strlen((char *)buffer));

    free(ssid);
    free(password);

    return ESP_AT_RESULT_CODE_OK;
}
/**
 * @brief 配网设置指令处理函数
 *        AT+SCWJAP="sku","token"  
 * @param para_num 
 * @return uint8_t 
 */
uint8_t at_setup_cmd_SCWJAP(uint8_t para_num)
{
    uint8_t *para_sku = NULL;
    uint8_t *para_token = NULL;
    int32_t para_encrypt=0;
    uint8_t num_index = 0;
    uint8_t buffer[64] = {0};
    uint8_t *mac = malloc(6);
    uint8_t MAC[32] = {0};

    uint8_t *tcp_data = malloc(2048);
    uint8_t tcp_data1[1024]={0};
    static wifi_config_t wifi_config;

    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = inet_addr(TCP_SERVER_IP);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(TCP_SERVER_PORT);
    /**  解析sku          **/
    if (esp_at_get_para_as_str(num_index++, &para_sku) != ESP_AT_PARA_PARSE_RESULT_OK) {
         return ESP_AT_RESULT_CODE_ERROR;
    }
#if esp_debug_enable
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 64, "+SCWJAP:%s\r\n",para_sku);
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    /**  解析 token      **/
    if (esp_at_get_para_as_str(num_index++, &para_token) != ESP_AT_PARA_PARSE_RESULT_OK){
        return ESP_AT_RESULT_CODE_ERROR;
    }
#if esp_debug_enable
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 64, "+SCWJAP:%s\r\n",para_token);
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif        
    /**  解析 encrypt    **/
    if (esp_at_get_para_as_digit(num_index++, &para_encrypt) != ESP_AT_PARA_PARSE_RESULT_OK){
        return ESP_AT_RESULT_CODE_ERROR;
    }
#if esp_debug_enable
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 64, "+SCWJAP:%d\r\n",para_encrypt);
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif               
    /**  获取MAC信息      **/
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf((char *)MAC, 32, "%x%x%x%x%x%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    for (size_t i = 0; i < 12; i++)
    {
        if (MAC[i] >= 'a' && MAC[i] <= 'z')
        {
            MAC[i] = MAC[i] - 32;
        }
    }
    /**  清除旧的AP信息     **/
    esp_wifi_disconnect(); //断开连接

    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
    if (wifi_config.sta.ssid != 0)
    {
        memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
        memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));
    }
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    tcpip_adapter_ip_info_t ip_info;
#if esp_debug_enable==2
    snprintf((char *)buffer, 64, "old AP clrea OK\r\n");
    esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    /**  创建soft ap 的ssid **/
    int j = 0;
    while (SoftAP_ssid[j] != 'X')
        j++;
    for (size_t i = 0; i < 6; i++)
        SoftAP_ssid[i + j] = MAC[5 + i];
   
#if esp_debug_enable==2
    memset(buffer, 0, 64);
    snprintf((char *)buffer, 64, "%s\r\n", SoftAP_ssid);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif

    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char *)wifi_config.ap.ssid, (const char *)SoftAP_ssid);
    wifi_config.ap.max_connection = 2;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
    IP4_ADDR(&ip_info.ip,10,10,10,1);
    IP4_ADDR(&ip_info.gw,10,10,10,1);
    IP4_ADDR(&ip_info.netmask,255,255,255,0);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);

    esp_wifi_set_mode(WIFI_MODE_AP); //配置成 AP模式
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
    //设置AP的IP地址  
#if esp_debug_enable==2
    memset(buffer, 0, 64);
    snprintf((char *)buffer, 64, "Soft ap waiting connect.....\r\n");
    esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    /**  创建 socket 连接  **/
    int c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    setsockopt(c_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    esp_err_t err = bind(c_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if (err != 0)
    {
#if esp_debug_enable==2
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 64, "soket bind fail\r\n");
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    }
    err = listen(c_socket, 1);
    if (err == 0)
    {
#if esp_debug_enable
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 64, "soket listen...\r\n");
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    }
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);
    int sock = accept(c_socket, (struct sockaddr *)&source_addr, &addr_len);
    if (sock < 0)
    {
#if esp_debug_enable
        
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 64, "Unable to accept connection: errno\r\n");
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
        return ESP_AT_RESULT_CODE_ERROR;
    }
    else
    {
#if esp_debug_enable
       dest_addr_ip4=(struct sockaddr_in *)&source_addr;
       memset(buffer, 0, 64);
       //*tcp_data+= '\0';
        snprintf((char *)buffer, 64, "tcp client:%s\r\n", inet_ntoa(dest_addr_ip4->sin_addr.s_addr));
        esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    /** 发送Message 到手机APP **/
    
    /*for (size_t i = 0; i < 12; i++)
    {
        if (MAC[i] >= 'A' && MAC[i] <= 'Z')
        {
            MAC[i] = MAC[i] + 32;
        }
    }*/
    memset(tcp_data,0,1024);
    snprintf((char *)tcp_data,1024,"{ \"msgType\": \"challenge\", \"data\": \"%s\", \"mac\": \"%s\", \"encrypt\": %s}",para_token,MAC,para_encrypt==0? "fail":"true");
    tcp_data_handle(tcp_data1,tcp_data);
    
   //int ret=send(sock,tcp_data1,strlen((const char *)tcp_data1),0);
    int ret = lwip_send(sock,tcp_data1,strlen((const char *)tcp_data)+6,0);
    /** 接受 respons 进行校验 **/
#if esp_debug_enable
    if (ret>0)
    {
        memset(buffer, 0, 64);
        snprintf((char *)buffer, 1024, "Send OK\r\n");
        esp_at_port_write_data(buffer, strlen((char *)buffer));
    }

    memset(tcp_data,0,2048);
    memset(buffer, 0, 64);
    recv(sock, tcp_data, 2048, 0);
    //*tcp_data+= '\0';
    tcp_data_analysis(buffer,tcp_data);
    //tcp_data_analysis(buffer,tcp_data);
    //snprintf((char *)buffer, 1024, "get:%s\r\n", tcp_data);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
#endif
    /** 发送校验结果到手机App **/
    memset(tcp_data1,0,1024);
    memset(tcp_data,0,1024);
    snprintf((char *)tcp_data,1024,"{ \"data\": \"success\", \"msgType\": \"verify\"}");
    tcp_data_handle(tcp_data1,tcp_data);
    send(sock,tcp_data1,strlen((const char *)tcp_data)+6,0);
     /** 收到校验成功标志位   **/
    memset(tcp_data,0,2048);
    memset(buffer, 0, 64);
    recv(sock, tcp_data, 2048, 0);
    tcp_data_analysis(buffer,tcp_data);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
    
    /** 发送 sku 到手机App  **/
    memset(tcp_data1,0,1024);
    memset(tcp_data,0,1024);
    snprintf((char *)tcp_data,1024,"{ \"sku\": \"%s\", \"msgType\": \"putsku\"}",para_sku);
    tcp_data_handle(tcp_data1,tcp_data);
    send(sock,tcp_data1,strlen((const char *)tcp_data)+6,0);
    /** 获取错误码       **/
    memset(tcp_data,0,2048);
    memset(buffer, 0, 64);
    recv(sock, tcp_data, 2048, 0);
    tcp_data_analysis(buffer,tcp_data);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
    /** 返回是否有错        **/
    memset(tcp_data1,0,1024);
    memset(tcp_data,0,1024);
    snprintf((char *)tcp_data,1024,"{ \"data\": \"%x\", \"msgType\": \"puterr\"}",0x20);
    tcp_data_handle(tcp_data1,tcp_data);
    send(sock,tcp_data1,strlen((const char *)tcp_data)+6,0);
    /** 获取指令**/
    memset(tcp_data,0,2048);
    memset(buffer, 0, 64);
    recv(sock, tcp_data, 2048, 0);
    tcp_data_analysis(buffer,tcp_data);
    esp_at_port_write_data(buffer, strlen((char *)buffer));
    /** 扫描 AP列表 并发送到手机App **/

    uint16_t numble=5;
    int i=1;
    wifi_ap_record_t ap_info[5];
    esp_wifi_scan_start(NULL,true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&numble,ap_info));
    memset(tcp_data1,0,1024);
    memset(tcp_data,0,1024);
    
    snprintf((char *)tcp_data,1024,
    "{ \"msgType\": \"putssid\", \"data\": { \"total_group_num\": 2, \"part_group_num\": 1, \"list\": [{\"partid\": 1, \"ssid\": \"FAE@Seahi\", \"scrty_type\": 5, \"sig_strength\": -45, \"channel\": 0}, {\"partid\": 2, \"ssid\": \"FAE@Seahi\", \"scrty_type\": 5, \"sig_strength\": -45, \"channel\": 7}]}}");
    
    tcp_data_handle(tcp_data1,tcp_data);
    send(sock,tcp_data1,strlen((const char *)tcp_data)+6,0);
    /** 接受到ssid及password 进行解密**/
    memset(tcp_data,0,2048);
    memset(buffer, 0, 64);
    recv(sock, tcp_data, 2048, 0);
    tcp_data_analysis(buffer,tcp_data);
    esp_at_port_write_data(buffer, strlen((char *)buffer));

    }
    /** 开始连接 AP 并返回连接状态给App**/

    //完成配网
    // 复位
    //解析数字参数
    //解析字符串参数
    free(mac);
    free(tcp_data);
    close(sock);
    close(c_socket);
    esp_wifi_stop();
    return ESP_AT_RESULT_CODE_OK;
}
