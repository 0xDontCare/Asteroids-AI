/**
 * @file asteroidsObjects.h
 * @author 0xDontCare (you@domain.com)
 * @brief Game object structures required for asteroids game.
 * @version 0.1
 * @date 21.10.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ASTEROIDS_OBJECTS_H
#define ASTEROIDS_OBJECTS_H

#include <raylib.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// components
//

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
} ComponentMotion;

typedef struct {
    float rotation;
    float rotationSpeed;
} ComponentRotation;

typedef struct {
    int isAlive;
    float lifeTime;
} ComponentLifeTime;

typedef struct {
    Vector2 hitbox;
} ComponentCollision;

//
// entities
//

typedef struct {
    int movementID;
    int rotationID;
    int accelerationID;
    int hitboxID;
} PlayerObject;

typedef struct {
    int movementID;
    int rotationID;
    int hitboxID;
} AsteroidObject;

typedef struct {
    int movementID;
    int hitboxID;
    int lifeTimeID;
} BulletObject;

// function declarations
void DrawPlayer(ComponentMotion *motionComponent, ComponentRotation *rotationComponent, ComponentCollision *collisionComponent);

#ifdef __cplusplus
}
#endif

#endif  // ASTEROIDS_OBJECTS_H