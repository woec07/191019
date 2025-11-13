# General Apporach - Optimization
most important: save between optimization steps and copy files so you have something to fall back!!!!

## 0. Find a good Algorithm
find an algorithm which at best does not use strided load/stores and also reduction operations, at best an algorithm which doesn't use them at all
- BUT: before spending too much time finding a proper algorithm, use an easier algorithm one and try to minimize strided accesses/reduction

## 1. Implement a basic vectorized code
implement first version, just keep LMUL at minimum (1 before any sign-extensions) and get it to run
- [ ] Attention: you need one loop + variable combination handeling the "chunk-access"
```  
size_t rem_colsB = numColsB;
while (rem_colsB > 0U) { 
    ...
    rem_colsB -= vl; 
}
```
- [ ] Think about pointer access carefully!
## 2. Maximize LMUL
maximize LMUL, just so within a certain scope (e.g. a while loop) not more than 32 physical registers are used
-> but already seperate into respective pipelines to utilize vector chaining!

## 3. Optimize vector_config.cmake
- [ ] identify which unit is used for which operation and then seperate in respective pipelines (no overhead, minimal seperation)
- [ ] try to "play around" with REG_W, but very likely it shouldn't be too high (either 64/128), can either reduce/boost performance
- [ ] minimize pipeline width to get to LUT target (but maximize within LUT target)
-> do everything within the LUT target (minimize everything to achieve LUT target, but maximize within LUT target)

## 4. Loop Unrolling
try to process multiple (2x, 4x, 8x) vector operations within a single loop cycle, use the following constructs for it
```
while (2* i < numRowsB){...}
i = 2*i;
while (i < numRowsB){...}
```
or
```
size_t vl_max = __riscv_vsetvlmax_e16m4();
while (len >= 2* vl_max) {...} // in here only load vl_max elements
while (len > 0U){...}
```
try some interleaving (so as possible, use different units after each other, so no immediate stall, if no data dependency)

## 5. Play Around
use as little vector registers as possible, use wideining instructions, play around with vector_config.cmake in order to hit targets


# COMMAND Overview
- Load/Store (strided): VLSU
- addition/subtraction (also w), sign-extension: VALU
- multiplication (also w), mul-accumulate (also w): VMUL
- move vector -> scalar (and the other way round): VELEM
- redum (also w): VELEM
