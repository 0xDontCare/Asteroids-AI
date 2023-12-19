# Asteroids-AI
Asteroids-AI is a project that aims to create an artificial intelligence agent that can play the classic arcade game Asteroids. The agent will be trained using a genetic algorithm and will be implemented using a neural network. The project is written in C and uses the [Raylib](https://github.com/raysan5/raylib) library for graphics and input. The project is divided into three parts:
- Game: The game itself that can be played by a human player or controlled by a neural network agent
- Neural network agent: The neural network agent that controls the game
- Management program: A program that manages the training of the neural network agent

## Detailed description
### Game
The game is a classical arcade game where the player controls a spaceship and has to destroy asteroids. The player can move the spaceship in all directions and shoot bullets. The asteroids move in random directions and wrap around the screen edges. When a bullet hits an asteroid, the asteroid is destroyed and the player gets points. When an asteroid hits the spaceship, the spaceship is destroyed and the game is over. For training purposes, the game has shared memory interface that allows the game to be controlled by a neural network agent and has ability to be started in headless mode. The game can be run in three different modes:
- Normal mode: The game is controlled by the player (shared memory is disabled)
- Neural network mode: The game is controlled by a neural network agent (shared memory is enabled and keyboard input is disabled, except for the escape key which exits the game)
- Training mode: The game is controlled by a neural network agent (shared memory is enabled and keyboard input is disabled, except for the escape key which exits the game)

### Neural network agent
To be implemented soon...

### Management program
Management program is currently responsible for creating shared memory between game and external programs as well as starting instance of a game. Once game stops running, it will free shared memory from the system.

## Installation
### Linux
1. Install [Raylib](https://github.com/raysan5/raylib)
2. Clone the repository
3. Run `make build` in the root directory of the cloned repository

### Windows
- Not available (easiest way would be to use WSL or virtual machine for now)

## Controls
- `W` - Move forward
- `A` - Rotate left
- `D` - Rotate right
- `<n/a>` - Shoot

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
- [x] Segment program source code into separate files
- [x] Add shared memory for game state
- [x] Separate logic and rendering logic
- [ ] Basic manager program
- [ ] Headless mode (partially implemented, control over shared input needs testing)
- [ ] Signal based exiting
- [ ] Thread synchronization

### Phase 1.2
- [x] Add asteroids
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
- [ ] Add level counter
- [ ] Add game over screen
- [ ] Add sound effects (optional)

### Phase 2.0
- [ ] Neural network architecture model
- [ ] Random agent (random actions)

### Phase 2.1
- [ ] Training management program
- [ ] Running multiple agents and games in parallel
- [ ] Evaluation of agents (score, time, level)

### Phase 3.0
- [ ] Fitness function (score, time)
- [ ] Serialization of neural network data
- [ ] Deserialization of neural network data

### Phase 3.1
- [ ] Implement genetic algorithm
- [ ] Add training mode
- [ ] Montage of training generations (optional)