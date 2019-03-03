#ifndef NANOPBCODEC_H
#define NANOPBCODEC_H


#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "MessageIds.h"



namespace messaging{ 
	
	
	struct Message {

		//the message id.  This is also included in the payload as part of the metadata.
		uint8_t messageId;

		//the entire message length including any metadata (i.e. the message id)
		// size of the metadata header will be message_length - payload_length
		size_t message_length;

		//the raw protobuf payload length.
		size_t payload_length;

		//the entire payload + metadata
		uint8_t* payload;

		//success or fail to encode - says whether the payload is valid or not
		bool isSuccessEncoded;

		//Destructor
		~Message() {
			delete[] payload;
		}

	};
	
	
	namespace nanopb{
	
		const uint8_t header_size = 1;


	//! Decodes a message payload
	//! buffer - the buffer the entire message (includes all metadata)
	//! length - the message length in the buffer
	//! fieldsType[out] - a reference to the fields type corresponding to the payload message
	//! decodededMessage[out] - a reference to the container to store the decoded payload message in
	//!
	//! returns bool - successfully decoded
	template<class MSGTYPE>
	bool decodeMessage(uint8_t* buffer, const uint16_t length, const pb_field_t fields[], MSGTYPE& decodedMessage)
	{
		//make a stream to read from the buffer
		pb_istream_t istream = pb_istream_from_buffer(&buffer[header_size], length - header_size);

		//pull out the header and check if this is the destination board
		bool success = pb_decode(&istream, fields, &decodedMessage);

		return success;
	};


	//! Encodes a message payload.  returns the number of bytes that were written to the buffer.  If 
	//! encoding failed, then 0 bytes are returned as written.  Make sure the buffer passed in is cleaned up!
	//! message - the message to be encoded
	//! encodedMessage (out) - the serialized message
	template<class MSGTYPE>
	Message* encodeMessage(uint8_t messageId, const MSGTYPE* message, const pb_field_t fields[])
	{

		//encode the message
		static const int BufferMaxSize = 512;
		uint8_t* encodedMessage = new uint8_t[BufferMaxSize];

		//create the stream that will write to the buffer
		pb_ostream_t stream = pb_ostream_from_buffer(encodedMessage, BufferMaxSize);


		//encode the message
		bool status = pb_encode(&stream, fields, message);

		//add a byte for the message id
		size_t payload_length = stream.bytes_written;
		uint8_t* payloadWithId = new uint8_t[payload_length + 1];
		//copy the payload to the new payload
		memcpy(&payloadWithId[1], encodedMessage, payload_length);
		delete[] encodedMessage;

		//set the message id in the first byte
		payloadWithId[0] = messageId;

		//format a struct and return it
		Message*  m = new Message;
		m->payload_length = payload_length;
		m->message_length = payload_length + 1;
		m->payload = payloadWithId;
		m->isSuccessEncoded = status;
		m->messageId = messageId;

		return m;
	};
		
}}

#endif