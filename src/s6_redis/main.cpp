#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "include/s6_redis.h"
#include "include/cJSON.h"

#define HOST    "127.0.0.1"
#define PORT    6379

/* define the redis key struct */
#define N_FIELDS  45
#define REDIS_KEY "KY_ZK_RUN_DATA_RESULT_HASH"
struct redis_fields{
    const char *field[N_FIELDS];
};
struct redis_fields fields={"TimeStamp"     , "DUT1"          , "Receiver"      ,
                            "SDP_PhaPos_X"  , "SDP_PhaPos_Y"  , "SDP_PhaPos_Z"  , "SDP_AngleM"    ,
                            "SDP_Beam00_RA" , "SDP_Beam00_DEC", 
                            "SDP_Beam01_RA" , "SDP_Beam01_DEC", 
                            "SDP_Beam02_RA" , "SDP_Beam02_DEC",
                            "SDP_Beam03_RA" , "SDP_Beam03_DEC",
                            "SDP_Beam04_RA" , "SDP_Beam04_DEC", 
                            "SDP_Beam05_RA" , "SDP_Beam05_DEC",
                            "SDP_Beam06_RA" , "SDP_Beam06_DEC",
                            "SDP_Beam07_RA" , "SDP_Beam07_DEC",
                            "SDP_Beam08_RA" , "SDP_Beam08_DEC",
                            "SDP_Beam09_RA" , "SDP_Beam09_DEC",
                            "SDP_Beam10_RA" , "SDP_Beam10_DEC",
                            "SDP_Beam11_RA" , "SDP_Beam11_DEC",
                            "SDP_Beam12_RA" , "SDP_Beam12_DEC",
                            "SDP_Beam13_RA" , "SDP_Beam13_DEC", 
                            "SDP_Beam14_RA" , "SDP_Beam14_DEC", 
                            "SDP_Beam15_RA" , "SDP_Beam15_DEC", 
                            "SDP_Beam16_RA" , "SDP_Beam16_DEC", 
                            "SDP_Beam17_RA" , "SDP_Beam17_DEC", 
                            "SDP_Beam18_RA" , "SDP_Beam18_DEC"};

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
        for(int i = 0; i < N_FIELDS; i++)
        {
            memset(value,0,100);
            get_info_from_redis(HOST,PORT,REDIS_KEY, fields.field[i],value);
            cJSON *key = cJSON_CreateString(value);
            cJSON_AddItemToObject(redis_info,fields.field[i],key);
        }
        string = cJSON_Print(redis_info);
        //printf(string);
        write_json(string);
        //fprintf(stdout,"%s\r\n","Read data from redis database!");
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