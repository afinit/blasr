// Author: Yuan Li

#include <unistd.h> // getcwd
#include <iostream>
#include <memory>

#include "Bam2Bax.h"
#include "Converter.h"

using namespace std;

int Bam2Bax::Run(Settings& settings) {

    bool success = false;
    Converter converter(settings);

    if (converter.Run()) {
        success = true;
    }

    // return success/fail
    if (success)
        return EXIT_SUCCESS;
    else {
        for (const string& e : converter.Errors())
            cerr << "ERROR: " << e << endl;
        return EXIT_FAILURE;
    }
}
