# Asteroids-AI
Asteroids-AI is a project that aims to create an artificial intelligence agent that can play the classic arcade game Asteroids. The agent will be trained using a genetic algorithm and will be implemented using a feedforward neural network. The project is fully written in C and uses the [Raylib](https://github.com/raysan5/raylib) library for graphics and input. There are three mayor parts to this project:
- Game: The game itself that can be played by a human player or controlled by a neural network agent via IPC
- Neural network agent: The neural network agent that controls the game
- Management program: A program that is capable of starting and managing multiple instances of the game and neural network agent pairs for the purpose of training the agent as well as evaluating its performance

## Detailed description
### Game
The game is just like the classical arcade game, where the player controls a spaceship and has to destroy asteroids. Player can move the spaceship forwards, turn left or right and shoot bullets. The asteroids start at random positions with random directions and wrap around the screen edges as they move. When a bullet hits an asteroid, the asteroid is destroyed and the player gets points (more points with smaller asteroids being hit). When an asteroid hits the spaceship, the spaceship is destroyed and the game is over. For training purposes, game has shared memory interface that allows the game to be controlled by a neural network agent and has ability to be started in headless mode. The game can be run in three different modes:
- Normal (standalone) mode: The game is controlled by the player (shared memory is disabled)
- Neural network standalone mode: The game runs same as regular standalone mode, but neural network takes control of the spaceship (shared memory is enabled and managed by game program)
- Managed mode: The game is started by the management program along with neural network agent which controls the spaceship (shared memory is enabled and managed by management program)

### Neural network agent
Neural network agent comes as standalone program which connects to the game using shared memory interface. It is nothing more than a feedforward neural network which constantly evaluates game output and sends its input to the game until terminated. The neural network model is trained using genetic algorithm which is implemented as part of the management program. The agent can not be run standalone because it requires shared memory keys to be passed as arguments on startup. There are two "modes" in which the agent can be started:
- Random agent: The agent initializes its weights and biases to random values with fixed architecture (5-32-4 from input to output layer)
- Loaded agent: The agent loads its model from a specially formatted file which contains all the information about layers, weights and biases

### Management program
Management program is responsible for starting and handling multiple instances of the game-agent pairs running in parallel. It is capable of creating initial (random) generation of agents, evaluating their performance and creating new generations based on the best performing individuals. The management program is also responsible for creating shared memory keys, handling them and passing them to the game and agent programs to work in sync. The program is implemented as shell interface with multiple commands that can be used to control the training process. For list of available commands and their usage, run `help` command within the management program.

## Installation
### Linux
1. Install [Raylib](https://github.com/raysan5/raylib)
2. Clone the repository
3. Run `make all` in the root directory of the cloned repository
4. Run any of compiled programs (except management) with flag `--help` to see available options

### Windows
- Not available (project currently heavily relies on POSIX functions which are not available on Windows, easiest way to run the project on Windows is to use WSL or a virtual machine with Linux installed)

## Game controls (in standalone mode)
- `W` - Move forward
- `A` - Rotate left
- `D` - Rotate right
- `Space` - Shoot
- `Esc` - Exit game

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

### Phase 3.0
- [x] Network serialization and deserialization
- [x] Management program
- [x] Creation of initial generation
- [x] Running multiple agents and games in parallel
- [x] Fitness function (score, time)
- [x] Evaluation of agents (score, time, level)
- [x] Crossover mechanism
- [x] Mutation mechanism
- [x] Creating new networks from previous generation

### Optionals 1.1
- [ ] Add game menu screen
- [ ] Sprites for player, asteroids and bullets
- [ ] Add game sound effects
