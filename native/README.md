# TODO

## DSECT to CHDR

To create a C header from an HLASM DSECT:

- create a `.s` file in `asmchdr` folder, e.g. `asasymbp.s`
- upload, e.g. `npm run tools:deploy asmchdr/asasymbp.s`
- allocate output adata data set if none exists, e.g. `zowex data-set create-adata <hlq>.adata`
- allocate output chdr data set if none exists, e.g. `zowex data-set create-vb <hlq>.chdr`
- build `.s` file, e.g. `as -madata --gadata="//'<hlq>.USER(ASASYMBP)'" asasymbp.s`
- convert the file via `ccnedsct`, e.g. `zowex tool ccnedsct --ad "<hlq>.adata(asasymbp)" --cd "<hlq>.chdr(asasymbp)"`
- copy, download the `.h` file where needed, e.g. `zowe files download ds "<hlq>.chdr(asasymbp)" --file native/c/chdsect/asasymbp.h`

## Debug Message

- `export _BPXK_JOBLOG=STDERR`
- add `zwto_debug(...)` in `zwto.h`

## Logstreams

- todo

## Runtime options (recovery)

- `_CEE_RUNOPTS` https://www.ibm.com/docs/en/zos/3.1.0?topic=options-how-specify-runtime
- `TRAP(ON,NOSPIE)` https://www.ibm.com/docs/en/zos/3.1.0?topic=ulero-trap