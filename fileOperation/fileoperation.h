#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include "hf_types.h"

using namespace hf_types;

class fileOperation
{
public:
    fileOperation();
    ~fileOperation();

//    hf_uint8 OpenFile(hf_char* filePath);
    void ReadFile(hf_char* filePath);


};

#endif // FILEOPERATION_H
