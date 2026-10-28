#include "link.h"

#include <stdio.h>
#include <string.h>

#include "ca_root.h"

// Build with REGISTER_URL=... to talk to another server (see the Makefile).
#ifndef REGISTER_URL
#define REGISTER_URL "https://account.protarium.online/protarium-connect/register"
#endif

#define SECRET_MAX 0x80
#define HTTP_SHARED_MEMORY 0x2000
#define RESPONSE_TIMEOUT_NS (20ULL * 1000 * 1000 * 1000)

// Result codes the HTTP module hands back through libctru, see the HTTP page on 3dbrew.
#define RES_HTTP_NO_CONNECTION 0xD8A0A046
#define RES_HTTP_CERT_REJECTED 0xD8A0A03C

static LinkResult failure(LinkStatus status, Result code)
{
	LinkResult result = { status, code, 0, 0 };
	return result;
}

// The admin friends service is the default in libctru. The user one is a fallback for firmwares that refuse the first.
static Result open_friend_service(void)
{
	Result res = frdInit(false);
	if (R_FAILED(res)) res = frdInit(true);
	return res;
}

// The server only sends a short answer, but libctru hangs on close if a body is left unread.
static void drain_body(httpcContext *ctx)
{
	u8 sink[256];
	Result res;
	do {
		res = httpcReceiveDataTimeout(ctx, sink, sizeof(sink), RESPONSE_TIMEOUT_NS);
	} while (res == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING);
}

static LinkStatus classify(Result code)
{
	if (code == (Result)RES_HTTP_NO_CONNECTION) return LINK_NETWORK;
	if (code == (Result)RES_HTTP_CERT_REJECTED) return LINK_TLS;
	return LINK_INTERNAL;
}

static LinkResult post_identity(u32 principal, const char *secret)
{
	char principal_text[16];
	snprintf(principal_text, sizeof(principal_text), "%lu", (unsigned long)principal);

	httpcContext ctx;
	Result res = httpcOpenContext(&ctx, HTTPC_METHOD_POST, REGISTER_URL, 1);
	if (R_FAILED(res)) return failure(classify(res), res);

	LinkResult result = failure(LINK_INTERNAL, 0);

	res = httpcAddTrustedRootCA(&ctx, CA_ROOT, CA_ROOT_SIZE);
	if (R_SUCCEEDED(res)) res = httpcAddPostDataAscii(&ctx, "pid", principal_text);
	if (R_SUCCEEDED(res)) res = httpcAddPostDataAscii(&ctx, "password", secret);
	if (R_SUCCEEDED(res)) res = httpcBeginRequest(&ctx);
	if (R_FAILED(res)) {
		result = failure(classify(res), res);
		goto done;
	}

	u32 status = 0;
	res = httpcGetResponseStatusCodeTimeout(&ctx, &status, RESPONSE_TIMEOUT_NS);
	if (R_FAILED(res)) {
		result = failure(classify(res), res);
		goto done;
	}

	drain_body(&ctx);

	result = failure(status == 200 ? LINK_OK : LINK_REJECTED, 0);
	result.http = status;

done:
	httpcCloseContext(&ctx);
	return result;
}

LinkResult link_console(LinkProgress progress)
{
	char secret[SECRET_MAX] = { 0 };
	FriendKey key = { 0 };
	LinkResult result;

	progress("Reading the console account");
	Result res = open_friend_service();
	if (R_FAILED(res)) return failure(LINK_NO_FRIEND_SERVICE, res);

	res = FRD_GetMyFriendKey(&key);
	if (R_FAILED(res) || key.principalId == 0) {
		frdExit();
		return failure(LINK_NO_ACCOUNT, res);
	}

	res = FRD_GetMyPassword(secret, sizeof(secret));
	secret[SECRET_MAX - 1] = '\0';
	frdExit();
	if (R_FAILED(res) || secret[0] == '\0') {
		memset(secret, 0, sizeof(secret));
		return failure(LINK_NO_SECRET, res);
	}

	progress("Sending it to Protarium");
	res = httpcInit(HTTP_SHARED_MEMORY);
	if (R_FAILED(res)) {
		memset(secret, 0, sizeof(secret));
		return failure(LINK_INTERNAL, res);
	}

	result = post_identity(key.principalId, secret);
	result.principal = key.principalId;

	httpcExit();
	memset(secret, 0, sizeof(secret)); // the password must not stay on the stack
	return result;
}

const char *link_explain(const LinkResult *result)
{
	switch (result->status) {
		case LINK_OK:
			return "Done. In Nimbus pick Protarium and reboot.";
		case LINK_NO_FRIEND_SERVICE:
			return "The friends service did not open. Reboot the console and try again.";
		case LINK_NO_ACCOUNT:
			return "No Pretendo account yet. Pick Pretendo in Nimbus, reboot, open the Friends List online, then come back.";
		case LINK_NO_SECRET:
			return "The console secret could not be read. Open the Friends List online once, then try again.";
		case LINK_NETWORK:
			return "Cannot reach the server. Check the Wi-Fi, and the DNS if you changed it.";
		case LINK_TLS:
			return "The server certificate was refused. Check the console date and time.";
		case LINK_REJECTED:
			return "The server refused the registration.";
		default:
			return "Something failed before reaching the server.";
	}
}
