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

void serialize(uint8_t *buf, struct canscribe_msg *msg, int len) {
    
	uint8_t array_zeroes[len+2];

	/* Caste the cansribe message to a byte pointer */
	uint8_t *byte_msg = (uint8_t *)msg;
  
  buf = (uint8_t *)malloc((len+2) * sizeof(uint8_t));

	/* Store the data and the zeros in the message array */
	buf[0] = 0; // First element of message
	for (int i = 0; i < len; i++) {
		buf[i+1] = byte_msg[i]; //assign values from struct in message
	}
	buf[len+2-1] = 0; //Last element of message
   
	uint8_t zero_count = 0; //variable to count number of zeros in message 
    
	/* Store the position of 0's in the array_zeros */
	for (int i = 0; i < len+2; i++) {
		if (buf[i] == 0) {
			array_zeroes[zero_count] = i; 
			zero_count++;
		}
	}

	int k = 0; /* variable to store the difference in the pos of adjacent 0's */
    
	/* Update the serialized array with the difference in the position of adjacent 0's */
	for (uint8_t i = 0; i < zero_count - 1; i++) {
		k = array_zeroes[i+1] - array_zeroes[i]; 
		buf[array_zeroes[i]] = k;
	}
}

void deserialize(uint8_t* buf, struct canscribe_msg *msg, int len) {
  
  uint8_t array_zeros[len+2];
  uint8_t decoded_msg[len];
  
  buf = (uint8_t *)malloc((len+2) * sizeof(uint8_t));

  for (int i = 0; i < len+2; i++) {
    array_zeros[i] = 0;
  }

  for (int i = 0; i < len; i++) {
    decoded_msg[i] = 0;
  }

  int i = 0;
  int j = 0;
  
  while (j < len) {
    array_zeros[j] = i;
    i = buf[i];
    j++;
  }
  
  int new_len = sizeof(array_zeros)/sizeof(array_zeros[0]);

  for (int k = 0; k < new_len; k++) {
    buf[array_zeros[k]] = 0;
  }

  for (int k = 0; k < len + 1; k++) {
    decoded_msg[k] = buf[k+1];
  }
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
    /*printf("Serialized: ");
    for (int i = 0; i < 16 + 2; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");*/

    deserialize(data, &msg, 16);
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
