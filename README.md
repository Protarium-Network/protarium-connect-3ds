# Protarium Connect (3DS)

A small 3DS homebrew that links a console to a Protarium server. Protarium's self-hosted 3DS games sign consoles in with the NEX
account the console got from Pretendo, so the server has to learn that account once. This app sends it.

## What it does

1. Opens the friends service and reads the console's principal ID and NEX password
   (`FRD_GetMyFriendKey`, `FRD_GetMyPassword`).
2. Sends both to the server as the form fields `pid` and `password` in an HTTPS POST to
   `https://account.protarium.online/protarium-connect/register`.
3. The server answers `200` when it stored them.

The password is cleared from memory as soon as the request is done. The connection only trusts ISRG Root X1 (Let's Encrypt),
which is built in, so it does not depend on the console's certificate store.

## Using it

1. In Nimbus pick Pretendo, reboot, and open the Friends List online once so the console has a Pretendo account.
2. Run Protarium Connect from the Homebrew Launcher and press A.
3. In Nimbus pick Protarium and reboot.

If something goes wrong the bottom screen says what and shows the error code. The usual one is `D8A0A046`: the console could not
reach the server, which is the Wi-Fi or a custom DNS.

## Building

Needs [devkitPro](https://devkitpro.org/) with devkitARM and libctru.

    make

Another server:

    make REGISTER_URL=https://your.server/path

`tools/gen_ca.py` rebuilds `source/ca_root.h` from a PEM root certificate if you need to trust a different one.

## Layout

- `source/link.c`: reading the account and the HTTPS request, no UI.
- `source/main.c`: the screens and the button handling.
- `tools/gen_ca.py`: turns a PEM certificate into the C array.

## License

GNU AGPL v3, see `LICENSE`.
