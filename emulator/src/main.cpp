#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <unistd.h>
#include <termios.h>
#include "Emulator.h"

using namespace std;

#define SLEEP_TIME_MS 500

Emulator *emulator;

void timerTick() {
    while (!Emulator::finishExecution) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));      
    
        Emulator::isTimerTick = true;
    }
}

void stdInput() {
    struct termios old_tio, new_tio;
    unsigned char c;

    /* get the terminal settings for stdin */
    tcgetattr(STDIN_FILENO,&old_tio);

    /* we want to keep the old setting to restore them a the end */
    new_tio=old_tio;

    /* disable canonical mode (buffered i/o) and local echo */
    new_tio.c_lflag &=(~ICANON & ~ECHO);

    /* set the new settings immediately */
    tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);

    do {
        c = getchar();
        cout << c;
        emulator->writeByte(c, 32 * 4 + 1);
        Emulator::isInputInterrupt = true;
        Emulator::isReadInput = false;
    } while(!Emulator::finishExecution);

    /* restore the former settings */
    tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
}

int main() {
    emulator = new Emulator("../../assembler/src/izlaz.txt");

    Emulator::finishExecution = false;

    cout << "EMULATOR STARTED: " << endl;

    thread timerTickThread(timerTick);
    thread stdInputThread(stdInput);

    emulator->readInputStructures();
    bool result = emulator->fillMemory();

    if (!result) {
        cout << emulator->getErrorDescription() << endl;

        timerTickThread.join();
        stdInputThread.join();

        return -1;
    }

    emulator->correctRelocationMemory();

    result = emulator->execute();


    if (!result) {
        cout << emulator->getErrorDescription() << endl;
        
        timerTickThread.join();
        stdInputThread.join();

        return -1;
    }

    timerTickThread.join();
    stdInputThread.join();

    return 0;
}