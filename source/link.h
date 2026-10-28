#pragma once

#include <3ds.h>

// What the server needs to know about this console: the NEX identity the console got from Pretendo.
typedef enum {
	LINK_OK,
	LINK_NO_FRIEND_SERVICE, // the friends service did not open
	LINK_NO_ACCOUNT,        // no principal ID: the console never signed in to Pretendo
	LINK_NO_SECRET,         // the NEX password could not be read
	LINK_NETWORK,           // no connection to the server (Wi-Fi, DNS)
	LINK_TLS,               // the server certificate was not accepted
	LINK_REJECTED,          // the server answered with something other than 200
	LINK_INTERNAL,          // the HTTP module refused something before talking to the server
} LinkStatus;

typedef struct {
	LinkStatus status;
	Result code;   // the libctru result behind a failure, 0 when there is none
	u32 http;      // HTTP status when the server answered
	u32 principal; // the console's principal ID once it has been read
} LinkResult;

// Called before each step so the caller can draw it: every step after this one may block for a while.
typedef void (*LinkProgress)(const char *step);

LinkResult link_console(LinkProgress progress);

// One line a person can act on.
const char *link_explain(const LinkResult *result);
