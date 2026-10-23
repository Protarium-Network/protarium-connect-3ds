// Protarium Connect: sends the console's Pretendo NEX principal ID and password to Protarium
// so self-hosted 3DS games can authenticate it. Run once after the console has connected to Pretendo.
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "root_ca.h"

// Override at build time with: make REGISTER_URL=https://your.server/protarium-connect/register
#ifndef REGISTER_URL
#define REGISTER_URL "https://account.protarium.lol/protarium-connect/register"
#endif
#define PASSWORD_SIZE 0x80

static void fail(const char *what, Result res)
{
	printf("%s: %08lX\n", what, res);
	if (res == 0xD8A0A046) { // HTTP module: no connection, usually a DNS lookup failure
		puts("Cannot reach the server: check the Wi-Fi and the DNS of the connection.");
	}
}

static void connect_account(void)
{
	char pid_str[16] = {0};
	char password[PASSWORD_SIZE] = {0};
	FriendKey key = {0};
	httpcContext ctx;
	bool frd_ready = false, http_ready = false, ctx_open = false;
	Result res;

	puts("\nConnecting...");

	res = frdInit(true); // frd:a, needed for FRD_GetMyPassword
	if (R_FAILED(res)) { fail("FRD init failed", res); goto out; }
	frd_ready = true;

	res = FRD_GetMyFriendKey(&key);
	if (R_FAILED(res) || key.principalId == 0) {
		fail("Friends PID unavailable", res);
		puts("Select Pretendo in Nimbus, reboot,");
		puts("then open the Friends List online.");
		goto out;
	}

	res = FRD_GetMyPassword(password, sizeof(password));
	password[PASSWORD_SIZE - 1] = '\0';
	if (R_FAILED(res) || password[0] == '\0') { fail("Pretendo NEX secret unavailable", res); goto out; }

	snprintf(pid_str, sizeof(pid_str), "%lu", key.principalId);
	printf("Friends PID: %s\n", pid_str);

	res = httpcInit(0x2000);
	if (R_FAILED(res)) { fail("HTTP init failed", res); goto out; }
	http_ready = true;

	res = httpcOpenContext(&ctx, HTTPC_METHOD_POST, REGISTER_URL, 1);
	if (R_FAILED(res)) { fail("HTTPS open failed", res); goto out; }
	ctx_open = true;

	res = httpcAddTrustedRootCA(&ctx, root_ca_der, sizeof(root_ca_der));
	if (R_FAILED(res)) { fail("TLS certificate setup failed", res); goto out; }

	res = httpcAddPostDataAscii(&ctx, "pid", pid_str);
	if (R_FAILED(res)) { fail("PID form failed", res); goto out; }

	res = httpcAddPostDataAscii(&ctx, "password", password);
	if (R_FAILED(res)) { fail("Secret form failed", res); goto out; }

	res = httpcBeginRequest(&ctx);
	if (R_FAILED(res)) { fail("HTTPS request failed", res); goto out; }

	u32 status = 0;
	res = httpcGetResponseStatusCode(&ctx, &status);
	if (R_FAILED(res)) { fail("HTTPS response failed", res); goto out; }

	if (status == 200) {
		printf("Account connected for PID %s.\n", pid_str);
		puts("Select Protarium in Nimbus, then reboot.");
	} else {
		printf("Registration failed: HTTP %lu\n", status);
	}

out:
	memset(password, 0, sizeof(password)); // do not leave the secret on the stack
	if (ctx_open) httpcCloseContext(&ctx);
	if (http_ready) httpcExit();
	if (frd_ready) frdExit();
	puts("\nA: retry    START: exit");
}

int main(void)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	puts("Protarium Connect\n");
	puts("First connect to Pretendo Network online.");
	puts("This app sends your console NEX credential");
	puts("to Protarium over HTTPS to link the account.\n");
	puts("A: connect    START: exit");

	while (aptMainLoop()) {
		hidScanInput();
		u32 down = hidKeysDown();
		if (down & KEY_START) break;
		if (down & KEY_A) connect_account();

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}
