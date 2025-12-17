//
// Created by screemer on 2018-12-16.
//

#include "ecm_helper.h"
#include <iostream>
#include "../log.h"
#include "../dir_entry.h"

using namespace std;

extern "C" {
void eccedc_init(void);
int unecmify(FILE *in, FILE *out);
}

// ECM files are Error Code Modeler files used in disc images.
// https://www.lifewire.com/ecm-file-2620956
// this class removes them from the bin files to save space.

//*******************************
// Ecmhelper::unecm
//*******************************
bool Ecmhelper::unecm(string input, string output) {
    PLOG_DEBUG << "Unpacking: " << input << " to " << output;
    if (!DirEntry::matchExtension(output, EXT_BIN)) {
        output = output + ".bin";
    }
    eccedc_init();
    FILE *fin = fopen(input.c_str(), "rb");
    if (!fin) {
        return false;
    }
    FILE *fout = fopen(output.c_str(), "wb");
    if (!fout) {
        fclose(fin);
        return false;
    }
    int result = unecmify(fin, fout);
    fclose(fout);
    fclose(fin);
    return result == 0;
}