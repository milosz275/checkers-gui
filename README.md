# Checkers GUI
This is a functional C++ checkers game that utilizes the SFML library. The game supports Player vs Player and Player vs Bot gameplay. The bot uses the MiniMax algorithm with alpha-beta pruning to determine the best move.

## Project structure
The project is structured as follows:
- checkers-gui/: Main project directory. Contains source and header directories, Visual Studio files, a Makefile, Class Diagram and game state written into txt files.
- checkers-gui/include/: Contains the header files for the game.
- checkers-gui/src/: Contains the source files for the game.
- dependencies/: Contains the SFML library used in the game.

## Building the Project
To build the project, you can start it in Visual Studio by cloning the project and opening the .sln file. Then you can select DEBUG or RELEASE modes on the top bar and finally build with Ctrl+F5.

Alternatively, you can use the provided Makefile in the checkers-gui/ directory:

```bash
cd checkers-gui/
make
```

or

```bash
cd checkers-gui/
make debug
```

This will generate the executable main/main.exe file depending on the platform.

## Running the Game
After building the project, you can run the game with the following command:

```bash
./main
```

Enjoy the game!
