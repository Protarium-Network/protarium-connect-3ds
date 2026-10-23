# Protarium Connect (3DS)

Homebrew app that sends the console's Pretendo NEX principal ID and password (`FRD_GetMyFriendKey` /
`FRD_GetMyPassword`, via `frd:a`) to `https://account.protarium.online/protarium-connect/register` as the form fields `pid`
and `password`. The server stores them so self-hosted 3DS games can log the console in. HTTP 200 means success.
TLS uses the embedded ISRG Root X1 (`include/root_ca.h`).

Run it once, after the console has connected to Pretendo (Nimbus set to Pretendo, then the Friends List opened online).

## Build
Needs devkitARM + libctru and `3dsxtool`/`smdhtool`: `make`. To talk to another server: `make REGISTER_URL=https://host/protarium-connect/register`.

## Provenance
The original source was lost. `source/main.c` was rewritten from the behaviour of the released `ProtariumConnect.3dsx` (call sequence,
messages, form fields and the embedded certificate all match). It builds with devkitARM and libctru and has been run on a console; the code generated differs from the release,
the behaviour should not.

## License
GNU AGPL v3, see `LICENSE`.
