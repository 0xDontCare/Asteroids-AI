/**
 * @file neuronsMain.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Neural network program related enums, structs, etc.
 * @version 0.1
 * @date 06.01.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 */

#ifndef MAIN_H
#define MAIN_H

// ------------------------------------------------------------------
// neural network enum definitions

/* Command line flags:
 * 0x01 - help
 * 0x02 - version
 * 0x04 - standalone mode (+2 parameters)
 * 0x08 - managed mode (+3 parameters)
 * 0x10 - load config file (+1 parameter)
 */
enum cmdFlag_e {
    CMD_FLAG_NONE = 0x00,
    CMD_FLAG_HELP = 0x01,
    CMD_FLAG_VERSION = 0x02,
    CMD_FLAG_STANDALONE = 0x04,
    CMD_FLAG_MANAGED = 0x08,
    CMD_FLAG_LOADCFG = 0x10
};

/* Runtime flags of neural network program:
 * 0x01 - running
 * 0x02 - paused
 * 0x04 - exit
 */
enum neuronsRuntime_e {
    RUNTIME_NONE = 0x00,
    RUNTIME_RUNNING = 0x01,
    RUNTIME_PAUSED = 0x02,
    RUNTIME_EXIT = 0x04
};

// ------------------------------------------------------------------
// neural network constant definitions
#define ACTIVATION_THRESHOLD 0.5f  // threshold for binary activation of network output

#endif  // MAIN_H