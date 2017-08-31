#include <iostream>
#include <thread>
#include "Emulator.h"

using namespace std;

#define SLEEP_TIME_MS 100

void timerTick() {
    while (!Emulator::finishExecution) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
        cout << "SPAVAO SAM 100ms" << endl;        
    }
}

void stdInput() {
    while (!Emulator::finishExecution) {
        char character;

        cin >> character;
    }
}

int main() {
    Emulator *emulator = new Emulator("../../assembler/src/izlaz.txt");

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