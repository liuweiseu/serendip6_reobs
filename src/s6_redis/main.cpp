#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "include/s6_redis.h"
#include "include/cJSON.h"

/* define the redis key struct */
#define N_KEYS  4
#define HOST    "127.0.0.1"
#define PORT    6379

struct redis_keys{
    const char *key[N_KEYS];
};
struct redis_keys keys={"name", "age", "address", "major"};

// write json file
void write_json(char *string)
{
    FILE *fp;
    fp = fopen("redis_info.json","a+");
    fwrite(string, strlen(string),1,fp);
    fclose(fp);    
}

int main()
{   
    char *string = NULL;
    // add key-valus from redis database
    char value[100];
    // create a json object
    time_t t;

    while(1){
        cJSON *redis_info = cJSON_CreateObject();
        // add linux time 
        time(&t);
        cJSON *current_time = cJSON_CreateString(ctime(&t));
        cJSON_AddItemToObject(redis_info, "Time", current_time);
        for(int i = 0; i < N_KEYS; i++)
        {
            memset(value,0,100);
            get_info_from_redis(HOST,PORT,keys.key[i],value);
            cJSON *key = cJSON_CreateString(value);
            cJSON_AddItemToObject(redis_info,keys.key[i],key);
        }
        string = cJSON_Print(redis_info);
        //printf(string);
        write_json(string);
        fprintf(stderr,"%s\r\n","Read data from redis database!");
        cJSON_Delete(redis_info);
        sleep(1);
    }
    // parse JSON
    /*
    printf("---------------------------------------------\r\n");
    printf("-------------------Prase---------------------\r\n");
    FILE *fp;
    char *json_str;
    json_str = (char*)malloc(1000);
    free(json_str);
    fp = fopen("redis_info.json","r");
    fread(json_str,1000,1,fp);
    fclose(fp);
    cJSON *json = cJSON_Parse((const char*)json_str);
    cJSON *name;
    name = cJSON_GetObjectItemCaseSensitive(json, "name");
    printf("name: %s\r\n",name->valuestring);
    */
    // never arrive here
    return 0;
}