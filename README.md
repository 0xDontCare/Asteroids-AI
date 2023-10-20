# Neural Asteroids 
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
### Phase 00
- [ ] Window creation
- [ ] Game loop tick and render tick separation
- [ ] Game loop timing
- [ ] Game loop exit

### Phase 10
- [ ] Add player
- [ ] Player movement
- [ ] Screen wrapping

### Phase 11
- [ ] Add shared memory for game state
- [ ] Separate logic and rendering threads
- [ ] Thread synchronization

### Phase 12
- [ ] Add asteroids
- [ ] Asteroid collision
- [ ] Asteroid movement
- [ ] Add bullets
- [ ] Bullet collision
- [ ] Bullet movement

### Phase 13
- [ ] Add score
- [ ] Add time counter
- [ ] Add game over screen
- [ ] Add sound (optional)
- [ ] Add music (optional)

### Phase 20
- [ ] Neural network architecture modelling
- [ ] Random agent (random actions)

### Phase 21
- [ ] Fitness function (score, time)
- [ ] Serialization of neural network data
- [ ] Deserialization of neural network data

### Phase 30
- [ ] Implement genetic algorithm
- [ ] Add training mode
- [ ] Montage of training generations (optional)