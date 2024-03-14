#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

uint8_t* str_to_byte(char str[]) {
    int len = strlen(str);
    uint8_t* byte_val = malloc(len * sizeof(uint8_t));

    for (int i = 0; i < len; i++) {
        byte_val[i] = (uint8_t)str[i];
    }
    return byte_val;
}

uint8_t* serialize(uint8_t* data, int len) {
    //printf("%d", len);
    uint8_t* serialized = malloc((len+2)*sizeof(uint8_t));
    uint8_t* array_zeros = malloc((len+2)*sizeof(uint8_t));

    serialized[0] = 0;
    for (int i = 0; i < len; i++) {
      serialized[i+1] = data[i]; //data accessing beyond length
    }
    serialized[len+2-1] = 0;
   
    int zero_count = 0; 
    for (int i = 0; i < len+2; i++) {
      if (serialized[i] == 0) {
        array_zeros[zero_count] = i;
        zero_count++;
      }
    }

    int k = 0;
    //array_zeros = malloc((zero_count)*sizeof(uint8_t));
    for (int i = 0; i < zero_count - 1; i++) {
      k = array_zeros[i+1] - array_zeros[i];
      serialized[(int)array_zeros[i]] = k;
    }

    return serialized;
    //serialized[len + 1] = 0x00;
}

int main() {

    /*
    * Get the string and convert it to byte values
    */
    const int MAX_LEN = 255;
    char str[MAX_LEN];

    printf("Type anything: ");
    scanf("%s", str);
    int len = strlen(str);

    uint8_t* val = str_to_byte(str);

    // Print the byte values
    printf("Byte values: ");
    for (int i = 0; i < len; i++) {
        printf("%x ", val[i]);
    }

    printf("\n");

    //free(val); // Free the allocated memory

    /*------------------------------------------------*/

    /*
    * Serialize the byte values
    */
    uint8_t* data = serialize(val, len);
    printf("Serialized: ");
    for (int i = 0; i < len + 2; i++) {
        printf("%x ", data[i]);
    }
    printf("\n");
    /*-----------------------------------------------------*/
    return 0;
}
