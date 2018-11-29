#include <stdio.h>
#include <string.h>
#include "ssj.h"

typedef struct {
	int a;
	int b[5];
}  __attribute__((packed)) test_c_t;

struct test_d {
	int a;
	float b;
	long c;
	long long d;
	char *e;
	double f;
	test_c_t g;
};

typedef struct {
	char str_a[30];
	char *str_b;
	int int_c;
} __attribute__((packed)) test_b_t;

typedef struct {
	int abcdefg;
	short b;
	char c[3][30];
	double d;
	test_b_t e[2];
	int f;
	test_c_t g[2];
	struct test_d h;
} __attribute__((packed)) test_struct_t;

int test_check_define(ss_obj_t *obj)
{
	SS_DEFINE(
typedef struct {
	int abcdefg;
	short b;
	char c[3][30];
	double d;
	test_b_t e[2];
	int f;
	test_c_t g[2];
	struct test_d h;
} __attribute__((packed)) test_struct_t,
	test_struct_t,
	obj
	);

	SS_DEFINE(
typedef struct {
	char str_a[30];
	char *str_b;
	int int_c;
} __attribute__((packed))  test_b_t,
	test_b_t,
	&obj[1]
	);

	SS_DEFINE(
typedef struct {
	int a;
	int b[5];
}  __attribute__((packed)) test_c_t,
	test_c_t,
	&obj[2]
	);

	SS_DEFINE(
struct test_d {
	int a;
	float b;
	long c;
	long long d;
	char *e;
	double f;
	test_c_t g;
},
	struct test_d,
	&obj[3]
	);

	return 0;
}

int test_check_ssj(void)
{
	test_struct_t test = {1, 2, {"xiongmaitech", "aaaa", "ddd"}, 2.5, 
		{{"aaa", "bb", 22}, {"def", "bbb", 10}}, 33,
		{{12, {34, 1, 2, 3, 4}}, {33, {0x2a, 0x34, 0x11, 0x12, 0x13}}},
		{1,2.3,3,4, "222xxxx", 55999.3333, {0x10, {1, 2, 3, 4, 5}}}
	};
	test_struct_t test2 = {};
	ss_obj_t *obj = NULL;
	cJSON *json, *json2;
	int ret = -1;

	/*1*/
	obj = ss_init(4);
	DASSERT(obj, return -1);

	/*2*/
	test_check_define(obj);

	/*3*/
	DASSERT(!ss_entry(obj, &test, 1), goto __ret);
	/*4*/
	json = ss_struct_to_json(obj);	

	printf("%s\n", cJSON_Print(json));

	/*3*/
	DASSERT(!ss_entry(obj, &test2, 1), goto __ret);
	/*4*/
	DASSERT(!ss_json_to_struct(json, obj), goto __ret);

	/*char* -> char[]*/
#if 0
	if (!memcmp(&test, &test2, sizeof(test))) {
		printf("ss_json_to_struct ok!\n");
	} else {
		printf("ss_json_to_struct err!\n");
	}
#endif

	/*3*/
	DASSERT(!ss_entry(obj, &test2, 1), goto __ret);
	/*4*/
	json2 = ss_struct_to_json(obj);	

	printf("%s\n", cJSON_Print(json2));

	ret = 0;

__ret:
	if (json) {
		cJSON_Delete(json);
	}
	if (json2) {
		cJSON_Delete(json2);
	}

	/*5*/
	if (obj) {
		ss_destroy(obj);
	}

	return ret;
}

int main(int argc, char *argv[])
{
	return test_check_ssj();
}
