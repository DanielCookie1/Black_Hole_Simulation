/* Usage: Read DARR_README.md */

typedef struct {
	size_t elem_sz, len, cap;
	void *data;
} da_t;

typedef uint8_t da_byte_t;

/* elem size independent */

da_t da_new(size_t elem_num, size_t elem_sz) {
	const size_t len = elem_num;
	const size_t cap = 2 * elem_num;
	void *alloced = calloc(cap, elem_sz);

	const da_t new = {
		.elem_sz = elem_sz,
		.len = len,
		.cap = cap,
		.data = alloced,
	};
	return new;
}

void da_terminate(const da_t *arr) {
	free(arr->data);
}

// void da_pop(const da_t *arr) {
// 	if (0 == arr->len) {
// 		return;
// 	}
// 	da_byte_t *slot = arr->data + ((arr->len-1) * arr->elem_sz);
// 	for (size_t i = 0; i < arr->elem_sz; i++) {
// 		*(slot + i) = 0;
// 	}
// 	arr->len--;
// }

/* elem size dependent */

#define CONCAT(a, b) CONCAT_(a, b)
#define CONCAT_(a, b) a ## b

#define dyn_arr_methods_for_type(method_suffix, method_type)\
\
method_type CONCAT(da_last_, method_suffix) (const da_t *arr) {\
	if (arr->len == 0) {\
		const method_type def_val = {0};\
		return def_val;\
	}\
	const method_type *slot = arr->data;\
	return *slot;\
}\
\
method_type CONCAT(da_fst_, method_suffix) (const da_t *arr) {\
	if (arr->len == 0) {\
		const method_type def_val = {0};\
		return def_val;\
	}\
	const method_type *slot = arr->data + ((arr->len-1) * arr->elem_sz);\
	return *slot;\
}\
\
method_type CONCAT(da_get_, method_suffix) (const da_t *arr, size_t idx) {\
	if (arr->len <= idx) {\
		const method_type def_val = {0};\
		return def_val;\
	}\
	const method_type *slot = arr->data + (idx * arr->elem_sz);\
	return *slot;\
}\
\
void CONCAT(da_push_, method_suffix) (da_t *arr, method_type elem) {\
	if (arr->len < arr->cap) {\
		method_type *slot = arr->data + (arr->len * arr->elem_sz);\
		*slot = elem;\
		arr->len++;\
		return;\
	}\
\
	const size_t new_cap = arr->cap * 2;\
	void *alloced = calloc(new_cap, arr->elem_sz);\
\
	const size_t data_byte_len = arr->len * arr->elem_sz;\
	for (size_t i = 0; i < data_byte_len; i++) {\
		da_byte_t *extant_slot = arr->data + i;\
		da_byte_t *new_slot = alloced + i;\
		*new_slot = *extant_slot;\
	}\
\
	free(arr->data);\
	arr->data = alloced;\
	arr->cap = new_cap;\
\
	method_type *slot = arr->data + (arr->len * arr->elem_sz);\
	*slot = elem;\
	arr->len++;\
}\
\
void CONCAT(da_put_, method_suffix) (const da_t *arr, size_t idx, method_type elem) {\
	if (idx < arr->len) {\
		method_type *slot = arr->data + (idx * arr->elem_sz);\
		*slot = elem;\
	}\
}\
\
void CONCAT(da_ins_, method_suffix) (da_t *arr, size_t idx, method_type elem) {\
	if (idx < arr->len) {\
		size_t i;\
		const size_t new_len = arr->len + 1;\
\
		if (new_len <= arr->cap) {\
			for (i = arr->len - 1; idx < i; i--) {\
				method_type *cur_slot = arr->data + (i * arr->elem_sz);\
				*(cur_slot + 1) = *cur_slot;\
			}\
			method_type *idx_slot = arr->data + (idx * arr->elem_sz);\
			*(idx_slot + 1) = *idx_slot;\
			*idx_slot = elem;\
			arr->len = new_len;\
			return;\
		}\
\
		const size_t new_cap = 2 * arr->cap;\
		void *alloced = calloc(new_cap, arr->elem_sz);\
\
		for (i = 0; i < idx; i++) {\
			method_type *cur_slot = arr->data + (i * arr->elem_sz);\
			method_type *new_slot = alloced + (i * arr->elem_sz);\
			*new_slot = *cur_slot;\
		}\
		method_type *idx_slot = alloced + (idx * arr->elem_sz);\
		*idx_slot = elem;\
		for (i = idx; i < arr->len; i++) {\
			method_type *cur_slot = arr->data + (i * arr->elem_sz);\
			method_type *new_slot = alloced + ((i+1) * arr->elem_sz);\
			*new_slot = *cur_slot;\
		}\
\
		free(arr->data);\
		arr->data = alloced;\
		arr->cap = new_cap;\
		arr->len = new_len;\
		return;\
	}\
	if (idx == arr->len) {\
		CONCAT(da_push_, method_suffix) (arr, elem);\
	}\
}\
\
void CONCAT(da_pop_, method_suffix) (da_t *arr) {\
	if (0 < arr->len) {\
		const method_type def_val = {0};\
		method_type *last_slot = arr->data + ((arr->len-1) * arr->elem_sz);\
		*last_slot = def_val;\
		arr->len--;\
	}\
}\
\
void CONCAT(da_del_, method_suffix) (da_t *arr, size_t idx) {\
	if (idx < arr->len) {\
		for (size_t i = idx + 1; i < arr->len; i++) {\
			method_type *slot = arr->data + (i * arr->elem_sz);\
			*(slot - 1) = *slot;\
		}\
		const method_type def_val = {0};\
		method_type *last_slot = arr->data + ((arr->len-1) * arr->elem_sz);\
		*last_slot = def_val;\
		arr->len--;\
	}\
}
