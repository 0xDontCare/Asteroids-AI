/**
 * @file asteroidsStructures.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Various data strucutres used for game implementation.
 * @version 0.1
 * @date 31.10.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ASTEROIDS_STRUCTURES_H
#define ASTEROIDS_STRUCTURES_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <raylib.h>
#include <stddef.h>

/**
 * @brief A simple dynamic array implementation.
 *
 */
typedef struct dyn_array_s {
    void **data;
    size_t size;
    size_t capacity;
} DynArray;

//
////  Entities
//

typedef struct asteroid_s {
    int transformID;
    int renderID;
} Asteroid;

typedef struct saucer_s {
    int transformID;
    int renderID;
} Saucer;

typedef struct bullet_s {
    int transformID;
    int renderID;
} Bullet;

typedef struct ship_s {
    int transformID;
    int renderID;
} Ship;

typedef struct game_s {
    // game entities
    Ship ship;
    DynArray bullets;
    DynArray asteroids;
    DynArray saucers;

    // game components
    DynArray transforms;
    DynArray renders;
    DynArray physics;

    // game state
    int score;
    int level;
    int gameOver;
} Game;

//
////  Components
//

typedef struct transform_s {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float rotationSpeed;
} ComponentTransform;

typedef struct render_s {
    float radius;
    Color color;
} ComponentRender;

typedef struct physics_s {
    Rectangle hitbox;
} ComponentPhysics;

//
//// Systems
//

typedef struct system_s {
    void (*update)(Game *game);
} System;

typedef struct system_transform_s {
    System base;
} SystemTransform;

typedef struct system_render_s {
    System base;
} SystemRender;

typedef struct system_physics_s {
    System base;
} SystemPhysics;

//
////  Game
//

/**
 * @brief Initialize game state.
 *
 * @param game Game state to initialize.
 */
void initGame(Game *game);

/**
 * @brief Update game state.
 *
 * @param game Game state to update.
 */
void updateGame(Game *game);

/**
 * @brief Draw game state.
 *
 * @param game Game state to draw.
 */
void drawGame(Game *game);

//
////  Transform
//

/**
 * @brief Initialize transform component.
 *
 * @param transform Transform component to initialize.
 * @param position Initial position.
 * @param velocity Initial velocity.
 * @param rotation Initial rotation.
 * @param rotationSpeed Initial rotation speed.
 */
void initTransform(ComponentTransform *transform, Vector2 position, Vector2 velocity, float rotation, float rotationSpeed);

/**
 * @brief Update transform component.
 *
 * @param transform Transform component to update.
 */
void updateTransform(ComponentTransform *transform);

//
////  Render
//

/**
 * @brief Initialize render component.
 *
 * @param render Render component to initialize.
 * @param radius Initial radius.
 * @param color Initial color.
 */
void initRender(ComponentRender *render, float radius, Color color);

/**
 * @brief Update render component.
 *
 * @param render Render component to update.
 */
void updateRender(ComponentRender *render);

//
////  Physics
//

/**
 * @brief Initialize physics component.
 *
 * @param physics Physics component to initialize.
 * @param hitbox Initial hitbox.
 */
void initPhysics(ComponentPhysics *physics, Rectangle hitbox);

/**
 * @brief Update physics component.
 *
 * @param physics Physics component to update.
 */
void updatePhysics(ComponentPhysics *physics);

//
////  Systems
//

/**
 * @brief Initialize transform system.
 *
 * @param system Transform system to initialize.
 */
void initSystemTransform(SystemTransform *system);

/**
 * @brief Update transform system.
 *
 * @param system Transform system to update.
 * @param game Game state to update.
 */
void updateSystemTransform(SystemTransform *system, Game *game);

/**
 * @brief Initialize render system.
 *
 * @param system Render system to initialize.
 */
void initSystemRender(SystemRender *system);

/**
 * @brief Update render system.
 *
 * @param system Render system to update.
 * @param game Game state to update.
 */
void updateSystemRender(SystemRender *system, Game *game);

/**
 * @brief Initialize physics system.
 *
 * @param system Physics system to initialize.
 */
void initSystemPhysics(SystemPhysics *system);

/**
 * @brief Update physics system.
 *
 * @param system Physics system to update.
 * @param game Game state to update.
 */
void updateSystemPhysics(SystemPhysics *system, Game *game);

//
////  Utils
//

/**
 * @brief Create a new dynamic array.
 *
 * @param capacity Initial capacity.
 * @return DynArray* Pointer to the new dynamic array.
 */
DynArray *newDynArray(size_t capacity);

/**
 * @brief Destroy a dynamic array.
 *
 * @param dynArray Dynamic array to destroy.
 */
void destroyDynArray(DynArray *dynArray);

/**
 * @brief Add an element to a dynamic array.
 *
 * @param dynArray Dynamic array to add to.
 * @param element Element to add.
 */
void dynArrayAdd(DynArray *dynArray, void *element);

/**
 * @brief Remove an element from a dynamic array.
 *
 * @param dynArray Dynamic array to remove from.
 * @param index Index of the element to remove.
 */
void dynArrayRemove(DynArray *dynArray, size_t index);

/**
 * @brief Get an element from a dynamic array.
 *
 * @param dynArray Dynamic array to get from.
 * @param index Index of the element to get.
 * @return void* Pointer to the element.
 */
void *dynArrayGet(DynArray *dynArray, size_t index);

/**
 * @brief Get the size of a dynamic array.
 *
 * @param dynArray Dynamic array to get the size of.
 * @return size_t Size of the dynamic array.
 */
size_t dynArraySize(DynArray *dynArray);

/**
 * @brief Get the capacity of a dynamic array.
 *
 * @param dynArray Dynamic array to get the capacity of.
 * @return size_t Capacity of the dynamic array.
 */
size_t dynArrayCapacity(DynArray *dynArray);

/**
 * @brief Get the last element of a dynamic array.
 *
 * @param dynArray Dynamic array to get the last element of.
 * @return void* Pointer to the last element.
 */
void *dynArrayLast(DynArray *dynArray);

/**
 * @brief Get the first element of a dynamic array.
 *
 * @param dynArray Dynamic array to get the first element of.
 * @return void* Pointer to the first element.
 */
void *dynArrayFirst(DynArray *dynArray);

/**
 * @brief Filter a dynamic array.
 *
 * @param dynArray Dynamic array to filter.
 * @param filter Filter function.
 * @return DynArray* Filtered dynamic array.
 */
DynArray *dynArrayFilter(DynArray *dynArray, int (*filter)(void *element));

/**
 * @brief Map a dynamic array.
 *
 * @param dynArray Dynamic array to map.
 * @param map Map function.
 * @return DynArray* Mapped dynamic array.
 */
DynArray *dynArrayMap(DynArray *dynArray, void *(*map)(void *element));

/**
 * @brief Reduce a dynamic array.
 *
 * @param dynArray Dynamic array to reduce.
 * @param reduce Reduce function.
 * @param initial Initial value.
 * @return void* Reduced value.
 */
void *dynArrayReduce(DynArray *dynArray, void *(*reduce)(void *accumulator, void *element), void *initial);

/**
 * @brief Find an element in a dynamic array.
 *
 * @param dynArray Dynamic array to find in.
 * @param find Find function.
 * @return void* Pointer to the found element.
 */
void *dynArrayFind(DynArray *dynArray, int (*find)(void *element));

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // ASTEROIDS_STRUCTURES_H