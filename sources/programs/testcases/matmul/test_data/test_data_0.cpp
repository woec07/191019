#include "test_data.hpp"
const int8_t test_mat_a[] = {
-108, 89, -50, 28, 
-87, -91, 33, 115, 
-108, -114, 76, -98, 
15, -125, 27, -92
};

const int8_t test_mat_b[] = {
-4, -67, 87, 88, 
-13, -80, -15, -16, 
-68, -110, 59, -3, 
-40, 68, -111, -41
};

const int32_t test_mat_c[] = {
1555, 7520, -16789, -11926, 
-5313, 17299, -17022, -11014, 
666, 1332, 7676, -3890, 
3409, -231, 14985, 7011
};

const test_metadata test_meta = {
.numRowsA = 4,
.numColsA = 4,
.numColsB = 4
};

const void * mat_a_array[] = {
(void*)test_mat_a
};

const void * mat_b_array[] = {
(void*)test_mat_b
};

const void * mat_c_array[] = {
(void*)test_mat_c
};

const void * meta_array[] = {
(void*)&test_meta
};

