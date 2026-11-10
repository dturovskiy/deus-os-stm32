/* Deus Node Link v0 — frozen REV2, source-only contract.
 * A000017 E002 T007 G001; frozen source SHA-256:
 * 899E2FBF4366CE91C31D6B1E6B481195DEECE5DF21AD4495F78E1D450DE406BA
 * ISO C11, no HAL/SDK, no heap, no hardware I/O.
 */
#ifndef DEUS_NODE_LINK_H
#define DEUS_NODE_LINK_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DNL_VERSION 1u
#define DNL_NODE_ID 1u
#define DNL_MAX_PAYLOAD 24u
#define DNL_MAX_FRAME 40u
#define DNL_HEADER_BYTES 14u
#define DNL_FRAME_OVERHEAD 16u
#define DNL_MAX_JOB_BYTES 16u
#define DNL_PARTIAL_TIMEOUT_MS 50u

enum {
 DNL_HELLO=0x01, DNL_PING=0x02, DNL_JOB_CRC32=0x03, DNL_GET_STATUS=0x04,
 DNL_HELLO_ACK=0x81, DNL_PONG=0x82, DNL_JOB_RESULT=0x83,
 DNL_STATUS=0x84, DNL_ERROR=0xff
};
enum {
 DNL_ERR_UNSUPPORTED=1, DNL_ERR_BAD_PAYLOAD=2, DNL_ERR_BUSY=3,
 DNL_ERR_BAD_SEQUENCE=4, DNL_ERR_NOT_READY=5
};
enum {
 DNL_OK=0, DNL_DROP=1, DNL_BAD_ARGUMENT=2, DNL_BAD_FRAME=3,
 DNL_BAD_MESSAGE=4, DNL_OUTPUT_TOO_SMALL=5, DNL_NO_SESSION=6,
 DNL_WRONG_DIRECTION=7, DNL_UNSOLICITED=8, DNL_STALE_SESSION=9,
 DNL_WRONG_SEQUENCE=10, DNL_WRONG_REPLY_TYPE=11,
 DNL_BAD_RESULT_LENGTH=12, DNL_BAD_HELLO_ECHO=13,
 DNL_REMOTE_ERROR=14, DNL_REHANDSHAKE_REQUIRED=15,
 DNL_NOT_READY_OR_BUSY=16, DNL_ALREADY_RETRIED=17
};
typedef struct {
 uint8_t type;
 uint16_t sequence;
 uint32_t session_id;
 uint8_t payload_length;
 uint8_t payload[DNL_MAX_PAYLOAD];
} dnl_frame_t;

typedef struct {
 uint8_t bytes[DNL_MAX_FRAME];
 uint8_t used;
 uint8_t has_started;
 uint32_t started_at_ms;
} dnl_stream_t;

typedef struct {
 uint32_t session_id;
 uint16_t highest_sequence;
 uint16_t completed_jobs;
 uint8_t last_request[DNL_MAX_FRAME];
 uint8_t last_request_length;
 uint8_t last_reply[DNL_MAX_FRAME];
 uint8_t last_reply_length;
 uint8_t hello_reply[DNL_MAX_FRAME];
 uint8_t hello_reply_length;
 uint8_t has_last;
} dnl_worker_t;

typedef struct {
 uint32_t session_id;
 uint16_t last_sequence;
 uint8_t ready;
 uint8_t pending;
 uint8_t pending_type;
 uint8_t retry_count;
 uint8_t requested_job_length;
 uint8_t request_bytes[DNL_MAX_FRAME];
 uint8_t request_length;
} dnl_controller_t;

/* Pure functions: output only on success, never call hardware functions. */
uint16_t dnl_crc16_ccitt_false(const uint8_t *bytes, size_t count);
uint32_t dnl_crc32_ieee(const uint8_t *bytes, size_t count);
int dnl_encode(const dnl_frame_t *frame, uint8_t *output, size_t capacity,
               size_t *output_length);
int dnl_decode(const uint8_t *bytes, size_t count, dnl_frame_t *frame);
int dnl_validate_payload(const dnl_frame_t *frame);
int dnl_is_request(uint8_t type);
int dnl_is_reply(uint8_t type);
uint32_t dnl_response_timeout_ms(uint8_t request_type);

void dnl_stream_init(dnl_stream_t *stream);
/* Returns DNL_OK only when one complete frame was emitted. DNL_DROP otherwise.
 * Must be fed one byte at a time and time must be monotonic modulo uint32_t.
 */
int dnl_stream_feed(dnl_stream_t *stream, uint8_t byte, uint32_t time_ms,
                    dnl_frame_t *frame);

void dnl_worker_init(dnl_worker_t *worker);
/* DNL_OK means a reply was placed in output. DNL_DROP means no reply.
 * Valid request with unsupported type returns DNL_ERROR. */
int dnl_worker_process(dnl_worker_t *worker,
                       const uint8_t *request, size_t request_length,
                       uint8_t *reply, size_t capacity, size_t *reply_length);

void dnl_controller_init(dnl_controller_t *controller);
int dnl_controller_hello(dnl_controller_t *controller, uint32_t challenge,
                         uint8_t *output, size_t capacity, size_t *output_length);
int dnl_controller_request(dnl_controller_t *controller, uint8_t type,
                           const uint8_t *payload, uint8_t length,
                           uint8_t *output, size_t capacity, size_t *output_length);
/* Returns DNL_OK only for matched success, DNL_REMOTE_ERROR for matched failure.
 * Unsolicited/wrong-session/out-of-order/wrong payload never consume pending.
 */
int dnl_controller_receive(dnl_controller_t *controller,
                           const uint8_t *reply, size_t length);
/* On first allowed timeout: DNL_OK + byte-identical retry.
 * On final timeout or JOB timeout: DNL_DROP + fail closed.
 */
int dnl_controller_timeout(dnl_controller_t *controller,
                           uint8_t *output, size_t capacity, size_t *output_length);
#ifdef __cplusplus
}
#endif
#endif
