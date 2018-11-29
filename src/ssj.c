/*
 * ssj.c
 *
 * Copyright (C) 2018 xiaochangzhen <xiaochangzhen@yeah.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * xiaochangzhen, Hangzhou, China, 2018/11/29
 */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "ssj.h"

static const char *__ss_skip(const char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}

static  int __ss_string_parse(ss_obj_t *obj, const char **pstring_x, int *flag, ss_base_type_t *val)
{
	if (!*flag) {
		while (**pstring_x) {
			if (**pstring_x == '{') {
				*flag = 1;	
				(*pstring_x)++;
				break;
			}
			(*pstring_x)++;
		}
	}
	int i, num;
	ss_obj_t *obj_start = obj - obj->index;
	memset(val, 0, sizeof(ss_base_type_t));

	DASSERT(**pstring_x, return -1);

	while (**pstring_x && **pstring_x != ';') {
		*pstring_x = __ss_skip(*pstring_x);
		if (!strncmp(*pstring_x, "const", 4)) {
			*pstring_x += 4;
			continue;
		} else if (!strncmp(*pstring_x, "signed", 6)) {
			*pstring_x += 6;
			continue;
		} else if (!strncmp(*pstring_x, "unsigned", 8)) {
			*pstring_x += 8;
			val->flag |= 1<<SS_TYPE_FLAG_UNSIGNED;
			continue;
		} else if (**pstring_x == '*') {
			*pstring_x += 1;
			val->flag |= 1<<SS_TYPE_FLAG_POINTER;
			val->item_size = sizeof(long);
			val->total_size = sizeof(long);	
			continue;
		} else if (**pstring_x == '[') {
			*pstring_x += 1;
			DASSERT(!(val->flag&(1<<SS_TYPE_FLAG_ARRARY))
			|| (val->type == SS_BASE_TYPE_CHAR && !(val->flag&(1<<SS_TYPE_FLAG_UNSIGNED))), return -1);
			num = strtol(*pstring_x, NULL, 0);	
			DASSERT(num > 0, return -1);
			if (val->flag&(1<<SS_TYPE_FLAG_ARRARY)) {
				val->item_size *= num;
			}
			val->flag |= 1<<SS_TYPE_FLAG_ARRARY;
			DASSERT(val->type != SS_BASE_UNKOWN && val->total_size, return -1);
			val->total_size *= num;	
			while (*((*pstring_x)++) != ']') {}
			continue;
		} else if (**pstring_x == '}') {
			*flag = 0;
			break;
		}

		if (val->type != SS_BASE_UNKOWN) {
			for (i = 0; **pstring_x!=' ' && **pstring_x!='[' && **pstring_x!=';'; (*pstring_x)++,i++) {
				val->name[i] = **pstring_x;
			}
			continue;
		} else if (!strncmp(*pstring_x, "char", 4)) {
			*pstring_x += 4;
			val->type = SS_BASE_TYPE_CHAR;
			val->item_size = sizeof(char);
			val->total_size = val->item_size;
			continue;
		} else if (!strncmp(*pstring_x, "short", 5)) {
			*pstring_x += 5;
			val->type = SS_BASE_TYPE_SHORT;
			val->item_size = sizeof(short);
			val->total_size = val->item_size;
			continue;
		} else if (!strncmp(*pstring_x, "int", 3)) {
			*pstring_x += 3;
			val->type = SS_BASE_TYPE_INT;
			val->item_size = sizeof(int);
			val->total_size = val->item_size;
			continue;
		} else if (!strncmp(*pstring_x, "float", 5)) {
			*pstring_x += 5;
			val->type = SS_BASE_TYPE_FLOAT;
			val->item_size = sizeof(float);
			val->total_size = val->item_size;
			continue;
		} else if (!strncmp(*pstring_x, "double", 6)) {
			*pstring_x += 6;
			val->type = SS_BASE_TYPE_DOUBLE;
			val->item_size = sizeof(double);
			val->total_size = val->item_size;
			continue;
		} else if (!strncmp(*pstring_x, "long", 4)) {
			*pstring_x += 4;
			*pstring_x = __ss_skip(*pstring_x);
			if (!strncmp(*pstring_x, "long", 4)) {
				val->type = SS_BASE_TYPE_LONGLONG;
				val->item_size = sizeof(long long);
				val->total_size = val->item_size;
				*pstring_x += 4;
			} else {
				val->type = SS_BASE_TYPE_LONG;
				val->item_size = sizeof(long);
				val->total_size = val->item_size;
			}
			continue;
		} else {
			for (i = 0; i < obj_start->count; i++) {
				if (!strncmp(*pstring_x, obj_start[i].struct_name, strlen(obj_start[i].struct_name))) {
					*pstring_x += strlen(obj_start[i].struct_name);
					val->type = SS_BASE_TYPE_STRUCT;
					val->struct_index = i;
					val->item_size = obj_start[i].struct_size;
					val->total_size = obj_start[i].struct_size;
					break;
				}
			}
			if (val->type == SS_BASE_UNKOWN) {
				ERR("unkonwn: %s\n", *pstring_x);
				return -1;		
			}
		}
	}
	if (**pstring_x == ';') {
		(*pstring_x)++;
	}
	
	DASSERT(val->type != SS_BASE_UNKOWN || !*flag, return -1);
	return 0;
}

