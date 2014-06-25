#include "HistogramPyramids.clh"





#ifdef cl_khr_3d_image_writes
__kernel void constructHPLevel3D(
    __read_only image3d_t readHistoPyramid,
    __write_only image3d_t writeHistoPyramid
    ) {

    int4 writePos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int4 readPos = writePos*2;
    int writeValue = read_imageui(readHistoPyramid, hpSampler, readPos).x + // 0
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[1]).x + // 1
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[2]).x + // 2
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[3]).x + // 3
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[4]).x + // 4
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[5]).x + // 5
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[6]).x + // 6
    read_imageui(readHistoPyramid, hpSampler, readPos+cubeOffsets[7]).x; // 7

    write_imageui(writeHistoPyramid, writePos, writeValue);
}
#endif

__kernel void constructHPLevel2D(
    __read_only image2d_t readHistoPyramid,
    __write_only image2d_t writeHistoPyramid
    ) {

    int2 writePos = {get_global_id(0), get_global_id(1)};
    int2 readPos = writePos*2;
    int writeValue =
        read_imageui(readHistoPyramid, hpSampler, readPos).x +
        read_imageui(readHistoPyramid, hpSampler, readPos+(int2)(1,0)).x +
        read_imageui(readHistoPyramid, hpSampler, readPos+(int2)(0,1)).x +
        read_imageui(readHistoPyramid, hpSampler, readPos+(int2)(1,1)).x;

    write_imageui(writeHistoPyramid, writePos, writeValue);
}



__kernel void createPositions3D(
        __global int * positions,
        __private int HP_SIZE,
        __private int sum,
        __read_only image3d_t hp0, // Largest HP
        __read_only image3d_t hp1,
        __read_only image3d_t hp2,
        __read_only image3d_t hp3,
        __read_only image3d_t hp4,
        __read_only image3d_t hp5
        ,__read_only image3d_t hp6
        ,__read_only image3d_t hp7
        ,__read_only image3d_t hp8
        ,__read_only image3d_t hp9
    ) {
    int target = get_global_id(0);
    if(target >= sum)
        target = 0;
    int4 pos = traverseHP3D(target,HP_SIZE,hp0,hp1,hp2,hp3,hp4,hp5,hp6,hp7,hp8,hp9);
    vstore3(pos.xyz, target, positions);
}

__kernel void createPositions2D(
        __global int * positions,
        __private int HP_SIZE,
        __private int sum,
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5
        ,__read_only image2d_t hp6
        ,__read_only image2d_t hp7
        ,__read_only image2d_t hp8
        ,__read_only image2d_t hp9
        ,__read_only image2d_t hp10
        ,__read_only image2d_t hp11
        ,__read_only image2d_t hp12
        ,__read_only image2d_t hp13
    ) {
    int target = get_global_id(0);
    if(target >= sum)
        target = 0;
    int2 pos = traverseHP2D(target,HP_SIZE,hp0,hp1,hp2,hp3,hp4,hp5,hp6,hp7,hp8,hp9,hp10,hp11,hp12,hp13);
    vstore2(pos, target, positions);
}


__kernel void constructHPLevelCharChar(
        __global uchar * readHistoPyramid,
        __global uchar * writeHistoPyramid,
        __private int sizeX,
        __private int sizeY,
        __private int sizeZ
    ) {
	uint3 size = {sizeX,sizeY,sizeZ};

    uint writePos = EncodeMorton3(get_global_id(0), get_global_id(1), get_global_id(2));
    int4 readPos = (int4)(get_global_id(0)*2, get_global_id(1)*2, get_global_id(2)*2,0);
    uchar writeValue;
    if(readPos.x >= size.x || readPos.y >= size.y || readPos.z >= size.z) {
    	writeValue = 0;
    } else {
		writeValue = readHistoPyramid[NLPOS(readPos)] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[1])] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[2])] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[3])] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[4])] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[5])] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[6])] +
                    readHistoPyramid[NLPOS(readPos+cubeOffsets[7])];
    }

    writeHistoPyramid[writePos] = writeValue;
}

