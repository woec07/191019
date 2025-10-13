#include "test_data.hpp"
const int8_t test_vec_a[] = {
-7, -88, -127, -5
};

const int8_t test_vec_b[] = {
-25, 72, -65, 59
};

const int16_t test_vec_c[] = {
-32, -16, -192, 54
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

