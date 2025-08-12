#ifndef IEANTASM_H
#define IEANTASM_H

#pragma pack(packed)

#ifndef __ieatasmw__
#define __ieatasmw__

struct ieatasmw
{
  int noexist; /* THIS IS NOT A REAL FIELD */
};

/* Values for field "noexist" */
#define ieant___task___level 1
#define ieant___home___level 2
#define ieant___primary___level 3
#define ieant___system___level 4
#define ieant___taskauth___level 11    /* @P1A */
#define ieant___homeauth___level 12    /* @P1A */
#define ieant___primaryauth___level 13 /* @P1A */
#define ieant___nopersist 0
#define ieant___persist 1
#define ieant___nocheckpoint 0 /* @L1A */
#define ieant___checkpointok 2 /* @L1A */
#define ieant___ok 0
#define ieant___dup___name 4
#define ieant___not___found 4
#define ieant___24bitmode 8
#define ieant___not___auth 16
#define ieant___srb___mode 20
#define ieant___lock___held 24
#define ieant___level___invalid 28
#define ieant___name___invalid 32
#define ieant___persist___invalid 36
#define ieant___ar___invalid 40
#define ieant___not___amode64 44
#define ieant___unexpected___err 64

#endif

#pragma pack(reset)

#endif // ieantasm.h
