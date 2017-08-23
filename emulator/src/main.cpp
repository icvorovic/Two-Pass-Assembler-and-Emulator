#include <iostream>
#include "Emulator.h"

using namespace std;

int main() {
    Emulator *emulator = new Emulator("../../assembler/src/izlaz.txt");

    cout << "EMULATOR STARTED: " << endl;
 
    emulator->readInputStructures();
    bool result = emulator->fillMemory();

    result = emulator->execute();

    if (!result) {
        cout << emulator->getErrorDescription() << endl;
    }

    return 0;
}