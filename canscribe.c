#include <canscribe.h>

/*
 * Serialize with COBS
 * */ 
void serialize(uint8_t *buf, struct canscribe_msg *msg, int len) {
    
	uint8_t array_zeroes[len+2];

	/* Caste the cansribe message to a byte pointer */
	uint8_t *byte_msg = (uint8_t *)msg;

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

/*
 * Deserialize with COBS
 * */
void deserialize(uint8_t *buf, struct canscribe_msg *msg, int len) {
  
  uint8_t array_zeros[len+2];
  uint8_t decoded_msg[len];

  uint8_t *data = buf;

  for (int i = 0; i < len + 2; i++) {
    array_zeros[i] = 0;
  }

  for (int i = 0; i < len; i++) {
    decoded_msg[i] = 0;
  }

  int i = 0, j = 0;

  while (j < len + 2) {
    array_zeros[j] = i;
    i = data[i];
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