__kernel void constructHPLevelCharShort(
        __global uchar * readHistoPyramid,
        __global ushort * writeHistoPyramid
    ) {

    uint writePos = EncodeMorton3(get_global_id(0), get_global_id(1), get_global_id(2));
    uint readPos = EncodeMorton3(get_global_id(0)*2, get_global_id(1)*2, get_global_id(2)*2);
    ushort writeValue = readHistoPyramid[readPos] +
                    readHistoPyramid[readPos + 1] +
                    readHistoPyramid[readPos + 2] +
                    readHistoPyramid[readPos + 3] +
                    readHistoPyramid[readPos + 4] +
                    readHistoPyramid[readPos + 5] +
                    readHistoPyramid[readPos + 6] +
                    readHistoPyramid[readPos + 7];

    writeHistoPyramid[writePos] = writeValue;
}
__kernel void constructHPLevelShortShort(
        __global ushort * readHistoPyramid,
        __global ushort * writeHistoPyramid
    ) {

    uint writePos = EncodeMorton3(get_global_id(0), get_global_id(1), get_global_id(2));
    uint readPos = EncodeMorton3(get_global_id(0)*2, get_global_id(1)*2, get_global_id(2)*2);
    ushort writeValue = readHistoPyramid[readPos] +
                    readHistoPyramid[readPos + 1] +
                    readHistoPyramid[readPos + 2] +
                    readHistoPyramid[readPos + 3] +
                    readHistoPyramid[readPos + 4] +
                    readHistoPyramid[readPos + 5] +
                    readHistoPyramid[readPos + 6] +
                    readHistoPyramid[readPos + 7];

    writeHistoPyramid[writePos] = writeValue;
}

__kernel void constructHPLevelShortInt(
        __global ushort * readHistoPyramid,
        __global int * writeHistoPyramid
    ) {

    uint writePos = EncodeMorton3(get_global_id(0), get_global_id(1), get_global_id(2));
    uint readPos = EncodeMorton3(get_global_id(0)*2, get_global_id(1)*2, get_global_id(2)*2);
    int writeValue = readHistoPyramid[readPos] +
                    readHistoPyramid[readPos + 1] +
                    readHistoPyramid[readPos + 2] +
                    readHistoPyramid[readPos + 3] +
                    readHistoPyramid[readPos + 4] +
                    readHistoPyramid[readPos + 5] +
                    readHistoPyramid[readPos + 6] +
                    readHistoPyramid[readPos + 7];

    writeHistoPyramid[writePos] = writeValue;
}

__kernel void constructHPLevelBuffer(
        __global int * readHistoPyramid,
        __global int * writeHistoPyramid
    ) {

    uint writePos = EncodeMorton3(get_global_id(0), get_global_id(1), get_global_id(2));
    uint readPos = EncodeMorton3(get_global_id(0)*2, get_global_id(1)*2, get_global_id(2)*2);
    int writeValue = readHistoPyramid[readPos] +
                    readHistoPyramid[readPos + 1] +
                    readHistoPyramid[readPos + 2] +
                    readHistoPyramid[readPos + 3] +
                    readHistoPyramid[readPos + 4] +
                    readHistoPyramid[readPos + 5] +
                    readHistoPyramid[readPos + 6] +
                    readHistoPyramid[readPos + 7];

    writeHistoPyramid[writePos] = writeValue;
}


__kernel void createPositions3DBuffer(
		__private int sizeX,
        __private int sizeY,
        __private int sizeZ,
        __global int * positions,
        __private int HP_SIZE,
        __private int sum,
        __global uchar * hp0, // Largest HP
        __global uchar * hp1,
        __global ushort * hp2,
        __global ushort * hp3,
        __global ushort * hp4,
        __global int * hp5,
        __global int * hp6,
        __global int * hp7,
        __global int * hp8,
        __global int * hp9
    ) {
    int target = get_global_id(0);
    if(target >= sum)
        target = 0;
    uint3 size = {sizeX,sizeY,sizeZ};
    int4 pos = traverseHP3DBuffer(size,target,HP_SIZE,hp0,hp1,hp2,hp3,hp4,hp5,hp6,hp7,hp8,hp9);
    vstore3(pos.xyz, target, positions);
}

