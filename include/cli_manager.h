#include "inttypes.h"
#include "tcp_c_events.h"

#define CLI_PORT                        27015
#define CLI_KEEPALIVE_IDLE              5               ///< TCP keep-alive idle time(s).
#define CLI_KEEPALIVE_INTERVAL          5               ///< TCP keep-alive interval time(s).
#define CLI_KEEPALIVE_COUNT             3               ///< TCP keep-alive packet retry send counts.
#define CLI_CMD_INTS_NUMBER             2               ///< Number of ints used to indicate command in received data.
#define CLI_CMD_ARGS_NUMBER             64              ///< Number of chars used to indicate arguments in received data.
#define CLI_CMD_MAX_COMMANDS            64              ///< Max number of commands to register.

#define CLI_ERR_TOO_MANY_CMD            12

struct cli_cmd_obj {
  uint8_t cmd_num[CLI_CMD_INTS_NUMBER];
  char args[CLI_CMD_ARGS_NUMBER];
};

typedef struct cli_cmd_obj cli_cmd_t;

struct cli_cmd_config_obj {
  uint8_t cmd_num[CLI_CMD_INTS_NUMBER];
  char args[CLI_CMD_ARGS_NUMBER];
  void (*cmd_fun)(void*);
};

typedef struct cli_cmd_config_obj cli_cmd_config_t;



int cli_manager_init(void);

int cli_manager_end_transmission(void);

int cli_manager_add_log(char* log);

int cli_manager_log(char* log);

/**
 * @brief Used to register commands and callback function to be executed.
 * 
 * @warning Try not to use vTaskDelay inside callback function,
 * this may result in undefined behaviour as it's called from cli_manager task.
 * 
 * 
 * @param cmd1 First character to check as command.
 * @param cmd2 Second character to check as command.
 * @param cmd_fun Callback function to execute when both commands are true.
 * @return int 
 */
int cli_manager_register_command(char cmd1, char cmd2, void (*cmd_fun)(void* args));

int cli_manager_read_from_rec_buf(char space_to_read[], uint16_t space_to_read_len);

int cli_manager_add_to_send_buf(const char* to_write);

int cli_manager_write_to_send_buf(const char to_write[], uint16_t to_write_len);