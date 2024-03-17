#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct can_frame {
    uint8_t data[8];
    uint32_t id;
};

struct canscribe_msg {
    struct can_frame frame;
    uint32_t crc;
};

uint8_t* str_to_byte(char str[]) {
    int len = strlen(str);
    uint8_t* byte_val = malloc(len * sizeof(uint8_t));

    for (int i = 0; i < len; i++) {
        byte_val[i] = (uint8_t)str[i];
    }
    return byte_val;
}

int serialize(uint8_t *message, struct canscribe_msg *msg, int len) {
    
	/*Allocate Memory*/
  message = malloc((len+2)*sizeof(uint8_t));
  uint8_t* array_zeros = malloc((len+2)*sizeof(uint8_t));

	/*
	* Store the data and the zeros in the message array
	*/
  uint8_t *p = (uint8_t *)msg; 

  message[0] = 0; //First element of message
	for (int i = 0; i < len; i++) {
  message[i+1] = p[i]; //assign values from struct in message
  }
	message[len+2-1] = 0; //Last element of message
   
  int zero_count = 0; //variable to count number of zeros in message 
    
	/*
	* Store the position of 's in the array_zeros
	*/
	for (int i = 0; i < len+2; i++) {
    if (message[i] == 0) {
      array_zeros[zero_count] = i; 
      zero_count++;
    }
  }

  int k = 0; //variable to store the difference in the pos of adjacent 0's
    
	/*
	* Update the serialized array with the difference in the position of adjacent 0's
	*/
	for (int i = 0; i < zero_count - 1; i++) {
    k = array_zeros[i+1] - array_zeros[i]; //Compute difference
    message[(int)array_zeros[i]] = k;
  }

  free(array_zeros);

  return 0;
}

uint8_t* deserialize(uint8_t* data, int len) {
  
  uint8_t* array_zeros = malloc(len*sizeof(uint8_t));
  uint8_t* deserialized = malloc(len*sizeof(uint8_t));
  
  int i = 0;
  int j = 0;
  
  while (j < len) {
    array_zeros[j] = i;
    i = data[i];
    j++;
  }

  for (int i = 0; i < len+2; i++) {
    data[(int)array_zeros[i]] = 0;
  }

  for (int i = 0; i < len; i++) {
    deserialized[i] = data[i+1];
  }

  return deserialized;
}

int main() {

    /*
    * Get the string and convert it to byte values
    */
    const int MAX_LEN = 255;
    char str[MAX_LEN];

    /*printf("Type anything: ");
    scanf("%s", str);
    int len = strlen(str);

    uint8_t* val = str_to_byte(str);

    // Print the byte values
    printf("Byte values: ");
    for (int i = 0; i < len; i++) {
        printf("%x ", val[i]);
    }

    printf("\n"); */

    //free(val); // Free the allocated memory

    /*------------------------------------------------*/

    /*
    * Serialize the byte values
    */

    struct canscribe_msg msg;
    for(int i = 0; i < 8; i++) {
      msg.frame.data[i] = i;
    }
    msg.frame.id = 123;

    msg.crc = 0;
    
    uint8_t *data;

    serialize(data, &msg, 16);
    printf("Serialized: ");
    for (int i = 0; i < 16 + 2; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
    /*-----------------------------------------------------*/
    free(data);
    /*uint8_t* decoded_data = deserialize(data, len+2);
    printf("Deserialized: ");
    for (int i = 0; i < len; i++) {
      printf("%x ", decoded_data[i]);
    }
    printf("\n");
    return 0;*/
}
