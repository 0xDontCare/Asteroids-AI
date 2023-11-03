/**
 * @file asteroidsObjects.c
 * @author 0xDontCare (you@domain.com)
 * @brief Implementation of functions from asteroidsObjects.h
 * @version 0.1
 * @date 21.10.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "asteroidsObjects.h"

#include <raylib.h>
#include <raymath.h>

void DrawPlayer(ComponentMotion *motionComponent, ComponentRotation *rotationComponent, ComponentCollision *collisionComponent) {
    DrawRectanglePro((Rectangle){motionComponent->position.x, motionComponent->position.y, collisionComponent->hitbox.x, collisionComponent->hitbox.y}, (Vector2){collisionComponent->hitbox.x / 2, collisionComponent->hitbox.y / 2}, rotationComponent->rotation * RAD2DEG, (Color){255, 0, 0, 255});
}
