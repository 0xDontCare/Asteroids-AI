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
Management program is currently responsible for creating shared memory between game and external programs as well as starting instance of a game. Once game stops running, it will free shared memory from the system. Further development will be continued in phase 3.0 onwards and is for now halted (currently implemented functionality is not guaranteed to work as game and neural network agent are developed in meantime).

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
- [x] Basic manager program
- [x] Headless mode (for game)
- [x] Signal based exiting (manager to game)

### Phase 1.2
- [x] Add asteroids
- [x] Asteroid collision
- [x] Asteroid movement
- [x] Add bullets
- [x] Bullet collision
- [x] Bullet movement

### Phase 1.3
- [x] Structure code into more separate files (code had multiple restructurings done already -> skipped)
- [x] Add game over screen
- [x] Add score
- [x] Add time counter
- [x] Add level counter

### Phase 2.0
- [x] Neural network architecture model
- [x] Random agent (random actions)

### Phase 2.1
- [ ] Management program
- [ ] Network serialization and deserialization
- [ ] Running multiple agents and games in parallel
- [ ] Creation of initial generation
- [ ] Evaluation of agents (score, time, level)

### Phase 3.0
- [ ] Fitness function (score, time)
- [ ] Crossover mechanism
- [ ] Mutation mechanism
- [ ] Creating new networks from previous generation

### Optionals 1.1
- [ ] Add game menu screen
- [ ] Sprites for player, asteroids and bullets
- [ ] Add game sound effects

### Optionals 2.1
- [ ] Montage of training generations (optional)