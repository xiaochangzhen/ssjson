/*
 *	xcz@20181127
 */
#ifndef SSJ_H
#define SSJ_H

#include "cJSON.h"

typedef enum {
	SS_TYPE_FLAG_UNSIGNED,
	SS_TYPE_FLAG_POINTER,
	SS_TYPE_FLAG_ARRARY,
} ss_type_flag_e;

typedef enum {
	SS_BASE_UNKOWN,
	SS_BASE_TYPE_INT,
	SS_BASE_TYPE_CHAR,
	SS_BASE_TYPE_SHORT,
	SS_BASE_TYPE_DOUBLE,
	SS_BASE_TYPE_FLOAT,
	SS_BASE_TYPE_LONG,
	SS_BASE_TYPE_LONGLONG,
	SS_BASE_TYPE_STRUCT,
} ss_base_type_e;

typedef struct {
	ss_base_type_e type;
	int flag;
	char name[32];
	int item_size;
	int total_size;
	int struct_index;/*only for SS_BASE_TYPE_STRUCT*/
} ss_base_type_t;

typedef struct {
	int index;
	int count;
	void *struct_ptr;
	int struct_size;
	const char *struct_name;
	const char *struct_string;
	int arrary_num;
} ss_obj_t;

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif
#ifndef ERR
#define ERR(string,args...) fprintf(stderr,"\033[31m""%s(%d) [%s]: "string"\033[0m",__FILE__,__LINE__,__FUNCTION__,##args)
#endif
#ifndef DASSERT
#define DASSERT(b,action) do {if(unlikely(!(b))){ ERR("debug assertion failure (%s)\n", #b);action;} } while (0)
#endif

ss_obj_t *ss_init(int count);

#define SS_DEFINE(struct_x, name, obj)  do { \
	(obj)->struct_size = sizeof(name); \
	(obj)->struct_name = #name; \
	(obj)->struct_string = #struct_x; \
} while(0)

int ss_entry(ss_obj_t *obj, void *struct_ptr, int num);

cJSON *ss_struct_to_json(ss_obj_t *obj);

int ss_json_to_struct(cJSON *json, ss_obj_t *obj);

#endif
