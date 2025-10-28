#include "test_data.hpp"
const int16_t test_vec_a[] = {
-16989, 8244, -6101, -30513
};

const int16_t test_vec_b[] = {
32340, -32127, -916, 22548
};

const int32_t test_vec_c[] = {
-549424260, -264854988, 5588516, -688007124
};

const test_metadata test_meta = {
.vec_len = 4
};

const void * vec_a_array[] = {
(void*)test_vec_a
};

const void * vec_b_array[] = {
(void*)test_vec_b
};

const void * vec_c_array[] = {
(void*)test_vec_c
};

const void * meta_array[] = {
(void*)&test_meta
};

