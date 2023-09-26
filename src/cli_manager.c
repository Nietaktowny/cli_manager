#include "cli_manager.h"
#include "err_controller.h"
#include "wifi_controller.h"
#include "tcp_controller.h"

#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "freertos/task.h"

#define MY_SSID "TP-LINK_AD8313"
#define MY_PSK "20232887"

static cli_cmd_t cli_read_cmd(const char recbuf[]);

static const char* TAG = "cli_manager";
static cli_cmd_config_t cmd_configs[CLI_CMD_MAX_COMMANDS];
static uint16_t last_cmd = 0;

static EventGroupHandle_t tcp_event_group;
static char receive_buffer[TCP_C_RECEIVE_BUFLEN];
static char send_buffer[TCP_C_SEND_BUFLEN];

static void cli_set_sta_ssid(void) {
    memset(send_buffer, 0, sizeof(send_buffer));
    ESP_LOGI(TAG, "TODO set sta SSID.");
    memcpy(send_buffer, "TODO set sta SSID.", strlen("TODO set sta SSID."));
}

static void cli_set_sta_password(void) {
    memset(send_buffer, 0, sizeof(send_buffer));
    ESP_LOGI(TAG, "TODO set sta password.");
    memcpy(send_buffer, "TODO set sta password.", strlen("TODO set sta password."));
}

static void cli_no_command(void) {
    memset(send_buffer, 0, sizeof(send_buffer));
    ESP_LOGI(TAG, "No command found");
    memcpy(send_buffer, "No command found", strlen("No command found"));
    xEventGroupSetBits(tcp_event_group, TCP_C_DATA_READY_TO_SEND);
}

static void cli_do_scan(void) {
    wifi_c_scan_result_t scan_result;
    char temp_buf[312];
    wifi_c_scan_all_ap(&scan_result);

    memset(send_buffer, 0, sizeof(send_buffer));
    memset(temp_buf, 0, sizeof(temp_buf));

    strcat(&send_buffer[0], "Scanned APs:");
    wifi_c_store_scanned_ap(temp_buf, 312);

    strncat(&send_buffer[12], &temp_buf[0], strlen(temp_buf));
    xEventGroupSetBits(tcp_event_group, TCP_C_DATA_READY_TO_SEND);
}

static cli_cmd_t cli_read_cmd(const char recbuf[]) {
    cli_cmd_t cmd;
    memcpy(&cmd.cmd_num, &recbuf[0], CLI_CMD_INTS_NUMBER);
    memcpy(&cmd.args, &recbuf[CLI_CMD_INTS_NUMBER], CLI_CMD_ARGS_NUMBER);
    ESP_LOGD(TAG, "Received commands: %d %d", cmd.cmd_num[0], cmd.cmd_num[1]);
    ESP_LOGD(TAG, "Received arguments: %s", cmd.args);
    return cmd;
}

int cli_manager_register_command(char cmd1, char cmd2, char args[64], void (*cmd_fun)(void)) {
    err_c_t err = ERR_C_OK;

    if(last_cmd < CLI_CMD_MAX_COMMANDS) {
        cli_cmd_config_t new_cmd;
        new_cmd.cmd_fun = cmd_fun;
        new_cmd.cmd_num[0] = cmd1;
        new_cmd.cmd_num[1] = cmd2;
        if(args != NULL)
            memcpy(&new_cmd.args, &args, sizeof(new_cmd.args));
        cmd_configs[last_cmd] = new_cmd;
        last_cmd++;
    } else {
        err = CLI_ERR_TOO_MANY_CMD;
    }
    return err;
} 

static void cli_do_cmd(cli_cmd_t* cmd) {
    bool found_one = false;
    for (uint16_t i = 0; i < last_cmd; i++)
    {
        if(cmd->cmd_num[0] == cmd_configs[i].cmd_num[0] && cmd->cmd_num[1] == cmd_configs[i].cmd_num[1]) {
            found_one = true;
            cmd_configs[i].cmd_fun();
        }
    }
    if (found_one == false)
    {
        cli_no_command();
    }
}

void tcp_server_listen_task(void* args) {
    tcp_c_server_loop();
}

void cli_manager_task(void* args) {
    cli_cmd_t cmd;
    while(1) {
        xEventGroupWaitBits(tcp_event_group, TCP_C_ACCEPTED_SOCKET_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        tcp_c_receive(receive_buffer);
        ESP_LOGD(TAG, "%s", receive_buffer);

        cmd = cli_read_cmd(receive_buffer);
        cli_do_cmd(&cmd);
        ESP_LOGD(TAG, "Data to send: %s", send_buffer);

        xEventGroupWaitBits(tcp_event_group, TCP_C_DATA_READY_TO_SEND, pdTRUE, pdFALSE, pdMS_TO_TICKS(4000));
        //memcpy(send_buffer, receive_buffer, sizeof(receive_buffer));
        tcp_c_send(send_buffer, strlen(send_buffer));
        xEventGroupSetBits(tcp_event_group, TCP_C_FINISHED_TRANSMISSION);
    }
}

int cli_manager_init(void) {
    tcp_event_group = xEventGroupCreate();

    wifi_c_init_wifi(WIFI_C_MODE_STA);
    wifi_c_start_sta(MY_SSID, MY_PSK);
    
    tcp_c_start_tcp_server(tcp_event_group);

    xTaskCreate(tcp_server_listen_task, "tcp_task", 2048, NULL, 2, NULL);
    xTaskCreate(cli_manager_task, "cli_task", 2048, NULL, 3, NULL);

    cli_manager_register_command('1', '0', NULL, cli_do_scan);
    cli_manager_register_command('2', '0', NULL, cli_set_sta_ssid);
    cli_manager_register_command('3', '0', NULL, cli_set_sta_password);
    return ERR_C_OK;
}