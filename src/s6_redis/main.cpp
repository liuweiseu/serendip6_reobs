#include <stdio.h>

#include "include/s6_redis.h"

int main()
{
    char value[100];
    get_info_from_redis("127.0.0.1",6379,"first_key",value);
    printf("value: %s\r\n", value);

    put_info_to_redis("127.0.0.1",6379,"first_key","goodbye-wei");
    get_info_from_redis("127.0.0.1",6379,"first_key",value);
    printf("value: %s\r\n", value);
    return 0;
}