#include <iostream>
#include "Emulator.h"

using namespace std;

int main() {
    Emulator *emulator = new Emulator("../../assembler/src/izlaz.txt");

    emulator->readInputStructures();

    cout << "EMULATOR STARTED: " << endl;

    return 0;
}