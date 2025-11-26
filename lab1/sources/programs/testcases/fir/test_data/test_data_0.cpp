#include "test_data.hpp"
const int16_t test_input[] = {
0, 0, 0, -609, 2910, 3085, 2456, 929, -2473, -2233, 2257, 780, 3618, 0, 0, 0
};

const int16_t test_filter[] = {
-1256, -3055, 4081, -3968,
-1456, -2339, 1934, -3243
};

const int32_t test_ref_output[] = {
2416512, -14032209, 1494925, -5280669, -6742971, 2226273, -7154600, -11680458, 16043680, -15263531, 9547366, -12032670, -4544208,
1974987, -10614936, -2952264, -7918204, -9715618, -419719, -3290030, -7206350, 10659173, -12252529, 1886600, -9598182, -5267808
};

const test_metadata test_meta = {
.input_len = 16,
.filter_len = 4,
.filter_channels = 2
};

const void * input_array[] = {
(void*)test_input
};

const void * filter_array[] = {
(void*)test_filter
};

const void * ref_output_array[] = {
(void*)test_ref_output
};

const void * meta_array[] = {
(void*)&test_meta
};

