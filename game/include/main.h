/**
 * @file main.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Game related enums, structs, etc.
 * @version 0.1
 * @date 11.12.2023.
 *
 * @copyright All rights reserved (c) 2023
 *
 */

#ifndef MAIN_H
#define MAIN_H

/* Command line flags:
 * 0x01 - help
 * 0x02 - version
 * 0x04 - standalone mode
 * 0x08 - headless mode
 * 0x10 - use neural network (+2 parameters)
 * 0x20 - managed mode (+3 parameters)
 */
enum cmdFlag_e {
    CMD_FLAG_NONE = 0x00,
    CMD_FLAG_HELP = 0x01,
    CMD_FLAG_VERSION = 0x02,
    CMD_FLAG_STANDALONE = 0x04,
    CMD_FLAG_HEADLESS = 0x08,
    CMD_FLAG_USE_NEURAL = 0x10,
    CMD_FLAG_MANAGED = 0x20,
};

/* Control input flags
 * 0x01 - W
 * 0x02 - A
 * 0x04 - D
 * 0x08 - SPACE
 * 0x10 - EXIT
 */
enum input_e {
    INPUT_NONE = 0x00,
    INPUT_W = 0x01,
    INPUT_A = 0x02,
    INPUT_D = 0x04,
    INPUT_SPACE = 0x08,
    INPUT_EXIT = 0x10
};

/* Game runtime flags
 * 0x01 - running
 * 0x02 - paused
 * 0x04 - exit
 * 0x08 - window active
 */
enum gameRuntime_e {
    RUNTIME_NONE = 0x00,
    RUNTIME_RUNNING = 0x01,
    RUNTIME_PAUSED = 0x02,
    RUNTIME_EXIT = 0x04,
    RUNTIME_WINDOW_ACTIVE = 0x08
};

#endif  // MAIN_H