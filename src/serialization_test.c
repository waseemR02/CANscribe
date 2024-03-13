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

uint8_t* serialize(uint8_t* data, uint8_t len) {
    uint8_t* serialized = malloc(len + 2);
    serialized[0] = 0x00;
    for (int i = 0; i < len; i++) {
        serialized[i+1] = data[i];
    }
    serialized[len + 2] = 0x00;

    for (int i = len; i >= 0; i--) {
        if (data[i] == 0x00) {
            data[i] = (uint8_t) i;
        }
    }
    return serialized;
}

int main() {

    /*
    * Get the string and convert it to byte values
    */
    int len;
    char str[len];

    printf("Type anything: ");
    scanf("%s", str);
    len = strlen(str);

    uint8_t* val = str_to_byte(str);

    // Print the byte values
    printf("Byte values: ");
    for (int i = 0; i < len; i++) {
        printf("%x ", val[i]);
    }

    printf("\n");

    free(val); // Free the allocated memory

    /*------------------------------------------------*/

    /*
    * Serialize the byte values
    */
    uint8_t* data = serialize(val, len);

    printf("Serialized: ");
    for (int i = 0; i < len + 2; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
    /*-----------------------------------------------------*/
    return 0;
}