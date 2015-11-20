#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include "hf_types.h"

using namespace hf_types;

class fileOperation
{
public:
    fileOperation();
    ~fileOperation();

    void ReadFile(hf_char* filePath, hf_uint16 MapID);

    //判断移动方向
    hf_uint8 JudgeMoveDirect(hf_float current_x, hf_float current_z, hf_uint32 MapID, hf_float target_x, hf_float target_z);

    hf_char* buffMap1;
    hf_char* buffMap2;
    hf_char* buffMap3;
};

#endif // FILEOPERATION_H