static cJSON *__ss_create_item(ss_type_flag_e flag, ss_base_type_e type, void *data)
{
	cJSON *json = NULL;

	if ((type==SS_BASE_TYPE_CHAR)
		&& !(flag&(1<<SS_TYPE_FLAG_UNSIGNED))
		&& !(flag&(1<<SS_TYPE_FLAG_POINTER))
		&& (flag&(1<<SS_TYPE_FLAG_ARRARY))) {
		json = cJSON_CreateString((char *)data);
	} else if ((type==SS_BASE_TYPE_CHAR)
		&& (flag&(1<<SS_TYPE_FLAG_POINTER))) {
		json = cJSON_CreateString(*(char **)data);
	} else if (type==SS_BASE_TYPE_CHAR
		&& (flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((unsigned char *)data));
	} else if (type==SS_BASE_TYPE_SHORT
		&& (flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((unsigned short *)data));
	} else if (type==SS_BASE_TYPE_SHORT
		&& !(flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((short *)data));
	} else if (type==SS_BASE_TYPE_INT
		&& (flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((unsigned int *)data));
	} else if (type==SS_BASE_TYPE_INT
		&& !(flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((int *)data));
	} else if (type==SS_BASE_TYPE_LONG
		&& (flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((unsigned long *)data));
	} else if (type==SS_BASE_TYPE_LONG
		&& !(flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((long *)data));
	} else if (type==SS_BASE_TYPE_LONGLONG
		&& (flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((unsigned long long *)data));
	} else if (type==SS_BASE_TYPE_LONGLONG
		&& !(flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		json = cJSON_CreateNumber((double)*((long long *)data));
	} else if (type==SS_BASE_TYPE_FLOAT) {
		json = cJSON_CreateNumber((double)*((float *)data));
	} else if (type==SS_BASE_TYPE_DOUBLE) {
		json = cJSON_CreateNumber(*((double *)data));
	}

	return json;
}

static cJSON *__ss_base_type_to_json(ss_base_type_t *base, void *data)
{
	cJSON *root = NULL, *item = NULL;
	int i, num = 1;
	unsigned char *ptr = data;
	num = base->total_size/base->item_size;

	if (base->flag&(1<<SS_TYPE_FLAG_POINTER)
		&& base->type != SS_BASE_TYPE_CHAR) {
		ERR("unsported type!\n");
		return NULL;
	} else if ((base->flag&(1<<SS_TYPE_FLAG_ARRARY))
		&& !(base->flag&(1<<SS_TYPE_FLAG_POINTER))
		&& base->type == SS_BASE_TYPE_CHAR
		&& base->item_size == sizeof(char)) {
		return cJSON_CreateString((char *)data);	
	}

	if (num > 1) {
		root = cJSON_CreateArray();
		DASSERT(root, return NULL);
	}

	for (i = 0; i < num; i++) {
		item = __ss_create_item(base->flag, base->type, data);
		data = (unsigned char *)data + base->item_size;
		if (root) {
			cJSON_AddItemToArray(root, item);	
		} else {
			root = item;
			break;
		}
	}

	return root;
}

static int __ss_do_json_to_base_type(cJSON *json, ss_base_type_t *base, void *data)
{
	
	if ((base->type==SS_BASE_TYPE_CHAR)
		&& !(base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))
		&& !(base->flag&(1<<SS_TYPE_FLAG_POINTER))
		&& (base->flag&(1<<SS_TYPE_FLAG_ARRARY))) {
		DASSERT(json->type == cJSON_String, return -1);
		DASSERT(base->item_size > strlen(json->valuestring), return -1);
		memcpy(data, json->valuestring, strlen(json->valuestring));
	} else if ((base->type==SS_BASE_TYPE_CHAR)
		&& (base->flag&(1<<SS_TYPE_FLAG_POINTER))) {
		*(char **)data = json->valuestring;
	} else if (base->type==SS_BASE_TYPE_CHAR
		&& (base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(unsigned char *)data = (unsigned char)json->valueint;
	} else if (base->type==SS_BASE_TYPE_SHORT
		&& (base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(unsigned short *)data =(unsigned short)json->valueint;
	} else if (base->type==SS_BASE_TYPE_SHORT
		&& !(base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(short *)data = (short)json->valueint;
	} else if (base->type==SS_BASE_TYPE_INT
		&& (base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(unsigned int *)data = (unsigned int)json->valueint;
	} else if (base->type==SS_BASE_TYPE_INT
		&& !(base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(int *)data = json->valueint;
	} else if (base->type==SS_BASE_TYPE_LONG
		&& (base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(unsigned long *)data = (unsigned long)json->valueint;
	} else if (base->type==SS_BASE_TYPE_LONG
		&& !(base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(long *)data = (long)json->valueint;
	} else if (base->type==SS_BASE_TYPE_LONGLONG
		&& (base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(unsigned long long *)data = (unsigned long long)json->valueint;
	} else if (base->type==SS_BASE_TYPE_LONGLONG
		&& !(base->flag&(1<<SS_TYPE_FLAG_UNSIGNED))) {
		*(long long *)data = (long long)json->valueint;
	} else if (base->type==SS_BASE_TYPE_FLOAT) {
		*(float *)data = (float)json->valuedouble;
	} else if (base->type==SS_BASE_TYPE_DOUBLE) {
		*(double *)data = json->valuedouble;
	}
	return 0;
}

static int __ss_json_to_base_type(cJSON *json, ss_base_type_t *base, void *data)
{
	cJSON *item = NULL;
	int i, num = 1;
	unsigned char *ptr = data;
	num = base->total_size/base->item_size;

	if (base->flag&(1<<SS_TYPE_FLAG_POINTER)
		&& base->type != SS_BASE_TYPE_CHAR) {
		ERR("unsported type!\n");
		return -1;
	} else if ((base->flag&(1<<SS_TYPE_FLAG_ARRARY))
		&& !(base->flag&(1<<SS_TYPE_FLAG_POINTER))
		&& base->type == SS_BASE_TYPE_CHAR
		&& base->item_size == sizeof(char)) {
		DASSERT(num > strlen(json->valuestring), return -1);
		memcpy(data, json->valuestring, strlen(json->valuestring));
		return 0;
	}

	item = json;
	for (i = 0; i < num; i++) {
		if (num > 1) {
			item = cJSON_GetArrayItem(json, i);
			DASSERT(item, return -1);
		}
		DASSERT(!__ss_do_json_to_base_type(item, base, data), return -1);
		data = (unsigned char *)data + base->item_size;
	}

	return 0;
}

ss_obj_t *ss_init(int count)
{
	int i;
	DASSERT(count, return NULL);
	ss_obj_t *obj = malloc(count*sizeof(ss_obj_t));		
	DASSERT(obj, return NULL);
	memset(obj, 0, count*sizeof(ss_obj_t));
	for (i = 0; i < count; i++) {
		obj[i].index = i;			
		obj[i].count = count;
	}
	return obj;
}

cJSON *ss_struct_to_json(ss_obj_t *obj)
{
	DASSERT(obj && obj->struct_ptr && obj->struct_name && obj->struct_string, return NULL);
	cJSON *root = NULL, *item = NULL, *child = NULL;
	unsigned char *ptr = NULL;
	const char *ptr_char = NULL;
	int struct_flag = 0;
	ss_base_type_t member = {};
	int i;
	ss_obj_t *obj_start = obj - obj->index;
	
	if (obj->arrary_num == 1) {
		root = cJSON_CreateObject();
		DASSERT(root, return NULL);
		item = root;
	} else {
		root = cJSON_CreateArray();
		DASSERT(root, return NULL);
	}

	ptr = obj->struct_ptr;
	for (i = 0; i < obj->arrary_num; i++) {
		ptr_char = obj->struct_string;
		if (obj->arrary_num > 1) {
			item = cJSON_CreateObject();
			DASSERT(item, return NULL);
		}
		while (ptr_char && *ptr_char) {
			if (__ss_string_parse(obj, &ptr_char, &struct_flag, &member)) {
				ERR("%s struct_flag %d\n", member.name, struct_flag);
				break;
			}
			if (!struct_flag) {
				break;
			}
			if (member.type == SS_BASE_TYPE_STRUCT) {
				obj_start[member.struct_index].struct_ptr = ptr;
				obj_start[member.struct_index].arrary_num = member.total_size/member.item_size;
				child = ss_struct_to_json(&obj_start[member.struct_index]);
			} else {
				child = __ss_base_type_to_json(&member, ptr);
			}
			if (!child) {
				break;
			}
			ptr += member.total_size;
			cJSON_AddItemToObject(item, member.name, child);
		}
		if (!struct_flag && obj->arrary_num > 1) {
			cJSON_AddItemToArray(root, item);
			item = NULL;
		} else if (obj->arrary_num > 1) {
			break;
		}
	}

	if (struct_flag) {
		cJSON_Delete(root);
		root = NULL;
	}

	return root;
}

int ss_json_to_struct(cJSON *json, ss_obj_t *obj)
{
	DASSERT(json && obj && obj->struct_ptr && obj->struct_name && obj->struct_string, return -1);
	cJSON *root, *item = NULL, *child = NULL;
	unsigned char *ptr = NULL;
	const char *ptr_char = NULL;
	int struct_flag = 0;
	ss_base_type_t member = {};
	int i;
	ss_obj_t *obj_start = obj - obj->index;

	ptr = obj->struct_ptr;
	memset(ptr, 0, obj->struct_size);

	for (i = 0; i < obj->arrary_num; i++) {
		ptr_char = obj->struct_string;
		if (obj->arrary_num > 1) {
			if (i >= cJSON_GetArraySize(json)) {
				break;
			}
			root = cJSON_GetArrayItem(json, i);
		} else {
			root = json;
		}
		while (ptr_char && *ptr_char) {
			if (__ss_string_parse(obj, &ptr_char, &struct_flag, &member)) {
				ERR("%s struct_flag %d\n", member.name, struct_flag);
				break;
			}
			if (!struct_flag) {
				break;
			}
			item = cJSON_GetObjectItem(root, member.name);
			if (!item) {
				ptr += member.total_size;
				continue;
			}
			if (member.type == SS_BASE_TYPE_STRUCT) {
				obj_start[member.struct_index].struct_ptr = ptr;
				obj_start[member.struct_index].arrary_num = member.total_size/member.item_size;
				DASSERT(!ss_json_to_struct(item, &obj_start[member.struct_index]), return -1);
			} else {
				DASSERT(!__ss_json_to_base_type(item, &member, ptr), return -1);
			}
			ptr += member.total_size;
		}
	}

	DASSERT(!struct_flag, return -1);
	return 0;
}

int ss_entry(ss_obj_t *obj, void *struct_ptr, int num)
{
	DASSERT(obj && struct_ptr && num >= 1, return -1);	
	obj->struct_ptr = struct_ptr;
	obj->arrary_num = 1;

	return 0;
}

int ss_destroy(ss_obj_t *obj)
{
	if (obj) {
		free(obj);
	}
	return 0;
}

