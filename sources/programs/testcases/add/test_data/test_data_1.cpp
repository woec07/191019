#include "test_data.hpp"
const int8_t test_vec_a[] = {
100, -41, -30, -52, 53, 80, -94, 125
};

const int8_t test_vec_b[] = {
59, 95, -63, -87, -56, -44, -83, -53
};

const int16_t test_vec_c[] = {
159, 54, -93, -139, -3, 36, -177, 72
};

const test_metadata test_meta = {
.vec_len = 8
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

