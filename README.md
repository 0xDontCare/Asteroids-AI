# Asteroids-AI
A classical arcade game implemented in C using Raylib. The game is created as platform for training neural network agents.

## Installation
### Linux
1. Install [Raylib](https://github.com/raysan5/raylib)
2. Clone the repository
3. Run `make build` in the root directory of the repository
4. Run `./bin/asteroids`

### Windows
- Not available yet (easiest way would be to use WSL for now)

## Controls
- No controls implemented yet

## Development roadmap
### Phase 0.0
- [x] Window creation
- [x] Game loop tick and render tick separation
- [x] Game loop timing
- [x] Game loop exit

### Phase 1.0
- [x] Add player
- [x] Player movement
- [x] Screen wrapping

### Phase 1.1
- [ ] Segment program source code into separate files
- [ ] Add shared memory for game state
- [ ] Separate logic and rendering threads
- [ ] Thread synchronization

### Phase 1.2
- [ ] Add asteroids
- [ ] Asteroid collision
- [ ] Asteroid movement
- [ ] Add bullets
- [ ] Bullet collision
- [ ] Bullet movement

### Phase 1.3
- [ ] Structure code into more separate files
- [ ] Sprites for player, asteroids and bullets

### Phase 1.4
- [ ] Add score
- [ ] Add time counter
- [ ] Add game over screen
- [ ] Add sound effects (optional)

### Phase 2.0
- [ ] Neural network architecture model
- [ ] Random agent (random actions)

### Phase 2.1
- [ ] Fitness function (score, time)
- [ ] Serialization of neural network data
- [ ] Deserialization of neural network data

### Phase 3.0
- [ ] Implement genetic algorithm
- [ ] Add training mode
- [ ] Montage of training generations (optional)