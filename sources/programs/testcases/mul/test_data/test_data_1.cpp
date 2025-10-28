#include "test_data.hpp"
const int16_t test_vec_a[] = {
-11882, -16890, 27908, 5202, -16547, -32375, -28746, -24607
};

const int16_t test_vec_b[] = {
-27738, -15324, 10944, -24798, -28977, 15317, -6757, 1847
};

const int32_t test_vec_c[] = {
329582916, 258822360, 305425152, -128999196, 479482419, -495887875, 194236722, -45449129
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

