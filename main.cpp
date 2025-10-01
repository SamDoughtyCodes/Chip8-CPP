#include <iostream>
#include <chrono>
#include "Chip8.h"
#include "Platform.h"
using namespace std;

const unsigned int VIDEO_HEIGHT = 32;           // Stores height of the display
const unsigned int VIDEO_WIDTH = 64;            // Stores width of the display

// Called when run
/* CLI ARGS:
    1 - The file to run (this file)
    2 - The scale to increase the display size by
    3 - Delay (essentially clock speed, the time between instructions)
    4 - ROM file to open
*/
int main(int argc, const char* argv[]) {
    // argc: Number of command line args
    // argv: Pointer to array of command line arguaments
    if (argc != 4) {  // There must be 4 command line args (3 for the games, 1 for the file itself)
        cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";  // Output error message for wrong num of args
        exit(EXIT_FAILURE);  // Stop the program
    }

    // Store args
    int videoScale = stoi(argv[1]);  // Stoi: Cast string to int
    int cycleDelay = stoi(argv[2]);
    char const* romFilename = argv[3];

    // Instantiate platform layer
    Platform platform(
        "Chip-8 Emulator",
        VIDEO_WIDTH * videoScale,
        VIDEO_HEIGHT * videoScale,
        VIDEO_WIDTH,
        VIDEO_HEIGHT
    );

    // Instantiate emulator
    Chip8 chip8;
    chip8.LoadROM(romFilename);

    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;
    auto lastCycleTime = chrono::high_resolution_clock::now();  // Get the current time
    bool quit = false;

    while (!quit) {  // Keep iterating until the user quits
        quit = platform.ProcessInput(chip8.keypad);
        auto currentTime = chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::milli>(currentTime - lastCycleTime).count();  // Get the time since last instruction in milliseconds

        if (dt > cycleDelay) {  // If it is time to do an instruction
            lastCycleTime = currentTime;
            chip8.Cycle();
            platform.Update(chip8.video, videoPitch);
        }
    }

    return 0;
}