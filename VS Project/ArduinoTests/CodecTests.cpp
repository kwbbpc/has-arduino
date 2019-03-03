#define BOOST_TEST_MODULE arduino_tests

#include <boost/test/unit_test.hpp>
#include "flowCommand.pb.h"
#include "NanoPbCodec.h"

BOOST_AUTO_TEST_SUITE(codec_test_suite)


BOOST_AUTO_TEST_CASE(test_decode_flow_control_message)
{
	uint8_t msg[] = { 3,8,2,16,1,24,136,39 };

	FlowCommandMessage decodedMsg = FlowCommandMessage_init_zero;
	messaging::nanopb::decodeMessage(msg, 8, FlowCommandMessage_fields, decodedMsg);


}


BOOST_AUTO_TEST_SUITE_END()