#include "inttypes.h"

#define CLI_PORT                        27015
#define CLI_KEEPALIVE_IDLE              5               ///< TCP keep-alive idle time(s).
#define CLI_KEEPALIVE_INTERVAL          5               ///< TCP keep-alive interval time(s).
#define CLI_KEEPALIVE_COUNT             3               ///< TCP keep-alive packet retry send counts.
#define CLI_CMD_INTS_NUMBER             2               ///< Number of ints used to indicate command in received data.
#define CLI_CMD_ARGS_NUMBER             64              ///< Number of chars used to indicate arguments in received data.
#define CLI_CMD_MAX_COMMANDS            64              ///< Max number of commands to register.

#define CLI_ERR_TOO_MANY_CMD            12

enum cli_cmd_list_t {
    CLI_CMD_SCAN = 49,
    CLI_CMD_SET_STA_SSID,
    CLI_CMD_SET_STA_PASS,
};

struct cli_cmd_obj {
  uint8_t cmd_num[CLI_CMD_INTS_NUMBER];
  char args[CLI_CMD_ARGS_NUMBER];
};

typedef struct cli_cmd_obj cli_cmd_t;

struct cli_cmd_config_obj {
  uint8_t cmd_num[CLI_CMD_INTS_NUMBER];
  char args[CLI_CMD_ARGS_NUMBER];
  void (*cmd_fun)(void);
};

typedef struct cli_cmd_config_obj cli_cmd_config_t;



int cli_manager_init(void);

int cli_manager_register_command(char cmd1, char cmd2, char args[64], void (*cmd_fun)(void));