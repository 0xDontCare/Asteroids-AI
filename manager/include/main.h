/**
 * @file main.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Manager program related enums, structs, etc.
 * @version 0.1
 * @date 13.12.2023.
 *
 * @copyright All rights reserved (c) 2023
 *
 */

#ifndef MAIN_H
#define MAIN_H

enum runtimeFlags_e {
    RUNTIME_UNSET = 0x00,
    RUNTIME_SINGLE_INSTANCE = 0x01,
    RUNTIME_MULTI_INSTANCE = 0x02,
    RUNTIME_EXIT = 0x04,
    RUNTIME_TRAINING = 0x08,
    RUNTIME_SHARED_MEMORY_READY = 0x10
};

enum instanceFlags_e {
    INSTANCE_UNSET = 0x00,
    INSTANCE_USING_GAME = 0x01,
    INSTANCE_USING_AI = 0x02,
    INSTANCE_HAS_SHARED_MEMORY = 0x04,
    INSTANCE_IS_RUNNING = 0x08,
    INSTANCE_IS_PAUSED = 0x10,
    INSTANCE_IS_HEADLESS = 0x20,
    INSTANCE_HAS_ENDED = 0x40
};

#endif /* MAIN_H */