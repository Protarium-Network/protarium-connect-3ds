// Protarium Connect: tells a Protarium server which Pretendo account this console uses, so self-hosted 3DS games can sign it in.
#include <3ds.h>
#include <stdio.h>

#include "link.h"

static PrintConsole top_screen;
static PrintConsole bottom_screen;

// The HTTP module blocks, so whatever is going to be shown has to reach the screen first.
static void present(void)
{
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
}

static void show_step(const char *step)
{
	consoleSelect(&bottom_screen);
	printf("- %s...\n", step);
	present();
}

static void draw_home(void)
{
	consoleSelect(&top_screen);
	consoleClear();
	puts("Protarium Connect\n");
	puts("Links this console to Protarium by sending");
	puts("its Pretendo NEX account (ID and password)");
	puts("over HTTPS.\n");
	puts("Do this once, after the console has signed");
	puts("in to Pretendo.\n");
	puts("A: link this console     START: exit");
}

static void run_link(void)
{
	consoleSelect(&bottom_screen);
	consoleClear();

	LinkResult result = link_console(show_step);

	consoleSelect(&bottom_screen);
	if (result.principal != 0) printf("\nAccount: %lu\n", (unsigned long)result.principal);
	if (result.http != 0) printf("Server answered: HTTP %lu\n", (unsigned long)result.http);
	if (result.code != 0) printf("Error: %08lX\n", (unsigned long)result.code);
	printf("\n%s\n", link_explain(&result));
}

int main(void)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, &top_screen);
	consoleInit(GFX_BOTTOM, &bottom_screen);
	draw_home();

	while (aptMainLoop()) {
		hidScanInput();
		const u32 pressed = hidKeysDown();
		if (pressed & KEY_START) break;
		if (pressed & KEY_A) run_link();
		present();
	}

	gfxExit();
	return 0;
}
