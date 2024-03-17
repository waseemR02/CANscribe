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
  
  /* Initialize the arrays with 0 */
  for (int i = 0; i < len + 2; i++) {
    array_zeros[i] = 0;
  }
  
  for (int i = 0; i < len; i++) {
    decoded_msg[i] = 0;
  }

  int i = 0, j = 0;

  /*Store the indices of 0's in the array_zeros */
  while (i < len + 1) {
    array_zeros[j] = i;
    i += buf[i];
    j++;
  }
  
  array_zeros[j] = len+1;

  /* New length of array_zeros */
  int new_len = j+1;
  
  /* Update the buffer with 0's at the positions stored in array_zeros */
  for (int k = 0; k < new_len; k++) {
    buf[array_zeros[k]] = 0;
  }
  
  /* Remove preceding and trailing 0's in buf and store in decoded_msg */
  for (int k = 0; k < len + 1; k++) {
    decoded_msg[k] = buf[k+1];
  }
  
  /* 
   * Assign the decoded message to the struct 
   */
  
  /* Assign first 4 bytes as id */
  msg->frame.id = (uint32_t)decoded_msg[0] << 24 | (uint32_t)decoded_msg[1] << 16 | (uint32_t)decoded_msg[2] << 8 | (uint32_t)decoded_msg[3];

  /* Assign next 1 byte as dlc */
  msg->frame.dlc = decoded_msg[4];

  /* Assign next 1 byte as flags */
  msg->frame.flags = decoded_msg[5];

  /* Assign next 2 bytes as timestamp */
  msg->frame.timestamp[k] = (uint32_t)decoded_msg[6] << 8 | (uint32_t)decoded_msg[7];

  /* Assign next 1 byte as 8 bit data */
  msg->frame.union.data[0] = decoded_msg[8];
  
  /* Assign next 4 bytes as 32 bit data */
  int index = 0;
  for (int k = 9; k < 13; k++) {
    msg->frame.union.data_32[index] = decoded_msg[k];
    index++;
  }

  /* Assign remaining 4 bytes as crc */
  msg->crc = (uint32_t)decoded_msg[13] << 24 | (uint32_t)decoded_msg[14] << 16 | (uint32_t)decoded_msg[15] << 8 | (uint32_t)decoded_msg[16];

}

