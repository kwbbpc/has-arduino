#pragma once

#include <stdint.h>

struct Message {
	size_t message_length;
	size_t payload_length;
	uint8_t* payload;
	bool isSuccessEncoded;

	~Message() {
		delete payload;
	}

};


