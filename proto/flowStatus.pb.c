/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.9 at Sat Mar 02 14:46:22 2019. */

#include "flowStatus.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t ValveStatus_fields[5] = {
    PB_FIELD(  1, INT32   , OPTIONAL, STATIC  , FIRST, ValveStatus, pinNumber, pinNumber, 0),
    PB_FIELD(  2, BOOL    , OPTIONAL, STATIC  , OTHER, ValveStatus, isOn, pinNumber, 0),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, ValveStatus, isScheduled, isOn, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, ValveStatus, flowTimeRemainingMs, isScheduled, 0),
    PB_LAST_FIELD
};

const pb_field_t FlowStatusMessage_fields[6] = {
    PB_FIELD(  1, MESSAGE , OPTIONAL, STATIC  , FIRST, FlowStatusMessage, valveStatus, valveStatus, &ValveStatus_fields),
    PB_FIELD(  2, INT32   , OPTIONAL, STATIC  , OTHER, FlowStatusMessage, maxRunTimeMs, valveStatus, 0),
    PB_FIELD(  3, INT32   , OPTIONAL, STATIC  , OTHER, FlowStatusMessage, mainValveRunTimeMs, maxRunTimeMs, 0),
    PB_FIELD(  4, BOOL    , OPTIONAL, STATIC  , OTHER, FlowStatusMessage, mainValveIsOn, mainValveRunTimeMs, 0),
    PB_FIELD(  5, INT32   , OPTIONAL, STATIC  , OTHER, FlowStatusMessage, mainValvePinNumber, mainValveIsOn, 0),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(FlowStatusMessage, valveStatus) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_ValveStatus_FlowStatusMessage)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(FlowStatusMessage, valveStatus) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_ValveStatus_FlowStatusMessage)
#endif


/* @@protoc_insertion_point(eof) */
