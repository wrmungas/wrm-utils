#ifndef WRM_LOG_H
#define WRM_LOG_H
/* --- HEADER DESCRIPTION -----------------------------------------------------

File log.h

Created Feb 7, 2026
by William R Mungas (wrm)

Last modified Feb 7, 2025

Contributors: 
wrm: creator

DESCRIPTION:
System for buffering messages from each module

FEATURES:
- manages message buffers
- provides default INFO and ERROR buffers and default print functions for each
- users can create new buffers as well for specific purposes
- users can determine when and how each buffer should print its messages

IMPORTANT DETAILS:
Messages are currently stored contiguously with a NUL separating each;
this means a single byte will be wasted for each new message, which might
play a factor in the limiting message sizes on non-dynamic buffers.

Dynamic buffers will resize to make room for additional messages; furthermore,
all buffers will automatically clear themselves to using the start of their 
memory when they detect the user has polled the last message queued

REQUIREMENTS:
- wrm common functionality

---------------------------------------------------------------------------- */

#include "common.h"

/* --- TYPE DECLARATIONS --------------------------------------------------- */

/*
Really, an independent wrm_Index; used so this doesn't have a circular
dependency with the memory module
*/
struct wrm_log_Buffer;

/* --- TYPE DEFINITIONS ---------------------------------------------------- */

struct wrm_log_Buffer {
    u32 idx;
};

/* --- CONSTANTS ----------------------------------------------------------- */

// none

/* --- GLOBALS ------------------------------------------------------------- */

extern const wrm_log_Buffer wrm_log_info_buffer;
extern const wrm_log_Buffer wrm_log_error_buffer;

/* --- FUNCTION DECLARATIONS ----------------------------------------------- */

/*
Initialize the logging system
*/
void wrm_log_init(void);
/*
Shut down the logging system - free all buffers
Should be shut down after most wrm modules, since they use this
*/
void wrm_log_quit(void);
/*
Log a message to the info buffer
If `src` is not null, the buffer will store the message with a header
consisting of "{src}:\n"
*/
void wrm_log_info(const char *src, const char *format, ...);
/*
Log a message to the error buffer
If `src` is not null, the buffer will store the message with a header
consisting of "{src}:\n"
*/
void wrm_log_error(const char *src, const char *format, ...);
/*
Create a buffer capable of storing a total of `bytes` bytes
Can store a single message up to `bytes - 1` ascii characters, making room
for NUL
If the buffer is `dynamic`, it will allocate extra space for new messages
If the buffer is `smart`, it will reset its memory as soon as a user has
polled the last message that was added
*/
wrm_log_Buffer wrm_log_createBuffer(size_t bytes, bool dynamic, bool smart);
/*
Log a message to a buffer
If `src` is not NULL, the buffer will store the message with a header
consisting of "{src}:\n"
*/
void wrm_log(wrm_log_Buffer buffer, const char *src, const char *format, ...);
/*
Sets a buffer's message handler
If this is NULL, the buffer will queue messages until the user asks for them
By default, the info and log buffers will set a handler that immediately 
gets the the message and prints it to standard out/error
*/
void wrm_log_setBufferHandler(wrm_log_Buffer buffer, 
    wrm_FUNC(print, void, const char *) );
/*
Used to poll the next message from a buffer, if there is any
If the buffer is `smart`, the returned message should be used immediately,
since it might be overwritten if another message is added to the buffer:
"if storage for later is desired, a strcopy is required"
*/
const char * wrm_log_pollBuffer( wrm_log_Buffer buffer );

#endif // end include guards