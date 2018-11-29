#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ssj.h"

typedef struct {
	unsigned char start;
	unsigned short key;
	unsigned char val;	
} __attribute__((packed)) FIO_Key_s;

typedef struct {
	unsigned int cmd;
	unsigned int muxctl;
	unsigned int muxval;
	int port;
	int bit;
	int setval;
} __attribute__((packed)) FIO_GPIO_s;

typedef struct {
	int count;
	FIO_GPIO_s io[10];
} __attribute__((packed))FIO_GPIO_Group_s;

typedef struct {
	FIO_GPIO_Group_s wr;
	FIO_GPIO_Group_s row;
	FIO_GPIO_Group_s col;
	FIO_Key_s keymap[25];
} __attribute__((packed)) FIO_Obj_s;


int test_file_simple_read(const char * path, char *buf, int count)
{
	DASSERT(path && buf && count > 0, return -1);
    int fd = -1;

    if ((fd=open(path,O_RDONLY)) < 0)
    {
        perror("open");
        return -1;
    }

    if (read(fd,buf,count)<=0)
    {
        perror("read");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

int test_file_simple_get_size(const char *path)
{
    struct stat statbuff;

    if (!stat(path, &statbuff)) {
        return statbuff.st_size;
    }  else {
        return -1;
    }
}

int test_fbdio_define(ss_obj_t *obj)
{
	SS_DEFINE(
typedef struct {
	FIO_GPIO_Group_s wr;
	FIO_GPIO_Group_s row;
	FIO_GPIO_Group_s col;
	FIO_Key_s keymap[25];
} __attribute__((packed)) FIO_Obj_s,
	FIO_Obj_s,
	obj	
	);
	SS_DEFINE(
typedef struct {
	int count;
	FIO_GPIO_s io[10];
} __attribute__((packed)) FIO_GPIO_Group_s,
	FIO_GPIO_Group_s,
	&obj[1]	
	);
	SS_DEFINE(
typedef struct {
	unsigned int cmd;
	unsigned int muxctl;
	unsigned int muxval;
	int port;
	int bit;
	int setval;
} FIO_GPIO_s,
	FIO_GPIO_s,
	&obj[2]	
	);
	SS_DEFINE(
typedef struct {
	unsigned char start;
	unsigned short key;
	unsigned char val;	
} __attribute__((packed)) FIO_Key_s,
	FIO_Key_s,
	&obj[3]	
	);

	return 0;
}

int test_fbdio_struct_from_my_work(const char *file)
{
	ss_obj_t *obj = NULL;
	cJSON *json = NULL, *json2 = NULL;
	int file_size;
	int ret = -1;
	char *buffer = NULL;
	FIO_Obj_s fio = {};

	/*1*/
	obj = ss_init(4);
	DASSERT(obj, return -1);

	/*2*/
	test_fbdio_define(obj);

	file_size = test_file_simple_get_size(file);
	DASSERT(file_size > 0, goto __ret);
	buffer = malloc(file_size);
	
	DASSERT(!test_file_simple_read(file, buffer, file_size), goto __ret);
	json = cJSON_Parse(buffer);
	DASSERT(json, goto __ret); 

	/*3*/
	DASSERT(!ss_entry(obj, &fio, 1), goto __ret);
	/*4*/
	DASSERT(!ss_json_to_struct(json, obj), goto __ret);	

	/*3*/
	DASSERT(!ss_entry(obj, &fio, 1), goto __ret);
	/*4*/
	json2 = ss_struct_to_json(obj);	

	printf("%s\n", cJSON_Print(json2));

	ret = 0;

__ret:
	if (buffer) {
		free(buffer);
	}
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
	DASSERT(argc > 1, return -1);

	DASSERT(!test_fbdio_struct_from_my_work(argv[1]), return -1); 

	return 0;
}
