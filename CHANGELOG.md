<a name="v1.18.3"></a>

## v1.18.3 (2025-02-25)

### Release notes

Translations, UI improvements, notifications, and some bugfixes.

This release is bringing several UI improvements and translation fixes thanks to
contributions and suggestions from our users. Also, we've merged with another
fork of qTox and the maintainer of that fork has contributed all their
improvements, making this the most stable version of qTox in years.

There are still many things on our TODO list, most notably we'll be focussing on
adding new group chat support with moderation, roles, and group ownership. As
always, if you find any issues with this release, please let us know by filing
an issue.

#### Bug Fixes

- Use correct bug template in report bug button. ([d2842696](https://github.com/TokTok/qTox/commit/d2842696399fbe26474655175a5221d960055606))

<a name="v1.18.3-rc.1"></a>

## v1.18.3-rc.1 (2025-02-20)

### Release notes

Release candidate. We'll write proper release notes in the production release.

#### Bug Fixes

- **About:** Retranslate the important message when changing language. ([3b6b821c](https://github.com/TokTok/qTox/commit/3b6b821c83ae95a956dde710dfe3db8392ef147c))
- **Clipboard:** Improve "copy link to clipboard" on Linux. ([8b1ac36a](https://github.com/TokTok/qTox/commit/8b1ac36adc9a838b26d67975fb1f53875b74beb5))
- **Notify:**
  - Always put username in conference notifications. ([3c42042b](https://github.com/TokTok/qTox/commit/3c42042b5eedb328ca11cb037c902494ad85ef2a))
  - Notify sound setting disables all sounds. ([6f75c720](https://github.com/TokTok/qTox/commit/6f75c7208db5c721a0e3785f9a028d3796060bd2))
  - Ungroup dbus notifications. ([becaa39d](https://github.com/TokTok/qTox/commit/becaa39d25769c23eaa868b1da530517f2db3ea1))
  - Use notification categories on Linux. ([10c124a1](https://github.com/TokTok/qTox/commit/10c124a1bf5278c818232abeae2e78c727aa2ecc), closes [#424](https://github.com/TokTok/qTox/issues/424))
- **Settings:** Don't allow invalid proxy hosts in settings. ([63eb1f02](https://github.com/TokTok/qTox/commit/63eb1f02166ed00fff290138addd3e3c6a8e5407))
- **Translations:** Various improvements on the UI strings. ([4a81049b](https://github.com/TokTok/qTox/commit/4a81049bfa6bcbf65924c3393807fe0be6fb9fa6))
- **chatform:**
  - fix the status button alignment ([e4c03765](https://github.com/TokTok/qTox/commit/e4c0376523870b4c7b2d1c2b88c5d91d3869802b))
  - Remove assertion that history is on when the friend details is being called. ([9785e439](https://github.com/TokTok/qTox/commit/9785e439f9741efb4bc6578b287894956e642988))
- **video:**
  - fix the way camera devices are taken, remove warning, when no device is selected ([6d6d83ee](https://github.com/TokTok/qTox/commit/6d6d83eed68c2c5b740c60938141582894939582))
  - fix rare deadlock during call cancelation ([2c5d899f](https://github.com/TokTok/qTox/commit/2c5d899f362c7dcb870dd10cd9e4fe97722dd14f))

#### Features

- **About:** Show update available in nightly builds. ([5d6087a5](https://github.com/TokTok/qTox/commit/5d6087a5192b5e5fe0ac780098db58a6b8d94644))
- **Chat:** Allow user to control chat log chunk size. ([3ae47ec6](https://github.com/TokTok/qTox/commit/3ae47ec6b596daee44a2cf9cef011655134aa689))
- **Debug:** Add stack trace logging on crash. ([c31c09c1](https://github.com/TokTok/qTox/commit/c31c09c1ed7d7377528a74ab0d78f0ed6a5869b6))
- **Groups:** Add a "copy peer ID" context menu action in conferences. ([8cd886a0](https://github.com/TokTok/qTox/commit/8cd886a04e756a4c3d98e74c5823e3eb6a80e150))
- **Screenshot:** Add Freedesktop portal screenshot support. ([fdb860f8](https://github.com/TokTok/qTox/commit/fdb860f8c6df1631e14cc4628829e10264b67d62))
- **Web:** Preliminary support for running qTox in the browser. ([b5994646](https://github.com/TokTok/qTox/commit/b59946467ff9ad2e7d6a0ecce950d758bbb08c63))
- **ci:** add CI/CD pipeline, creating rpm package on fedora ([7c46b01c](https://github.com/TokTok/qTox/commit/7c46b01c3e2f44276d73cc27f0391a1374caf1ac))

<a name="v1.18.2"></a>

## v1.18.2 (2025-01-13)

### Release notes

This is a security-focussed release that also comes with some bugfixes.

- We've added QOI image support and dropped some image support plugins that we haven't properly vetted.
- We have added fuzzing tests for all the image plugins we do use (and filed some bugs for the ones we don't yet use).
- We've fixed a heap buffer overflow in exif handling. This overflow was not a vulnerability (it was an out of bounds read that would mess up image rotations when receiving broken exif data).
- We've added a setting to disable automatic image previews in chat. If you're very security-conscious and you have friends you don't trust, you may want to disable image previews. In the future, we'll add a per-friend setting for this.
- We've fixed some bugs that caused multi-line messages to be received as a single line. This was caused by our defense-in-depth security measures that were a little too strict.

<a name="v1.18.2-rc.1"></a>

## v1.18.2-rc.1 (2025-01-12)

### Release notes

Getting ready for a release. Here's a release candidate.

#### Bug Fixes

- cmake `AUTORCC_OPTIONS` global setting ([80f7cbb8](https://github.com/TokTok/qTox/commit/80f7cbb8a003413d0df04d467a5560aaac61b0e6))
- cmake `POSITION_INDEPENDENT_CODE` global setting ([027e8e02](https://github.com/TokTok/qTox/commit/027e8e020d9aebbb1ab4b0dd475ea3f3936d90a4))
- **Chat:**
  - Allow tab characters in incoming messages. ([2f70ebf7](https://github.com/TokTok/qTox/commit/2f70ebf7a7efb345844e58d658d094f2be3f9286))
  - Always show the date at the top of the chat. ([0883a0b6](https://github.com/TokTok/qTox/commit/0883a0b60822521b8fb0b8ebf85c74af7ef7c1ca))
  - No longer turn regular ASCII into emoticons. ([03692eca](https://github.com/TokTok/qTox/commit/03692ecace9b57546e94a543344a9d175e81efef))
  - Correctly display newlines in incoming chat messages. ([b70b3138](https://github.com/TokTok/qTox/commit/b70b31386e159f71e5db83aec5eaedaec671d4dc), closes [#386](https://github.com/TokTok/qTox/issues/386))
- **Security:** Fix heap buffer overflow in receiving pictures. ([4b75e633](https://github.com/TokTok/qTox/commit/4b75e633539dbf3b96701ff53a7049335fefa01d))
- **UI:** Message edit context menu now changes colour to hover/selection. ([7e168439](https://github.com/TokTok/qTox/commit/7e1684398dae512e511f92075c23b6cedd2cebc8), closes [#387](https://github.com/TokTok/qTox/issues/387))
- **tools:** Use tag strings in release tarballs. ([6e0b9a70](https://github.com/TokTok/qTox/commit/6e0b9a709574a4a11fa8c1870b41bf9139c4e09b))

#### Features

- **Chat:** Add spelling check and QOI support to Windows builds. ([165449c2](https://github.com/TokTok/qTox/commit/165449c25493441a363ea8f9274711df8a7e5e1f))
- **Pictures:** Add QOI image format support. ([87707a5d](https://github.com/TokTok/qTox/commit/87707a5d2a1503fa450a05f7ad332bf0ea4fa196))
- **Settings:** Add an option to disable image previews. ([823fde42](https://github.com/TokTok/qTox/commit/823fde429a7a014868f532c2877cf80388af67e9))

<a name="v1.18.1"></a>

## v1.18.1 (2025-01-06)

### Release notes

A pretty quick turnaround to the next release here, fixing an issue with the release builder workflows that caused release builds from v1.18.0 to claim they are unstable and untested.

Besides that, here are some highlights:

- We have significantly increased the translation coverage using Google Translate (and for Lojban, Baidu translate). All but two languages are now fully automatically translated. In many cases, this automated translation is not perfect, so we've also added a link next to the language selector to our Weblate page where you can fix translations you think could be improved.
- Using LLMs, we have finished the Pirate English translation, so: Ahoy! Come aboard the qTox ship, and set sail with this scurvy-free release! We've battened down the hatches and plugged some leaks, so no more unstable builds claimin' to be untested. Shiver me timbers, we've even charted new waters with more translations than ye can shake a parrot at!
- You can now run `qtox --update-check` on the command line to quickly see what the latest version is and what version you are running without starting the GUI. This might be useful for anyone who wants to write scripts around that.

Finally, we have tightened the security of qTox by avoiding any DNS lookups in the Tox bootstrap path. This means qTox will work properly even if you have no DNS server, and importantly, you will not leak access to the Tox network to any DNS servers. The only place where we still do DNS lookups is in the update check (which connects to the GitHub API), but you can disable that.

<a name="v1.18.1-rc.3"></a>

## v1.18.1-rc.3 (2025-01-05)

### Release notes

Release candidate 3.

#### Bug Fixes

- Update check is now working on Windows and AppImage ([5b3a3478](https://github.com/TokTok/qTox/commit/5b3a3478298be6e75c2adf2b8e4eb70d2ab0d504))
- **Audio:** Notifications sounds are no longer cut off. ([1624d3a9](https://github.com/TokTok/qTox/commit/1624d3a95af68ef18c87435a8057301f0426fdfe))

#### Features

- Add a command line option to run the update-check. ([4d5f9f94](https://github.com/TokTok/qTox/commit/4d5f9f9496028a78163e7912708976cc0ce81635))
- **Security:** Never perform DNS lookups for tox node connections. ([e0cd0b75](https://github.com/TokTok/qTox/commit/e0cd0b75cf20dc468c601352ce929d8f1c31660d))

<a name="v1.18.1-rc.2"></a>

## v1.18.1-rc.2 (2025-01-03)

### Release notes

Release candidate 2. RC1 didn't have binaries because the CI scripts were broken.

<a name="v1.18.1-rc.1"></a>

## v1.18.1-rc.1 (2025-01-03)

### Release notes

Release Candidate 1.

#### Features

- **Languages:**
  - Add a weblate link to the settings. ([7024d02c](https://github.com/TokTok/qTox/commit/7024d02cc3ef047c9b98444ff3959b2dcadfb89d))
  - Translate all of qTox to Pirate English. ([715caf13](https://github.com/TokTok/qTox/commit/715caf13cf2906727bffe7c4b6e964455d1c6bc5))
  - Add Limburgish translation. ([607ba4d2](https://github.com/TokTok/qTox/commit/607ba4d2cc19de4dba7cab9e783a0ae6821208fc))
  - Fully translate every supported language. ([779cdb39](https://github.com/TokTok/qTox/commit/779cdb3958a006e02c6c8131c878626a8c0aa2c0))
- **Login:** Add a tooltip and WhatsThis for the password field. ([04efcf82](https://github.com/TokTok/qTox/commit/04efcf82099c216cd401eb28bc332d86df9b843c), closes [#299](https://github.com/TokTok/qTox/issues/299))
- **UI:** Better integration with system dark modes. ([ff2a252d](https://github.com/TokTok/qTox/commit/ff2a252d5faee60554d2c64cdc8e45d7f445b2e9))

#### Bug Fixes

- **Languages:** Recover the long lost Pirate translation. ([d10ccdc8](https://github.com/TokTok/qTox/commit/d10ccdc80fa3ee83b27216234029b016fc3ebb61))

#### Performance

- **Video:** Avoid copying frame buffers unnecessarily. ([7e843537](https://github.com/TokTok/qTox/commit/7e8435375c6bb4ba519cb173e8cc8d875fded670))

<a name="v1.18.0"></a>

## v1.18.0 (2025-01-01)

### Release notes

Happy New Year 2025!

It's taken us some time, but we're finally here. We hope you enjoy our new and updated qTox v1.18.0. Many bugs, especially around video calls, have been fixed. We also bring some performance improvements, but most importantly, **the RCE fear is over**.

There have been many rumours about remote code execution attacks on qTox for the past 2 years. Although nobody has ever actually been able to demonstrate any of them working, we've done a deep dive audit on the relevant security aspects of the areas of potential vulnerability and have made a number of changes:

- We've completely rewritten the notification system from scratch. We now use the built-in Qt system tray notifications on all systems. Additionally, on Linux, we use the Freedesktop notification system directly (you can turn this off if it doesn't work or you're afraid we've made a mistake) instead of going through an unaudited third party library.
- We've put additional filtering in place for any incoming text messages from the Tox network, including friend request messages. We now filter out any non-printable characters. This may break certain newer emojis such as a skin-toned handshake emoji (🤝🏾) on older systems (from 2022 or earlier). If you use our provided binaries, it should just work, as we build our binaries with the latest Qt version and dependencies.
- We've hardened some of the low level load/store functions used for settings. There almost certainly wasn't a vulnerability here, but they can no longer be abused directly if there ever will be.

We have, as a side effect, also upgraded the toxcore used in the (windows) release. There are a great number of outdated toxcore nodes still present in the network, holding back new feature adoption such as the new group chats with moderation capabilities.

Check out the release candidates' release notes as well for a full list of changes since the 1.17.6.

As always, report any bugs or issues you find or features you'd like to see to our [issue tracker](https://github.com/TokTok/qTox/issues). We've got a long way to go, but we're come a long way as well. Enjoy the release!

#### Bug Fixes

- Avoid occasional crash when changing video devices or closing qTox. ([2bd629fe](https://github.com/TokTok/qTox/commit/2bd629fe5a0fc320fe78da7cddcd19965b7128dd))
- **Security:** Harden the persistence/serialization functions. ([e4735668](https://github.com/TokTok/qTox/commit/e47356681c126dffe9f07fbfa4ee9d441446c230))

<a name="v1.18.0-rc.4"></a>

## v1.18.0-rc.4 (2024-12-31)

### Release notes

One more release candidate before our final release.

#### Features

- add appimage back to ci ([8f13b826](https://github.com/TokTok/qTox/commit/8f13b8261e199821cb79c7f02c9ff01cab7e838e))

#### Bug Fixes

- Fix uninitialised read in contact circle widget. ([afe24fbe](https://github.com/TokTok/qTox/commit/afe24fbe3ebf1ea395849a8012a031b1705bfe76))
- Don't crash when closing the app after logout. ([b4b0d8c9](https://github.com/TokTok/qTox/commit/b4b0d8c9203a4937424a3d00c8ad86dca47553fe))
- Don't crash on logout. ([b244e206](https://github.com/TokTok/qTox/commit/b244e2062845c1a03798bc2ff4af629b54d8f907))
- Avoid hanging when capture device permissions denied on macOS. ([9e4b17b8](https://github.com/TokTok/qTox/commit/9e4b17b8605dd8c21db6ed6cfad7fca2331897da))

#### Performance

- Improve performance of video frame buffering. ([ffcef564](https://github.com/TokTok/qTox/commit/ffcef56404f2d8797edc730ebd222726119c74cc))

<a name="v1.18.0-rc.3"></a>

## v1.18.0-rc.3 (2024-12-23)

### Release notes

qTox version 1.18.0 release candidate 3

#### Performance

- Don't log verbose toxav messages. ([349b6101](https://github.com/TokTok/qTox/commit/349b6101976add3a0a3d71df8dc33d539e8d024e))

#### Bug Fixes

- Fix occasional deadlock in ending a video call. ([73c253d7](https://github.com/TokTok/qTox/commit/73c253d70d14048e1a2711ba798aa9e72b861c03))
- Disable video device selection box during call. ([527c9096](https://github.com/TokTok/qTox/commit/527c90969eabbb9416666106091d75729d952983))
- Make sure camera access is gained on macOS before calling. ([3ebea6e9](https://github.com/TokTok/qTox/commit/3ebea6e962889c313c15b01ddcb6e6a4e6a3512e))
- Make camera input work again on macOS. ([a5d0bcc4](https://github.com/TokTok/qTox/commit/a5d0bcc4e3a787f3e226f0e368b0d9e10f1c87fd))
- Don't crash on quit during AV call. ([ffd7b4e1](https://github.com/TokTok/qTox/commit/ffd7b4e15f03c1464edff48c2a1b9385da7ce31e))

#### Features

- Add many missing emojis from the smiley packs. ([2897ee56](https://github.com/TokTok/qTox/commit/2897ee564fae480e31f62dad5270801e54dd2cfb))
- Add DBus desktop notification support. ([fb4df420](https://github.com/TokTok/qTox/commit/fb4df420d8821fe60e9982dfb119d7eb32f39e7c))

<a name="v1.18.0-rc.2"></a>

## v1.18.0-rc.2 (2024-12-15)

### Release notes

#### Performance

- Delay debug log loading to show time. ([7525725b](https://github.com/TokTok/qTox/commit/7525725b76210bbebb10624c10555415859b3c48))

#### Bug Fixes

- Use the _last_ `/src/` to find the source root. ([62c093a9](https://github.com/TokTok/qTox/commit/62c093a9f1d9e3ba58c1b769b5d56baa0a8bb62f))
- Show actual smileys in the smileypack selector. ([97bc8fcd](https://github.com/TokTok/qTox/commit/97bc8fcd06d4a71fdd4fa937d5704aef360b1d85))
- cmake file dependencies ([de7c4909](https://github.com/TokTok/qTox/commit/de7c4909aac1d363638a626043358491d55b93ff))
- Fix two memory leaks. ([91a02bc5](https://github.com/TokTok/qTox/commit/91a02bc5c4e22b06f7dd0be21d5c9dc52d48b99c))
- Fix use-after-free when closing qTox during a call. ([6da12d60](https://github.com/TokTok/qTox/commit/6da12d6086a528aa993d39953ceb364f6c903c70))
- Typo fix unavaliable -> unavailable. ([037ac452](https://github.com/TokTok/qTox/commit/037ac452214d18b8af868b74ac7349ade0458467))
- Allow building qtox without update check. ([55ec9119](https://github.com/TokTok/qTox/commit/55ec9119999f21aeed5b37cd726f8fe32bedca31))
- memory error on exit ([3721ad0d](https://github.com/TokTok/qTox/commit/3721ad0ded993f10763d2b4c7ba10ae8297ec788))
- Correct the tab completion regex so it works with Qt6. ([1680064e](https://github.com/TokTok/qTox/commit/1680064e71bb5e677f4d4c703273c45251d619e9))
- **security:** Only allow printable characters in incoming messages. ([18df99a3](https://github.com/TokTok/qTox/commit/18df99a392cbf46efd3b44f64643026f0f4889a7))

#### Features

- Add caps-lock indicator in password edit on macOS. ([70344bad](https://github.com/TokTok/qTox/commit/70344bad6732d5cbb0151848ceb77dc80cde71ae))
- Add a Qt object tree view in the debug widget. ([a3828eca](https://github.com/TokTok/qTox/commit/a3828ecaea05d25cc9a1f3abe5f76a1b0f01a490))
- Add system tray notification support. ([cfbd2bff](https://github.com/TokTok/qTox/commit/cfbd2bffa1b05709556fc60593c833ade95d6a57))

<a name="v1.18.0-rc.1"></a>

## v1.18.0-rc.1 (2024-11-23)

### Release notes

#### Bug Fixes

- Fix use-after-free bug in desktop notifications. ([34526c8e](https://github.com/TokTok/qTox/commit/34526c8e65628d15df27d5171318d4e6fd52871a))
- Add Caucasian Albanian Script writing system mapping. ([42b0bc15](https://github.com/TokTok/qTox/commit/42b0bc152c394e6770cc23d0c5bbc4786b325fa9))
- remove extra newline in log ([8fe87f17](https://github.com/TokTok/qTox/commit/8fe87f17f6e3b0a4fee3f53d0771ee27d5072eca))
- Make checkboxes more visible on wayland. ([76713ff8](https://github.com/TokTok/qTox/commit/76713ff80fc1aa67a6268331d87761cd2b605657))
- Don't create huge font weights in themes. ([9d0562a7](https://github.com/TokTok/qTox/commit/9d0562a740332475769c462cdebf39142b6e1701))
- Fixed the style selector in UI settings. ([ee92a02e](https://github.com/TokTok/qTox/commit/ee92a02e36f1363af96ecf19f6af9e9fe0eda7e0))
- Don't show unicode replacement boxes on unsupported languages. ([824637cb](https://github.com/TokTok/qTox/commit/824637cba1d6cd7d9ab9175486506c2fab2cc0e0))
- Update more deprecated stuff ([e6cb2b2b](https://github.com/TokTok/qTox/commit/e6cb2b2b186293de903345af2b47eb183178caa4))
- don't link emoji resources into targets that don't need them ([cd8af735](https://github.com/qTox/qTox/commit/cd8af735fc3ac6494cd6112a068916f7d8e9baf4))
- change color equation for groupchat names ([bde6dc0d](https://github.com/qTox/qTox/commit/bde6dc0df0583a2c869612a010533bb6246ad7e2))
- correctly destroy on group remove ([c9505728](https://github.com/qTox/qTox/commit/c95057283e3c3d6c05a178b1f24cdeda6ae8eea2))
- Replace deprecated `QMutex::Recursive` with QRecursiveMutex ([8276141e](https://github.com/qTox/qTox/commit/8276141ef973f9e5b7334e0e9238e3970834bac8))
- clear custom style before update style ([a97c5331](https://github.com/qTox/qTox/commit/a97c53313a351d32051dc19db65a92a7902cfc56))
- Qt ressource system usage ([0335f203](https://github.com/qTox/qTox/commit/0335f20362e2b32e00d6b3fd286d801be1e014ab), closes [#6144](https://github.com/qTox/qTox/issues/6144))
- Usage of random ([f5f42233](https://github.com/qTox/qTox/commit/f5f42233cad3e84d895261c25d693c6da637ef01))
- add missing type for Qt slot ([ad139d67](https://github.com/qTox/qTox/commit/ad139d67c2d8ee6cdc408cffbbdc26d52591c274))
- remove unused variables ([e67cdadf](https://github.com/qTox/qTox/commit/e67cdadf3d41521f563611bca1a688336a8bb254))
- Remove unnecessary null pointer check ([20f948bc](https://github.com/qTox/qTox/commit/20f948bc0842b1c9dd22a4fc386841fe04f2814c), closes [#5785](https://github.com/qTox/qTox/issues/5785))
- Remove empty file ([e6381861](https://github.com/qTox/qTox/commit/e6381861c8cf3a30eb859618869868749e0f67a6))
- empty username causes mention on ever message ([641fba35](https://github.com/qTox/qTox/commit/641fba3553bc5a664cec593d38f7d80a0bf09a76), closes [#2119](https://github.com/qTox/qTox/issues/2119))
- update workerStb ([dedbd184](https://github.com/qTox/qTox/commit/dedbd184b0df6bc708a2274231d288dae2f22c81))
- data validation during the search (fix: #5791, #5723) ([e3c35c91](https://github.com/qTox/qTox/commit/e3c35c9153cd5f843848eadea47a969b83da33da), closes [#5723](https://github.com/qTox/qTox/issues/5723), closes [#5791](https://github.com/qTox/qTox/issues/5791))
- **AppManager:** Don't create temporary copy of argc ([2197bce6](https://github.com/qTox/qTox/commit/2197bce610f2abf5e561b1ee528313660b0f0fb9), closes [#6641](https://github.com/qTox/qTox/issues/6641))
- **CI:**
  - Fix commit message check on single commit PRs ([e48c5573](https://github.com/qTox/qTox/commit/e48c557387f75b12cfcccc5b3c7806e9a8a113fa))
  - Remove "setup" from Windows zip file name ([2f92a5f2](https://github.com/qTox/qTox/commit/2f92a5f2f00a7f757479e21706dda99a58b99010))
  - Don't upload Windows setup exe to nightly release ([c726b9ce](https://github.com/qTox/qTox/commit/c726b9ce23980b950d7a812adaa7549226a782b0))
  - Correct Wine lib path for winehq manual install ([80a0a4ae](https://github.com/qTox/qTox/commit/80a0a4ae620d299d2379c28613d3298643f3fd19), closes [#6481](https://github.com/qTox/qTox/issues/6481))
  - Build in current dir in build-qtox-linux.sh ([11f7f619](https://github.com/qTox/qTox/commit/11f7f6193a53092a84c284a6a8781470e8a99d88))
  - Fix memory leak in `core_test` ([9cd9da4a](https://github.com/qTox/qTox/commit/9cd9da4a5360028bab32bb7f04d95e67e895cc0d))
  - Update Wine from 5 to 7 in docker image to fix unit tests ([79feb19d](https://github.com/qTox/qTox/commit/79feb19d7d74be5c5b5862298818cc69507fda15))
  - Grant action content write permission for release upload ([1bc72ab1](https://github.com/qTox/qTox/commit/1bc72ab1a476e8eb9ca3c58e01515421ab1c29f4))
  - Fix regression in logging from docker compose run ([44d867f6](https://github.com/qTox/qTox/commit/44d867f624e93c4ede2446d45a847463c009d35d))
- **FriendList:** exclusion of possible undefined behavior ([a1b08d64](https://github.com/qTox/qTox/commit/a1b08d64934107a824215834f366a4a363c0c03b))
- **History:**
  - Heal duplicate peer entries with different case ([9a8706a6](https://github.com/qTox/qTox/commit/9a8706a65f1737f875eccc9717c8389ff51798d2))
  - Strictly check `public_key` during database upgrade ([f2957013](https://github.com/qTox/qTox/commit/f29570138fa7d377e1dae7df6319277027eaac57))
  - Fix alias removal SQL query after group history addition ([4f45f30e](https://github.com/qTox/qTox/commit/4f45f30ef9da8ba0a8511acd53bdcf2054d91a18))
  - Heal invalid `resume_file_id's` saved to history ([f777aa88](https://github.com/qTox/qTox/commit/f777aa885cde638167c96792647726fc65d096ae), closes [#6553](https://github.com/qTox/qTox/issues/6553))
  - Don't cast `file_id` to QString before inserting ([47ee51c6](https://github.com/qTox/qTox/commit/47ee51c61d99a7f3b0a8c63fa4d7bbfc32fb2a6d))
  - Correct SQL query for empty finished file transfer ([2d6cc254](https://github.com/qTox/qTox/commit/2d6cc25405d1c7dbb55f0d2cc8a8a2cd10098572))
  - Deep copy QByteArrays from SQL selects ([27f19ec2](https://github.com/qTox/qTox/commit/27f19ec2fa1b7a1d0deb99393d296d697e3d890e))
  - Replace invalid Tox ID saved in peers table with Tox public key ([9ac44ee0](https://github.com/qTox/qTox/commit/9ac44ee09c365075b6c1ae5f343f7d632a46cd5f), closes [#6485](https://github.com/qTox/qTox/issues/6485))
  - Fix qt deprecation warning ([6c34fad9](https://github.com/qTox/qTox/commit/6c34fad9b6fa6b4c519ea1568443da4fb715694d))
- **Paths:**
  - Track portable state in Paths to updates paths correctly ([0eb56fb9](https://github.com/qTox/qTox/commit/0eb56fb9bb61d953215e1531022a6460deb2a009), closes [#6443](https://github.com/qTox/qTox/issues/6443))
  - Default to auto paths detection mode ([d0c120e0](https://github.com/qTox/qTox/commit/d0c120e0a8f4802cd03bd1fb305ade45c46799be))
- **UI:**
  - Centre message boxes on main window ([c5fdb786](https://github.com/qTox/qTox/commit/c5fdb786764540358d255d32a9cc2d3d31614542))
  - Pass NetCamView's parent pointer to base QWidget ([809c6957](https://github.com/qTox/qTox/commit/809c69571829df3aeee4e93c54c15da2ffebc637))
  - Use GenericChatItemWidget construction compact argument ([f25e8558](https://github.com/qTox/qTox/commit/f25e8558399c024278b167dc17198156571e405f))
  - reorder the window title ([d621d745](https://github.com/qTox/qTox/commit/d621d7451170941105b4ba581b63e90cc41f60e1))
  - contact list optimization ([45b69519](https://github.com/qTox/qTox/commit/45b69519c4f511bf5eedb85b1f6a8cd31f1f6307))
  - hide update related elements when `-DUPDATE_CHECK=OFF` ([db8af056](https://github.com/qTox/qTox/commit/db8af0561d3128ed5598f9fa61ef381827747cae))
  - passing arguments from the CircleWidget constructor ([e6bc355b](https://github.com/qTox/qTox/commit/e6bc355bc741af195628f55aa50c09222499e34d))
  - add group widgets on start with "Activity" mode ([b98801f4](https://github.com/qTox/qTox/commit/b98801f42f55547fce227aaad7dabd71b8895338))
  - system translation not being updated ([6527fd4f](https://github.com/qTox/qTox/commit/6527fd4fca271b3787050ab6af6319c0c9896de8))
  - move update elements in about ([31de0eaf](https://github.com/qTox/qTox/commit/31de0eaf08d807732fb50124c1e129a90e0f970a))
  - implement and connect reloadTheme in leaf classes ([3bf3128a](https://github.com/qTox/qTox/commit/3bf3128a4fa4fe53694789da07a9669d12020179), closes [#5592](https://github.com/qTox/qTox/issues/5592), closes [#5924](https://github.com/qTox/qTox/issues/5924))
  - don't notify of available update if local version is newer ([82547263](https://github.com/qTox/qTox/commit/82547263f815c2acd1ad5ad4e31019be196d1f2c), closes [#6112](https://github.com/qTox/qTox/issues/6112))
  - using a separate css file ([03124454](https://github.com/qTox/qTox/commit/03124454cd223262b3d66fb9ff7cab7e44592514))
  - updating friends list color at runtime ([d01fed92](https://github.com/qTox/qTox/commit/d01fed92f08dfaf9ed8a54fdc275b51b22b56ba7))
  - quit qTox on Ctrl+Q when "Close to tray" is enabled ([f9f634b3](https://github.com/qTox/qTox/commit/f9f634b326f359e9f605c1215f94670053d57069), closes [#5925](https://github.com/qTox/qTox/issues/5925))
- **Widget:**
  - Incorrect removing of contacts ([039f6ff0](https://github.com/qTox/qTox/commit/039f6ff0b3b66732ee7f37a427447e1a45bd0a49))
  - Update widget's core on core update ([b429e562](https://github.com/qTox/qTox/commit/b429e562e25c6c2e65f1c511996637f74369b2f9))
- **Windows:**
  - Fix starting qTox from installer ([beb38fa3](https://github.com/qTox/qTox/commit/beb38fa33b51be97361b11edc03c96a13d616a47))
  - Add missed openal dll since CI refactor ([68ca7354](https://github.com/qTox/qTox/commit/68ca7354beae098773e407fe3481be15709bfb0a))
  - iconengines not being installed ([1e50c123](https://github.com/qTox/qTox/commit/1e50c123dc906ad61dc093526ba5b4886fd4a471))
  - libsnore-qt5 not being installed ([827b2c22](https://github.com/qTox/qTox/commit/827b2c22103489b8cdbb0ca6ba973be14181780e))
- **appImage:**
  - Patch snore to enable notifications in AppImage ([b05c069d](https://github.com/qTox/qTox/commit/b05c069dfe92c229fdde95f2f420b11f6d83c434), closes [#6411](https://github.com/qTox/qTox/issues/6411))
  - Bundle libpangocairo which was incompatible on Arch ([75648d04](https://github.com/qTox/qTox/commit/75648d04b97c469fd1fc867ea9db4eadcac7ff3a), closes [#6454](https://github.com/qTox/qTox/issues/6454))
  - Include additional needed glib for RH based distros ([12ac8332](https://github.com/qTox/qTox/commit/12ac8332544036bb11864bcbbe0894165cc9af98), closes [#6339](https://github.com/qTox/qTox/issues/6339))
  - Patch manually copied libs to include rpath for local dir ([cb8a095f](https://github.com/qTox/qTox/commit/cb8a095fd295999ea574845d9688577fbab484b5))
  - Handle no `TRAVIS_TAG` or `TRAVIS_COMMIT` in AppImage build ([aee189d4](https://github.com/qTox/qTox/commit/aee189d46d1da37a353bcbc22e99152e9488c223))
  - Use external binary path when enabling autorun from appimage ([da5c165f](https://github.com/qTox/qTox/commit/da5c165f4116dce736def63b1b917523b848db09), closes [#6241](https://github.com/qTox/qTox/issues/6241))
- **apparmor:**
  - Allow access to Aspell personal dictionaries ([6a21d962](https://github.com/qTox/qTox/commit/6a21d962141bd6dc7a6b051d20c7dbda471685a9))
  - Allow spellchecking ([2ebf51b5](https://github.com/qTox/qTox/commit/2ebf51b5b77184efefe3d02469748d67427c8d63))
- **build:**
  - Remove unnecessary dependencies for Windows build ([4006d9b5](https://github.com/qTox/qTox/commit/4006d9b5e8f8fe8e12044a0ed1c66f830ea4b912))
  - Remove unnecessary dependencies ([0681614e](https://github.com/qTox/qTox/commit/0681614ecc9c48f081e2e51f31ed4e151f3c1df5))
  - Add std threading support in Windows build ([057f921a](https://github.com/qTox/qTox/commit/057f921ace8894f7c193f8504bc44bdb78488fce))
  - don't connect UpdateCheck when `-DUPDATE_CHECK=OFF` ([50063f76](https://github.com/qTox/qTox/commit/50063f76abf2e4d3e20c9414ab61bae65335c7cb))
  - include QVector type ([89fa7db9](https://github.com/qTox/qTox/commit/89fa7db90a74100c1e3ff584b42557c0896101fd))
  - cache debug deps during 2nd stage ([a686468b](https://github.com/qTox/qTox/commit/a686468bfdaf7ecbed3dc5aabf522e9884cd1bb2))
  - make pkg-config verbose about why it fails ([e4c7d81a](https://github.com/qTox/qTox/commit/e4c7d81ab29299413d4b361f7dd23d4c6b83a879))
  - appease appstream-glib validate ([dbf58826](https://github.com/qTox/qTox/commit/dbf58826f6c970cc468667bfdb77e92755c09134))
  - fix OpenAL Soft failing to build with newer CMake ([b8d3182d](https://github.com/qTox/qTox/commit/b8d3182d069f30c10909956d602db3702d47e212))
  - fix SQLCipher build issue with OpenSSL 1.1.1 on Windows ([cf8b89c5](https://github.com/qTox/qTox/commit/cf8b89c5cb5d6c40c2c9b6d33a636a2960b34d96))
- **buildscripts:** Fix license text for MIT derived works ([88edc66f](https://github.com/qTox/qTox/commit/88edc66f88bb2eaaeda156f55a0731999bd81541))
- **chat:**
  - Undo removing of chat message insertion ([b82580c8](https://github.com/qTox/qTox/commit/b82580c81427e4350369fcaa4f9a986e84f31f9e))
  - Stop spellcheck from leaking messages to logs ([813643ad](https://github.com/qTox/qTox/commit/813643adbf3d1eb943f88cbb5fff36120d9d9a02))
- **chatform:** Scroll to bottom on load, broken on refactor ([69cd33b7](https://github.com/qTox/qTox/commit/69cd33b7331368f4f0092e565ac8abcec62fc698), closes [#6314](https://github.com/qTox/qTox/issues/6314))
- **chathistory:** file transfers sometimes show wrong author name ([1bc97ef2](https://github.com/qTox/qTox/commit/1bc97ef219916685a2dc704d5941237cfbec749e))
- **chatlog:**
  - Display correct system message timestamp in chatlog ([25223119](https://github.com/qTox/qTox/commit/25223119c9c17fab3c973680d914abb31a503350))
  - fixed rendering of new messages when scrolling up ([1aad9194](https://github.com/qTox/qTox/commit/1aad91944ee8cad86b124f65531948e7bf186bb5))
  - Remove invalid usages of raw chatlog indexes ([4b691f93](https://github.com/qTox/qTox/commit/4b691f9392cc60394a577292becc67c0416e76e7))
- **cmake:** use the 'STATIC' argument for `'translations_library'` ([5ba63af9](https://github.com/qTox/qTox/commit/5ba63af9cb3f88c41e25bf4abe084c32aee9b405))
- **contacts:** window with multiple friends contacts ([15459452](https://github.com/qTox/qTox/commit/1545945236681bb1561df9bf2e8cafd701b3699a))
- **core:**
  - Correct getSelfDhtId's toxcore API ([faa72d56](https://github.com/qTox/qTox/commit/faa72d567e049797ebbbb5322d0238cb83728abc))
  - One time reset of user bootstrap node list to default ([4580d3f1](https://github.com/qTox/qTox/commit/4580d3f1066ad946c7532af968d333cc0400eb4a), closes [#6452](https://github.com/qTox/qTox/issues/6452))
  - Fix logic error retrieving TCP port ([6f912370](https://github.com/qTox/qTox/commit/6f9123705a726c1a219a0f447a7bf744710de328))
  - Use node's TCP ports when connecting to TCP relay ([e7e30ada](https://github.com/qTox/qTox/commit/e7e30ada8cd33f0dd4adfdb3c021b75e28232057))
  - Don't apply Windows file transfer rename on other platforms ([3180d23e](https://github.com/qTox/qTox/commit/3180d23ee2a7780e2314955e6cd7649ba40ef0ec), closes [#5242](https://github.com/qTox/qTox/issues/5242))
  - Fix missed ToxId to ToxPk constructions ([56eed63f](https://github.com/qTox/qTox/commit/56eed63fdebd09f832a5eefd397f07fa26c1d71f))
  - Track avatar offer file size to avoid cancelling transfers ([3ac37a54](https://github.com/qTox/qTox/commit/3ac37a549646ce03d9ca736e5bb44b0b881334ba))
  - extend RAII locker lifetime until end of function ([55fb28b0](https://github.com/qTox/qTox/commit/55fb28b08b7eeb60fc2698ad9cb8e36e9db93e19))
  - read proxy type after personal settings are loaded ([5dcea74d](https://github.com/qTox/qTox/commit/5dcea74db500f09151dfb73fd937519c1d92d1ea))
- **coreav:**
  - prevent racy access to call variable ([c507d266](https://github.com/qTox/qTox/commit/c507d2665d4c5a352661f7f2d701d2679b4e805a))
  - change some lock to write locks ([dfcfb7d5](https://github.com/qTox/qTox/commit/dfcfb7d5e00eb561d659fe236f4aa06b014ee643))
  - fix assert because c-toxcore calls from unexpected thread ([141cbf88](https://github.com/qTox/qTox/commit/141cbf887026835d83c3763f920daed3276b674e))
  - avoid deadlock between CoreAV, main and Audio thread ([4b9e4a57](https://github.com/qTox/qTox/commit/4b9e4a571d9ecd22c0cc339951cc0464cb8c24cc))
  - cleanup assertions ([9499925f](https://github.com/qTox/qTox/commit/9499925fb2bb4603dc0aa83bbf443a0ad61424a7))
- **corefile:** Close files of broken file transfers ([5de54039](https://github.com/qTox/qTox/commit/5de5403964854d1372efda7b8a8d7d324e85aba3), closes [#6500](https://github.com/qTox/qTox/issues/6500))
- **crash:** Handle nightly tag name in update check ([ddebf6ee](https://github.com/qTox/qTox/commit/ddebf6ee7a68d5ffb42aeaccdfdd83f6a0fe5083))
- **db:** prepare and execute queued statements one at a time ([af02542e](https://github.com/qTox/qTox/commit/af02542e05992bf94fcff37c365f638ad7b53d8d))
- **docs:**
  - Add brew prefix to macOS build instructions ([3c682abc](https://github.com/qTox/qTox/commit/3c682abc69b5a1b39d5679c76868c9bbdf49c527))
  - Correct Windows cross compile example command ([0706c906](https://github.com/qTox/qTox/commit/0706c906337180dfeed66950ee0cb97592caa542))
  - add sqlcipher to required arch deps ([82d5beee](https://github.com/qTox/qTox/commit/82d5beeee94f47fc6366e470b2469fbf497c330b))
- **doxygen:** Re-enable deploying of doxygen to qtox.github.io ([d3995bc8](https://github.com/qTox/qTox/commit/d3995bc87eeb4144c27568eb2059aed4c56de4cf))
- **extensions:** Add toxext to CI scripts ([670457a7](https://github.com/qTox/qTox/commit/670457a77bcb42f22153f0a08f8ecb351cf230ff))
- **filetransfer:** Fix UI inconsistencies with pause/resume ([3bc4aa52](https://github.com/qTox/qTox/commit/3bc4aa52bee98fe8b44c004d0f9f2e6759d9f73e))
- **flatpak:**
  - Build flatpak qTox and dependencies in Release mode ([9ea6d7e8](https://github.com/qTox/qTox/commit/9ea6d7e8fcff3244cb793f53e3a8aad1dfeaf35d))
  - workaround unstable flathub downloads ([69677831](https://github.com/qTox/qTox/commit/69677831aba0763f5c8fdc1847db3e868e25c983))
- **gitstats:** Re-enable deploying of gitstats to qtox.github.io ([5a77a18f](https://github.com/qTox/qTox/commit/5a77a18fc09f8af5ffa7bc7a949eea505abe989b))
- **group:** Change join leave message setting default to false ([0df98ce6](https://github.com/qTox/qTox/commit/0df98ce64fe6532ce9e0025e7e4c8ec7c2f0038d))
- **groups:** Fix invalid group list on group member join ([fd243738](https://github.com/qTox/qTox/commit/fd243738c1a4773726e7edfa7c5988a4319364bd), closes [#5838](https://github.com/qTox/qTox/issues/5838))
- **i18n:** Various English fixes ([11b34c84](https://github.com/qTox/qTox/commit/11b34c84ef50358770248f689aaa606df9e595c5))
- **ipc:** allow IPC under windows by not using `std::random_device` with mingw ([6105af82](https://github.com/qTox/qTox/commit/6105af8279b609107f5116b0ac5039b8dbcd17b8), closes [#2917](https://github.com/qTox/qTox/issues/2917))
- **l10n:** Added comments in qTox.desktop file ([f4b7ea52](https://github.com/qTox/qTox/commit/f4b7ea52b2333c39cb18f6e729c4dbc80e14a947))
- **libav:** remove usage of a private field see https://github.com/signalwire/freeswitch/issues/2202 ([42303a9b](https://github.com/TokTok/qTox/commit/42303a9b76b012aa84089710be4241e4e0a8b1bb))
- **logging:** check if version is stable before other checks ([0ee37a7a](https://github.com/qTox/qTox/commit/0ee37a7a099aec6cd86d3361ca5e5065b8b4f501))
- **login:**
  - fix a crash when many dialogs are open ([c37a05f4](https://github.com/qTox/qTox/commit/c37a05f40cce84f449527d7c381ee8285d91ba86))
  - show login screen when autologin fails ([a3bbf5c4](https://github.com/qTox/qTox/commit/a3bbf5c41d95470323afa162771f50f5901bc109), closes [#5781](https://github.com/qTox/qTox/issues/5781))
- **macOS:**
  - Represent autorun state in UI persistently ([ad6b5104](https://github.com/qTox/qTox/commit/ad6b5104179bbcbbebf54197e81c487666d106d1))
  - Fix macOS autorun not loading at start ([00936911](https://github.com/qTox/qTox/commit/009369118f7e3175ab5857b39b98980a6f75d380), closes [#2814](https://github.com/qTox/qTox/issues/2814))
  - Update video API usage for newer libavcodec ([15673a52](https://github.com/qTox/qTox/commit/15673a52b6b4805d482b69281e21947fb7096e05))
  - update deprecated AVFoundation API ([65ff532a](https://github.com/qTox/qTox/commit/65ff532a54122a09357a0240b287219c0306c0be))
- **main:** Construct ToxSave event handler ([7565f8b3](https://github.com/qTox/qTox/commit/7565f8b350c54347e3c78a91d05bf50dff1ffe3c))
- **model:**
  - take QObject receiver argument to interface signal connection macro ([24e4ec37](https://github.com/qTox/qTox/commit/24e4ec37510c11a38d7aedc6b533f11a653c65a6))
  - stop interfaces from inheriting from QObject ([82a4f1b4](https://github.com/qTox/qTox/commit/82a4f1b41292eae981b9374824ff1499ca1cfcb4))
  - return connection from interface macro ([41b2b35c](https://github.com/qTox/qTox/commit/41b2b35ce3037a32f705caf92600c42b0ae4798a))
- **net:** check if the node has all needed fields ([ca4f14cc](https://github.com/qTox/qTox/commit/ca4f14cc4c2ca2d80e73a9da503325ef13de807c))
- **notification:** hide snore warning log spam ([ca4f9df1](https://github.com/qTox/qTox/commit/ca4f9df1eecb43779ec7469ec24908a1a5f53a47))
- **osx:**
  - Fix product name when prompting for mic and camera permissions ([fb449056](https://github.com/qTox/qTox/commit/fb4490569865113d07dbdd6e2da33890e0aff813), closes [#6261](https://github.com/qTox/qTox/issues/6261))
  - Allow rebuild using build script ([56151ea6](https://github.com/qTox/qTox/commit/56151ea6fd1ff27dfb7b71eb2219a4b714a08469))
  - Fix missing variables in macOS build script ([360cde9d](https://github.com/qTox/qTox/commit/360cde9d3697cff9c29cb50d57393af57ba985bc))
  - OSX updater dependency moved. ([6d99e909](https://github.com/qTox/qTox/commit/6d99e9099f2a5d013a915b334d5d41b5f8bdfb9d))
  - Add camera and microphone usage description. ([5f1338dd](https://github.com/qTox/qTox/commit/5f1338dd78b4b9597c15ac187dce03e516977a5f))
- **preview:**
  - handle libexif missing fields, don't accept 0-value orientation ([535b01a8](https://github.com/qTox/qTox/commit/535b01a8413ca3b51e739de7d57e534eecc8db6e))
  - Don't log a warning on missing exif orientation metadata ([9da1e3bb](https://github.com/qTox/qTox/commit/9da1e3bbdf01c8aba2dc5029f70a99fd0d85fd87))
  - Fix exif orientations ([99c1753a](https://github.com/qTox/qTox/commit/99c1753a76260afcd8cf4b5e3e75ba5b1b3b3909))
- **profile:**
  - load settings before starting Core ([bb26d4a0](https://github.com/qTox/qTox/commit/bb26d4a08623981b5f423e2d217d45d3672e1939))
  - write .tox file immediately on creation ([49c8f9b9](https://github.com/qTox/qTox/commit/49c8f9b9b5dc3d364aa4ef13e22d199673ed68c3))
- **security:** Do a better job of anonymising the log output. ([ac999816](https://github.com/TokTok/qTox/commit/ac999816e6e76db73b9dd947bf7baa167d50ea7e))
- **settings:**
  - Heal invalid ToxPk saved to Personal Settings ([dd952408](https://github.com/qTox/qTox/commit/dd9524085fdc11ab39bdb7d44419959cc45e1790), closes [#6506](https://github.com/qTox/qTox/issues/6506))
  - Initialize gain slider to correct position ([ce1ecb42](https://github.com/qTox/qTox/commit/ce1ecb4234444090bd290a4796109783dffc555a), closes [#6285](https://github.com/qTox/qTox/issues/6285))
  - Unlock Settings mutex before emitting setting changed signals ([0f5ba08f](https://github.com/qTox/qTox/commit/0f5ba08fd22ca2f0208be2bac574a4430d755f25), closes [#6218](https://github.com/qTox/qTox/issues/6218))
  - Emit correct signal when spellchecking is updated ([68010986](https://github.com/qTox/qTox/commit/68010986511e3215e05e89f8c1ce27acbfc20de5))
- **smileys:** fix flag emojis ([034d7671](https://github.com/qTox/qTox/commit/034d7671a5837a6320feaba1218f3152cc9e78fe))
- **test:** Don't rely on row IDs after moving tables ([dc462678](https://github.com/qTox/qTox/commit/dc46267833e69628f13c34cdfd2fb0ca1315eea8))
- **tests:** Mock Settings for SmileyPack test ([b894531c](https://github.com/qTox/qTox/commit/b894531cdb6f079f7064aca74ec2ef7e461dddfb), closes [#6530](https://github.com/qTox/qTox/issues/6530))
- **tools:**
  - Add Windows to toxcore version update script ([1d1d2b26](https://github.com/qTox/qTox/commit/1d1d2b26af2a176f1861126cf5342a810843f988))
  - Use correct hash when automatically updating flatpak version ([1f1cb2f5](https://github.com/qTox/qTox/commit/1f1cb2f5f9187cc987e2fbdb1957dde34f96c647))
  - use a subdirectory for release source archives ([7dfd93bd](https://github.com/qTox/qTox/commit/7dfd93bd9138b2c6a50cc2b2f32f68e70a59ecdf), closes [#6203](https://github.com/qTox/qTox/issues/6203))
- **toxcall:**
  - move ToxCall ownership to correct Thread ([6b468e41](https://github.com/qTox/qTox/commit/6b468e41fab5f158a26a9166cc15e190ea845d5f))
  - handle Null sink correctly ([bf3921ce](https://github.com/qTox/qTox/commit/bf3921ce94251a3f56d5f83d1c03735d7fad3821))
- **toxext:**
  - Update toxext to fix memory corruption issues ([761c2fb2](https://github.com/qTox/qTox/commit/761c2fb29260908b674ace7f797eae829bdf652e))
  - Protect use of toxext modifying functions ([5ddb8caa](https://github.com/qTox/qTox/commit/5ddb8caa97cf2f13eeaec3b26e97df2c33d4494d))
- **translation:**
  - match text to translation files part 2 ([98976547](https://github.com/qTox/qTox/commit/98976547feac60c8509a1c3917235d381767b496))
  - match text to translation files ([251beb5b](https://github.com/qTox/qTox/commit/251beb5b70fc50915720ed8217ecc22a949e052f))
  - Fix Simplified Chinese format and add a message ([d48d02a1](https://github.com/qTox/qTox/commit/d48d02a1edc539afface8a9c0b807a5d83312150))
- **travis:**
  - Don't call brew update in Travis CI ([abad3fc0](https://github.com/qTox/qTox/commit/abad3fc095f09e33a481a7eaa3e555dbc1efd4a2))
  - Install virtualenv for nightly build upload script ([a68f5bcc](https://github.com/qTox/qTox/commit/a68f5bcc6f1bcc1240a00ec73f1cc336d70def03))
- **video:**
  - Close video device after checking V4L video modes ([9688a1e8](https://github.com/qTox/qTox/commit/9688a1e82f49da5cdb4b77e49a8d6118e1b385cd), closes [#6346](https://github.com/qTox/qTox/issues/6346))
  - only list video devices with video capture capability ([98a818cb](https://github.com/qTox/qTox/commit/98a818cbea9907947c420d68eb3720554895a27e), closes [#6276](https://github.com/qTox/qTox/issues/6276))

#### Features

- Add desktop notifications support on Linux. ([edbbee9b](https://github.com/TokTok/qTox/commit/edbbee9b2ca865026e5d1be16114dcccfda9d9d7))
- Add debug log view. ([bd07e0ff](https://github.com/TokTok/qTox/commit/bd07e0ffa871af2fe06c1ead0d0d922b59dba244))
- Add new translations: Bengali, Latvian, Flemish, Vietnamese. ([3423eb59](https://github.com/TokTok/qTox/commit/3423eb597b72441901ec9f94849d98d57d9755d2))
- add Fcitx and Uim support to AppImage ([bafce795](https://github.com/qTox/qTox/commit/bafce7953dc784b92abfbdd13958c71dbc5efd63))
- save selected search text after scrolling up ([a6a0481d](https://github.com/qTox/qTox/commit/a6a0481ddccfddbc2237c067c709d4a3d8d28612))
- check chat status before start a search ([76679de2](https://github.com/qTox/qTox/commit/76679de2e09a0a5abae794289bdce15f2871f9e4))
- **CI:**
  - Disallow unsafe implicit casts to QString ([9571434d](https://github.com/qTox/qTox/commit/9571434df97794b5291470ea41791a715d7a153a))
  - Disallow implicit casting of QString to char\* ([b5aa5dac](https://github.com/qTox/qTox/commit/b5aa5dac6332e62ddf66d4eb75b2b5ff2a2607cf))
  - Disallow implicit casting of QByteArray to char\* or void\* ([f1b40116](https://github.com/qTox/qTox/commit/f1b401166a8c876f64436e6a65b989a168ab5c7f))
  - Run clang-tidy check in CI ([c828a16b](https://github.com/qTox/qTox/commit/c828a16b7b1b0c027eb1306a832622b30031c83d))
  - Run AddressSanitizer on Fedora build ([bf6d0159](https://github.com/qTox/qTox/commit/bf6d01598ff9e6d3ebd621d5f286e046d00920c4))
  - Output on test failure in Linux CI ([8d1b0d6c](https://github.com/qTox/qTox/commit/8d1b0d6c6e3cef900bf1ad68d9009af49df0a02c))
- **Core:**
  - Add error parsing for `Tox_Err_Options_New` ([2a2b0799](https://github.com/qTox/qTox/commit/2a2b0799928816a5b2af7d8b74439e9f9a1d9ebf))
  - Add error parsing for `Toxav_Err_Bit_Rate_Set` ([a5660fb6](https://github.com/qTox/qTox/commit/a5660fb6e350801dde68644a03381b481662cacd))
  - Add error parsing for `Toxav_Err_Call` ([47a05c75](https://github.com/qTox/qTox/commit/47a05c75929043a038ed63a435ee402d5f2be151))
  - Add error parsing for `Toxav_Err_Call_Control` ([47a20f60](https://github.com/qTox/qTox/commit/47a20f606160666a6725f6478b905504adcb04bc))
  - Add error parsing for `Tox_Err_File_Send_Chunk` ([6c9c3809](https://github.com/qTox/qTox/commit/6c9c3809152d0dd3f1b68fb7e8dd131c595a6944))
  - Add error parsing for `Tox_Err_File_Control` ([978bcde5](https://github.com/qTox/qTox/commit/978bcde57203510bd6a1bf41eb875f956fa5176e))
  - Add error parsing for `Tox_Err_File_Get` ([040beae5](https://github.com/qTox/qTox/commit/040beae56a2febff8d78972e60b7d7b3681bbad7))
  - Extend and deduplicate `Tox_Err_File_Send` error parsing ([aff56633](https://github.com/qTox/qTox/commit/aff56633515fb5872f0f03ca420d7cec1d53f59a))
  - Add support for retrieving self DHT ID and port ([4f9ca0a4](https://github.com/qTox/qTox/commit/4f9ca0a4113bf12b556d5a6a2c65e40d13583567))
  - Store default bootstrap list separate from user list ([08fdd3a2](https://github.com/qTox/qTox/commit/08fdd3a2c727f5eac34eac4f9c7b2c48f722e1a7))
  - use user editable bootstrap node list ([365a452f](https://github.com/qTox/qTox/commit/365a452fb8d887b03cf6543f4d2aa80be5faca76), closes [#5767](https://github.com/qTox/qTox/issues/5767))
- **History:**
  - Add a GUI error on database schema mismatch ([f4e64ee7](https://github.com/qTox/qTox/commit/f4e64ee7a6149d5fcb9b4bb1003437771bb8957a), closes [#5683](https://github.com/qTox/qTox/issues/5683))
  - Enable group history ([0352ada0](https://github.com/qTox/qTox/commit/0352ada08c7efa4ad6523071bde566f506daeb4d), closes [#5452](https://github.com/qTox/qTox/issues/5452))
  - Remove group history on quit when selected ([1142cc03](https://github.com/qTox/qTox/commit/1142cc03f849811b12da16298a2ae3d34fd7f714))
  - Add self join/part messages ([e740859e](https://github.com/qTox/qTox/commit/e740859ef9007dc6e2b8e708895f83c3a3bbdb69))
  - Save/load system messages to history ([18109b2f](https://github.com/qTox/qTox/commit/18109b2f7fa795c0d310e7e4ab053250d1a45540))
- **Settings:**
  - Add GUI notice for global settings upgrade failure ([efc641ef](https://github.com/qTox/qTox/commit/efc641efd775b33531bde511bc102b8ca313aa49))
  - Add system for versioning and upgrading Settings ([2b41a06b](https://github.com/qTox/qTox/commit/2b41a06b55f2174c4a0393caf45cb8be3f609ca0))
  - Add setting for hiding group join and leave system messages ([1be5b99d](https://github.com/qTox/qTox/commit/1be5b99d1741b0f4c92f9e8b51150e0d44ca0a42))
- **UI:**
  - Add UI for removing group history ([cd3c7b39](https://github.com/qTox/qTox/commit/cd3c7b392eeed2bfc3bc7e9cd83aaf58a2d384ba))
  - Add UI For controlling group join and leave system messages setting ([069ab92f](https://github.com/qTox/qTox/commit/069ab92fd013b915855723f20ac72ad30838d325))
  - add warning in GUI when running unstable version ([b51e4668](https://github.com/qTox/qTox/commit/b51e46683e660ad035f04d0797458ad7ccd08f7d))
  - Make auto-accept size 0 wording less ambiguous ([cbead167](https://github.com/qTox/qTox/commit/cbead167422f405f5a64b5c56c8f978f6e60f1c9))
- **apparmor:**
  - Update AppArmor 2.13.3 profile ([a01d3144](https://github.com/qTox/qTox/commit/a01d31445f403b73ec9a15655e24e0993b0ec097))
  - Add AppArmor 2.13.3 profile ([2d22a76e](https://github.com/qTox/qTox/commit/2d22a76ed32f2faa822a5bc81668e2c0c45f6fe8))
- **build:**
  - Add option to enable ThreadSanitizer ([be167c92](https://github.com/qTox/qTox/commit/be167c92298baae7587b78b0b62a93101bff5e97))
  - add nix-shell support ([faeb699f](https://github.com/qTox/qTox/commit/faeb699fd08d7a6bbd648fed0647cc1f06788b4b))
  - use Debian Buster for Windows cross-compilation ([4fdedba6](https://github.com/qTox/qTox/commit/4fdedba6c041dfcec1089d94029dd635742a3f01))
- **chat:** Add snore to Windows build and install for desktop notifications ([7409e6b4](https://github.com/qTox/qTox/commit/7409e6b4cc584f1d1737e180b4066be371303801))
- **chatlog:**
  - Add support for ed2k file links ([7a3474bf](https://github.com/qTox/qTox/commit/7a3474bf553aa085eea101182345c61eb418a5aa))
  - Add gemini:// to hyperlink regex ([355dfb7a](https://github.com/qTox/qTox/commit/355dfb7a7f4db5bfd30a14bced81c930398a0d28))
  - Disable join and leave system messages based on setting ([de924cfd](https://github.com/qTox/qTox/commit/de924cfdc793f4e71da47f877afa85dcb7a3e91a))
  - Re-implement sliding window ChatLog view ([b36a38e7](https://github.com/qTox/qTox/commit/b36a38e716859876a09ba0a5c8accee1a9355af0), closes [#5878](https://github.com/qTox/qTox/issues/5878), closes [#5940](https://github.com/qTox/qTox/issues/5940), closes [#6223](https://github.com/qTox/qTox/issues/6223))
  - Add image preview on paste ([7c218b38](https://github.com/qTox/qTox/commit/7c218b389db6745aefd43b7d032d142be728333e))
  - Add SystemMessages to SessionChatLog ([916834fb](https://github.com/qTox/qTox/commit/916834fb9c8129a35f8c88bc35ceba32560ce4ca))
  - Upgrade db schema to support system messages ([e9131d33](https://github.com/qTox/qTox/commit/e9131d33aa757604021c741e46fd392f86ccd1d4), closes [#6221](https://github.com/qTox/qTox/issues/6221))
- **coreav:** add assertions to check for threading errors ([29f659c6](https://github.com/qTox/qTox/commit/29f659c6599796340c4d0b99a498c54adf51e882))
- **docs:** Add a security policy for disclosing vulnerabilities ([a99735d0](https://github.com/qTox/qTox/commit/a99735d0e02dc0f5c8183071154359109a00eb87))
- **extensions:**
  - Split messages on extended messages ([26701283](https://github.com/qTox/qTox/commit/26701283cd96aa2b0094035f6b6090351ec297f4))
  - Update documentation ([a11a65af](https://github.com/qTox/qTox/commit/a11a65af2a73be7920a6dc6dfa3ac28e52968cb6))
  - UI updates for extension support ([b7158150](https://github.com/qTox/qTox/commit/b71581501165ccb48c29799ddb857489c28a2736))
- **filesform:** Add in progress transfers to files form ([257a19ca](https://github.com/qTox/qTox/commit/257a19caaaa3a31b0890e3eeb728ef9e19f28ede))
- **groups:** Allow being in group call if only member ([46d57c68](https://github.com/qTox/qTox/commit/46d57c6864fccf8124232efefe7cc4e884cd3509))
- **haiku:**
  - Haiku doesn't use sys/errno.h ([36463f6f](https://github.com/qTox/qTox/commit/36463f6f9dd9b07b65d9b044520a39086b990393))
  - Don't use fstack-protector on Haiku ([d413e7b1](https://github.com/qTox/qTox/commit/d413e7b13c918561361c1c7f7e1f95f5afac1d66))
- **i18n:**
  - Handle English plural forms in translations ([0cb222c0](https://github.com/qTox/qTox/commit/0cb222c0567698d70d4115321def32794a874136))
  - Check for new translatable strings on every PR ([11450afa](https://github.com/qTox/qTox/commit/11450afa810f59837519e919a13f399b8d9ef9a9))
- **ipc:** make IPC user specific ([2fe19180](https://github.com/qTox/qTox/commit/2fe1918083f99cde499eb8bc73c08851553bbd32), closes [#6076](https://github.com/qTox/qTox/issues/6076))
- **l10n:**
  - update Chinese (Simplified) translation from Weblate ([60f8ae49](https://github.com/qTox/qTox/commit/60f8ae49027c892b8f36936c312959e9f60f2359)), ([0d80f04d](https://github.com/qTox/qTox/commit/0d80f04de74d6b9ed72302938cb3cba6cd2dc39b)), ([dbc9b4f0](https://github.com/qTox/qTox/commit/dbc9b4f049fc3939040e675481953b1965feee73))
  - update Russian translation from Weblate ([25506531](https://github.com/qTox/qTox/commit/25506531460d27033029ec411e0f30cd76b427c1)), ([62548f1f](https://github.com/qTox/qTox/commit/62548f1fec96f8e7adc8574b61797d13985a08d3)), ([84837965](https://github.com/qTox/qTox/commit/848379652f19912f06d02a2209b8a4c90edd34c4)), ([479f6b72](https://github.com/qTox/qTox/commit/479f6b72694748cc76ce06417c71243c8d9dbdbd))
  - update Bengali translation from Weblate ([d64154a5](https://github.com/qTox/qTox/commit/d64154a593d11ae62c44528619c581c93b989103))
  - update Portuguese translation from Weblate ([72ed0758](https://github.com/qTox/qTox/commit/72ed0758552ad4c5c6cfb7bd57a064d00d8061c0))
  - update Korean translation from Weblate ([1c0616cc](https://github.com/qTox/qTox/commit/1c0616cca92137fd7f7f8569e4f1b32357d2bcf9))
  - update Polish translation from Weblate ([b66caa58](https://github.com/qTox/qTox/commit/b66caa5895b9409355d4154fe34df11319414fe6)), ([adf4332c](https://github.com/qTox/qTox/commit/adf4332c42be5978139e517d538b94c45b107505)), ([c0c2d1fa](https://github.com/qTox/qTox/commit/c0c2d1fad33fe5ded0e8bc48ef61e9b171b0d534))
  - update Portuguese (Brazil) translation from Weblate ([e5748056](https://github.com/qTox/qTox/commit/e5748056c8d77302c2237b33954f1edefc439c16)), ([5b1661c4](https://github.com/qTox/qTox/commit/5b1661c486336c5c090053fd4d09709b4363e2de))
  - update Greek translation from Weblate ([44bebc75](https://github.com/qTox/qTox/commit/44bebc758251e0dc658cb91b6d2ab205fb449676))
  - update Slovak translation from Weblate ([197c9ebe](https://github.com/qTox/qTox/commit/197c9ebe5b781802d0d3ff3e28aecf3d9345e24d))
  - update Hungarian translation from Weblate ([15864a32](https://github.com/qTox/qTox/commit/15864a3281372412109a61d1607bdb6ed5b97c3a))
  - update Italian translation from Weblate ([ceec7d7f](https://github.com/qTox/qTox/commit/ceec7d7f16e9abb9994b2dbe6c40f510f2d8d52b))
  - update Turkish translation from Weblate ([e4b96179](https://github.com/qTox/qTox/commit/e4b96179d9d65db4d9d18c2568383acb654359d6)), ([db5178c6](https://github.com/qTox/qTox/commit/db5178c6ca8a616ae656c1a4231b98d413211d1a)), ([5968999a](https://github.com/qTox/qTox/commit/5968999aff2961128ab46ecbb451c3af5ca8bf44))
  - update Bulgarian translation from Weblate ([986b5359](https://github.com/qTox/qTox/commit/986b5359a631b055f1766df8982a2f4b5e4da0a6)), ([d797fc27](https://github.com/qTox/qTox/commit/d797fc27a92d6f96b45e6034aa54e5586eb1a59b))
  - update Vietnamese translation from Weblate ([30637137](https://github.com/qTox/qTox/commit/3063713704876bef8462bc25c7a6501541c2ad77))
  - update Norwegian Bokmål translation from Weblate ([78670eda](https://github.com/qTox/qTox/commit/78670eda55a09e33e5953a50779f71666bf1f8ee))
  - update French translation from Weblate ([497feec9](https://github.com/qTox/qTox/commit/497feec9bd63f2195da476527f9a50987944581a))
  - update Danish translation from Weblate ([a494bc78](https://github.com/qTox/qTox/commit/a494bc78e4a2f7b2ac97cdd0ba7200c6a011bc2e)), ([cb659bc8](https://github.com/qTox/qTox/commit/cb659bc83fa8f02522a59426bcb3345b98490dcb))
  - Add Urdu and Icelandic translations ([188fe340](https://github.com/qTox/qTox/commit/188fe340f2cf3fd314cc39c46713b6f53e00ea5c))
- **logging:**
  - Hide toxcore spam when using IPv4 ([ac4f01cf](https://github.com/qTox/qTox/commit/ac4f01cf7f5752e7e470a5d1c0ad074a01270709))
  - check if current code is tagged ([90cc9628](https://github.com/qTox/qTox/commit/90cc962802a5909e0692d5a6f84ac6808803fc28))
- **login:** generate a GUI error when auto login fails ([88322347](https://github.com/qTox/qTox/commit/8832234758d5a025afa15514f0526d3915e1460f))
- **macOS:** Build deps from source, giving backwards compatibility ([e80db6a5](https://github.com/qTox/qTox/commit/e80db6a532bb8af4c03a796aa3c19766cfb92a41), closes [#6260](https://github.com/qTox/qTox/issues/6260))
- **messages:**
  - History and offline message support for extended messages ([5f5f6128](https://github.com/qTox/qTox/commit/5f5f612841a89d575239702edcff499a5ff6019d))
  - Multipacket message support ([7474c6d8](https://github.com/qTox/qTox/commit/7474c6d8acff9347786b569a8cf1badb54c4b1ae))
- **notification:**
  - Notifications now always replace the previous one ([6e163ca5](https://github.com/qTox/qTox/commit/6e163ca5edf3d3ac23ac6fed6521f921a22492a4))
  - Notification string generator for multiple messages ([a9f6543e](https://github.com/qTox/qTox/commit/a9f6543e43c53c6cb6759b40876e082a8ae02480))
- **proxy:** provide commandline tools for proxy settings ([3e1e1f60](https://github.com/qTox/qTox/commit/3e1e1f608797dde7fba21d8894c7daf3fde524ee))
- **style:** Add clang-tidy config ([877ef4ae](https://github.com/qTox/qTox/commit/877ef4ae5b982fc2d539f67d35d7ff36c8e3907e))
- **test:**
  - Bootstrap `core_test` off second local tox instance ([5f870c28](https://github.com/qTox/qTox/commit/5f870c28e5912a1543a3e19261ec0bdf91ae3243))
  - Add mock bootstrap list generator ([6f3fb30f](https://github.com/qTox/qTox/commit/6f3fb30f244cc1de156dcce4f4af504ec62ec4c2))
- **tests:** Merge core and `core_online` tests ([7d773930](https://github.com/qTox/qTox/commit/7d773930c16c1a7fa50c83d426afd386df218a99))
- **windows:**
  - Build our own gdb ([ffc03599](https://github.com/qTox/qTox/commit/ffc0359919f5f9b8886c34a411bd12de350c5c8a))
  - Make sure no dlls are missing ([bfa49df0](https://github.com/qTox/qTox/commit/bfa49df006ee33d8615e88bd2cd3952422b311cc))
  - Link all Windows deps dynamically ([ef38c6b7](https://github.com/qTox/qTox/commit/ef38c6b7b77f7ff1d37ae9fe50d997c421aa0311))

#### Performance

- reduce repainting in animations ([3c58b992](https://github.com/qTox/qTox/commit/3c58b992c63f598c9b0e4f703c30ffa177c9e5dd))
- **Settings:** Make personal settings saving consistently async ([a1f7c71f](https://github.com/qTox/qTox/commit/a1f7c71feb6a79dae75ce76a28f415cb8c12dbf5))

<a name="v1.17.6"></a>

## v1.17.6 (2022-03-07)

### Release notes

Fix Windows installer lax permissions at non-default install locations

#### Bug Fixes

- **Windows:** Correct Program Files directory for 32-bit Windows ([47406e74](https://github.com/qTox/qTox/commit/47406e742297b167a0a6c606ddd9a20823959a06))
- **macos:** update deprecated AVFoundation API ([484ea7ce](https://github.com/qTox/qTox/commit/484ea7ce9be971266a890c9bb76b4e23d0c8a622))

<a name="v1.17.5"></a>

## v1.17.5 (2022-03-05)

### Release notes

Fix Windows installer lax permissions at non-default install locations

#### Bug Fixes

- Update video API usage for newer libavcodec ([f5fabc2f](https://github.com/qTox/qTox/commit/f5fabc2fe24b6f01e47a527b982395a5305d31f6))
- **Windows:**
  - Restrict non-default install directory permissions ([553bd47e](https://github.com/qTox/qTox/commit/553bd47e8171fd4f15e062e4faf734e32002f6fb))
  - Build NSIS installer in Unicode mode ([9f84184b](https://github.com/qTox/qTox/commit/9f84184ba815bfc892691fa611c6756721ba1333))
  - Define installer language before trying to access it ([1353fc93](https://github.com/qTox/qTox/commit/1353fc934ed70e9bfab3e50e42dba5eb139cd59e))

#### Features

- **Settings:** Add setting for hiding group join and leave system messages ([916e797c](https://github.com/qTox/qTox/commit/916e797c10d10ba556e9a3339353f1bd97663d15))
- **UI:** Add UI For controlling group join and leave system messages setting ([423049db](https://github.com/qTox/qTox/commit/423049db50ffea14ec222e1a83ee976029a6afaf))
- **chatlog:** Disable join and leave system messages based on setting ([ee0334ac](https://github.com/qTox/qTox/commit/ee0334acc55215ed8e94bae8fa4ff8976834af20))

<a name="v1.17.4"></a>

## v1.17.4 (2021-12-15)

### Release notes

This release only updates dependency versions including toktok/c-toxcore to v0.2.13 to address CVE-2021-44847. No code changes.

<a name="v1.17.3"></a>

## v1.17.3 (2020-11-24)

### Release notes

Fix failing to start on macOS Big Sur. Update deps for Windows, macOS, flatpak.

#### Features

- **apparmor:**
  - Update AppArmor 2.13.3 profile ([99321c95](https://github.com/qTox/qTox/commit/99321c95c381198b9163d274b4ac13e0e08a2979))
  - Add AppArmor 2.13.3 profile ([3db20a60](https://github.com/qTox/qTox/commit/3db20a608b50e22527de98c07467a7db18da9735))
- **osx:** Add support for macOS 10.16, remove support for macOS 10.13 ([a93c5994](https://github.com/qTox/qTox/commit/a93c5994971a9a0dfe28fc1e916f13e7e0a12805), closes [#6252](https://github.com/qTox/qTox/issues/6252))
- **windows:**
  - Build our own gdb ([a0569e41](https://github.com/qTox/qTox/commit/a0569e41a729809ffa8a2baa187066beb569bf0e))
  - Make sure no dlls are missing ([e06cebad](https://github.com/qTox/qTox/commit/e06cebad5e646847f3bb527c30ce2c4a91857e22))
  - Link all Windows deps dynamically ([2e1349e2](https://github.com/qTox/qTox/commit/2e1349e23bf3b5f5aacc6fd7688b0fcc1241cd92))

#### Bug Fixes

- **apparmor:**
  - Allow access to Aspell personal dictionaries ([44299604](https://github.com/qTox/qTox/commit/44299604b0c23fb9e13e03197a6380e21a87a025))
  - Allow spellchecking ([7cc0e695](https://github.com/qTox/qTox/commit/7cc0e69591f59cdad1cd69980e63e0abe639def9))
- **build:** cache debug deps during 2nd stage ([19005926](https://github.com/qTox/qTox/commit/1900592646be78522fd64effc489ac6bf5e76718))
- **osx:**
  - Update deprecated QDateTime and QProcess APIs ([b42b1f5e](https://github.com/qTox/qTox/commit/b42b1f5e15bfdf341495148f76180648869c2e1e))
  - Don't tap kde-mac/kde since it us deprecated and unused ([bc2d41b4](https://github.com/qTox/qTox/commit/bc2d41b42db36b8e43e08e2c0143a614fdf76b22))
  - Allow rebuild using build script ([8f3a621f](https://github.com/qTox/qTox/commit/8f3a621fc36679f6c36e91753da3455e04187b18))
  - Fix missing variables in macOS build script ([b40ba27b](https://github.com/qTox/qTox/commit/b40ba27be53021494179e2d40c7d9d72751814ff))
- **tools:**
  - Add Windows to toxcore version update script ([bd45095b](https://github.com/qTox/qTox/commit/bd45095b5c14f45a0b84d3b99cc626679f6e8c97))
  - Use correct hash when automatically updating flatpak version ([4fc0fea0](https://github.com/qTox/qTox/commit/4fc0fea072a82c5dd42574726bcc463cedd7c61a))
  - use a subdirectory for release source archives ([953e542d](https://github.com/qTox/qTox/commit/953e542d53271d8d62f23a45875673d24534d948), closes [#6203](https://github.com/qTox/qTox/issues/6203))
- **travis:** Install virtualenv for nightly build upload script ([0413e8b3](https://github.com/qTox/qTox/commit/0413e8b372925e742559e07d870469f9622646d5))
- **ui:** don't notify of available update if local version is newer ([9e99db51](https://github.com/qTox/qTox/commit/9e99db51d92f4d3ee15e6b5f5dca98d76d1cbd87), closes [#6112](https://github.com/qTox/qTox/issues/6112))
- **windows:** iconengines not being installed ([bedcf683](https://github.com/qTox/qTox/commit/bedcf6839ab83f09ff7336d545edac5ebfd42efa))

<a name="v1.17.2"></a>

## v1.17.2 (2020-04-26)

### Release notes

This is a release to fix our automatic deployment scripts, no code changes.

<a name="v1.17.1"></a>

## v1.17.1 (2020-04-23)

### Release notes

qTox v1.17.1 release. For details see CHANGELOG.md.

#### Bug Fixes

- **ci:** remove jenkins deploy step ([48c688bf](https://github.com/qTox/qTox/commit/48c688bf1b1939f1afb239e19764791e4fc98b7a))

<a name="v1.17.0"></a>

## v1.17.0 (2020-04-20)

### Release notes

Since this is the first release in more than a year the changelog is quite huge.
The most notable user facing changes are group chats that stay intact after
disconnects or client shutdown as well as the very often requested dark theme.
Advanced history search was added, as well as file transfers staying in chat
history after client restart. There are of course also tons of fixes, namely
much increased stability of audio and video calls as well as audio group calls.

For v1.17.0 binaries, see v1.17.1.

#### Bug Fixes

- add missing type for Qt slot ([b5785a1b](https://github.com/qTox/qTox/commit/b5785a1b0c03688f534b810482d08debbc66002c))
- remove unused variables ([7e4f7f04](https://github.com/qTox/qTox/commit/7e4f7f0489084cfbdf9a959d0d78566949cbf269))
- hide current date at the top of the chat ([bbbbc6aa](https://github.com/qTox/qTox/commit/bbbbc6aade1b32485888b1842db15f65167fbf81))
- scroll bar stuck to bottom (fix #5755) ([38df897e](https://github.com/qTox/qTox/commit/38df897e024393ba15c80017930dac19a92e13b7), closes [#5755](https://github.com/qTox/qTox/issues/5755))
- remove reconnect button ([909deb0f](https://github.com/qTox/qTox/commit/909deb0febba2d122c7c6f2455c7cb85e83ee389))
- update workerStb ([177bf12f](https://github.com/qTox/qTox/commit/177bf12f1150a842e5263eb573fe110fb717ea5a))
- data validation during the search (fix: #5791, #5723) ([acb91ed7](https://github.com/qTox/qTox/commit/acb91ed731bece1fbdac90064446c61ab68b721f), closes [#5723](https://github.com/qTox/qTox/issues/5723), closes [#5791](https://github.com/qTox/qTox/issues/5791))
- empty username causes mention on ever message ([db802822](https://github.com/qTox/qTox/commit/db802822f3b2625ffa19d491d786bdb72666da49), closes [#2119](https://github.com/qTox/qTox/issues/2119))
- check time for get num messages for friend (fix #5714) ([b0f32379](https://github.com/qTox/qTox/commit/b0f32379d098cf525bcbe003d7add071c708bd69), closes [#5714](https://github.com/qTox/qTox/issues/5714))
- Call 'tr' in place, where text is accessible ([15d72a96](https://github.com/qTox/qTox/commit/15d72a9610bc6e4e2e93a9c36ee6536b0aa13429))
- register RowId meta type for use in fileInserted signal ([18b52ce5](https://github.com/qTox/qTox/commit/18b52ce56841f682bf193ad5820c55a62990aa2b))
- initialization theme ([d517c3a6](https://github.com/qTox/qTox/commit/d517c3a638a9355ba8a799fd00119a56a3e9c88d))
- fix uninitialized variable ([ea18b613](https://github.com/qTox/qTox/commit/ea18b613ba4157529af2e1e888bd93d73d7ec7be)), ([52f0e2db](https://github.com/qTox/qTox/commit/52f0e2db9417af76eedf133f4435bc926bf659b1))
- Store groupId in D&D mime data ([8499a710](https://github.com/qTox/qTox/commit/8499a710929e045be8f031b25f44e19fed5d35f8))
- check valid date in getDateWhereFindPhrase ([c3095ab1](https://github.com/qTox/qTox/commit/c3095ab150a360cfc1787d84ab3c6a503cc31c9e))
- fix memory leak in CoreAV and missuse of `std::uniqe_ptr` ([d776e6c3](https://github.com/qTox/qTox/commit/d776e6c34f495dfec445a55dd16ff71b2f0b7af5))
- add remove history prompt, clear log area after remove ([e6d40be7](https://github.com/qTox/qTox/commit/e6d40be72eff546f07b30a2ad33dc95fc8a29362))
- show the date every new day (fixes #5280) ([8808c630](https://github.com/qTox/qTox/commit/8808c630f25f72ebb9974fa2786d83a9e11e49b8), closes [#5280](https://github.com/qTox/qTox/issues/5280))
- correct format log files ([ee0d4bb8](https://github.com/qTox/qTox/commit/ee0d4bb880a986416ed9d0ca4c147adb0bfbc37a))
- **AVForm:** don't load gain when device not ready ([cef773c4](https://github.com/qTox/qTox/commit/cef773c4f69d079340f1379d4a7d787cea03acce))
- **CircleCI:** make cache depend on script files ([7bbbb737](https://github.com/qTox/qTox/commit/7bbbb7377a180fc3e277ecb0a02606beb072b76b))
- **TravisCI:** build windows for release tags too ([c53a58e6](https://github.com/qTox/qTox/commit/c53a58e64e737a206378d3a92a5282df0758d4e7))
- **UI:**
  - add vertical spacer to profileform ([c13ede85](https://github.com/qTox/qTox/commit/c13ede85df80faa67aee12b0c096dee52d94c464))
  - fix auto-accept directory setup display ([8c5cab93](https://github.com/qTox/qTox/commit/8c5cab935ccf3d902d0a9c10509798bbe236a86c), closes [#5917](https://github.com/qTox/qTox/issues/5917))
  - fix reversed mute/unmute hover text ([0055c776](https://github.com/qTox/qTox/commit/0055c7760cd10caa7814acc36d41a12e2bca8af3))
  - fix away icon shape ([b4450319](https://github.com/qTox/qTox/commit/b445031912ae943d7580ea21f6813e9f71bc03ee), closes [#5684](https://github.com/qTox/qTox/issues/5684))
  - fix 2 ([1c02dc2d](https://github.com/qTox/qTox/commit/1c02dc2d6ba4f6d217c2fc55fa513d252aa4dbde))
  - minor fix ([161a773d](https://github.com/qTox/qTox/commit/161a773dbbc4546442ad67dc96400fcd583fdccf)), ([4a6ebff6](https://github.com/qTox/qTox/commit/4a6ebff63a78650279856fcf3e1737d54e2523a0))
  - revert CMakeLists.txt ([821ed526](https://github.com/qTox/qTox/commit/821ed5261641043121d0b716d381346ac7502a15))
  - apply requested changes ([f8b54b39](https://github.com/qTox/qTox/commit/f8b54b39f311b6d05b29e517c88c7fcf5fa4f5de))
  - don't forget to update UI ([84e41303](https://github.com/qTox/qTox/commit/84e41303ce6297df68886d4a1615a07286aed794))
  - Added option to hide message sender and contents ([3b305dd0](https://github.com/qTox/qTox/commit/3b305dd05a8210928a2bed7026b7f4fa7d9003bf))
  - Improved notifications ([1bbe210c](https://github.com/qTox/qTox/commit/1bbe210c254a8ebbc6f529def5632ac47a941077))
  - don't duplicate group peer name in tooltip ([87a53fad](https://github.com/qTox/qTox/commit/87a53fad96eb372630914748a864a45f55f13d29))
  - show empty status as placeholder instead of as status ([e9f87951](https://github.com/qTox/qTox/commit/e9f8795197613326ba7e3d6d79a88c5ecd2cce30))
  - fix typo, add plural to translation ([0dc46cdc](https://github.com/qTox/qTox/commit/0dc46cdc81bd49f892b75fc49f2944ed826d296b))
  - update peer label's style after setting audio playing property ([47795073](https://github.com/qTox/qTox/commit/477950737ffe2e6350862f00aa644da4235c99fc))
  - update UI when leaving group call due to being last member ([7f802f59](https://github.com/qTox/qTox/commit/7f802f593eed3f49f9f209aa45f64851e3c32b23))
  - add tooltip to about friend window ([4e76084c](https://github.com/qTox/qTox/commit/4e76084c6f634c2dde87c5797c1a18f6592fbda8))
  - provide context when emitting friendWidgetRenamed() ([0b6d6d77](https://github.com/qTox/qTox/commit/0b6d6d771539ee7c67d4850dbd7586807cdaa92d), closes [#5495](https://github.com/qTox/qTox/issues/5495))
  - tighten idealSize() for chatlog Timestamps ([c9f3830b](https://github.com/qTox/qTox/commit/c9f3830bc28dc0b0c7e6597f86a27fff0657cbd1), closes [#3957](https://github.com/qTox/qTox/issues/3957))
  - remove useless variable ([2557f5e0](https://github.com/qTox/qTox/commit/2557f5e04521a7e19708e6254936e9d0eb625beb))
  - oops ([d54e2106](https://github.com/qTox/qTox/commit/d54e21064c596dceef87b43b41d5b2723f244131))
  - fix anything ([89cb2425](https://github.com/qTox/qTox/commit/89cb24254dc21077e2d0b8d014932e87940586ba))
  - groupcolors fix2 ([a4a8e361](https://github.com/qTox/qTox/commit/a4a8e361d0e5aed1b3461dacd63b6a3600339a6f))
  - groupcolors fix1 ([ab89ca2d](https://github.com/qTox/qTox/commit/ab89ca2d8ebb641d7af2d5b1100eafe8bde759d4))
  - allow adding friends with Tox URI ([dec90ad8](https://github.com/qTox/qTox/commit/dec90ad84da5edd2c530196af8024a1259c58068), closes [#5385](https://github.com/qTox/qTox/issues/5385))
  - require . in www.url regex matching ([13b15a87](https://github.com/qTox/qTox/commit/13b15a87d71a91cbe49d7f868e914caf8d0aadc5))
  - re-sort friends list on friend rename ([aa7542f7](https://github.com/qTox/qTox/commit/aa7542f729346fc87b113259c361d781bbac5da8), closes [#5387](https://github.com/qTox/qTox/issues/5387))
  - only add chat message for call once ([5f48fd4e](https://github.com/qTox/qTox/commit/5f48fd4e100fc41874aabc57a84884260a248894), closes [#5370](https://github.com/qTox/qTox/issues/5370))
  - amend QCheckBox background chromatic aberration ([b7019a4f](https://github.com/qTox/qTox/commit/b7019a4f667f4247f8eac7e52f36ebf6e6349b93))
  - avatar highlight on hover in profile ([064dccf8](https://github.com/qTox/qTox/commit/064dccf8b49212b65c2c114fc87a9226b5ecf518))
  - remove focus window option, fixing alert() on Windows ([91bdd40f](https://github.com/qTox/qTox/commit/91bdd40f10aa8731e5a3a1ee7d14682e142cc8f5), closes [#4762](https://github.com/qTox/qTox/issues/4762))
  - by default, don't grab focus on new message ([dac1582b](https://github.com/qTox/qTox/commit/dac1582b72290df846fd29da5a9922a83d709056), closes [#5284](https://github.com/qTox/qTox/issues/5284))
  - don't scale avatar to point of cropping in chat form header ([dc7ddbeb](https://github.com/qTox/qTox/commit/dc7ddbebad370bb7c7bc851a738037fc3c18d3fa))
  - Display most recent group member's name instead of `<Empty>` ([11131fa0](https://github.com/qTox/qTox/commit/11131fa083d2c6373358ade5e1d3bb0f08c43a72), closes [#5294](https://github.com/qTox/qTox/issues/5294))
  - fix the context menu enabled/disabled behaviors ([bc69c8c9](https://github.com/qTox/qTox/commit/bc69c8c9196f3ec3682c415145e5b7003c3a3006))
  - don't emit filename change windows for every chat ([291e86d9](https://github.com/qTox/qTox/commit/291e86d9f9ea877fd92daa3b83ac283d56b305d5))
- **activity:** change last activity time from QDate to QDateTime ([d55332ee](https://github.com/qTox/qTox/commit/d55332ee0a0fdda8f63c6ac49f4feb8762bbcc9d))
- **alias:** allow clearing alias from chatformheader ([dfec934f](https://github.com/qTox/qTox/commit/dfec934ff043d631b3a445f557454f0afecb981c))
- **apparmor:**
  - Make network rules more strict ([11a32e33](https://github.com/qTox/qTox/commit/11a32e337182df5161299e648dc9fdc2b41e47cc))
  - Fix typo in file path ([4d9cc721](https://github.com/qTox/qTox/commit/4d9cc7216af59ebf9300d433ba3bfc4e2709977a))
  - Fix screenshot capture under AppArmor ([5304ba4c](https://github.com/qTox/qTox/commit/5304ba4cb0de52008cc9f9b6ae3c6838fa54b717))
  - Backport fix from dri-common abstraction ([f6c11c9b](https://github.com/qTox/qTox/commit/f6c11c9b6d2f6412ca5204a491e22cc12decab5b))
  - Fix openSUSE-related AppArmor denies ([7a1fb927](https://github.com/qTox/qTox/commit/7a1fb927ec4e4d74e5381aab40d57228c072fade))
  - Fix font-related denies on openSUSE ([488b8a86](https://github.com/qTox/qTox/commit/488b8a869628d142e87663e36dc9e86981016d96))
  - fix file dialog denies ([4565ac1b](https://github.com/qTox/qTox/commit/4565ac1b19903023d6d71043b37f120977e9b451))
  - fix file dialog on KDE desktop ([dffe00b4](https://github.com/qTox/qTox/commit/dffe00b4e3ddcef5a56ec37fd3d4fcfd847c2704))
  - backport kde abstraction ([e1ba972d](https://github.com/qTox/qTox/commit/e1ba972d8bfa2e1c1b98bd73417d783b4abd2077))
  - Fix spam of DENIED messages on openSUSE ([c8eb34f0](https://github.com/qTox/qTox/commit/c8eb34f028e2051cc83b877c65799c289c6dc21e))
  - Fix DBUS denies on Kubuntu 18.04 ([1d120b15](https://github.com/qTox/qTox/commit/1d120b15c27706bfe3b4b4d8ad911db37b33ef41))
  - Backport dri-enumerate abstraction ([79f800b3](https://github.com/qTox/qTox/commit/79f800b39a3b0b12faf4959429104e14ecf881e5))
  - Fix .local/share/qTox/ access ([e13b8a97](https://github.com/qTox/qTox/commit/e13b8a973eb8be8252a21feb066bd1853e9002a0))
  - Fix access to openssl configuration ([514cd368](https://github.com/qTox/qTox/commit/514cd368264fdacdf36013ffb496910475506422))
  - Fix dbus access ([a6c01eb0](https://github.com/qTox/qTox/commit/a6c01eb00713c21d8bc64cba47e511b0b5737908))
  - Fix hunspell access ([577aeb8f](https://github.com/qTox/qTox/commit/577aeb8fa36b54d95f5b68b6c1533b170105e6ed))
  - Fix accessibility DBus access ([a67faf29](https://github.com/qTox/qTox/commit/a67faf2976d2b8bcb67ae2feceb3772e3d3266fd))
  - Fix qTox cache access ([aef47056](https://github.com/qTox/qTox/commit/aef47056367642d842e090d2362698dfcf9cc06a))
  - Add ibus abstraction ([9fc89338](https://github.com/qTox/qTox/commit/9fc89338830a1c208b79991b9b2451f059827265))
  - Backport qt5 abstraction for v2.12.1 profile ([6aa4435d](https://github.com/qTox/qTox/commit/6aa4435d174e71d4a788bbb27514387a34d1c6b9))
  - Fix loading libraries from custom install prefix ([5fad77b9](https://github.com/qTox/qTox/commit/5fad77b9f8813d87b97d4b1e8a7d862e2deb13cf))
  - Fix AppArmor profile for version 2.12.1 ([f8f7a2d1](https://github.com/qTox/qTox/commit/f8f7a2d14554919b424929403fb74f665c50a6fd))
- **audio:**
  - correctly loop incoming call sound ([85f90ab2](https://github.com/qTox/qTox/commit/85f90ab2219b3c6bd5e9eaf3b948ad56016e6aaa), closes [#5680](https://github.com/qTox/qTox/issues/5680))
  - input device not closed under certain circumstances ([80f5cb5f](https://github.com/qTox/qTox/commit/80f5cb5f7a275f2dc498e8f3fe3a84d702133789), closes [#3625](https://github.com/qTox/qTox/issues/3625))
  - specify format for sounds and make sounds follow it ([5d65ab38](https://github.com/qTox/qTox/commit/5d65ab387631cf76adfc36f3f0bf356ec7fd6f7b))
- **autocomplete:** don't auto-complete own nick ([f188409b](https://github.com/qTox/qTox/commit/f188409b8ce85515fc3b45a2aedb3af79af2b009))
- **avatar:**
  - reject avatars that are larger than 64KB ([6e2ac12d](https://github.com/qTox/qTox/commit/6e2ac12d84df141b85b427765a74a704146d5b70))
  - set friend to identicon on empty avatar receipt ([0c757356](https://github.com/qTox/qTox/commit/0c757356d3efe79438fdbe232e5c4b0354b7a740), closes [#4724](https://github.com/qTox/qTox/issues/4724))
- **blocked:** change blocked icon to match other used icons ([ff5c9052](https://github.com/qTox/qTox/commit/ff5c90525de6ee7e0b1c6ea6d27cc203f81c5aea))
- **build:**
  - disable Werror by default, keep enabled on CI ([9888bc2d](https://github.com/qTox/qTox/commit/9888bc2d8072f0360a10ba32b6a5834668f0349f))
  - appease appstream-glib validate ([98a364ea](https://github.com/qTox/qTox/commit/98a364ea2e13041f9c94dc9b70ed626226383683))
  - fix OpenAL Soft failing to build with newer CMake ([2419b627](https://github.com/qTox/qTox/commit/2419b6276282d4ccc8f9dd26592872318684cced))
  - fix SQLCipher build issue with OpenSSL 1.1.1 on Windows ([8be99c08](https://github.com/qTox/qTox/commit/8be99c0861a1e9864a27d01b6d2b5ab8a38c578f))
  - fix cmake command in appimage/build.sh ([e9665d51](https://github.com/qTox/qTox/commit/e9665d517aada920d81e57b9a5370bfb22773fbd))
  - install and use AppImageUpdaterBridge ([837416b9](https://github.com/qTox/qTox/commit/837416b9039252720295a58568e36be50e36c3f5))
  - bundle missing libjack.so\* to work with Fedora Workstation ([e50b3bbd](https://github.com/qTox/qTox/commit/e50b3bbd5c2e25a7d0dad8e7a374f785b57a172f))
  - copy OpenSSL libs to AppDir ([7153c1f2](https://github.com/qTox/qTox/commit/7153c1f2bf27c999be125fb136ab6383305eb875))
  - add required private slots ([a9a953cf](https://github.com/qTox/qTox/commit/a9a953cf1cdd8a6142081b4ecf54060ce6ca993b))
  - fix ifdefs ([d23e4fd7](https://github.com/qTox/qTox/commit/d23e4fd7095ed07d7d0173bab0edb90bdb799f7d))
  - fixup CircleCI 2.0 config ([9caeb943](https://github.com/qTox/qTox/commit/9caeb943616c3092f26b9b1fc2f42a971f8a85c3))
  - show brew compilation progress to avoid being killed by Travis ([fd709722](https://github.com/qTox/qTox/commit/fd7097222d48304e3e509d20488eb158eef9c493))
  - manually update brew to workaround autoupdate bug ([c9cb5fef](https://github.com/qTox/qTox/commit/c9cb5fefdd34a431d8f65caf477117f043365d2a))
  - add `cmake_policy` for CMP0071 to suppress warinings in cmake ([6a240141](https://github.com/qTox/qTox/commit/6a240141eafbc5304bc0fcc572d3b7f8e2eb6f5b))
  - fix default cmake build not actually being debug ([edb306c7](https://github.com/qTox/qTox/commit/edb306c723cbbf4e7aa18fd4f08446ecf6053718))
- **calls:** Fix SIGSEGV on quit while in call ([87eed97a](https://github.com/qTox/qTox/commit/87eed97ab3daf432a7ed215f8dbeca95ce265dff))
- **chatform:**
  - Prevent date line oscillations maxing CPU ([64bae38b](https://github.com/qTox/qTox/commit/64bae38b999ee4ddd955695300beb1e5bb20446f), closes [#5620](https://github.com/qTox/qTox/issues/5620))
  - don't try to send empty action messages ([89e94b6f](https://github.com/qTox/qTox/commit/89e94b6f895c8c57f4541147b6e56ec4d30adaac))
  - fix stuck spinner on messages not at end of chatform ([9819aefc](https://github.com/qTox/qTox/commit/9819aefc44767220e696b9c0de0594a5992ee706), closes [#5763](https://github.com/qTox/qTox/issues/5763))
  - fix a crash when there are no messages to load ([040c6b95](https://github.com/qTox/qTox/commit/040c6b95aed26dfc7950770ed4dca379e8be2bc5))
  - crash after opening chat in new window ([59a59fdc](https://github.com/qTox/qTox/commit/59a59fdc09ee5cbf32c915b94cb8ba8e8f0a29b1))
  - add hack to avoid Qt bug on chat show ([e8d48e87](https://github.com/qTox/qTox/commit/e8d48e87888548c92daa88880ee7689d58b0c490), closes [#5570](https://github.com/qTox/qTox/issues/5570))
  - Issue 5115, use QSharedPointer for groupChatForms ([37e5b6ce](https://github.com/qTox/qTox/commit/37e5b6ce8ba572b0f6e3dc067b7f7c99963515b7))
  - don't attempt to send messages to offline friends ([d9e587e4](https://github.com/qTox/qTox/commit/d9e587e4f5f85df979ed11fabfed443280716045))
  - mark message with triple click ([2cdff7e9](https://github.com/qTox/qTox/commit/2cdff7e9ed5efd5402a7cb053bbf24cd29034927), closes [#5211](https://github.com/qTox/qTox/issues/5211))
- **chatlog:**
  - enable dynamic view range in chatlog with history disabled ([a7f34959](https://github.com/qTox/qTox/commit/a7f349595696b3292897ac72b5cffb05b7300eda))
  - fix stick to bottom behavior ([f2fa6010](https://github.com/qTox/qTox/commit/f2fa601073373ae2ef9fba7952aed415af3ccee0))
  - update precise selection when chatlog content changes ([b95bac23](https://github.com/qTox/qTox/commit/b95bac238d1caa40086f19b83dc20e0e9243719d), closes [#5769](https://github.com/qTox/qTox/issues/5769))
  - Prepare geometry changes on chatline proxy ([74d0b47c](https://github.com/qTox/qTox/commit/74d0b47ceb39c3c8962f77f8f6b3060476f64e67), closes [#5818](https://github.com/qTox/qTox/issues/5818))
  - update multi line selection on chatlog change ([2bba1217](https://github.com/qTox/qTox/commit/2bba12175ee34b835c3888c391b3169f56d36bf7), closes [#5769](https://github.com/qTox/qTox/issues/5769))
  - update timestamp when db is slower than ack ([ca397aeb](https://github.com/qTox/qTox/commit/ca397aebbfb388df3754d36331d669c74d87a15c))
- **ci:**
  - quote shell comparison to avoid errors when unset ([475128d1](https://github.com/qTox/qTox/commit/475128d171984de5f9ff34f98020a119dce65b42))
  - fix travis' window build in debian docker by upgrading libseccomp2 ([93c9eef5](https://github.com/qTox/qTox/commit/93c9eef51f8143c3e3e5c1e63e0cc64d1d533f6e), closes [#5874](https://github.com/qTox/qTox/issues/5874))
  - upgrade travis to xcode9.3 to get macos 10.13 with brew support ([6fb5b205](https://github.com/qTox/qTox/commit/6fb5b205f41d4e37d2d4964af2a99fc5abc937f6), closes [#5515](https://github.com/qTox/qTox/issues/5515))
  - increase `bsu_test` timeout ([81901696](https://github.com/qTox/qTox/commit/819016960d33664eb3b0af11c85dbe24c4277777))
- **core:**
  - message when peer changes group name ([0b5f7511](https://github.com/qTox/qTox/commit/0b5f751104664fb1d624910ed930ab1b05a7cc4f), closes [#6001](https://github.com/qTox/qTox/issues/6001))
  - avoid using stack allocated memory past end of scope ([9b23abc6](https://github.com/qTox/qTox/commit/9b23abc6dedf2ecba84920ef878335c9a76c592f))
  - avoid logging which bootstrap node is being connected to ([731a4985](https://github.com/qTox/qTox/commit/731a49854a6b94b2b237048765e76f2695125181))
  - don't log critical on every group join ([e4f73011](https://github.com/qTox/qTox/commit/e4f73011f9346332d27a8d59a1fbb07834c32edb), closes [#5118](https://github.com/qTox/qTox/issues/5118))
  - save tox profile when updating group or friend states ([3d7a872f](https://github.com/qTox/qTox/commit/3d7a872f1aeb2388e3a8f583fd7d494ee7f66185))
  - don't set core status on connect, just update UI ([602671c9](https://github.com/qTox/qTox/commit/602671c9dcb1c9f42dd2d7fb9dfbadd729fb8efd))
  - correctly relink ui/core when core is changed ([17d5d552](https://github.com/qTox/qTox/commit/17d5d55259ab9864c430df8bdc9b440a0315a59c), closes [#5710](https://github.com/qTox/qTox/issues/5710))
  - if your username is empty, use toxPK instead in groups ([72bcc6ac](https://github.com/qTox/qTox/commit/72bcc6acaf22ef78c2db852a5cabf45f42e5af90))
  - also print PKs in group userlist ([066bdc5c](https://github.com/qTox/qTox/commit/066bdc5cc1b0bbe05c35bb184f8b6981fd95e0ce))
  - this should resolve message handling in persistent groups ([ee500703](https://github.com/qTox/qTox/commit/ee500703aab6418fde60ddd7237e6461fb5c7f55))
  - fixed syntax ([7a437c74](https://github.com/qTox/qTox/commit/7a437c74493333bb77f18418059ff329dde55340))
  - ignore mentioning users with empty nicknames ([96ad27f6](https://github.com/qTox/qTox/commit/96ad27f644fd151a608abdffe045ad215db28564))
  - fix for users without nicknames ([2000bf55](https://github.com/qTox/qTox/commit/2000bf55f2f7e3c3573df8130e74850b1202b044))
  - update group peerLists on local changes ([01f79b0d](https://github.com/qTox/qTox/commit/01f79b0d084176c4b6deaa289060f60652bf7428))
  - support user aliases ([feee0e76](https://github.com/qTox/qTox/commit/feee0e767ffa15b2145f82d16da9c47c2d2f580e))
  - this fixes displaying nickname refreshes in groups ([9b261fd8](https://github.com/qTox/qTox/commit/9b261fd870f8a240cdf93c5acc2235efaa7a8116))
  - fix formatting ([c136a17f](https://github.com/qTox/qTox/commit/c136a17ff0f1eb29b7e9e3b56b0fb8f036ab0fbb))
  - simplify the code ([8c239c8e](https://github.com/qTox/qTox/commit/8c239c8ef6d47177cabfcfca79783d188ac9c76b))
  - fixed Timestamps ([6872ead8](https://github.com/qTox/qTox/commit/6872ead850eeab5cea2e036cb1701da8e116d51e))
  - remove pgc code ([10a4b249](https://github.com/qTox/qTox/commit/10a4b249bbbc7e08923f543a96916911a3a799c4))
  - set username and status on new profile ([109a4ffd](https://github.com/qTox/qTox/commit/109a4ffd43af5018d5848bfe8debfeddff3c4b9b), closes [#5369](https://github.com/qTox/qTox/issues/5369))
  - ensure QTimers are moved with the objects they belong to ([26206a35](https://github.com/qTox/qTox/commit/26206a35ebb7072b55877c91aa509eac4b96a17a))
  - Fix misuse of toxcore `tox_file_send` API ([f988177a](https://github.com/qTox/qTox/commit/f988177a9b6af866ef2cf3aed9e5237ef76d5771))
- **coreav:**
  - prevent racy access to call variable ([dfeca3e9](https://github.com/qTox/qTox/commit/dfeca3e90637a55ce97013ffcf0cfece0e1699a4))
  - change some lock to write locks ([26fcea06](https://github.com/qTox/qTox/commit/26fcea0639ba3af839b0e2017342f92cfdb83df7))
  - fix assert because c-toxcore calls from unexpected thread ([e340688b](https://github.com/qTox/qTox/commit/e340688b94845cf9a37848b757c7b97aec549d94))
  - avoid deadlock between CoreAV, main and Audio thread ([723a8e5d](https://github.com/qTox/qTox/commit/723a8e5dc75d5aaf0c0fb40ce041771c7bdc1abd))
  - cleanup assertions ([a4ac6d67](https://github.com/qTox/qTox/commit/a4ac6d67c7a8459627ca4e5451af1064a670dd67))
- **corefile:**
  - cancel file transfer when peer goes offline ([6522988e](https://github.com/qTox/qTox/commit/6522988e8c2ef408278c0602ac9fd756b235b77b))
  - handle empty avatar transfer in core ([1628d495](https://github.com/qTox/qTox/commit/1628d495d1c27a4864474d5e4a725e690b0dfba7))
  - include file id in map when sending empty avatar ([b1eefc3f](https://github.com/qTox/qTox/commit/b1eefc3f230de36c76a3b158bc55f412f0a65a2a))
- **db:**
  - Support opening and upgrading to any of three SQLCipher params ([2c59c920](https://github.com/qTox/qTox/commit/2c59c9203035e35d81adc982ffe7e661d37e9b1f), closes [#5952](https://github.com/qTox/qTox/issues/5952))
  - Add half-upgraded cipher params ([deb7fbb6](https://github.com/qTox/qTox/commit/deb7fbb67cb5e7f799b01a37791e613471fff95a))
  - fix schema log to display correct version ([92e51b05](https://github.com/qTox/qTox/commit/92e51b05acc550a71e58fe8ecc37181b72602a12))
  - show full sqlite error message ([86b55a0f](https://github.com/qTox/qTox/commit/86b55a0fb0bf1bf5e2fc5ff64918e36c36f88f8f))
  - prepare and execute queued statements one at a time ([d98fe85a](https://github.com/qTox/qTox/commit/d98fe85a582777b454724f0d3ea0642bd3a4b032))
  - preserve `user_version` when adding or removing database password ([7e07025d](https://github.com/qTox/qTox/commit/7e07025d3c74c9bff32d840f5b1165fa2768a0ec))
  - support databases with either SQLCipher 3.x or 4.x defaults ([dafb17b5](https://github.com/qTox/qTox/commit/dafb17b5fa81259a7a70804b56c233b05e76c3b9), closes [#5451](https://github.com/qTox/qTox/issues/5451))
  - use SQLCipher 3.x crypto even with SQLCipher 4.x ([e80dbe2d](https://github.com/qTox/qTox/commit/e80dbe2d83fdb133cedaddefad5fc2dff5a6ec91), closes [#5451](https://github.com/qTox/qTox/issues/5451))
- **docs:** remove extra comma in README.md ([fee147d7](https://github.com/qTox/qTox/commit/fee147d788706d0b43c6d17a7d57514683c6f207))
- **flatpak:**
  - workaround unstable flathub downloads ([55b1f6af](https://github.com/qTox/qTox/commit/55b1f6af255c3322b9b4a2b89e7d8d7a6cf27fed))
  - use SVG for icon ([489027ea](https://github.com/qTox/qTox/commit/489027ea1ef958543a5f90bca9288076497f4204))
- **friendlist:**
  - don't update friend's last activity on every start ([5f40ed8d](https://github.com/qTox/qTox/commit/5f40ed8d8c08cc7305506d796543c295d767515c), closes [#5035](https://github.com/qTox/qTox/issues/5035))
  - don't skip half of friends when sorting by activity ([3f922100](https://github.com/qTox/qTox/commit/3f922100af599e0c19925895f7d43b5a1b6932e5))
- **group:**
  - don't display netcam view for group voice calls ([5b31effd](https://github.com/qTox/qTox/commit/5b31effdb4f4b22c4d2ad64193ce799bef0db2b6), closes [#5918](https://github.com/qTox/qTox/issues/5918))
  - use valid pointer, allowing source invalidation ([765fce94](https://github.com/qTox/qTox/commit/765fce94b73b7e7f31680f352126575cbf129a19), closes [#5681](https://github.com/qTox/qTox/issues/5681))
  - Condense invalid title handling logic, clang-format ([aeddf482](https://github.com/qTox/qTox/commit/aeddf4822ad9930321a04a5a3cff4c7f3a5f1025))
  - fix logic oversight and code style ([a1a37497](https://github.com/qTox/qTox/commit/a1a37497006a5440ea5ab529c2fe413e133d4c4e))
  - set default group chat title when provided title is invalid ([f77a0621](https://github.com/qTox/qTox/commit/f77a062120f64bde9460e6caf4e71d0d802d4c21))
  - always retain own name when updating group peer list ([c772db3b](https://github.com/qTox/qTox/commit/c772db3baa3f9e4694fa7f956ffdfb9ffab36cef), closes [#5686](https://github.com/qTox/qTox/issues/5686))
  - treat empty peer names like empty friend names, by showing pk ([04f1ccda](https://github.com/qTox/qTox/commit/04f1ccda35595832dc2c7465bfada029b07bde80), closes [#5660](https://github.com/qTox/qTox/issues/5660))
- **groups:**
  - Correct color of labels in group call ([3205c2c4](https://github.com/qTox/qTox/commit/3205c2c4079c3ad20766314e642e4ba583cf1e8f)), ([f27eb5b7](https://github.com/qTox/qTox/commit/f27eb5b76c62c724096658df134de3d4f28289e0))
  - Avoid segfault when resizing group audio window ([ce9e820b](https://github.com/qTox/qTox/commit/ce9e820b377f1f899846ca05a3d548a4100fd333)), ([d4d4308e](https://github.com/qTox/qTox/commit/d4d4308e286ffcf9fc23aafb95eb4c9786d85cca))
  - Fix invalid group list on group member join ([836718aa](https://github.com/qTox/qTox/commit/836718aa263039c0e1daef0ba75593a3d35b5cdc), closes [#5838](https://github.com/qTox/qTox/issues/5838))
  - reduce group message size limit by 50 ([6c77d57d](https://github.com/qTox/qTox/commit/6c77d57da8cd35e46251e9835d2edc9c9737197f), closes [#5760](https://github.com/qTox/qTox/issues/5760))
  - remove logic that blocks parseConferenceSendMessageError ([9099eea0](https://github.com/qTox/qTox/commit/9099eea04ffdf9dc624a30ff8efa55b006a306d7))
  - enable AV groups after load ([33d42c97](https://github.com/qTox/qTox/commit/33d42c9766e20fb62bb6a679b5a3f087904fa478), closes [#5509](https://github.com/qTox/qTox/issues/5509))
  - don't add friend alias to groups they aren't in ([6801298e](https://github.com/qTox/qTox/commit/6801298e964f73b8dc8d69a026ea3b1951e00322), closes [#5657](https://github.com/qTox/qTox/issues/5657))
  - fix assert on group invite accept ([0f5ad725](https://github.com/qTox/qTox/commit/0f5ad725d7173ec1a63892d0e6e1ffea768c42b8))
  - add peers if already playing audio when netcam created ([e4891687](https://github.com/qTox/qTox/commit/e48916877536b34bf704414a77b65c2533078abc))
  - don't add peers to netcam view until they play audio ([7c13b8b7](https://github.com/qTox/qTox/commit/7c13b8b7db0b948fc7cf76b4d640efb2aec9c980), closes [#5536](https://github.com/qTox/qTox/issues/5536))
  - check for label in timer audio playing timer callback ([d2508e7e](https://github.com/qTox/qTox/commit/d2508e7eac26d731c746333b5d74e9a051d3f5b7), closes [#5511](https://github.com/qTox/qTox/issues/5511))
  - avoid having to lookup peer pk ([13afbf7e](https://github.com/qTox/qTox/commit/13afbf7ec6e80a50fe9f6b04f31962de1d0bd0c7))
  - don't invalidate all audio sources when peer list changes ([8422c09f](https://github.com/qTox/qTox/commit/8422c09f6a8a8043ce4c46ba4021680562aeb666), closes [#5508](https://github.com/qTox/qTox/issues/5508))
  - don't freeze when opening AV settings during group call ([093962e3](https://github.com/qTox/qTox/commit/093962e3ec05aa21a27083e9d7e096c00194568d), closes [#5510](https://github.com/qTox/qTox/issues/5510))
  - correctly show peers in the call after joining call ([d6abf76a](https://github.com/qTox/qTox/commit/d6abf76a279e45c21c4644b1bf571b1a95948b55), closes [#5506](https://github.com/qTox/qTox/issues/5506))
  - only leave audio call if in a call ([92d3c959](https://github.com/qTox/qTox/commit/92d3c959a1bc80688957c4775eb781e8804b76df))
- **history:**
  - check history settings when getting initial chatlog idx ([c906cdf5](https://github.com/qTox/qTox/commit/c906cdf57b3c4230cdb013d2f9cb4a5bdfc9aea4))
  - Prevent invalid history access ([e3e6e1d9](https://github.com/qTox/qTox/commit/e3e6e1d9c4e22d6f090f153628677ad427cf4900))
  - move stuck action messages to `broken_messages` table ([746314ba](https://github.com/qTox/qTox/commit/746314baf23118be28c5e38ee305448e5ad68ef3))
  - display broken messages UI with error icon ([1a726b54](https://github.com/qTox/qTox/commit/1a726b54cd2e1d72c6cb6f0f9a733e904f5e5371))
  - select broken messages from History, track in ChatLogMessage ([f6a15366](https://github.com/qTox/qTox/commit/f6a15366eff7254fc095f6a323272b602ebc9c30))
  - move stuck pending message into `broken_messages` table ([b28dc300](https://github.com/qTox/qTox/commit/b28dc300610d096966bf18864bda0616efdc93ba), closes [#5776](https://github.com/qTox/qTox/issues/5776))
  - handle errors during db upgrade ([f72f3f71](https://github.com/qTox/qTox/commit/f72f3f714daabd955b256ec696c8917d0820c8b7))
  - select only pending history for the requested friend ([64aa3eae](https://github.com/qTox/qTox/commit/64aa3eae4d687d899a56705c8105a10a8eac4db5))
  - create `file_transfers` table in upgrade ([c12605db](https://github.com/qTox/qTox/commit/c12605db6d645ca313997543c8057a3dfd87bb48))
  - save name to history for friend invite message ([2ccb1ec1](https://github.com/qTox/qTox/commit/2ccb1ec150ed721123b2d1d27b1f726390556522))
- **icon:**
  - use Qt tray icon in all cases, remove platform specific backends ([e85d3f52](https://github.com/qTox/qTox/commit/e85d3f522a4f38a5fb189dafa0b47b38f365dbf0), closes [#5859](https://github.com/qTox/qTox/issues/5859), closes [#5881](https://github.com/qTox/qTox/issues/5881))
  - make busy notification icon translucent in middle ([da4928b7](https://github.com/qTox/qTox/commit/da4928b70481fda5f60707ae000e3442c77eaf5b))
  - centre taskbars ([84836cf6](https://github.com/qTox/qTox/commit/84836cf6a0a549909b5be51c0016ee7c11a88a05))
- **interface:** use virtual destructors for all interfaces ([2b981d88](https://github.com/qTox/qTox/commit/2b981d88a2ddf9cca480ce1045ec558b63284a1f), closes [#6006](https://github.com/qTox/qTox/issues/6006))
- **ipc:** handle ipc failure gracefully ([9dd08397](https://github.com/qTox/qTox/commit/9dd083978ece575820469c38a3b78418e8a5dfaf), closes [#5740](https://github.com/qTox/qTox/issues/5740))
- **log:** don't attempt to close log file if it failed to open on exit ([fae9066b](https://github.com/qTox/qTox/commit/fae9066be6d982854d716ac5a077e4193047a85d))
- **logging:** only log toxcore messages above TRACE level ([7a98ea2d](https://github.com/qTox/qTox/commit/7a98ea2def1d97b759b6a017fe46f8c03aa4f0eb))
- **login:** show login screen when autologin fails ([e55e50d5](https://github.com/qTox/qTox/commit/e55e50d5105bc58cfb39b6ea64c8d2c6cc09ad76), closes [#5781](https://github.com/qTox/qTox/issues/5781))
- **loginScreen:** make loginScreen return values comply with Qt standards ([b4bc0934](https://github.com/qTox/qTox/commit/b4bc09345c682ef0c84521ff60cc8f58fb80b87a), closes [#5781](https://github.com/qTox/qTox/issues/5781))
- **main:** register IPC handlers only after starting up ([bc3d3b3b](https://github.com/qTox/qTox/commit/bc3d3b3b13ad64ebae3a235557a761f3eca35bfd))
- **messages:** Fix broken sanitized name for notifications/alert messages ([fef89d70](https://github.com/qTox/qTox/commit/fef89d70f97edbf2218009e9842f32ad70aa84e5))
- **model:**
  - take QObject receiver argument to interface signal connection macro ([3a5e28ff](https://github.com/qTox/qTox/commit/3a5e28fffb56c1907950b12635c363c3d20216b4))
  - stop interfaces from inheriting from QObject ([b7062b25](https://github.com/qTox/qTox/commit/b7062b251852347dff19595b5bf5eea43d53fcce))
  - return connection from interface macro ([40e43586](https://github.com/qTox/qTox/commit/40e43586f10bbda9ce8e6647c41b9613c775439a))
- **net:** check if the node has all needed fields ([bdb3b61e](https://github.com/qTox/qTox/commit/bdb3b61ee0420fb43a5097dc950f302370716558))
- **notification:** implement review comments ([cd50376c](https://github.com/qTox/qTox/commit/cd50376c2f71561dc8abf0d9318dd954a82fed78))
- **offlinemsg:**
  - don't invalidate iterator before use ([1f80173b](https://github.com/qTox/qTox/commit/1f80173b2d8337fb40068f57e14e1a08a70165ec))
  - fix offline message dispatching on history load ([dbef0b75](https://github.com/qTox/qTox/commit/dbef0b750865473c167683de2a630b4707aa755e))
- **offlingmsg:** only dipatch offline messages on friend online change ([479b39f5](https://github.com/qTox/qTox/commit/479b39f5364dc85817ea1cd31a8046c9bd48d989))
- **osx:**
  - Add camera and microphone usage description. ([28341d84](https://github.com/qTox/qTox/commit/28341d8457127f696b5c2c157d51ebd5d5700086))
  - create /usr/local/sbin directory for brew package installation ([e97b2705](https://github.com/qTox/qTox/commit/e97b27058463df88a28a366f8573cd2238c8bb9e))
- **paths:** fix bugs uncovered while developing test cases ([0ea40905](https://github.com/qTox/qTox/commit/0ea409054ab1a0b950bdcd36a448dc795cd571d9))
- **profile:**
  - load settings before starting Core ([0a30c1b1](https://github.com/qTox/qTox/commit/0a30c1b1c0d6587581dea4d00f3ae1651640575b))
  - avoid deadlock with between main and core thread ([aed820ba](https://github.com/qTox/qTox/commit/aed820ba60fad35b744cc3671ff7b90373c663bc))
- **search:** application crash when starting a search ([77d55d5d](https://github.com/qTox/qTox/commit/77d55d5d7d70a168b9b3a24d865eb1b03eee918a))
- **settings:**
  - apply toxcore settings for encrypted profiles ([74828c92](https://github.com/qTox/qTox/commit/74828c92e55f40ef22bf02e7440f31896fcea375), closes [#5682](https://github.com/qTox/qTox/issues/5682))
  - load personal settings before constructing core ([bef9d4b7](https://github.com/qTox/qTox/commit/bef9d4b77348d78f0c6536827a0fed511dcd9b95))
  - repair saved invalid proxy type due to #5311 ([c8ffa1f9](https://github.com/qTox/qTox/commit/c8ffa1f921cbe36391f5d90ba888451a9310b5d8))
  - Add mutex locks for consistency ([24f8bbf3](https://github.com/qTox/qTox/commit/24f8bbf35a06f2338f84eb8c1b3894cbf48d4d54))
  - save settings early on Windows shutdown ([7839a260](https://github.com/qTox/qTox/commit/7839a2608396c77b60d67ab7aeeecc9c086bb71a), closes [#1969](https://github.com/qTox/qTox/issues/1969))
  - set default proxy type to None, enabling UDP by default ([a2c44cbe](https://github.com/qTox/qTox/commit/a2c44cbeda247bf06953a641d8d745fd0d3df62c), closes [#5311](https://github.com/qTox/qTox/issues/5311))
- **status:** use enum as UI property instead of untranslated string ([881aa308](https://github.com/qTox/qTox/commit/881aa3083aac75153f8fcc9548d48951a01f8fd2))
- **test:**
  - create db tables in defined order, verify indexes ([05064771](https://github.com/qTox/qTox/commit/05064771abe045c59d747c8f778bb9eb829b37b9))
  - include `<set>` in `groupmessagedispatcher_test.cpp` ([34e1e25b](https://github.com/qTox/qTox/commit/34e1e25b7fb12dc662ed197e615c422ea56c867a))
- **toxcall:**
  - move ToxCall ownership to correct Thread ([df7fe223](https://github.com/qTox/qTox/commit/df7fe2239831484c1301350b5f940f59937744c5))
  - handle Null sink correctly ([f02943c1](https://github.com/qTox/qTox/commit/f02943c191ea961d97c343c0360fe550ebe7c75d))
- **toxoptions:** handle nullptr in parameters ([6cce0dae](https://github.com/qTox/qTox/commit/6cce0dae838a08464661f56177c3474d4477e236))
- **transfer:** Accurately represent pause state in UI ([293a1d61](https://github.com/qTox/qTox/commit/293a1d615c0187e906453e7df904ffb320acc554))
- **travis:**
  - update pyenv to python 3.7 after updating ubuntu to 16.04 ([a44cce65](https://github.com/qTox/qTox/commit/a44cce65beb60c5f280b651e0c084fa9c2bdb0dc))
  - fix latest nightly release not updating sometimes ([9b67d261](https://github.com/qTox/qTox/commit/9b67d2619c575cb24a30507d231ac7ca95181221))
- **video:**
  - support UYVY camera pixel format ([4c42e0ea](https://github.com/qTox/qTox/commit/4c42e0ea5b5dbb904bf28aaa5790d3f5acbdb75c), closes [#5479](https://github.com/qTox/qTox/issues/5479))
  - correctly align data passed to toxcore ([5c1fe520](https://github.com/qTox/qTox/commit/5c1fe520102cdfb5e47f6be87d314585e9e2a3c6), closes [#5402](https://github.com/qTox/qTox/issues/5402))
  - don't pass invalid pixel format strings to ffmpeg ([9de40e5a](https://github.com/qTox/qTox/commit/9de40e5a97930de5d002ae3460db25bb2cb6c54c))
  - workaround for webcams that provide no fps value ([3746bd13](https://github.com/qTox/qTox/commit/3746bd13bc345c7785ed3e06ac8f02e364a57ce3), closes [#5082](https://github.com/qTox/qTox/issues/5082))
- **widget:**
  - only change group title once ([f7a2a7d6](https://github.com/qTox/qTox/commit/f7a2a7d64840d9aaa276cc6a51a3d9c8611f76a7))
  - don't try to play audio if it's disable ([2ea50309](https://github.com/qTox/qTox/commit/2ea5030958d2bf47fbedc2294c46677881b62a6a))
  - fix freeze on showMainGui ([df62463e](https://github.com/qTox/qTox/commit/df62463e27f3ccf6cf9b1bf75008af27e41833ef))
  - always force show window on activate event ([08f368da](https://github.com/qTox/qTox/commit/08f368da43a85e79377540a228b607ace844e7ae), closes [#5459](https://github.com/qTox/qTox/issues/5459))

#### Features

- add Fcitx and Uim support to AppImage ([710c32de](https://github.com/qTox/qTox/commit/710c32ded0aad89d2dd14718d068f0146755296d))
- save selected search text after scrolling up ([dbf88007](https://github.com/qTox/qTox/commit/dbf880078e8b3207bf5c4f057bc6071b4c74b9ce))
- check chat status before start a search ([ce570927](https://github.com/qTox/qTox/commit/ce570927b145676ff3a63f36a3fe082fa52b228a))
- prohibition to remove messages in group chat ([5aeac56b](https://github.com/qTox/qTox/commit/5aeac56b761ad24b6a2829fca499b8eff480a306))
- remove part messages from chat ([4c7ecb60](https://github.com/qTox/qTox/commit/4c7ecb60247a0e0d84442e506ae5122204ffb328))
- edit position chat after load history ([c2d5b422](https://github.com/qTox/qTox/commit/c2d5b422b3ff09af329840dd829d9d2163b79e52))
- add action "Go to current date" ([2a9648d1](https://github.com/qTox/qTox/commit/2a9648d12c2f71efa8f9722f1c0fa6e39e701c47))
- edit load history in search ([8c4b1e00](https://github.com/qTox/qTox/commit/8c4b1e00a128b739904ed60543132b34817f0ba5))
- edit function "Load chat history" ([6de1173c](https://github.com/qTox/qTox/commit/6de1173c172a14aec3dba289dd63d5857fe69d19))
- load messages from the database after date ([b705ac80](https://github.com/qTox/qTox/commit/b705ac806059717d98cfd60b1b2f1abdaa84e6a9))
- load messages from the database before date ([fb2957c5](https://github.com/qTox/qTox/commit/fb2957c5ee9b0abffdc8c462e96be71c38d44949))
- add border for qrcode ([191f89ff](https://github.com/qTox/qTox/commit/191f89ffa4b01896a7ce94291f52fb482264eaa6))
- remove old boostrap nodes code ([acef759a](https://github.com/qTox/qTox/commit/acef759a586c38a314943555ee9e2949bbba90c5))
- load bootstrap nodes directly from JSON ([1f2bdf3a](https://github.com/qTox/qTox/commit/1f2bdf3a1b2e723989ae83b525b62b7a13dd610f))
- add color for links in palette ([d35dbcc8](https://github.com/qTox/qTox/commit/d35dbcc870d3f35c9ba075736f85ec27d7c84b42))
- edit reload themes ([e146c11f](https://github.com/qTox/qTox/commit/e146c11f0fbe52e91dda2267a54f83943768d410))
- show date in chat log ([d0e8ba8b](https://github.com/qTox/qTox/commit/d0e8ba8b9ca0de2ca9f5a847ea0c86cacd13833a))
- add class to retrieve bootstrap nodes from nodes.tox.chat ([c3363a1f](https://github.com/qTox/qTox/commit/c3363a1feab643eba488668e30d5afc8fedc5990))
- add message if text not found ([4253301c](https://github.com/qTox/qTox/commit/4253301c5606637c8b1beae082f922af10eb365a))
- Add spell checking ([671b9456](https://github.com/qTox/qTox/commit/671b9456a88c5ef12c6e29114eed4c708f845e69), closes [#1301](https://github.com/qTox/qTox/issues/1301))
- add function for generating a filter for search word only ([17a97f1f](https://github.com/qTox/qTox/commit/17a97f1ff68af50588012bf3b9b31a61cdf2b194))
- add startButton in SearchForm ([8dd83477](https://github.com/qTox/qTox/commit/8dd83477591f9c357abba1892e1a7a6b9ca11af9))
- add functions for change title and info in LoadHistoryDialog ([3b7ba023](https://github.com/qTox/qTox/commit/3b7ba023242ba9e2a5340eb9c1b8ed69db710aef))
- use search settings ([610e04aa](https://github.com/qTox/qTox/commit/610e04aa2698deb24af2f04db5cf3e049101a54d))
- create widget for search settings ([87b340f4](https://github.com/qTox/qTox/commit/87b340f4a14b3544cdc2ea50c1256bca974ac967))
- **apparmor:**
  - Add AppArmor profile install scripts ([2e682c6e](https://github.com/qTox/qTox/commit/2e682c6e6a11d500854b09b61e271683c70c1b12))
  - Add AppArmor v2.12.1 profile ([d6ef3d2e](https://github.com/qTox/qTox/commit/d6ef3d2eae072bea57d7e9fe913426b78f99b811))
  - Add AppArmor profile ([89514eee](https://github.com/qTox/qTox/commit/89514eee6d9ba2495fe1d4fe845f0253601242c5))
- **build:**
  - use Debian Buster for Windows cross-compilation ([6bb2c7c6](https://github.com/qTox/qTox/commit/6bb2c7c62933b6f64351c932cfe869e30f858615))
  - add the delta updater ([5eea8ba2](https://github.com/qTox/qTox/commit/5eea8ba27f6bbc89f534d1432e06c90a2b64e1b7))
  - remove timestamps from build to allow reproducible builds ([013771c1](https://github.com/qTox/qTox/commit/013771c13a9e68a97d113cf268e52a4751cb4505))
  - add option to enable AddressSanitizer ([fd99dfd0](https://github.com/qTox/qTox/commit/fd99dfd0a5989a5e5a92588c4b229951b9c6270d))
  - update docs and tools to provide signed tar.gz ([7ff1d605](https://github.com/qTox/qTox/commit/7ff1d6053a57a29ee6fcc062feb32894c848f415), closes [#5278](https://github.com/qTox/qTox/issues/5278))
- **chatfom:** make magnet links clickable ([5b1bc7e5](https://github.com/qTox/qTox/commit/5b1bc7e52323f6832e00a50c34f8143f4d7cbf33))
- **core:**
  - add send message error handling ([5289c999](https://github.com/qTox/qTox/commit/5289c99962b6be374e92163313bc620eef2cbd58))
  - print a chat log entry when a user joins/leaves the group chat ([cabed6de](https://github.com/qTox/qTox/commit/cabed6def3d4b83b3c10de098bb0defa684583de))
  - set group title for loaded groups ([8db744a5](https://github.com/qTox/qTox/commit/8db744a505fa3ebdb3afcc78661aa15b8586ad81))
  - prepare qTox for groupchat saving ([a82eb6f3](https://github.com/qTox/qTox/commit/a82eb6f36e9e443a361de4e7efa04b92cc60be82))
- **coreav:** add assertions to check for threading errors ([8e54805e](https://github.com/qTox/qTox/commit/8e54805e7d13fe6b28c21b4ed48bc5addeae84d0))
- **db:**
  - File transfer history review comments ([25005c5c](https://github.com/qTox/qTox/commit/25005c5c19c80a3cbd2d966a2bb6bfeaa20eab36))
  - add file hash to file history ([8427be66](https://github.com/qTox/qTox/commit/8427be6678eae2ef151b704feb6bf408038ccdd3))
  - Hookup file history to the rest of the system ([d9b39b31](https://github.com/qTox/qTox/commit/d9b39b3102eff686a072630610c199639f0d8219))
  - Database support for file history ([567ddfb2](https://github.com/qTox/qTox/commit/567ddfb2035dad1f3aad94789e76f8a4e4f77d07))
  - Support schema version upgrades ([fb805b9c](https://github.com/qTox/qTox/commit/fb805b9cdb5f108b15da33aa0bd6c250aac7a8b1))
- **files:** Add maximum size to autoaccept downloads ([c8716e9c](https://github.com/qTox/qTox/commit/c8716e9c4541d696ae6aa04dd6eb48bfaf76f360))
- **groups:**
  - Allow being in group call if only member ([caf4f59f](https://github.com/qTox/qTox/commit/caf4f59fb2a771cce848b316673787940eadd76c))
  - show who is in a group call before joining ([ec07fd72](https://github.com/qTox/qTox/commit/ec07fd7291f660f2dcc8cbb50972809eccb04b48), closes [#5507](https://github.com/qTox/qTox/issues/5507))
- **login:** generate a GUI error when auto login fails ([74377430](https://github.com/qTox/qTox/commit/74377430ce83b4d1e10d52125fc5fb28d328c1c0))
- **notify:**
  - integrate desktop notifications into settings ([4cb00957](https://github.com/qTox/qTox/commit/4cb00957f39faa375e6ad6a454c6c3dd2a97b24b))
  - add desktop notifications using snorenotify ([66e2c010](https://github.com/qTox/qTox/commit/66e2c01029efad7f6195cebcf96175152ce2ce22))
- **offlinemsg:**
  - Enable offline messages with no history ([2283d0c1](https://github.com/qTox/qTox/commit/2283d0c1b0221303197714cc2b6d90b3f0bb8144))
  - Force offline messages to always be enabled ([d934cf37](https://github.com/qTox/qTox/commit/d934cf372b8c030de783cb57a1269224a5a88c4b))
- **paths:** create class to combine all qTox managed paths ([3ee8c665](https://github.com/qTox/qTox/commit/3ee8c665df68a22d3068804961ac46e2b793903b))
- **proxy:** provide commandline tools for proxy settings ([31fec748](https://github.com/qTox/qTox/commit/31fec7488f74dc2fe38f0a8515b415c21f3e2109))
- **settings:** save friend list sorting mode ([c8b156b3](https://github.com/qTox/qTox/commit/c8b156b3a1f756f34a0ef9229fd82033e2a5cee9))
- **themes:** make themes follow standard paths ([133ac8de](https://github.com/qTox/qTox/commit/133ac8def80a79b44899a0d781352ecfeef047c9))
- **travis:** publish nightly builds off Travis-CI ([516c52ad](https://github.com/qTox/qTox/commit/516c52ad207e84fa88606170fdf8d8e94872314c))
- **ui:**
  - add event icons for all statuses ([17048c9c](https://github.com/qTox/qTox/commit/17048c9cc4b3f9ee58924637e57573e285ffa8ee))
  - add update notification enabled with `-DUPDATE_CHECK` ([6c9d7b59](https://github.com/qTox/qTox/commit/6c9d7b59c12a6fc79fc457f22e84060a9bcde940), closes [#5335](https://github.com/qTox/qTox/issues/5335))
  - Added feature to generate colors for user names in tox groups ([aaf5229e](https://github.com/qTox/qTox/commit/aaf5229ece69fb694381164f10bb85eb6fd53ee3))
  - Add ui to setup spell checking ([8d10fe47](https://github.com/qTox/qTox/commit/8d10fe47ecf806ad2a8d238d97edd14142a10b42))

#### Performance

- **history:** enable sql index on `chat_id` in history table ([edd72906](https://github.com/qTox/qTox/commit/edd72906fbd4ff7e8e48ebcd7fed764baf8cd85b))
- **smileys:**
  - create global regex object ([0f90abeb](https://github.com/qTox/qTox/commit/0f90abebddeb99b133e415e9a1e015eb08197690))
  - use one big regex instead of constructing many small ones ([58f8a14a](https://github.com/qTox/qTox/commit/58f8a14a48b7a023def122aff2e72b08b38c10de))

<a name="v1.16.3"></a>

## v1.16.3 (2018-07-20)

### Release notes

This point release fixes flatpak build. No feature changes.

<a name="v1.16.2"></a>

## v1.16.2 (2018-07-15)

### Release notes

This point release fixes dialog spam from receiving invalid filenames and logs
spam. No feature changes.

#### Bug Fixes

- **logging:** only log toxcore messages above TRACE level ([4dc74201](https://github.com/qTox/qTox/commit/4dc7420162e69095942b392048c309e6246d6b21))
- **ui:** don't emit filename change windows for every chat ([0ad80288](https://github.com/qTox/qTox/commit/0ad80288dc81483cd4099723a35d51e8c1f77af3))

<a name="v1.16.1"></a>

## v1.16.1 (2018-07-04)

### Release notes

This point release fixes our deployment of Flapak and AppImage on Github. No
feature changes.

#### Features

- **deploy:** upload Flatpak bundle to Github releases ([59b5578c](https://github.com/qTox/qTox/commit/59b5578c7bffc56f6227c60bfcb38f97d39ec8d9))

#### Bug Fixes

- **deploy:** fix file path in AppImage deployment ([64602f38](https://github.com/qTox/qTox/commit/64602f38f154a3f3d2429146ae5d370b2202d1b8))

<a name="v1.16.0"></a>

## v1.16.0 (2018-07-03)

### Release notes

The most notable additions in this release are a new fullscreen mode for
video calls, a new call end sound and support for more camera resolutions. To
distribute qTox in a more user friendly manner we now publish Flatpak and
AppImage packages.

#### Bug Fixes

- remove full screen btn from audio group chat ([0d3f061b](https://github.com/qTox/qTox/commit/0d3f061ba80d9f3f8a971d2b8e11a7d9b59d180a), closes [#5202](https://github.com/qTox/qTox/issues/5202))
- local toxcore install with bootstrap.sh ([9ca38750](https://github.com/qTox/qTox/commit/9ca3875079adf175f31f568e45aabc37e3409000), closes [#5199](https://github.com/qTox/qTox/issues/5199))
- `simple_make.sh` script ([ead2152d](https://github.com/qTox/qTox/commit/ead2152d6f0d15f7e662975fb3ed8525109794c3))
- Fix PR #5182. Eliminating the 'new' operator at ToxOptionsWrapper ([9b6cd1c0](https://github.com/qTox/qTox/commit/9b6cd1c0227006308d4fe556f2b721865c2d9b21))
- Fix usage of unitialized functions ([06ae7ead](https://github.com/qTox/qTox/commit/06ae7ead0c7c23935c1c05c75d9cb11ed516224b))
- two crashes, uncovered by the persistent groupchat patch ([48179b6a](https://github.com/qTox/qTox/commit/48179b6a19807383e298661a21f97db3b140eb44))
- delete double initialization callDuration ([dc1f5ea0](https://github.com/qTox/qTox/commit/dc1f5ea0a319bf4cbf05989c414ccaea898b4826))
- **Core:**
  - fix use after free of proxyAddrData ([26b59d31](https://github.com/qTox/qTox/commit/26b59d312375ad6391228308aabe45f0a85a1194))
  - Clean illegal chars from filenames ([ab85716f](https://github.com/qTox/qTox/commit/ab85716f00acfe00ff8035670919dd548d7f7f83), closes [#1304](https://github.com/qTox/qTox/issues/1304))
- **appimage:** build sqlcipher form source ([64a7c24b](https://github.com/qTox/qTox/commit/64a7c24b2b5ad11a6df5dbb11da6e3aa7c0fd6f3))
- **audio:**
  - fix error introduced in 67f2605971cf43093c72f811e4df90ab70544dd6 ([40d30153](https://github.com/qTox/qTox/commit/40d30153aed223b65b596dc7d3bf17573b04f3e9))
  - connect the correct audio callbacks ([a00af087](https://github.com/qTox/qTox/commit/a00af087778c6315ef55ed77c4209cbb63a6323d))
  - close the audio device after playing a sound ([a3370173](https://github.com/qTox/qTox/commit/a3370173df24cd6880e3e3845ddbbc7c090b7aed))
- **build:**
  - Elimination the build warnings (Wunused-variable, Wreorder) ([2cd65610](https://github.com/qTox/qTox/commit/2cd65610fcce0c3dcf8a5e9cb9f313a76167c09a))
  - correct install script nsis for win64 ([25e69572](https://github.com/qTox/qTox/commit/25e69572f89d816cfab5a8c0d1c261bae34d3cdd), closes [#4627](https://github.com/qTox/qTox/issues/4627))
  - make qTox compile with ffmpeg 4.0 and newer ([44193176](https://github.com/qTox/qTox/commit/441931765ffe3de349b28a28bf10a006edcc9949))
- **chatform:**
  - name in window title and close detached chats ([39968a31](https://github.com/qTox/qTox/commit/39968a313d78c727046837901e6cc3d6c31d18e0))
  - check for empty path when exporting profile ([757791ee](https://github.com/qTox/qTox/commit/757791eea4be390bb6d1cdc908d1cd3c4b18728d), closes [#5146](https://github.com/qTox/qTox/issues/5146))
- **docs:** update toxcore build instructions ([b00cbc1d](https://github.com/qTox/qTox/commit/b00cbc1d6f3a7f8406e4a96e732c534068fde22c), closes [#5133](https://github.com/qTox/qTox/issues/5133))
- **file:** don't clean the filenames of avatar transfer ([2a8ab03e](https://github.com/qTox/qTox/commit/2a8ab03e46dd08efc4051a01bea56fe6a4c38a11))
- **history:** don't save both action prefix and displayed name ([dfd2de83](https://github.com/qTox/qTox/commit/dfd2de836eae605e02a1afb270620dd9274f6385))
- **leak:** Fix few memory leaks ([daaa5518](https://github.com/qTox/qTox/commit/daaa5518dd7c02c2de45690daa3f592206fc4023))
- **login:** start login screen on profile select by -p option ([1af3ad69](https://github.com/qTox/qTox/commit/1af3ad69e884bc4e74a4fcdd452a6aff10bffd62))
- **settings:**
  - automatically disable UDP when a proxy is set ([977b7fc9](https://github.com/qTox/qTox/commit/977b7fc9a02b2b44164ffb77ab35f4cdfae90542), closes [#5174](https://github.com/qTox/qTox/issues/5174))
  - prevent segfault on wrong proxy settings ([dfd5232e](https://github.com/qTox/qTox/commit/dfd5232e2fb727685a20804d7ca3b932ea239332))
- **simple_make:** correct variable initialization ([1537f83e](https://github.com/qTox/qTox/commit/1537f83e85ff28dd73fb66161ae2cd5eeef692d1))
- **theme:** clear stylesheet cache on theme colour change ([8ba8ce91](https://github.com/qTox/qTox/commit/8ba8ce91f3317794b72fb4937c459dac2856d367), closes [#5092](https://github.com/qTox/qTox/issues/5092))
- **ui:** increase number of low res camera options ([72931514](https://github.com/qTox/qTox/commit/72931514695a8691593d6a5abd2df1e340f95002))
- **video:** unsubscribe the video device correctly ([e55f86c6](https://github.com/qTox/qTox/commit/e55f86c6a5b0344642fcb3d7a2550df6e899a6e5), closes [#5110](https://github.com/qTox/qTox/issues/5110))
- **wayland:** Fix desktop file name in Qt properties ([c1caeb58](https://github.com/qTox/qTox/commit/c1caeb585a8845eaa72c7db79fb334262eafdb8f), closes [#5141](https://github.com/qTox/qTox/issues/5141))

#### Features

- Add ability to remove dialog from content dialog with middle click ([aae567ed](https://github.com/qTox/qTox/commit/aae567ed8e299fc0cdd700e2e0020042ee1cba11))
- Add ability to quit group with middle click ([228c431c](https://github.com/qTox/qTox/commit/228c431c890a7e68d078b441311892c691643926), closes [#2605](https://github.com/qTox/qTox/issues/2605))
- Add middle mouse clicked signal for GenericChatroom ([65fc1dc2](https://github.com/qTox/qTox/commit/65fc1dc266da29e0679f2b645c31bc428f0cf575))
- **appimage:** build appimage on TravisCI ([f7345e4d](https://github.com/qTox/qTox/commit/f7345e4db264a5681490b9094981a65cac68d317))
- **call:** add call end sound ([65896e45](https://github.com/qTox/qTox/commit/65896e45017f8f748bc5b9db10a4400d7fd418dc), closes [#4905](https://github.com/qTox/qTox/issues/4905))
- **chat:**
  - add UI option to mute group peers ([2fae2a30](https://github.com/qTox/qTox/commit/2fae2a30f76978ce722c5b24236384c8052ebfc4))
  - full screen video chat ([d6df8883](https://github.com/qTox/qTox/commit/d6df8883e399b95a55c5a5870497c1dcd45a3917))
- **core:** put c-toxcore log messages in the qTox log ([4faab075](https://github.com/qTox/qTox/commit/4faab0750d3841beeb08c7d17e85044b5013aea8))
- **history:** load set number of messages from history ([ca32e77d](https://github.com/qTox/qTox/commit/ca32e77d7400e23a6a839f6a8d1f322bfe48bbf0), closes [#3004](https://github.com/qTox/qTox/issues/3004), closes [#3124](https://github.com/qTox/qTox/issues/3124))

<a name="v1.15.0"></a>

## v1.15.0 (2018-04-19)

### Release notes

qTox v1.15.0 release. For details see CHANGELOG.md.

#### Performance

- **ui:** cache stylesheets to reduce memory usage ([6d9d26db](https://github.com/qTox/qTox/commit/6d9d26db654981dbd22bdb0a70dfbc48f89b2e60))

#### Bug Fixes

- remove build date and time from main.cpp ([23f6ad70](https://github.com/qTox/qTox/commit/23f6ad7047a2391defd95b144cfcceac37994a51))
- don't use invalid reference after erasing element ([1afed5f7](https://github.com/qTox/qTox/commit/1afed5f72e78b96222af4ab1d747cc73d6e8df35), closes [#5002](https://github.com/qTox/qTox/issues/5002))
- **IPC:** Update profileName to update IPC ID after login screen ([d2ad2107](https://github.com/qTox/qTox/commit/d2ad2107f2ea9238a0e56ff8dd883d308615ab57), closes [#4948](https://github.com/qTox/qTox/issues/4948), closes [#5025](https://github.com/qTox/qTox/issues/5025))
- **audio:**
  - apply gain to both audio channels ([f6622e40](https://github.com/qTox/qTox/commit/f6622e40928196ae85724f340a760b4ea5f2fb2f), closes [#5039](https://github.com/qTox/qTox/issues/5039))
  - move audio timers to and start from audioThread ([5d0f9509](https://github.com/qTox/qTox/commit/5d0f9509a8d5fac940f402237efe4a8a6ab7e27b), closes [#5053](https://github.com/qTox/qTox/issues/5053))
- **avform:** Audio/Video settings tab #5011 issue bugfix ([ad6ddfc4](https://github.com/qTox/qTox/commit/ad6ddfc4f245f9d0e36a6ce0ef9646a50fde30f8))
- **build:** Remove unused vars, add default case for switch ([bc18990f](https://github.com/qTox/qTox/commit/bc18990fd248c117916c88d640456e6b967a4802), closes [#5006](https://github.com/qTox/qTox/issues/5006))
- **chatform:** include pressed key(s) when changing focus ([a8fc6e5c](https://github.com/qTox/qTox/commit/a8fc6e5c6bb0284950eab8495c8bc8ad45fd6355))
- **chattextedit:** ChatTextEdit bug fix for issue #5020 Corrected ([ebdc675c](https://github.com/qTox/qTox/commit/ebdc675cbc2dd0054306d461c382568c6148bf60))
- **core:** split messages on utf8 multibyte character boundary properly ([869036f3](https://github.com/qTox/qTox/commit/869036f3c6fc567b56fca1e36b7b4bdcda03a089), closes [#4917](https://github.com/qTox/qTox/issues/4917))
- **docs:**
  - include master key fingerprint instead of subkey fingerprint ([3748ca6b](https://github.com/qTox/qTox/commit/3748ca6bc849046a44b83f16104490a685e65055), closes [#5048](https://github.com/qTox/qTox/issues/5048))
  - update dependencies for Debian ([7627d60a](https://github.com/qTox/qTox/commit/7627d60a89c89adea514f97b2f803617a22e6171))
- **history:** don't save own messages when history is disabled ([b852809d](https://github.com/qTox/qTox/commit/b852809d0c554754496a54cadc7d14d3802bc183), closes [#5036](https://github.com/qTox/qTox/issues/5036))
- **import:** don't use java-style iterator before first element ([acea7c31](https://github.com/qTox/qTox/commit/acea7c315fd2b8cb89bac21956a16bca721f0712), closes [#4962](https://github.com/qTox/qTox/issues/4962))
- **login:** Don't dereference null pointer ([703876c3](https://github.com/qTox/qTox/commit/703876c3724386c540d7e62e4f5d19dad31477bb), closes [#5032](https://github.com/qTox/qTox/issues/5032))
- **name:** Clear alias on name change so that name changes are visible ([c2410e0a](https://github.com/qTox/qTox/commit/c2410e0a656f1708ed07bf8b2644300ae54af53c))
- **offlinemsg:** make faux offline messages purely event based ([4951f909](https://github.com/qTox/qTox/commit/4951f90964317a8a409f5ab75ecde3073c72e491))
- **profile:** don't error if no new avatar is selected ([cfdc1cdb](https://github.com/qTox/qTox/commit/cfdc1cdb2e74aeaece70c2798ec9b7fddbbbed39))
- **settings:**
  - connect enable LAN discovery checkbox ([55d8922d](https://github.com/qTox/qTox/commit/55d8922d69f035580dbea92285648d75645bfcf0))
  - add missing enableTestSound to settings ([65d59ba6](https://github.com/qTox/qTox/commit/65d59ba6b7d2f89bf1d4078df3a3647abc84858c))
- **ui:** remove placeholder update UI ([cb0f2635](https://github.com/qTox/qTox/commit/cb0f26356d419b9be945f4f910c7415f14bf024d), closes [#5040](https://github.com/qTox/qTox/issues/5040))
- **widget:** Fix double free crash on group leave ([81989406](https://github.com/qTox/qTox/commit/81989406073a70f7e9b17f03dee78ed7be5e978a), closes [#5004](https://github.com/qTox/qTox/issues/5004))

#### Features

- **Weblate:** create script to automatically merge Weblate changes ([96ae4284](https://github.com/qTox/qTox/commit/96ae4284a09df48fda2cdd0868779ec32a9d18de))
- **camera:** add higher resolution camera options ([04ecfe3f](https://github.com/qTox/qTox/commit/04ecfe3f344c29d9c598d38aaad46f2da8a17728), closes [#5065](https://github.com/qTox/qTox/issues/5065))
- **ui:**
  - add setting for disabling new message notification popup ([fcd88d65](https://github.com/qTox/qTox/commit/fcd88d65b2875c06312374186fd9a08a9637dac7), closes [#4979](https://github.com/qTox/qTox/issues/4979))
  - Add ability to disable LAN discovery ([9f8b0fed](https://github.com/qTox/qTox/commit/9f8b0fed07d5a1debb27252df6dc88e8941ae487), closes [#4074](https://github.com/qTox/qTox/issues/4074))

<a name="v1.14.0"></a>

## v1.14.0 (2018-03-12)

### Release notes

qTox 1.14.0 release

For details see CHANGELOG.md

#### Bug Fixes

- Fix crash on manipulation with friend ([2a0c71d0](https://github.com/qTox/qTox/commit/2a0c71d0d405e02aa4159df7d9e59a24ed3d5c4c), closes [#4991](https://github.com/qTox/qTox/issues/4991))
- Not quit on close if this setting is enabled ([e73dc10c](https://github.com/qTox/qTox/commit/e73dc10c7fd23b887cc5e2d5d4021bc02c8555ec), closes [#4949](https://github.com/qTox/qTox/issues/4949))
- add search symbol ' in history ([3e05279c](https://github.com/qTox/qTox/commit/3e05279c097b33b09cedcebae4150c839a23af35))
- Use real channels number ([e74cc37a](https://github.com/qTox/qTox/commit/e74cc37a2d02e9d4cbd016bac9dbb7697e8445e7))
- Allocate memory to input buffer ([900f2a1a](https://github.com/qTox/qTox/commit/900f2a1ad3b328359a0ae089e778b15280512a9d))
- Call doAudio on timer timeout ([2353a66f](https://github.com/qTox/qTox/commit/2353a66fded32174421c9663ced5cfe4ceabe00b), closes [#4904](https://github.com/qTox/qTox/issues/4904))
- [un]subscribe output in avform ([8c05399e](https://github.com/qTox/qTox/commit/8c05399e418f2c0147ce2d9c7dd220a0cdc97765))
- Correct display the call confirm window (CallConfirmWidget) ([f4fe343e](https://github.com/qTox/qTox/commit/f4fe343eca3eaf84f9ce300b59be9e83a70c204e))
- elimination of warning '-Wreorder' ([0869d3d8](https://github.com/qTox/qTox/commit/0869d3d8fdc9e9de2f1df51c377ddba71a1ce523))
- Use epsilon to compare float ([91dabf11](https://github.com/qTox/qTox/commit/91dabf11d31807f499d6e949373bf22762e80f5b), closes [#2394](https://github.com/qTox/qTox/issues/2394))
- **UI:** prevent deadlocks on logout and profile delete ([a49e3458](https://github.com/qTox/qTox/commit/a49e34589f40edfb3fc46d5700573f87d5dfe3d0), closes [#4896](https://github.com/qTox/qTox/issues/4896))
- **avform:** Hotfix for crash on video device change on 'None' ([e3726ead](https://github.com/qTox/qTox/commit/e3726eade680e2da5c7b235616a9f2f8b3cf8e16), closes [#4994](https://github.com/qTox/qTox/issues/4994))
- **build:**
  - move Appdata file installation to /usr/share/metainfo ([5db0bdd3](https://github.com/qTox/qTox/commit/5db0bdd381f0f08c5685501702f2a2eb9d2f5674))
  - add needed ffmpeg decoder to configuration ([8973a521](https://github.com/qTox/qTox/commit/8973a5216f49e65adc48d5fada8a574db598cced))
  - Add missing dependency for openSUSE ([f7e089f7](https://github.com/qTox/qTox/commit/f7e089f7a71c41ff31d311fe7148e57b5c6fb60a))
- **chatform:** Broaden URL matching to include unicode ([e564b85e](https://github.com/qTox/qTox/commit/e564b85e3c485b283855bfdf00dfc0ec5427fad4), closes [#4295](https://github.com/qTox/qTox/issues/4295), closes [#4853](https://github.com/qTox/qTox/issues/4853))
- **chatlog:**
  - Match multi-character emoticons again ([9643e48e](https://github.com/qTox/qTox/commit/9643e48ef1d68948d52feec4e1be28c3ad61c0da))
  - parse multi-length emoji properly ([5df63f9c](https://github.com/qTox/qTox/commit/5df63f9c2e6d78f4799447b0a22cdb9fb70c3fea))
- **chatwidget:** fix send file button not working ([af1aebfd](https://github.com/qTox/qTox/commit/af1aebfd1a7409ea821be2a616067561b62751c0), closes [#4854](https://github.com/qTox/qTox/issues/4854))
- **cmake:**
  - fix platform extensions for windows ([7ad68e2f](https://github.com/qTox/qTox/commit/7ad68e2f43b458cd00ca27b9cfb20abf0b9ae46c), closes [#4860](https://github.com/qTox/qTox/issues/4860))
  - add missing dependency ([423f0956](https://github.com/qTox/qTox/commit/423f095622824a34d081fb69bddd83cddf83ca03))
- **core:**
  - Adapt qtox to new conferences state change callback. ([1111949f](https://github.com/qTox/qTox/commit/1111949f450fb4fe63321386f7f452ee1663f07a))
  - Use new callback API for bitrate set ([d2deec7c](https://github.com/qTox/qTox/commit/d2deec7c554b3df651fe789dfb7964748329eff4))
  - Use new API for bitrate set ([2c8f03da](https://github.com/qTox/qTox/commit/2c8f03dada443e30d6189050c7cf6d42e01827c5), closes [#4935](https://github.com/qTox/qTox/issues/4935))
- **cpu:** Reduce CPU usage by avatar render ([8db61f96](https://github.com/qTox/qTox/commit/8db61f96ec78ac53479dd8db36eb192f6a1ddbcd), closes [#4843](https://github.com/qTox/qTox/issues/4843))
- **friendwidget:** Use queued connection to avoid removing 'this' ([9b4972e0](https://github.com/qTox/qTox/commit/9b4972e0459de2921370cda9de645eb64e37ecfc), closes [#4966](https://github.com/qTox/qTox/issues/4966))
- **group:** Show correct count of user on first creation ([0a590336](https://github.com/qTox/qTox/commit/0a590336b1467405a903464085dcdfc4474f93e6), closes [#4503](https://github.com/qTox/qTox/issues/4503))
- **install:** Fix gzip invalid usage ([266f63f6](https://github.com/qTox/qTox/commit/266f63f6dfb1869aa2339d48cdc9b52ece3597ce))
- **l10n:** Correction of the translation into Russian ([3fb42b75](https://github.com/qTox/qTox/commit/3fb42b75d75bf6c0240748ffff368b912b14a838)), ([9229fdd1](https://github.com/qTox/qTox/commit/9229fdd17e013a8bd60102648a200734890c2140))
- **smiley:** change license of classic smileys to CC BY-SA 4.0 ([da7c12e2](https://github.com/qTox/qTox/commit/da7c12e20cac1ac7340b4bb4ec89f782e2e4a159))
- **travis:**
  - try working around Travis + gitstats issue ([4c980945](https://github.com/qTox/qTox/commit/4c98094551ff4a1e7377a206b72fedd470b8be96))
  - switch back to older Ubuntu Image ([378daeaa](https://github.com/qTox/qTox/commit/378daeaad4c5992a7acd2b650ff081d213556e10))
- **video:**
  - improve debug message ([ff2fc18b](https://github.com/qTox/qTox/commit/ff2fc18be164fcbc89bfd46d64f4b0096a97aee5))
  - choose first available resolution in preview automatically ([81522dea](https://github.com/qTox/qTox/commit/81522deabdc3fb11fd8d3e1feb59274a96583121))
  - use float framerates also for V4L2 ([a2927de2](https://github.com/qTox/qTox/commit/a2927de27d4776b52303e07c07ce89e8dadf86c5))
  - allow not integer framerates ([db7ee65d](https://github.com/qTox/qTox/commit/db7ee65d0efbe23a45e385a148b20701e521a5c5), closes [#4866](https://github.com/qTox/qTox/issues/4866))
  - Fix square form of a video ([8de8c14a](https://github.com/qTox/qTox/commit/8de8c14a76908cf84a322a0bfd9e2c7ad2b4fa16))
- **widget:** Fix status pic alignment ([d9118cfc](https://github.com/qTox/qTox/commit/d9118cfc71e2b030914187df7fd9fb3d98378cf1))
- **windows:** %APPDATA -> %APPDATA% in template ([f53b8282](https://github.com/qTox/qTox/commit/f53b82825bf76be5a6793d18f2d102ed7b222313))

#### Features

- Add the cmake option `USE_CCACHE` ([aa9cff31](https://github.com/qTox/qTox/commit/aa9cff315d659a7ca2010fb4791893abc8c5abdb))
- update to the new c-toxcore 0.2.0 conferences api ([d3d81bbd](https://github.com/qTox/qTox/commit/d3d81bbdf3c198a7c1258c6ad6405c6ab61cedd4))
- add hot keys for search ([ffb51e8a](https://github.com/qTox/qTox/commit/ffb51e8a0ea7dc3fb01f1f7650edc80b779a9be2))
- optimise search in history ([18fa8a74](https://github.com/qTox/qTox/commit/18fa8a745bdafddc00ba2f577c36451f40edfd61))
- add search in text in group chats ([7718734c](https://github.com/qTox/qTox/commit/7718734c9ab9705c1a1274b2a447611c1a2e22b4))
- remove search button and add line in context menu ([8bb80c77](https://github.com/qTox/qTox/commit/8bb80c770c1d21d1bdfc03c3d0569fabe6535e8f))
- edit load history for search ([de9c9061](https://github.com/qTox/qTox/commit/de9c9061175c97a9ee203d18a39e73f77544d5e6))
- add text search ([b881d32d](https://github.com/qTox/qTox/commit/b881d32d1bddb7352b8d24e2442ef6277ff0d583))
- add form for search ([863c46c7](https://github.com/qTox/qTox/commit/863c46c73d1a2fc677f9142ba8d7a2e8dc659c2a))
- add a button to search ([47d9da98](https://github.com/qTox/qTox/commit/47d9da98cf6811a30d35a1204e5342a4f7f4bf94))
- Prefere new line as message break ([3b52402f](https://github.com/qTox/qTox/commit/3b52402fa20d2d5418e129e5f001b626401a9ae5), closes [#4113](https://github.com/qTox/qTox/issues/4113))
- **UI:** new status icons for message notifications ([4288785d](https://github.com/qTox/qTox/commit/4288785d31e215bc379223577f7d4dd65664ed86))
- **avatar:** Add outline hightlight on mouse hover ([bb26485d](https://github.com/qTox/qTox/commit/bb26485db6fed706f4ebccaffe35740394210032))
- **groupchat:** mark blocked users with different color ([a729f2f8](https://github.com/qTox/qTox/commit/a729f2f8c00d29d2837b6e380f5af1b95c344bad))
- **l10n:**
  - add Macedonian translation ([1a06f85d](https://github.com/qTox/qTox/commit/1a06f85d3ccc91ff6f759a38534483fa40aaaa29))
  - add Macedonian translation using Weblate ([41420331](https://github.com/qTox/qTox/commit/414203310a30720e02e06719bfcafbb8bcff9018))
  - update French translation from Weblate ([a7e90969](https://github.com/qTox/qTox/commit/a7e9096919d4c0b89f061e8b77741d517f574838))
  - update Portuguese translation from Weblate ([3bad087b](https://github.com/qTox/qTox/commit/3bad087bbff2fbff4c4d543df1f96931784c93df)), ([8c3be522](https://github.com/qTox/qTox/commit/8c3be5225f484469aed43dde04f03bc588ca2c15))

#### Performance

- **widget:** don't save on setExpanded if categorywidget is unchanged Fix #4932 ([b9845e1d](https://github.com/qTox/qTox/commit/b9845e1d23eb23380f447692e3a813413e897c2d), closes [#4932](https://github.com/qTox/qTox/issues/4932))

<a name="v1.13.0"></a>

## v1.13.0 (2017-11-26)

### Release notes

In this release we added microphone voice activation and the long-awaited
Github-like Identicon instead of default avatars. Of course also some bugs were
fixed and new ones added :P

#### Bug Fixes

- use only well known categories in the desktop file ([4d36c23c](https://github.com/qTox/qTox/commit/4d36c23c9ba88a350a9b7ae6a7657fda3b8a2af2))
- add default return in `Widget::getStatusIconPath` to fix warning ([8a9c34d8](https://github.com/qTox/qTox/commit/8a9c34d8886ac2db3def1d08e8c203839d68c37d))
- URL patterns based on RFC 3986 ([6ffe4cd8](https://github.com/qTox/qTox/commit/6ffe4cd8d892183ecdfcba571ff8dd4d62d595fd))
- using current setting in autoaccept dialog ([70b235f2](https://github.com/qTox/qTox/commit/70b235f271b2f3d1dfaf036d60bb7e8e63db2908), closes [#4777](https://github.com/qTox/qTox/issues/4777))
- remove unnecessary semicolon to compile with `-pedantic` flag ([37419825](https://github.com/qTox/qTox/commit/37419825cafcc0d4a239c2922abc074b753a4e29))
- add missed `<functional>` header ([5e455699](https://github.com/qTox/qTox/commit/5e455699730284e71709326b30f897d103db17ea))
- Use correct initialization order ([b41767d0](https://github.com/qTox/qTox/commit/b41767d0874eaf8887f908d2a9c5571f4342d5d5), closes [#4727](https://github.com/qTox/qTox/issues/4727))
- remove unused variables ([c089c80e](https://github.com/qTox/qTox/commit/c089c80e53be33a8d42ea28176763659c1eb2005))
- **build:**
  - install libexif-dev on apt-based systems ([2066c2e0](https://github.com/qTox/qTox/commit/2066c2e002009a75708c61f2ec3c39ce588d0e56))
  - do not build unix-specific test on Windows ([d69023c9](https://github.com/qTox/qTox/commit/d69023c9d78714f5e730e98bb251c8d0f723688a))
- **call:** fix bugs introduced from ToxCall refactor ([1394dd1b](https://github.com/qTox/qTox/commit/1394dd1b7fc59da57f0bd7470f4caed22231670a))
- **cameradevice:** Save string in local variable to avoid errors ([92def839](https://github.com/qTox/qTox/commit/92def839d084a97bed80e01e5afda49c593c34bd))
- **chatForm:**
  - Fix issue with mixing friend and group id ([5bc8ef4e](https://github.com/qTox/qTox/commit/5bc8ef4e74dee407447de52328c3ccf98307c46a), closes [#4828](https://github.com/qTox/qTox/issues/4828))
  - Subscribe on accept and reject buttons once ([d77fbb4b](https://github.com/qTox/qTox/commit/d77fbb4b1999da2e20d5ed0071b254eea99dce4b), closes [#4799](https://github.com/qTox/qTox/issues/4799))
  - Hide call confirm on call end ([f4a3bb28](https://github.com/qTox/qTox/commit/f4a3bb2812fbe3cab424a190a7f8114d26351cd7), closes [#4799](https://github.com/qTox/qTox/issues/4799))
- **core:** Check that result of get peer name is successful ([78262b41](https://github.com/qTox/qTox/commit/78262b41bd8d2bec9d50ac736302eaa389369404), closes [#4802](https://github.com/qTox/qTox/issues/4802))
- **ffmpeg:** don't use deprecated features ([2f13796a](https://github.com/qTox/qTox/commit/2f13796acc5e63a57a5af1e5a4ce102da506d5d5))
- **friend:** Add save friend alias on change ([c0a7488c](https://github.com/qTox/qTox/commit/c0a7488c12923354d0ed25f405994327ce2eb31a), closes [#4706](https://github.com/qTox/qTox/issues/4706))
- **group:**
  - Send all parts of long message ([7c76bebe](https://github.com/qTox/qTox/commit/7c76bebebef9d20da38bd4b45c3fb4aed2cf5915), closes [#4832](https://github.com/qTox/qTox/issues/4832))
  - Add second signal for titile changed ([2f9ba3c4](https://github.com/qTox/qTox/commit/2f9ba3c4d793a9b50f932c82792a5f8afce5f2b8), closes [#4800](https://github.com/qTox/qTox/issues/4800))
- **identicon:** don't set identicon as an avatar ([676be5f6](https://github.com/qTox/qTox/commit/676be5f625dc485433ba9137a027cad2bef1b5f5))
- **init:** register AV connects and call after AV is ready ([7170b485](https://github.com/qTox/qTox/commit/7170b48589c88eaa33263ffb0155dbf384b82d56), closes [#4651](https://github.com/qTox/qTox/issues/4651))
- **ipc:** Add check if IPC init failed ([c274cec8](https://github.com/qTox/qTox/commit/c274cec87e957894c021bd7f27994a2f6ce5473c), closes [#4785](https://github.com/qTox/qTox/issues/4785))
- **logout:** Synchronously call showLogin to avoid multiple deletion ([5046fc90](https://github.com/qTox/qTox/commit/5046fc90103f8c33d8e5221a1e655414aad53f44), closes [#4201](https://github.com/qTox/qTox/issues/4201))
- **platform:** use result to remove -Wunused-result warning ([729dc774](https://github.com/qTox/qTox/commit/729dc7740b04becae7c509c3aafbf553fd955b31))
- **preview:** only downscale preview images, never upscale ([45b3575d](https://github.com/qTox/qTox/commit/45b3575d7902d99a3c8a31987b4ef92d4ba5d78f), closes [#4733](https://github.com/qTox/qTox/issues/4733))
- **profile:** Fix status message and username signal connection typo ([d41031bd](https://github.com/qTox/qTox/commit/d41031bd15bfa6cce1a0f55b20952a98680c3f2f), closes [#4760](https://github.com/qTox/qTox/issues/4760))
- **settings:**
  - prevent signed overflow and associated warning ([6d1b1f62](https://github.com/qTox/qTox/commit/6d1b1f62aba08b84e4dddf3436a936cddea3518f))
  - Delete dynamically allocated members on destruction ([94cb6ce1](https://github.com/qTox/qTox/commit/94cb6ce1c562d7be0bc71e165f73c8ec5cd66353), closes [#4670](https://github.com/qTox/qTox/issues/4670))
- **test:** If json string not quoted is server error ([7c744912](https://github.com/qTox/qTox/commit/7c744912268c53db3509dd7b5575c72df1e9b542))
- **toxme:** Add #include `<ctime>` to toxmedata ([2f1bf107](https://github.com/qTox/qTox/commit/2f1bf107808953f5f080de3458f88abb33ad247a), closes [#4730](https://github.com/qTox/qTox/issues/4730))
- **travis:** make Windows cross-compilation caching work ([0966e91e](https://github.com/qTox/qTox/commit/0966e91e31684cbfdc8b5b339929db1f17907707))
- **ui:** Set tooltip colour and background for all tooltips ([b1f40dec](https://github.com/qTox/qTox/commit/b1f40decd868f4df81ac8784bdcbb6a791d945ea), closes [#4667](https://github.com/qTox/qTox/issues/4667))
- **video:**
  - don't use pointers to temporary objects ([5d6ae9ae](https://github.com/qTox/qTox/commit/5d6ae9ae372a95f8e6cd1950d87ba379eb1b8dfe), closes [#4764](https://github.com/qTox/qTox/issues/4764))
  - prevent segfault when ending videocall ([fed70602](https://github.com/qTox/qTox/commit/fed7060270932f3731d78c2f2ba30f5a10300f95))
  - reduce default video bitrate to make it more usable ([ecea4104](https://github.com/qTox/qTox/commit/ecea41045c50611fb8f8b506c25c6f613073a4ce))
  - Set toxav video bitrate to 0 if answering audio-only call ([1613044c](https://github.com/qTox/qTox/commit/1613044c68cee2ceee82b61e789e5225dda45f7d))

#### Features

- **audio:** microphone voice activation ([d24d4fb8](https://github.com/qTox/qTox/commit/d24d4fb8ea44999d037e7ca839c338140c5a59c7))
- **build:** hash everything in Windows cross-compilation ([319d871b](https://github.com/qTox/qTox/commit/319d871be38d6f92386f626c7fcbd25252e680bd))
- **identicon:**
  - use Identicons instead of empty avatars ([003fc6b0](https://github.com/qTox/qTox/commit/003fc6b0b0cbef99888a746d60566a1bcc3805f9), closes [#711](https://github.com/qTox/qTox/issues/711))
  - add algorithm to create identicons ([61b36d1b](https://github.com/qTox/qTox/commit/61b36d1bce87173fd290398db1045bd90a998dc9))
- **l10n:**
  - update German translation from Weblate ([e0235dff](https://github.com/qTox/qTox/commit/e0235dffc5e3240696d5350bdbc1d2c5386e8cec)), ([af2addec](https://github.com/qTox/qTox/commit/af2addec860de6fb986250b2f7d22c8ea039ffd7)), ([13df29e5](https://github.com/qTox/qTox/commit/13df29e5d1f5e3c5365695b287778e024a9fc220))
  - update Italian translation from Weblate ([45a053cc](https://github.com/qTox/qTox/commit/45a053ccb7dc414b4f464d32039b99707632e665))
  - update Lithuanian translation from Weblate ([79f96c90](https://github.com/qTox/qTox/commit/79f96c902665e4c0d9f314a232201d2f16c137a9)), ([7e80cbca](https://github.com/qTox/qTox/commit/7e80cbcaeee1c2187d3f31d83e0e31b615d5dac9))
  - update Slovak translation from Weblate ([5bd1b971](https://github.com/qTox/qTox/commit/5bd1b97170bc42561519448692d16c9f7fe3eee9))
  - update Hungarian translation from Weblate ([d344e060](https://github.com/qTox/qTox/commit/d344e06016076265940dd1c293f946cd99df05c8))
  - update Norwegian (old code) translation from Weblate ([64a83067](https://github.com/qTox/qTox/commit/64a8306731b1992cb49c79c3de8b921cb15ee92a)), ([bcdbd2ca](https://github.com/qTox/qTox/commit/bcdbd2caef16a8f4bc11da287dba01db2ed1c640))
  - update Russian translation from Weblate ([a62535a5](https://github.com/qTox/qTox/commit/a62535a5e4acac41e95e2a186806d6f387fde9fd)), ([e9c9eeea](https://github.com/qTox/qTox/commit/e9c9eeea9f3bd60105b8eeeb15b3275102d3be7f))
  - update Belarusian translation from Weblate ([652cdab8](https://github.com/qTox/qTox/commit/652cdab8d42101f6348dc183f5ad72a58082f8a2)), ([07cf8838](https://github.com/qTox/qTox/commit/07cf8838b59059450b90a0c8cab7fb5285d02d59)), ([01b9bcd9](https://github.com/qTox/qTox/commit/01b9bcd9d214389cfe7f84fb1ac300bbb5bfb4bc)), ([753ea45b](https://github.com/qTox/qTox/commit/753ea45b6d1b0eb5744370f96eae0492af9fda79))
  - update Romanian translation from Weblate ([fd7fe766](https://github.com/qTox/qTox/commit/fd7fe76697512ffc1188e0588b17c81f934207ba)), ([2d2cdd01](https://github.com/qTox/qTox/commit/2d2cdd010a6cdd890cfea6bec2b7bd0a296a1ed6)), ([47a83e3b](https://github.com/qTox/qTox/commit/47a83e3bf9d099dc084dd8098d43f73258c47b56))
  - update Estonian translation from Weblate ([d0394e88](https://github.com/qTox/qTox/commit/d0394e88e76752996bc24a8c179e17c0043d4160)), ([235e6bfd](https://github.com/qTox/qTox/commit/235e6bfd34959d0f0d11da167e7b63339780963a)), ([33d4cc4a](https://github.com/qTox/qTox/commit/33d4cc4a1dfc4c874eab8db916232e84000c7698)), ([f82d3449](https://github.com/qTox/qTox/commit/f82d34492872148757dfddcc5663d9f7a21ffb71)), ([53475ed2](https://github.com/qTox/qTox/commit/53475ed2dc334f4babebb14780dadc08b9b3e9a5)), ([8710563b](https://github.com/qTox/qTox/commit/8710563bd638d672271c62fab9a06ef4cb200313)), ([d5e484b8](https://github.com/qTox/qTox/commit/d5e484b8d94469e866597d1794ec2d6eb0f6396f)), ([681661f6](https://github.com/qTox/qTox/commit/681661f61c55ba7b63e54b38594e9a85abb92454))
  - update Spanish translation from Weblate ([00e33153](https://github.com/qTox/qTox/commit/00e3315360036b5db031139b2c4b05e6dca10eb5)), ([154002c0](https://github.com/qTox/qTox/commit/154002c0027eb7dea7f5b74e302ef29eb5a096fe))
  - update Serbian (latin) translation from Weblate ([6f4c675a](https://github.com/qTox/qTox/commit/6f4c675a46fe471fa00d9e63ae14306d0b88fe0d)), ([f171c232](https://github.com/qTox/qTox/commit/f171c2327f19041b908ec4883d8d3426aa5f1e20))
  - update Serbian translation from Weblate ([1aec68fd](https://github.com/qTox/qTox/commit/1aec68fdf0c32a4a0a782c9ef256563818ebbf41)), ([907c3e5c](https://github.com/qTox/qTox/commit/907c3e5c8b557f3051943d17ef6aa785f01b890f))
  - update Chinese (Simplified) translation from Weblate ([e828583b](https://github.com/qTox/qTox/commit/e828583b9af80c8aacbe208f2a6a831d8427e863)), ([1514b800](https://github.com/qTox/qTox/commit/1514b80093c701318e187c8eb9ab93a4bd71822c)), ([0145bb11](https://github.com/qTox/qTox/commit/0145bb11775d11dd2dc9b7025c2b1a0f8355fcc5))
  - update Persian translation from Weblate ([b3ceda6e](https://github.com/qTox/qTox/commit/b3ceda6e47d485037ab3c1df9c2f433801d1b688))
  - update Croatian translation from Weblate ([6f0b6925](https://github.com/qTox/qTox/commit/6f0b692562cbc303ac8f420f7a9aee82821ab8e8))
  - add Serbian (latin) translation to UI ([641a8084](https://github.com/qTox/qTox/commit/641a80842e0d3fabb6b4872d2a342d167ff80958))
  - add Serbian (latin) translation using Weblate ([66a680df](https://github.com/qTox/qTox/commit/66a680df8720880fda7254cedccb8ddd45f6ceed))
  - add Serbian translation to UI ([3c4b1f11](https://github.com/qTox/qTox/commit/3c4b1f117d29293d6ebbb68457eb96452a9b557b))
  - add Serbian translation using Weblate ([17c00487](https://github.com/qTox/qTox/commit/17c00487921d12c2f54ed25f08d943e6914231e1))
  - add Persian translation to UI ([76df9a45](https://github.com/qTox/qTox/commit/76df9a453511e98693c7302b3f48598caff45d9e))
  - add Persian translation using Weblate ([f2d3beb6](https://github.com/qTox/qTox/commit/f2d3beb67e734fc190fd331a1539a043590ffeee))
  - update Swedish translation from Weblate ([27a58b93](https://github.com/qTox/qTox/commit/27a58b938d381f6d9a39a78280f4bb5958b34193))
  - update Portuguese translation from Weblate ([9b7e23c6](https://github.com/qTox/qTox/commit/9b7e23c68be190487cf131018e4a5a071aa23152)), ([227bba8f](https://github.com/qTox/qTox/commit/227bba8fa2624714d8a73e6cd4dc50aab5eaafea)), ([1425dff6](https://github.com/qTox/qTox/commit/1425dff6e7a711b5c053842620b8fdeaaed653ce)), ([8c07a2b2](https://github.com/qTox/qTox/commit/8c07a2b22503be0ededabb261a553db132a38109)), ([ccc5296f](https://github.com/qTox/qTox/commit/ccc5296fd30fe531de552abf76da3062c9e31c9f))
- **settings:**
  - add an option to toggle identicons ([905ca770](https://github.com/qTox/qTox/commit/905ca7708662c6fe5520ad2707715075ad1aa41d))
  - make audio quality setting persistent ([7ed2d97a](https://github.com/qTox/qTox/commit/7ed2d97aadab8aa0acb39e54aafaea06c0ebe57c))
  - Add audio quality setting ([61eddc1f](https://github.com/qTox/qTox/commit/61eddc1f6b76f2da1a6d180426a2b7f53c757e2a), closes [#4693](https://github.com/qTox/qTox/issues/4693))
- **travis:**
  - run tests on Windows ([21af6875](https://github.com/qTox/qTox/commit/21af6875cc7a1f5a311b8149ced9970570a4b2e5))
  - Windows cross-compilation ([9358297a](https://github.com/qTox/qTox/commit/9358297af87ebb13dc4f74d2ced6033e38b25a5b))

#### Performance

- **smileys:**
  - Cleanup smileys icons by timer ([fa215949](https://github.com/qTox/qTox/commit/fa21594902beef73e70f69e80ae7acb016aa7ec6))
  - Use `shared_ptr` to automaticaly count references ([c6400077](https://github.com/qTox/qTox/commit/c64000777510e8b6c899475bf49fba31af9fda8b))
  - Use lazy smileys loading ([d83400bc](https://github.com/qTox/qTox/commit/d83400bcdc1bf177aabb6940ee3686f8e4f0e942))

<a name="v1.12.0"></a>

## v1.12.0 (2017-10-01)

### Release notes

This release provides many handy new features, most notable are mass import of
contacts from a file, improvements around images in the chatlog and an
experimental audio backend with echo cancelling.

#### Features

- UI to import a list of contacts from a file ([0974da54](https://github.com/qTox/qTox/commit/0974da543a6b76851ffa86d24883c324651a8ff9), closes [#4181](https://github.com/qTox/qTox/issues/4181))
- **audio:**
  - make the libfilteraudio dependency optional ([8652fe99](https://github.com/qTox/qTox/commit/8652fe99e3cb80fac22a601c89ba405962c5537d))
  - add setting to switch between new and old audio backend ([9d0498e0](https://github.com/qTox/qTox/commit/9d0498e0cfdf654f4319675f019669dacde4dbcc))
  - make echo cancellation work and improve some minor stuff ([5d60f09d](https://github.com/qTox/qTox/commit/5d60f09df4b59c3a42c7544fd92c78710dd9d5b0))
  - add libfilteraudio ([9c603e86](https://github.com/qTox/qTox/commit/9c603e86546da199cd94f22f4e4f9349d526fad1))
  - add alternative OpenAL backend ([c7157291](https://github.com/qTox/qTox/commit/c7157291679ccb0414789dd04acd43455ce6aafc))
- **build:** Check sha256 of tarballs for Windows cross-compilation ([7f6f8a45](https://github.com/qTox/qTox/commit/7f6f8a4513bfa07356f7ac6a88da346d8c325e7a))
- **exif:** Honour exif orientation tag ([414fa178](https://github.com/qTox/qTox/commit/414fa178b4908d8451ca5304ee9ca1ab7421b26a), closes [#1848](https://github.com/qTox/qTox/issues/1848))
- **l10n:**
  - update Norwegian (old code) translation from Weblate ([868fd55d](https://github.com/qTox/qTox/commit/868fd55d8e30c9b17e1ae60a42c0640b4f3d6cad))
  - update Estonian translation from Weblate ([035c158a](https://github.com/qTox/qTox/commit/035c158a9913389bd56faea4dcd5bd6b8f4a9de0))
  - update German translation from Weblate ([f0ca3bed](https://github.com/qTox/qTox/commit/f0ca3bed382cf4cc51118989cc39a87a12d37be2)), ([55425705](https://github.com/qTox/qTox/commit/5542570505bc4d7b3ca5e4ff33bbccf3b5e79f9a))
  - update Slovak translation from Weblate ([155812e4](https://github.com/qTox/qTox/commit/155812e444a4614c35e9c8ddeab619200c1d4643))
  - update Hungarian translation from Weblate ([3b1a8ac7](https://github.com/qTox/qTox/commit/3b1a8ac704dabf14a60201a3fd114bf58db13cb7))
  - update Russian translation from Weblate ([a6692e28](https://github.com/qTox/qTox/commit/a6692e28ee89fe56094d719d7235d281dfe07c6c))
  - update Finnish translation from Weblate ([814a6ea0](https://github.com/qTox/qTox/commit/814a6ea0cf83c68b959cdf53b73e7e18fcf18231))
  - update French translation from Weblate ([8a94ad75](https://github.com/qTox/qTox/commit/8a94ad75efffe07da97725d9a1cbba9f87887b18)), ([a225eec5](https://github.com/qTox/qTox/commit/a225eec5fd36c9f154b90a9aaa148139d1961521)), ([4abaf031](https://github.com/qTox/qTox/commit/4abaf03106d8d51b307d56d62f0ad7d1bfde8a4a)), ([d662866c](https://github.com/qTox/qTox/commit/d662866c413fccd0dc18fb1bdfba01e13023d1b5)), ([dbfc4794](https://github.com/qTox/qTox/commit/dbfc4794e53ed8523ba139031d03a512cd9a2dbb)), ([8d3a6310](https://github.com/qTox/qTox/commit/8d3a631084f4281b07b910da63ebbf1a27275748)), ([53264f4c](https://github.com/qTox/qTox/commit/53264f4cf90357893f297d8a79580c9f7f697465)), ([f2b648e4](https://github.com/qTox/qTox/commit/f2b648e4d3be5eba65c6444a544956133e9242db)), ([5aa47be7](https://github.com/qTox/qTox/commit/5aa47be72a68d62b6bd8c3a6442bc43ad82b8800)), ([6917def2](https://github.com/qTox/qTox/commit/6917def238287a51458012983d27d70f0003d697)), ([069feae3](https://github.com/qTox/qTox/commit/069feae311fef515eee948e90f4ed36a27160c87)), ([db4602ea](https://github.com/qTox/qTox/commit/db4602ead1cd7cbd031423429aca7747b96d5d03))
  - update Tamil translation from Weblate ([13c572c3](https://github.com/qTox/qTox/commit/13c572c393619f0da9777935b1c7bd8a5bcd437d)), ([092cae62](https://github.com/qTox/qTox/commit/092cae62d05eada032f207ac3d13d55201905eee))
  - update Swedish translation from Weblate ([57519ca7](https://github.com/qTox/qTox/commit/57519ca75de1f1426a3fe20ee78330733ce328aa))
  - update Polish translation from Weblate ([916f7aa2](https://github.com/qTox/qTox/commit/916f7aa250281c6beae8135df05aa627d18e6527))
  - update Lithuanian translation from Weblate ([c06abad8](https://github.com/qTox/qTox/commit/c06abad83ba2fc127502d272a4ee0c4e9c16a566))
  - update Bulgarian translation from Weblate ([0e688da1](https://github.com/qTox/qTox/commit/0e688da1b0f6228478a5e18c5d2d63a369afcb43))
  - update Romanian translation from Weblate ([227516d2](https://github.com/qTox/qTox/commit/227516d206a8fea9f408861f5c63aa8c2a530f54)), ([e441db87](https://github.com/qTox/qTox/commit/e441db87d3b0f8c54f59848796bb76f56d47702b))
  - update Chinese (Simplified) translation from Weblate ([78a0d339](https://github.com/qTox/qTox/commit/78a0d33942dcda153fac2fa30321227127cf60e1))
  - update Belarusian translation from Weblate ([d5c6d102](https://github.com/qTox/qTox/commit/d5c6d1027c4e9e892049cbd4fb4dff6829a99318))
- **login:** Add command line argument to open login screen ([0906b8eb](https://github.com/qTox/qTox/commit/0906b8eb1f9a413e80320d002d15540736a6fabb), closes [#4673](https://github.com/qTox/qTox/issues/4673))
- **paste:** Implement pasting images from clipboard ([5fed3736](https://github.com/qTox/qTox/commit/5fed37365f1d32f900814527bc8eb514986f2abd), closes [#1290](https://github.com/qTox/qTox/issues/1290))
- **settings:** add group chat local member black list ([27ecace7](https://github.com/qTox/qTox/commit/27ecace752424b67924011e5838fd1f11857b3cf))
- **ui:** output instructions when user tries to open a second instance ([48d65c26](https://github.com/qTox/qTox/commit/48d65c269a3dd3badd498b816f9d1cbfaa33bef4), closes [#3483](https://github.com/qTox/qTox/issues/3483))
- **video:** Error message on call fail ([ac75f7b5](https://github.com/qTox/qTox/commit/ac75f7b5944dc6dfc0b7334b22f53a159d8e7bc7))

#### Bug Fixes

- `ATOMIC_FLAG_INIT` can't be used inside braces initializer ([f790747f](https://github.com/qTox/qTox/commit/f790747f2adda60f8809bb0e1671f18c3f7c6168))
- Return value ([f97c776a](https://github.com/qTox/qTox/commit/f97c776a8722c5f230cb709270559fdfcd77ba47))
- Fix crash on auto accept click in AboutFriendForm ([bd6516ea](https://github.com/qTox/qTox/commit/bd6516eab707c3e4b208b937e91a7c0eef39fafa), closes [#4600](https://github.com/qTox/qTox/issues/4600))
- Fix crash on accept or reject friend request ([359a42f7](https://github.com/qTox/qTox/commit/359a42f78e8edf073d6132bca8954a55094385c5), closes [#4603](https://github.com/qTox/qTox/issues/4603))
- Keep open and share X11 connection ([ae5cb4bc](https://github.com/qTox/qTox/commit/ae5cb4bcc27754f46d30209bf1658e49ff559dc7))
- **IPC:**
  - Reorder initialize list ([43c2308b](https://github.com/qTox/qTox/commit/43c2308b7bd845aa1db572642858f9dde762a28a))
  - don't double lock shared memory ([0bf27a00](https://github.com/qTox/qTox/commit/0bf27a000aa42559a5b04b7e03781a29885f6adf), closes [#4678](https://github.com/qTox/qTox/issues/4678))
  - Call processEvent on timer timeout ([c2140e21](https://github.com/qTox/qTox/commit/c2140e21ad9a7edc893191f5e49307fc05933959), closes [#4609](https://github.com/qTox/qTox/issues/4609))
- **audio:**
  - Fix signed and unsigned comparation ([aa356bb7](https://github.com/qTox/qTox/commit/aa356bb73a7443720a0d73be1f5401940f014498))
  - Echo cancelling supports only mono audio ([809c5e6b](https://github.com/qTox/qTox/commit/809c5e6b04c991960afcc66a308bad63ca473610))
- **autoaway:** Improve autoaway algorithm. ([9fe503c7](https://github.com/qTox/qTox/commit/9fe503c7083b1e9f1600af0597455eb8237f8c6e), closes [#3992](https://github.com/qTox/qTox/issues/3992))
- **avatar:** Don't invert default avatar colour on connect ([be324b93](https://github.com/qTox/qTox/commit/be324b932b3e572b59f408faacd690110360dcbb), closes [#4629](https://github.com/qTox/qTox/issues/4629))
- **build:**
  - Fix regression in how cmake finds libraries ([b7ef73d4](https://github.com/qTox/qTox/commit/b7ef73d401aabbd0f5ee6ccfcd1a9f0a962bdaab))
  - Update toxcore and Qt versions for Windows cross-compilation ([80f1286e](https://github.com/qTox/qTox/commit/80f1286e7ca5025d3e9f93b4209f791e043ad367))
  - Add libexif to the Windows cross-compilation ([95548c6a](https://github.com/qTox/qTox/commit/95548c6a5fcce7a31ffcaafcaec8650fd6f4c968))
  - Fixed header file of libfilteraudio not being found ([8108bfdc](https://github.com/qTox/qTox/commit/8108bfdc358b5070d1dfbf57a02e25439ce09541))
  - Windows cross-compilation didn't see libfilterudio ([b0b157e7](https://github.com/qTox/qTox/commit/b0b157e77bc87eae1f8235e7d15d76e82c992ce6))
  - add install steps for libfilteraudio on OSX ([8f2401fe](https://github.com/qTox/qTox/commit/8f2401fee64d0b5535bd4d737a1caf2bffa30682))
  - make the OpenAL backend build with OpenALSoft &lt; 1.15 ([526701fa](https://github.com/qTox/qTox/commit/526701fa9f7a2a5c950e7b16b6578bf32bff4604))
  - remove newline from timestamp ([0843b770](https://github.com/qTox/qTox/commit/0843b770c64204a078e0d73fe5f3b71b8ef17e52))
  - fix sqlcipher header inclusion ([e7b5c93b](https://github.com/qTox/qTox/commit/e7b5c93bfe3d3fbcfe8659ab93c4d03492ddefb9))
- **chatform:** Hide author on history like on new messages ([28979f57](https://github.com/qTox/qTox/commit/28979f577165b0ce2a8e67b8eea870218bdd0b21), closes [#4619](https://github.com/qTox/qTox/issues/4619))
- **dependency:** remove unused qt sql dependency ([5d159e02](https://github.com/qTox/qTox/commit/5d159e0203bdd0e72153cce9ca5c0fe8483ba0db), closes [#4567](https://github.com/qTox/qTox/issues/4567))
- **desktop:** Improved the comment in qtox.desktop ([38ef3ad6](https://github.com/qTox/qTox/commit/38ef3ad69de0dfa3b3126f6c8fbfcb6392aa46f2))
- **exit:** delete heap-based QApplication before exit ([a601df31](https://github.com/qTox/qTox/commit/a601df31da0859255e1220d24f3c97eae9eb0cca), closes [#4648](https://github.com/qTox/qTox/issues/4648))
- **friends:** Friend request button target location ([7f36ec92](https://github.com/qTox/qTox/commit/7f36ec9219e93fbbe2b790efc68215f5ec8bfb4b), closes [#4631](https://github.com/qTox/qTox/issues/4631))
- **friendwidget:** Add correct index calculation ([ad58c176](https://github.com/qTox/qTox/commit/ad58c1767b2ab4ff8340076d5ede53aa6e889972))
- **group:** Fix crash on group quit ([e606d3cb](https://github.com/qTox/qTox/commit/e606d3cb5573d956aa4df58fbe39a21b8c423860))
- **i18n:** string `Call with %1 ended. %2` is no longer translatable ([56ac95da](https://github.com/qTox/qTox/commit/56ac95dad9a807b4799840d7b2c3b1c1c44869f2), closes [#4552](https://github.com/qTox/qTox/issues/4552))
- **l10n:** use native language names ([817a9897](https://github.com/qTox/qTox/commit/817a9897c2e1289c7ceda19f0335582ec46ad343))
- **platform:** handle terminating POSIX signals ([32b97cb9](https://github.com/qTox/qTox/commit/32b97cb9278e40c92337468fb61751cdf87a0542), closes [#4470](https://github.com/qTox/qTox/issues/4470))
- **popup:** Don't create two error popups for friend requests ([1f787cc4](https://github.com/qTox/qTox/commit/1f787cc475c804ef890dec7e24c51ba6c2a3f9c4), closes [#4633](https://github.com/qTox/qTox/issues/4633))
- **receipts:** Prevent double message send for received receipt ([e9d63397](https://github.com/qTox/qTox/commit/e9d63397e1008e57a23c763aaa418fc65f57577b), closes [#2726](https://github.com/qTox/qTox/issues/2726))
- **threads:** Always stop the camera device thread ([8522141b](https://github.com/qTox/qTox/commit/8522141b1e7db63dade973f1ba9048aecc7db41e))
- **tooltip:** Set font colour to black for beige tooltip background ([fc1ed101](https://github.com/qTox/qTox/commit/fc1ed10199a4f34faf38081bf9b1a58ee5bdf033), closes [#4641](https://github.com/qTox/qTox/issues/4641))
- **toxme:** Remove HTML tags from ID to un-break toxme integration ([5af1b4af](https://github.com/qTox/qTox/commit/5af1b4af73d05e8da4bbc174410f0854a810ecd6))
- **widget:** Add removing friends and group on main window close ([638cdba5](https://github.com/qTox/qTox/commit/638cdba58d4ed506edb6eb5fadbeae8fa0c22995))
- **windows:**
  - remove unneeded qt sql and fix problem when build dir exists ([68c5cdf0](https://github.com/qTox/qTox/commit/68c5cdf096fbdf5d18bad06b58241098f6873505))
  - workaround docker problem when extracting with tar ([af15cd06](https://github.com/qTox/qTox/commit/af15cd06acf549df753abf9f8521e24de1586a07))

<a name="v1.11.0"></a>

## v1.11.0 (2017-07-16)

### Release notes

The most interesting new features that were added in this release are export of
history to plaintext file and outgoing call sound.

#### Bug Fixes

- Invoke device methods in deviceThread ([d86912ea](https://github.com/qTox/qTox/commit/d86912eacd10f9c6472e1ef920a258f0e1ef3b83), closes [#2058](https://github.com/qTox/qTox/issues/2058))
- Fix warning about stack protection ([efcad35f](https://github.com/qTox/qTox/commit/efcad35fd53279b32b7b169af256c0ac14d6e9d1))
- Using foreach to iterate through `CFLAGS_OTHER` ([9a0632b0](https://github.com/qTox/qTox/commit/9a0632b0cf8bc78d1aae0f2e75b26b73cdb98e52), closes [#4175](https://github.com/qTox/qTox/issues/4175), closes [#4457](https://github.com/qTox/qTox/issues/4457))
- Replace hardcored roaming path on QStandartPath ([f616ff36](https://github.com/qTox/qTox/commit/f616ff36b6db9142f6e49b2b8bc436b6652aa329), closes [#4436](https://github.com/qTox/qTox/issues/4436))
- Fix freez on circle removing ([4d15aed5](https://github.com/qTox/qTox/commit/4d15aed53fa8986747b450b42d23befc92600986), closes [#4419](https://github.com/qTox/qTox/issues/4419))
- gcc-7 build ([246e23ab](https://github.com/qTox/qTox/commit/246e23abf3aa64c494c6abf42c83944eb78f1b16))
- **IPC:** Add update profileId in to IPC ([c2f82f78](https://github.com/qTox/qTox/commit/c2f82f7808ed075c9c31813151d2cf001c4c7d10), closes [#4384](https://github.com/qTox/qTox/issues/4384))
- **bootstrap:** qrencode url ([f90da3d0](https://github.com/qTox/qTox/commit/f90da3d07acdf25e7e871ee8447412bd315c5f02))
- **build:**
  - disable linux specific stuff on windows ([a9d2b03c](https://github.com/qTox/qTox/commit/a9d2b03c7c8ea33e34c238e1079c66687a6e1ab0))
  - when building without XSS extension ([ab622213](https://github.com/qTox/qTox/commit/ab62221375281ca1320bc7b437cbf33766827f94))
  - add missing rcc options ([6b7c0c73](https://github.com/qTox/qTox/commit/6b7c0c738b8439df6d2966de4a1296d32805366f))
- **chatform:** add space for current copy link ([41a781d3](https://github.com/qTox/qTox/commit/41a781d3f875a0af51aaa4c727ce94c6115c48ad))
- **cmake:** fix icon for .exe file ([cf06cc7a](https://github.com/qTox/qTox/commit/cf06cc7a1d8c94965f6ed76ce3d499ce531d946e))
- **core:**
  - use correct byte representation when bootstrapping ([4e5b1915](https://github.com/qTox/qTox/commit/4e5b191553d2dcd14e744aa4c9e81d8f4e5f66a7), closes [#4385](https://github.com/qTox/qTox/issues/4385))
  - Add action message with friend request text ([b25f5b5e](https://github.com/qTox/qTox/commit/b25f5b5ed69681a0549a7ff3377704536b34c19d))
- **emojis:** Add text variants for the blush emoji ([9a367ffb](https://github.com/qTox/qTox/commit/9a367ffbf010ad2698e32d2f1c24add9d6dbefb9))
- **macro:** Fixed clang warning about `USING_V4L` ([dd59f5ee](https://github.com/qTox/qTox/commit/dd59f5ee90b6b05bd810596cfa3ad55c9b449776))
- **main:** Use correct way to handle application quit ([0b5b3fcf](https://github.com/qTox/qTox/commit/0b5b3fcf1a91a8f163574a2650102fedead6431b))
- **message size:** Replaced `TOX_MAX_*_LENGTH` with API calls. ([3963d3c1](https://github.com/qTox/qTox/commit/3963d3c150fa4afe2fe69f493da6452c783a5655))
- **profile:** set maxLength on userName QLineEdit ([71a838b5](https://github.com/qTox/qTox/commit/71a838b53e75a239174a0de09988415f64325e36), closes [#4335](https://github.com/qTox/qTox/issues/4335))
- **qtox.pro:** add openal.h and openal.cpp ([d78a9790](https://github.com/qTox/qTox/commit/d78a979021578d6837f4ccb17a352b1ebfaeb8ff), closes [#4380](https://github.com/qTox/qTox/issues/4380))
- **settings:**
  - set default values for some important settings ([da4f6222](https://github.com/qTox/qTox/commit/da4f6222d806ff74f799a7e380eca20ab9ff3342), closes [#4513](https://github.com/qTox/qTox/issues/4513))
  - compute toxcore version in runtime ([2d0a4e79](https://github.com/qTox/qTox/commit/2d0a4e794dd10818dc830c8384fc0f76f0184b7f))
- **ui:**
  - Use native file picker dialog ([42a9534b](https://github.com/qTox/qTox/commit/42a9534b24fe789b23e2ab07e3c6963b503ea91a), closes [#3494](https://github.com/qTox/qTox/issues/3494))
  - wrong size of svg images on buttons ([5b0bf9fc](https://github.com/qTox/qTox/commit/5b0bf9fc9b31cedac5c8dccc51c96852a9a1e1ce))
  - add date message before info messages ([ed453598](https://github.com/qTox/qTox/commit/ed45359863cf617d94e8f39506971a515c495304), closes [#4388](https://github.com/qTox/qTox/issues/4388))
  - do not save splitter state in multiple windows mode ([7e5387ca](https://github.com/qTox/qTox/commit/7e5387cad283877c422b2e6d4a8bd9404fff67cd))
  - Set `Qt::Window` flag in multi windows mode ([6b767e47](https://github.com/qTox/qTox/commit/6b767e476551f9f7bb771803a3f356f3e45a8d9e))
  - switch to settings after multiple windows mode ([0c98f6a5](https://github.com/qTox/qTox/commit/0c98f6a5488cb8d8b27471f3cfecd6dd7f582300))
  - restore splitter state ([9d94cd0c](https://github.com/qTox/qTox/commit/9d94cd0c507afdbf4b69c8adcdd82546a6bc83b8), closes [#4387](https://github.com/qTox/qTox/issues/4387))
- **v4l:** Fixed CMakeLists.txt for v4l on FreeBSD ([dfe696e2](https://github.com/qTox/qTox/commit/dfe696e25a26a163430aefff3ba7a75428050598), closes [#4459](https://github.com/qTox/qTox/issues/4459))

#### Features

- Add message count in tooltip to LoadHistoryDialog ([bb65a18d](https://github.com/qTox/qTox/commit/bb65a18de59677cbbe3f1568380c9149b4939466))
- **aboutform:** Add OS name in issue template ([7b3bd45d](https://github.com/qTox/qTox/commit/7b3bd45d5089aacf3b72b4206dbff922cdef6dbb))
- **audio:**
  - OutgoingCallSound ([a06ad704](https://github.com/qTox/qTox/commit/a06ad7048e7f2a450da455e16b5d907566c3eedb))
  - split the audio interface from the backend library ([28c2298a](https://github.com/qTox/qTox/commit/28c2298ad97e8aec6097d63ead55214bac8152ba))
- **chatform:**
  - Highlight chat history ([3257770b](https://github.com/qTox/qTox/commit/3257770bfa61838ab398a31de28f623f7b28d77a), closes [#2296](https://github.com/qTox/qTox/issues/2296))
  - Export chat history to file ([5e4ab769](https://github.com/qTox/qTox/commit/5e4ab76944ea5952b7123ca64102de4f50b4f4d3), closes [#4143](https://github.com/qTox/qTox/issues/4143))
- **l10n:**
  - update Arabic translation from Weblate ([3ab423da](https://github.com/qTox/qTox/commit/3ab423dad25b185d8d8fefd5dae14c4acee05c62))
  - update Bulgarian translation from Weblate ([79e00832](https://github.com/qTox/qTox/commit/79e00832f6c8ee1e700449a02ab8781ba27e4c06))
  - update Polish translation from Weblate ([5e432c5e](https://github.com/qTox/qTox/commit/5e432c5ecfb23d9fb78a76c1291f2d7f08c727e1))
  - update Slovak translation from Weblate ([e8e1d41f](https://github.com/qTox/qTox/commit/e8e1d41ff56d45ae2e87961b19fe637aa561534f))
  - add Tamil translation to UI ([bf405e17](https://github.com/qTox/qTox/commit/bf405e17fb63036c23621b8abc96692fe813eb6b))
  - add Tamil translation using Weblate ([37a93042](https://github.com/qTox/qTox/commit/37a93042601965e59c47adaad60598e3046808c3))
  - update Swedish translation from Weblate ([040f14b3](https://github.com/qTox/qTox/commit/040f14b38ee3b531b9e34ae0f50be1b93d3a247e))
  - update Turkish translation from Weblate ([fd8decd5](https://github.com/qTox/qTox/commit/fd8decd52e10c8b128a757e2034ebd1aa1a37f78))
  - update Chinese (Simplified) translation from Weblate ([06519872](https://github.com/qTox/qTox/commit/065198724cc9cb6d9718b4f3ec2dfbd6f2a1eca0))
  - update Estonian translation from Weblate ([1d9e646b](https://github.com/qTox/qTox/commit/1d9e646b199fa615ebafdebdd2ba5b6014495bd5))
  - update Ukrainian translation from Weblate ([bcc3c0ef](https://github.com/qTox/qTox/commit/bcc3c0ef92ef27c8dcd7284a6cfbf98cf6801c8f))
  - update German translation from Weblate ([32cd745a](https://github.com/qTox/qTox/commit/32cd745af62be64ab385a9aeb69d873431a5e14a)), ([1196731f](https://github.com/qTox/qTox/commit/1196731fa1ef3ff79ad43faa5fe33ecbf5b6548f)), ([d8cd871e](https://github.com/qTox/qTox/commit/d8cd871e7e8e57b8aab1d32d5502f7cdb5cc97ee)), ([1ce73175](https://github.com/qTox/qTox/commit/1ce73175b3afa55cdd2d773bf18c29515bc0a6d0)), ([20d31c81](https://github.com/qTox/qTox/commit/20d31c81c9b9bf2f91f0e6a4cd0ec46fff6fea85))
  - update Finnish translation from Weblate ([e10c5247](https://github.com/qTox/qTox/commit/e10c5247deadb85aed06c673e9d671918192e7d4))
  - update Esperanto translation from Weblate ([dfdf11d1](https://github.com/qTox/qTox/commit/dfdf11d17d4a7b3957097fb14dd2a7469c28b5fb))
  - update Russian translation from Weblate ([d5831060](https://github.com/qTox/qTox/commit/d583106008bceb1e42624bd0dba007de7966d6f9)), ([317f250e](https://github.com/qTox/qTox/commit/317f250e68524f68c5f15a68bb722e0fe88fc76d)), ([3a252c9e](https://github.com/qTox/qTox/commit/3a252c9e96568a537baa24997a017efa8d4424d6))
  - update Greek translation from Weblate ([b2501ed1](https://github.com/qTox/qTox/commit/b2501ed114fdad88dbe64d2f2c346eab053584ea))
  - update Spanish translation from Weblate ([0cd34485](https://github.com/qTox/qTox/commit/0cd3448508d0839919c07a677f5b643d111607c3))
  - update Lithuanian translation from Weblate ([57f94139](https://github.com/qTox/qTox/commit/57f941398aac060c0fbb90ede1e80847905ff1c4))
  - update Belarusian translation from Weblate ([0c95284e](https://github.com/qTox/qTox/commit/0c95284ecc5b05586915d1573184baa5a0b9872c))
  - update Romanian translation from Weblate ([38374604](https://github.com/qTox/qTox/commit/383746043fa4c54a80769b2d57980e8785c5dda8))
- **toxid:** Show NoSpam and checksum parts of ToxID in colors. ([c0951a56](https://github.com/qTox/qTox/commit/c0951a5675bf6c8f6d2c4b05d18f48b88adae6a5))

#### Performance

- Optimize open/close device ([d704f5d2](https://github.com/qTox/qTox/commit/d704f5d21d7b33fbb52f89d9d2a5386015e6a3ce))
- Pass `std::function` by reference ([365d703e](https://github.com/qTox/qTox/commit/365d703e8ab57fff4930062e2ecf65420dccebec))

<a name="v1.10.0"></a>

## v1.10.0 (2017-05-07)

### Release notes

The more prominent new features are an option to autojoin groupchats and
ability to select messages with triple click. More about new stuff below.

#### Bug Fixes

- fix segfault while trying to send messages ([b5f4628c](https://github.com/qTox/qTox/commit/b5f4628c8984d063cac385e5f2ae57e7371e58bd))
- fix OpenAL name ([8a77723a](https://github.com/qTox/qTox/commit/8a77723a893e0211fd1f02e099408e8a92aa8f1c))
- fixed segfault after accepting group invite ([532e05cb](https://github.com/qTox/qTox/commit/532e05cb0f135d763890f4146e14da036cc3b248), closes [#4327](https://github.com/qTox/qTox/issues/4327))
- set CSS font-weight of editbox always to normal ([96b2977a](https://github.com/qTox/qTox/commit/96b2977a5f4ddad2be1d7d22b4628ff960f174aa))
- remove useless line that caused a warning ([e9515f48](https://github.com/qTox/qTox/commit/e9515f4898aedfd9c70094d61a6f3b85debb9e3e))
- fixed wrong formatting for multiple URL's in one message ([08208e9a](https://github.com/qTox/qTox/commit/08208e9aa515270a190ff85a1ba704c8a130c5cd), closes [#4275](https://github.com/qTox/qTox/issues/4275))
- silence various warnings ([f0cec44a](https://github.com/qTox/qTox/commit/f0cec44a8cb91a9867b5761fb3703a90691b892b))
- Stop using deprecated `avcodec_decode_video2` ([f07daaf1](https://github.com/qTox/qTox/commit/f07daaf1595e320a19408321fcb62fe76d48d118))
- Remove unnecessary calls to `av_frame_unref` ([91414b4f](https://github.com/qTox/qTox/commit/91414b4fcef944765507112583787b887dd62966))
- Stop using deprecated `AVStream::codec` ([9674bff0](https://github.com/qTox/qTox/commit/9674bff0cfe05fcd5b667afca6df5b214644939d))
- silence warning by not using dynamic arrays on stack ([300ac12e](https://github.com/qTox/qTox/commit/300ac12e3f220265fdd4685efb8e75e9da0ef3cd))
- **README:** Fix mardown to correct display on github ([84e73f30](https://github.com/qTox/qTox/commit/84e73f300ae9f44370960528bc5a70aee7edf047))
- **UI:**
  - fixed typos of pseudo-states in stylesheets ([f35103e5](https://github.com/qTox/qTox/commit/f35103e547b517106cb5048b43dfe6dece5fe8d8))
  - rename `qTox.desktop` → `qtox.desktop` to fix displaying icon ([6de129fe](https://github.com/qTox/qTox/commit/6de129fef1122c20d356cfb5fe504cb2f0999128), closes [#4323](https://github.com/qTox/qTox/issues/4323))
  - improve unclear message about text being resized in chatform ([c8ed535b](https://github.com/qTox/qTox/commit/c8ed535bbad3300c02871cbc718bee03accc6c26))
- **chatform:** disable Tab in add friend message text area ([195e891d](https://github.com/qTox/qTox/commit/195e891da156b87c4973b13e5decfb95445d885f))
- **core:**
  - add missing nullptr check ([407413c6](https://github.com/qTox/qTox/commit/407413c65b1d7a43a1788e00935c02fa2d9ffe33), closes [#4324](https://github.com/qTox/qTox/issues/4324))
  - don't continue when the tox file is corrupted ([20db8b74](https://github.com/qTox/qTox/commit/20db8b740bcccd0889b4d19210f98c1f902b87bd))
- **groupinvite:** prevent multiple groupinvites to the same group from showing up ([13029e30](https://github.com/qTox/qTox/commit/13029e3047cf021ff98ea816008e0f514b79fc0c), closes [#2305](https://github.com/qTox/qTox/issues/2305))
- **includes:** Added missing #include `<ctime>` ([e597d391](https://github.com/qTox/qTox/commit/e597d391fcd9b1630ae2b049b361549d4a34f9fc))
- **profile:** toxsave wasn't correctly encrypted ([5b31cf6d](https://github.com/qTox/qTox/commit/5b31cf6d9d188d064b7151cdb21959b064c0cef6), closes [#4344](https://github.com/qTox/qTox/issues/4344))
- **settings:**
  - remove legacy code ([6a6e30d6](https://github.com/qTox/qTox/commit/6a6e30d6393f29b0e15b61d31267bcd60a9c8426))
  - changing language sets title to "Add Friend" ([4886868e](https://github.com/qTox/qTox/commit/4886868eceec38bdb13031f36fb916828a399f0a))
- **widget:** Add friendWidget deletation ([c7202c8b](https://github.com/qTox/qTox/commit/c7202c8b243453d9630a51d7de1f745b4b865abb), closes [#4229](https://github.com/qTox/qTox/issues/4229), closes [#4254](https://github.com/qTox/qTox/issues/4254))

#### Features

- stop using plaintext passwords in the code ([084f3b06](https://github.com/qTox/qTox/commit/084f3b06262b2ca8272238b2ec450d11840d7551))
- **audio:** don't use dynamic memory allocation and remove resource leak ([b68ad4ce](https://github.com/qTox/qTox/commit/b68ad4ced2ce985c4e53b1469681ed3ec807d2b6))
- **chatform:** mark message with triple click ([3acbc148](https://github.com/qTox/qTox/commit/3acbc148f51afc96bf5bd96e88da053bdd64f702))
- **groups:** add option to automatically accept groupchat invites ([6a16a2bd](https://github.com/qTox/qTox/commit/6a16a2bdbc94f9eb20386794652f8a1766498a00), closes [#1197](https://github.com/qTox/qTox/issues/1197))
- **l10n:**
  - add Romanian translation to UI ([0192e927](https://github.com/qTox/qTox/commit/0192e9275f1ceb1cc4caa0b810f87da92d56b75a))
  - add Romanian translation using Weblate ([8b22e962](https://github.com/qTox/qTox/commit/8b22e962edc1f3779525f678a2b1f1f99415c7f5))
  - update German translation from Weblate ([1dfb4a63](https://github.com/qTox/qTox/commit/1dfb4a630b865f7f0e17a1d948eb7e71eee98ddc)), ([87d66c01](https://github.com/qTox/qTox/commit/87d66c019c5607fdcc8d68167202967aa8f6f753))
  - update Lithuanian translation from Weblate ([c970e065](https://github.com/qTox/qTox/commit/c970e065f681d8447c80c442df0f405f42ef1b97))
  - update Finnish translation from Weblate ([2f8e8b84](https://github.com/qTox/qTox/commit/2f8e8b840b426c27539e1b2e40caa890343ad481))
  - update Esperanto translation from Weblate ([2c158eb5](https://github.com/qTox/qTox/commit/2c158eb57c0ed5dd50420589b8f706f99ea9b5af))
  - update Chinese (Simplified) translation from Weblate ([9d6be415](https://github.com/qTox/qTox/commit/9d6be4151073762f323b497a9379cb67e5747898))
  - update Swedish translation from Weblate ([3145949c](https://github.com/qTox/qTox/commit/3145949cc27ed4445c6435726eb4e0f9b1eb655f))
  - update French translation from Weblate ([38bb82e5](https://github.com/qTox/qTox/commit/38bb82e59bdde27c62b8d836dfae47283c3cfd22)), ([0748b6c6](https://github.com/qTox/qTox/commit/0748b6c6c6abf16a1069c880554a1438ca268ea7))
  - update Greek translation from Weblate ([6e6d32a7](https://github.com/qTox/qTox/commit/6e6d32a7405a28fb76f28a542d90e7318ad02a40))
  - update Belarusian translation from Weblate ([57615230](https://github.com/qTox/qTox/commit/576152305aabb64b9b05e705f0eb0dfcb68c9329))
  - update Slovak translation from Weblate ([63d5caf3](https://github.com/qTox/qTox/commit/63d5caf37c03401398256a2443f10adc4b488850))
  - update Spanish translation from Weblate ([615c3614](https://github.com/qTox/qTox/commit/615c3614974678b2e4a9fca7af015ec65bcde1b0))
  - update Ukrainian translation from Weblate ([9dd40bec](https://github.com/qTox/qTox/commit/9dd40bec63b7470bdb455129a1d1e5a2ec60751c))
  - update Russian translation from Weblate ([e6b76561](https://github.com/qTox/qTox/commit/e6b76561062e85633305d0653cdb01c3c50dba88))
  - update Estonian translation from Weblate ([e580edd1](https://github.com/qTox/qTox/commit/e580edd184c3426669014a036e7dd23db3c3dcae))
- **ui:** grey out proxy settings if no proxy selected ([77aa2e92](https://github.com/qTox/qTox/commit/77aa2e92520de1323a3ac8ef9a154d0fdf041c0b))

<a name="v1.9.0"></a>

## v1.9.0 (2017-03-19)

### Release notes

The most noticeable change to UI are moved buttons in file transfer widget,
lessening chance of an accidental cancellation of a file transfer.

#### Features

- Switch windows portable links to stable versions ([b771f87e](https://github.com/qTox/qTox/commit/b771f87e0becbc649a5a3f0823efe153b6ffe2cb))
- Add version numbers in windows installer download links ([b73770d7](https://github.com/qTox/qTox/commit/b73770d703e98d47e9635918ad635eadcbbe2852), closes [#4112](https://github.com/qTox/qTox/issues/4112))
- **UI:** Move filetransferwidget buttons side-by-side. ([9322f29e](https://github.com/qTox/qTox/commit/9322f29ef4d6b45fb66623ef0655a496a3064b78), closes [#2597](https://github.com/qTox/qTox/issues/2597))
- **l10n:**
  - update German translation from Weblate ([015c8b00](https://github.com/qTox/qTox/commit/015c8b009264cd978b2d50ddab2460ec5697d6e6)), ([036ea14b](https://github.com/qTox/qTox/commit/036ea14b2689d39d89e9cd35782536048227b806))
  - update Chinese (Traditional) translation from Weblate ([9fadde67](https://github.com/qTox/qTox/commit/9fadde67a14fc4d125bab9e5f285ee42206f2bcc))
  - update Chinese (Simplified) translation from Weblate ([ae676246](https://github.com/qTox/qTox/commit/ae676246106f4890f34099840a92536d4831b92d))
  - update Slovak translation from Weblate ([a6665fe0](https://github.com/qTox/qTox/commit/a6665fe028cabb1269f0143bc2ebfda189b63427)), ([cf5e7f3a](https://github.com/qTox/qTox/commit/cf5e7f3ad194f171ffbd835a01ac20861fffba65))
  - update Belarusian translation from Weblate ([a335e32a](https://github.com/qTox/qTox/commit/a335e32af50788e2e94cd4a1db65344f62e7323f)), ([f92bbbb9](https://github.com/qTox/qTox/commit/f92bbbb9dd4b37535e80e7687935958d43592299)), ([6fe20861](https://github.com/qTox/qTox/commit/6fe20861485e1261a4da67b15212825cd5fd5126))
  - update Russian translation from Weblate ([17413c08](https://github.com/qTox/qTox/commit/17413c08641fddbc872176bc03f16807d6307531)), ([c2b7d7a5](https://github.com/qTox/qTox/commit/c2b7d7a533838ae8382bc42f953e15470149f3e2)), ([4d22b7cc](https://github.com/qTox/qTox/commit/4d22b7cc97401e447947e1150de8414e89e951eb))
  - update Italian translation from Weblate ([c6ca1518](https://github.com/qTox/qTox/commit/c6ca15188384993863dce217e3de815407be7084))
  - update Swedish translation from Weblate ([8eeee452](https://github.com/qTox/qTox/commit/8eeee4526c83b1b043bd517c0e5152e6fcd3b480)), ([c5067f65](https://github.com/qTox/qTox/commit/c5067f65d1845cbc89b4b4910283183807c45c5b))
  - update Greek translation from Weblate ([cebfe4a6](https://github.com/qTox/qTox/commit/cebfe4a63bfcda211b366a9fe61f500fee898760)), ([2551baab](https://github.com/qTox/qTox/commit/2551baab46415cea6f0f9bca48f65189cb201cc3))
  - update Chinese (Taiwan) translation from Weblate ([423fb83c](https://github.com/qTox/qTox/commit/423fb83c9f80a6b01f00ea980fc67e7fd1c0fbdb))
  - update Chinese (China) translation from Weblate ([fbe0de9b](https://github.com/qTox/qTox/commit/fbe0de9bbbce117f81c6ce2f52ac29fcd9854543)), ([bb3ee7d0](https://github.com/qTox/qTox/commit/bb3ee7d040edfe0389461f677cff0d691f5e7071)), ([e0cb8e5f](https://github.com/qTox/qTox/commit/e0cb8e5f530cfaeb640ab3754b36f8e8a45f789a)), ([95881c97](https://github.com/qTox/qTox/commit/95881c975dee764f1c545da99ddfcd9013ed54ac))
  - update Lithuanian translation from Weblate ([77ee1787](https://github.com/qTox/qTox/commit/77ee1787b11687bfc467880c720768c472c3a9f1))
  - update Korean translation from Weblate ([9fca7365](https://github.com/qTox/qTox/commit/9fca736506454779892b447eeaee8fdaf2e42f5e))
  - update Czech translation from Weblate ([99d34b04](https://github.com/qTox/qTox/commit/99d34b049db7709c42cb0bf074aff9db1ab7d11e))
  - add Chinese (Taiwan) translation to UI ([43d16d91](https://github.com/qTox/qTox/commit/43d16d91817f1224702b55aa2d6d53fb2aabd242))
  - add Chinese (Taiwan) translation using Weblate ([b80e964d](https://github.com/qTox/qTox/commit/b80e964d18763fcb5b3f2699789d17e95607bd9c))
  - update Bulgarian translation from Weblate ([6df92b73](https://github.com/qTox/qTox/commit/6df92b73629933f1d0a82da1b6ff4e7f8193670b))
  - update Pirate translation from Weblate ([6d2cf522](https://github.com/qTox/qTox/commit/6d2cf5222b83ed58562a766fafd0dac546c2ead8))
- **settings:** create ui when needed instead of show/hide ([12bcc261](https://github.com/qTox/qTox/commit/12bcc26154722a1f5028ae55709830c89a3d3b69))
- **shortcut:** Implemented F11 shortcut for toggling fullscreen. ([3a20a4ba](https://github.com/qTox/qTox/commit/3a20a4ba43e609c96385f25c1a9a20e55cbfa4fa))
- **toxencrypt:** add function to get min length for isEncrypted() to succeed ([a5955c67](https://github.com/qTox/qTox/commit/a5955c67ede3de72d4b9b76730216bdcc8f62334))
- **toxpk:** Add `'ToxPk::getPkSize()'` method ([f5c0d61a](https://github.com/qTox/qTox/commit/f5c0d61ae6cda2106c9d88ef7050b4003e725489))

#### Bug Fixes

- Fix incorrect headers order ([0fc39113](https://github.com/qTox/qTox/commit/0fc39113183afcf1bc0750b8727a48634d8a623a), closes [#4220](https://github.com/qTox/qTox/issues/4220))
- emoji packs path in user manual ([a3e64618](https://github.com/qTox/qTox/commit/a3e646182a719edd135bcf7308e7b37a27ab09f9))
- URL's are not affected by markdown ([18c3f3a2](https://github.com/qTox/qTox/commit/18c3f3a2170f32f1045a92290456a86737eb5254), closes [#4242](https://github.com/qTox/qTox/issues/4242))
- iterate all blocks ([7a5c5a86](https://github.com/qTox/qTox/commit/7a5c5a86fd82ab9812db019db2e98d700e08a900))
- fixed documentation mistake ([e8ed7e1f](https://github.com/qTox/qTox/commit/e8ed7e1f530b380d0b676933e6298327aeeb4398))
- Add splitter restorer ([a231532b](https://github.com/qTox/qTox/commit/a231532bd06a4a47432b5bc20d6d34028ca66941), closes [#2587](https://github.com/qTox/qTox/issues/2587))
- changed incorrect videocall button offline tooltip ([0effac2b](https://github.com/qTox/qTox/commit/0effac2bdd8834bd3b1edcc91f3c8ffed03abfe6))
- Autoupdater trying to open files twice could fail ([20ff68c3](https://github.com/qTox/qTox/commit/20ff68c3a69ec8c92d06b8e36b8e8443d76e094f))
- msleep in toxuri processEvents loops, to avoid 100% CPU ([2d0698af](https://github.com/qTox/qTox/commit/2d0698af95959a895e1cb86721d288d8ef5f3bae), closes [#1926](https://github.com/qTox/qTox/issues/1926))
- Don't even try to add ourselves as a friend in the Tox URI handler ([bfda028a](https://github.com/qTox/qTox/commit/bfda028a2a3ef63181dbc8ffa42817e08e616f20))
- Various IPC event handling and related bugs on startup ([c75ee8a6](https://github.com/qTox/qTox/commit/c75ee8a6619e6c546121462d28e221bb8df11f19), closes [#1926](https://github.com/qTox/qTox/issues/1926))
- Friend list avatars not updating ([055c41bc](https://github.com/qTox/qTox/commit/055c41bc30de6901b63d14dbc50e384de5779d16))
- Accept IDs as tox URIs, not just ToxDNS addresses ([1d307bcc](https://github.com/qTox/qTox/commit/1d307bcc0e3bde3282615f5f2db08578e7cf4984), closes [#1925](https://github.com/qTox/qTox/issues/1925))
- **UI:** actually load bundled Unicode font as soon as possible ([bfd47733](https://github.com/qTox/qTox/commit/bfd47733c241677163e38b0bc28a9de87f4dc535))
- **audio:** alternate audio fix implementation from #4139 ([62ac4801](https://github.com/qTox/qTox/commit/62ac480171e6b073689344ee8eb65430485166de))
- **cmake:** Add ability to real disable optional dependency ([fb43e306](https://github.com/qTox/qTox/commit/fb43e306774db7b040fde550c9d4c479793d1bad))
- **core:** Ignore online connection status ([ea50eaae](https://github.com/qTox/qTox/commit/ea50eaaef742141ec2023050d5d1ee8719583333), closes [#4010](https://github.com/qTox/qTox/issues/4010))
- **db:** Made `RawDatabase::execLater` executes statements asynchronously. ([54fb9f73](https://github.com/qTox/qTox/commit/54fb9f73ccb70735f0b551faaa2e713004a3864d))
- **font:**
  - Made the font combobox not editable. ([ba28f163](https://github.com/qTox/qTox/commit/ba28f163fdd243998f0962cf4ad2ca5e5134d9f5))
  - Made font changes in settings apply on screen instantly. ([742583bb](https://github.com/qTox/qTox/commit/742583bb9ed11eebea2686f985b7dd254f5a6d89))
- **icons:** Removed unnecessary icon preparation. ([fac0021a](https://github.com/qTox/qTox/commit/fac0021a4752bca7d5a72fc2c1c464d59ba81754))
- **qmake:** add forgotten files to qTox.pro ([db2e2d7a](https://github.com/qTox/qTox/commit/db2e2d7ab77836d65c27a57ece69220eeb17e0e9), closes [#4212](https://github.com/qTox/qTox/issues/4212))
- **toxencrypt:** return the plaintext after decryption ([14ee8af3](https://github.com/qTox/qTox/commit/14ee8af3cd156437b37ada4d06b07ccfbd2d3d4a), closes [#4162](https://github.com/qTox/qTox/issues/4162))

<a name="v1.8.1"></a>

## v1.8.1 (2017-02-05)

### Release notes

qTox 1.8.1 release.

Fixes OSX deployment.

<a name="v1.8.0"></a>

## v1.8.0 (2017-02-05)

### Release notes

With this version build system has been migrated to `cmake` and `qmake`
support is now deprecated and will be removed in the `v1.9.0` release.

#### Features

- **UI:** remove unnecessary spacings on the Profile page ([1fc412df](https://github.com/qTox/qTox/commit/1fc412df72156b264f77e62ba395fe25eeb39514))
- **chatform:** Changed mic and vol buttons ([5381d55e](https://github.com/qTox/qTox/commit/5381d55e942bb08ce6ff870626a0e10bfaa2358c))
- **l10n:**
  - update Polish translation from Weblate ([a6f52704](https://github.com/qTox/qTox/commit/a6f5270461db2dbd25f8589fe4dcbdfb5478aef3))
  - update Pirate translation from Weblate ([d570c2b3](https://github.com/qTox/qTox/commit/d570c2b3cd213a6c4a8b0618e222e4e1d5683048))
  - update Lithuanian translation from Weblate ([c6fa538e](https://github.com/qTox/qTox/commit/c6fa538ee547c27dd03dfcd37ae7a7d5f09131ed)), ([7fab1632](https://github.com/qTox/qTox/commit/7fab1632d501693611c6db762f4454773f9ad336))
  - update Estonian translation from Weblate ([3d639e00](https://github.com/qTox/qTox/commit/3d639e007c235ca556aaed69bc88e6cbc5434dc3))
  - update Slovak translation from Weblate ([4278d165](https://github.com/qTox/qTox/commit/4278d16554f5e0efb1ae427582e39bbfffeec75b)), ([3f0ee63e](https://github.com/qTox/qTox/commit/3f0ee63e02c842e0e10570f25f2146355ee841e8))
  - update Swedish translation from Weblate ([bd965086](https://github.com/qTox/qTox/commit/bd9650862f9230c8eccef60d40a69fa0a15dcf71))
  - update Czech translation from Weblate ([b7ec9016](https://github.com/qTox/qTox/commit/b7ec90167e89679552b4953ac3bcbb602c6dae4e)), ([9676e8c9](https://github.com/qTox/qTox/commit/9676e8c97a3ba7c2a12c9c4511d1db75c3006c04))
  - update French translation from Weblate ([d82f0670](https://github.com/qTox/qTox/commit/d82f0670e301edee962d1ccb5f177da80a23d9a3)), ([cd2b1b1a](https://github.com/qTox/qTox/commit/cd2b1b1ae62d6fc0ac27b860a78cced7acbacafb)), ([edab0fe3](https://github.com/qTox/qTox/commit/edab0fe3fd87df673b8563ac115d34d42c92ed9c)), ([275fdc6a](https://github.com/qTox/qTox/commit/275fdc6aa08222e0441f042ccfa50cc1c1922c68)), ([c990dfd7](https://github.com/qTox/qTox/commit/c990dfd7cf6090d2e68ee29990853061cca12d83))
  - update Korean translation from Weblate ([c8660c8b](https://github.com/qTox/qTox/commit/c8660c8b80238a8698a9f6ccc74aeca7bfac90f4))
  - update Russian translation from Weblate ([ce69f2b9](https://github.com/qTox/qTox/commit/ce69f2b997ea6d8c19d8479964c8a33da949b8ce)), ([5e04b72a](https://github.com/qTox/qTox/commit/5e04b72afd1261f787551930e0f3164b44e81891)), ([8bcaa221](https://github.com/qTox/qTox/commit/8bcaa221c8545301b462daa49b39af4b1d7e44df)), ([8c868e2c](https://github.com/qTox/qTox/commit/8c868e2cf587f7e465e550c29235ebb20ae2abf7))
  - update Chinese (China) translation from Weblate ([65f29811](https://github.com/qTox/qTox/commit/65f29811e39de1cdf9402c0e68cda905f2e87821))
  - update Belarusian translation from Weblate ([1494fbe4](https://github.com/qTox/qTox/commit/1494fbe4e1c3399b1b7cacde7dc4189c87c89763)), ([f938fef2](https://github.com/qTox/qTox/commit/f938fef23366681114099a6518b0612262681bac))
  - update Chinese translation from Weblate ([cf6e83ba](https://github.com/qTox/qTox/commit/cf6e83baf1bdd3a689ac9fd5c0bf652ce6cc736d))
  - update Spanish translation from Weblate ([09727af0](https://github.com/qTox/qTox/commit/09727af00b17ad3427ddbba1dc916765191a1725))
  - update Hungarian translation from Weblate ([29715335](https://github.com/qTox/qTox/commit/29715335782137e985e408f412ccf3e5dfeba043))
  - update German translation from Weblate ([2c8c5c52](https://github.com/qTox/qTox/commit/2c8c5c5259387a57c4de5e80e08227523a419478)), ([ac7352db](https://github.com/qTox/qTox/commit/ac7352db0574935b43116924c0bbda8e9757da01)), ([6a492ea4](https://github.com/qTox/qTox/commit/6a492ea4277e07c468fbfce74252b0c384962502)), ([e5f1cdcb](https://github.com/qTox/qTox/commit/e5f1cdcbc42183d088fe8dfc3433a11b9497b825))
  - update Finnish translation from Weblate ([48ebc821](https://github.com/qTox/qTox/commit/48ebc821d8568831a55322ee6b2988bcb12f8fff))
- **toxid:**
  - reduce passing Tox IDs around ([e07d8d35](https://github.com/qTox/qTox/commit/e07d8d358f6fc890a77e029aa230b69bdecd325e))
  - Improve validation of Tox IDs ([d196611f](https://github.com/qTox/qTox/commit/d196611ffeeccc200d42047c85af3f7fe93f0730))
  - Improve the ToxId class ([94ec5614](https://github.com/qTox/qTox/commit/94ec5614f2bb50d7d487d5b933393ed0e3c05e03))
  - Added correct checksum validation. ([b4c45692](https://github.com/qTox/qTox/commit/b4c4569299d734d8056e48a6d6dd62f4df2ae86f), closes [#3837](https://github.com/qTox/qTox/issues/3837))

#### Bug Fixes

- Now cannot send party invite to a friend which has "Offline" status ([034c507c](https://github.com/qTox/qTox/commit/034c507cc4fa83210a40b92c5aa458e9daf5ba5a), closes [#4018](https://github.com/qTox/qTox/issues/4018))
- Add workaround for FreeBSD and ICU ([22d3aa11](https://github.com/qTox/qTox/commit/22d3aa11debfc54ad32f3a4642ea81da058c0ee2))
- Remove old ChatForm code from Widget ([52642578](https://github.com/qTox/qTox/commit/52642578a0d66c79c03b6dbfe0ab2af86a30302b))
- Further Windows Jenkins link errors ([b6934927](https://github.com/qTox/qTox/commit/b693492702a5f65d278baf8d38b4f1a9ea537089))
- Jenkins Windows builds ([debb6e9c](https://github.com/qTox/qTox/commit/debb6e9cc897de62687ad39880059472d0eb8e48))
- fix warning about missing (removed) `cdata.h` ([4972ac1d](https://github.com/qTox/qTox/commit/4972ac1d537bab37aa73635c49067d75913f5662))
- Fix problem with unicode symbols on FreeBSD ([56d65650](https://github.com/qTox/qTox/commit/56d656502c82d81e5339fad7eb7bc77545adea8b), closes [#4012](https://github.com/qTox/qTox/issues/4012))
- **UI:** fix own status message not being properly displayed in tooltip ([dccef4d4](https://github.com/qTox/qTox/commit/dccef4d49f62d10427f4a625a9595da881a1370d))
- **addfriend:**
  - create ToxId from QString before emitting the signal ([a5d2c573](https://github.com/qTox/qTox/commit/a5d2c573ddca775567024ee9e64fe735146b1807), closes [#4086](https://github.com/qTox/qTox/issues/4086))
  - do not load own Tox ID from clipboard ([efcff576](https://github.com/qTox/qTox/commit/efcff57601a3dc04fc7376b6edbeff81487dc5b8), closes [#4024](https://github.com/qTox/qTox/issues/4024))
- **avatar:** wrong conversion of own ToxId ([0e4cff88](https://github.com/qTox/qTox/commit/0e4cff884db58e31e7a269721bc9e6138174b9b0), closes [#4048](https://github.com/qTox/qTox/issues/4048))
- **build:**
  - avoid version checking where unnecessary ([bf8b23e3](https://github.com/qTox/qTox/commit/bf8b23e373275f0d34d99ff79d0637f2fb558d18))
  - do not invoke version update on travis ([07639011](https://github.com/qTox/qTox/commit/07639011daa5ff84b61e76b0ad0dc8e659c5a68b))
  - attempt to make travis happy ([919a94b9](https://github.com/qTox/qTox/commit/919a94b978aa957ed3b7702600b90f72a952eb7e))
  - set generic macOS Info.plist version by default ([3a88abdd](https://github.com/qTox/qTox/commit/3a88abddd7cb96bbff48d5c6601798173328590f))
  - correct macOS deployment ([8912e9e0](https://github.com/qTox/qTox/commit/8912e9e016d7591eb1a1d1f782c90bd8da7b16c6))
  - add an option to compile with autotools toxcore on Windows ([482757e6](https://github.com/qTox/qTox/commit/482757e6a2ab74410f33fca364af1b69f2695ae4))
  - fix ASLR and DEP protection for windows builds ([ce0590d2](https://github.com/qTox/qTox/commit/ce0590d2d0788a3d0cb9318211e3c79761b49492))
- **chatform:**
  - Add ability to cancel call ([320099fa](https://github.com/qTox/qTox/commit/320099faf80ca51e60c68953463a60b50b23e4c2), closes [#4016](https://github.com/qTox/qTox/issues/4016))
  - Fixed buttons ([aff0c6f3](https://github.com/qTox/qTox/commit/aff0c6f3a5a6f883329b4e20598ff72852fe2cca))
- **clipboard:** Made clipboard operations work the same on all OSes. ([a18e0b6a](https://github.com/qTox/qTox/commit/a18e0b6a2cbea94229598c9e777ceea1aa46fff7))
- **cmake:**
  - Add search for toxcore or libtoxcore ([a4537549](https://github.com/qTox/qTox/commit/a4537549c0c53a88a47ca7a06cf276f0430eb43e))
  - Fix smileys resource adding ([fddda5de](https://github.com/qTox/qTox/commit/fddda5de61d302abe3f075dc35ef296c8a29a82f))
- **font:** Fixed the editbox font reset problem ([c84837d6](https://github.com/qTox/qTox/commit/c84837d662b6838a55caae7b3d4a121abd2d0192))
- **icon:**
  - add more size versions to qtox.ico ([ec95d7f5](https://github.com/qTox/qTox/commit/ec95d7f571431a8d46158266123cd0806fc4fdc3))
  - correct the shape of the logo ([f258cf20](https://github.com/qTox/qTox/commit/f258cf20e243c7b426cb4096108924871b4958d0), closes [#4114](https://github.com/qTox/qTox/issues/4114))
- **icons:** install icons for linux ([6b513819](https://github.com/qTox/qTox/commit/6b51381980e4d742efff0905a697040efbcce049), closes [#4052](https://github.com/qTox/qTox/issues/4052))
- **jenkins:** preserve the correct linking order ([5d4609bd](https://github.com/qTox/qTox/commit/5d4609bd0ee7854b27791c856ca7de93491232d1), closes [#4079](https://github.com/qTox/qTox/issues/4079))
- **l10n:** rename `zh` translation to `zh_CN` ([1df9f572](https://github.com/qTox/qTox/commit/1df9f5726dcd2605245f4164c7462922ef9464fb), closes [#4080](https://github.com/qTox/qTox/issues/4080))
- **nexus:** Add qRegisterMetaType call for ToxPk ([328d0b6d](https://github.com/qTox/qTox/commit/328d0b6ddfe4f74ab701d926503860c6b1c77c1a))
- **osx:** Remove useless '#ifdef `Q_OS_MAC'` ([f0467abc](https://github.com/qTox/qTox/commit/f0467abc34f3dc2308b9cd4473550e8f1d4ce787))
- **toxid:** Tox ID construction from ToxMe ([a223510c](https://github.com/qTox/qTox/commit/a223510cf7b8ca72dce7b2f3d6ed9c6ca2284fab))
- **warnings:** fix some warnings about the stack protector not working ([e7276e7b](https://github.com/qTox/qTox/commit/e7276e7b43dd7b8631074866c27dfd70e40ae9f5))

<a name="v1.7.0"></a>

## v1.7.0 (2016-12-25)

### Release notes

qTox v1.7.0 release

More information in the CHANGELOG.md.

#### Features

- **UI:**
  - make font in message input field use font settings ([c14cbdf4](https://github.com/qTox/qTox/commit/c14cbdf4fd99352d1a70ad0c9dfeb78e309b1228))
  - add `My` to text labels on Profile page ([785e546e](https://github.com/qTox/qTox/commit/785e546e9e76144f2f39135cd425e0d924c960dd))
  - make information text in settings selectable ([e9778e81](https://github.com/qTox/qTox/commit/e9778e81a238d2688ba5106b602d08208a9311a9))
  - make it possible to select text of the received friend request ([06606958](https://github.com/qTox/qTox/commit/0660695873ef8ab1f5e31761c113065560d3a75b))
  - Add 'Copy link' context menu item ([a7de2680](https://github.com/qTox/qTox/commit/a7de2680d971337c697357c052dc90ddb468883f), closes [#927](https://github.com/qTox/qTox/issues/927))
  - make link to filled issue template on About page visible ([feaedeed](https://github.com/qTox/qTox/commit/feaedeed841f419a8eb5b7f40a14e27ef1e104cb))
  - add accessibility labels to all remaining .ui files ([25e9d3c4](https://github.com/qTox/qTox/commit/25e9d3c46cf717bd623be1bda8dbc7a8541d84e1))
  - add accessibility labels to the add friend page ([8854589b](https://github.com/qTox/qTox/commit/8854589b0d3ddc50bf899413c1ef970c43d127c4))
  - add accessibility labels to the Profile page ([949e3cb8](https://github.com/qTox/qTox/commit/949e3cb830134a1412b93d751a1d16dd80877aba))
  - add some accessibility labels to login pages ([953fe241](https://github.com/qTox/qTox/commit/953fe2416a97ebdeb6de45a801e66fc3b434d551), closes [#2101](https://github.com/qTox/qTox/issues/2101))
  - rename `Toxme register` → `Register on ToxMe` ([2433fa9d](https://github.com/qTox/qTox/commit/2433fa9dcee8c10a9d8ec050b8bf06c0facdce7e))
- **database:**
  - make a backup before upgrading ([c1d471fa](https://github.com/qTox/qTox/commit/c1d471faa18ecb8f04001b97937e14d6c34a9a46))
  - use own public key as salt to encrypt the database ([c4b9d302](https://github.com/qTox/qTox/commit/c4b9d302d0c3007102436f7f76498a7a0b8edcaf), closes [#3583](https://github.com/qTox/qTox/issues/3583))
- **l10n:**
  - update Russian translation from Weblate ([eb15b157](https://github.com/qTox/qTox/commit/eb15b157d014555d5b91d60bd4c7421034e2a315)), ([2fa4073d](https://github.com/qTox/qTox/commit/2fa4073d903b4bc648580bb50fed51cede06ad5e)), ([15638d0c](https://github.com/qTox/qTox/commit/15638d0c31a008dc54fe63d1ac264c212d18c590)), ([16accc74](https://github.com/qTox/qTox/commit/16accc745ce13ef6ca2a35c67bae2714849b261d))
  - update German translation from Weblate ([9125060d](https://github.com/qTox/qTox/commit/9125060d35d00c71cb23167625f3ca3d35306ac2)), ([03197e5c](https://github.com/qTox/qTox/commit/03197e5c97140030d5bda334ecea3e1a464e6e6e))
  - update Hungarian translation from Weblate ([ff69b61f](https://github.com/qTox/qTox/commit/ff69b61f36541276b4a8764184f1927713ee900a)), ([cdb47dce](https://github.com/qTox/qTox/commit/cdb47dce74d8991a12f93f1e2acf4965ffe34f41)), ([b0a564da](https://github.com/qTox/qTox/commit/b0a564dacde79ac361d6a9ba53db1e43ac0cfa08)), ([8cb357fd](https://github.com/qTox/qTox/commit/8cb357fda788b2f71f4cdd17a427f1b50ab16fed))
  - update Chinese translation from Weblate ([0f5b2151](https://github.com/qTox/qTox/commit/0f5b2151ee334b397e1eaaefd49e47e2a71e2c90)), ([db990ac7](https://github.com/qTox/qTox/commit/db990ac71f21120bfeb7fe5307b3ab9c3a0ba212)), ([3a656bb3](https://github.com/qTox/qTox/commit/3a656bb3ca623b9ec33b6130eeab0c8212874cf0)), ([dd4673cf](https://github.com/qTox/qTox/commit/dd4673cf6ad5a59e7cfc39ba3dd6faa25f76b85e)), ([b4c740b1](https://github.com/qTox/qTox/commit/b4c740b1b70bcf7c6ab78f2c3b99c018da9bae70))
  - update Estonian translation from Weblate ([e6bb215a](https://github.com/qTox/qTox/commit/e6bb215a23363b6feab5a3f590ef0c0ea91378c6))
  - update Belarusian translation from Weblate ([a3c876c1](https://github.com/qTox/qTox/commit/a3c876c14af4a9fa581c7c627b81a7ec1fe821c5)), ([10b09036](https://github.com/qTox/qTox/commit/10b09036ca9dab7a9670b5b421897bbc23250d62)), ([b8aef1dc](https://github.com/qTox/qTox/commit/b8aef1dcebae378eb08c245ffb6d83cb3876e3b0)), ([fa550a12](https://github.com/qTox/qTox/commit/fa550a127f2a4bc9eb3c4f2e4f466ba773845a5b)), ([899d74f1](https://github.com/qTox/qTox/commit/899d74f1d95136eb9e97d1ebdcbcd0fde46753f5)), ([bb445a8e](https://github.com/qTox/qTox/commit/bb445a8e7155b18683ff77235ba64428b1b20069))
  - update Bulgarian translation from Weblate ([88049cc2](https://github.com/qTox/qTox/commit/88049cc2277810f0fa5e30c65c4732f4aa25db04)), ([595657c2](https://github.com/qTox/qTox/commit/595657c232f0826170c61ae605b831c495b58d34))
  - update Arabic translation from Weblate ([1969084b](https://github.com/qTox/qTox/commit/1969084baa892a2fab957f5c326973efd0b938d8)), ([b0d03622](https://github.com/qTox/qTox/commit/b0d03622b2a244cef4fbe8902aca53efcbebd624))
  - update Polish translation ([5ed1048e](https://github.com/qTox/qTox/commit/5ed1048e7254bb657436b852a8c4b3561932971d))
  - add Pirate translation to UI ([93feae0c](https://github.com/qTox/qTox/commit/93feae0c95bb9a6b3a8bbe1022c403e8167bc7b3))
  - add Pirate translation using Weblate ([c75f0b74](https://github.com/qTox/qTox/commit/c75f0b74a29449dac1c3320e4ea2c1702b03c4c9))
  - add Swahili translation to interface and docs ([8b671323](https://github.com/qTox/qTox/commit/8b67132337f3c90a9df8ed0ed06a7021386b6774))
  - add Swahili translation from Weblate ([f28e136e](https://github.com/qTox/qTox/commit/f28e136ec45b0f7ec26c4ebaa6e7eef7b39f9c0d))
  - update French translation from Weblate ([2519594a](https://github.com/qTox/qTox/commit/2519594aa2ec156bb6af3a192bdefe7a1ffa50f9)), ([cca4038f](https://github.com/qTox/qTox/commit/cca4038f655029b753e42cee45b46121c93fe3e6))
  - update Lithuanian translation from Weblate ([7d7a1845](https://github.com/qTox/qTox/commit/7d7a18454cef4118bf272b5849f122b68a2f1f5f))
  - add Korean translation to interface and docs ([5267b195](https://github.com/qTox/qTox/commit/5267b19503a815a269b4cc7bc4fb77b270157467))
  - add Korean translation from Weblate ([6e040b8e](https://github.com/qTox/qTox/commit/6e040b8e7c50b53b13c8bd46a6b38df7b34a0e33))
  - update Italian translation ([6a520c9c](https://github.com/qTox/qTox/commit/6a520c9c90eed3cc9eaab28f7b64550edc39eada)), ([ed472001](https://github.com/qTox/qTox/commit/ed47200181b29ec7b37651f6d450273f4002ec01))
  - update Slovak translation from Weblate ([051672af](https://github.com/qTox/qTox/commit/051672afb52f893bd24a1aacc431354a6a0e0258)), ([cc6efe93](https://github.com/qTox/qTox/commit/cc6efe93bcaba5c696616ccaa231b7e93efe272c))
  - update Czech translation from Weblate ([7c5eaeae](https://github.com/qTox/qTox/commit/7c5eaeae7b990aef63d82445a7b8f356c554f14d)), ([b12b1cdc](https://github.com/qTox/qTox/commit/b12b1cdc2ed37d14facea45f0f775c1539883a88))
  - update Portuguese translation from Weblate ([26ff01ba](https://github.com/qTox/qTox/commit/26ff01ba023c75e9f93914f7e082f3c3adab0643))
  - update Esperanto translation from Weblate ([1e39d944](https://github.com/qTox/qTox/commit/1e39d94425789702c1222f71ccb0524d608d1d47))

#### Bug Fixes

- Added Kiss smiley shortcut for Universe and emojione ([059bb9a3](https://github.com/qTox/qTox/commit/059bb9a3a2ebf4c4310723ab386babc96c77cc15))
- add missing `Q_OBJECT` macro to the PasswordEdit class ([f5011cdb](https://github.com/qTox/qTox/commit/f5011cdb2feb2da042e373b4e4094e967bde8120))
- Fixed compact layout ([f870bf19](https://github.com/qTox/qTox/commit/f870bf1953672d948907477415a5cd2143d14a03), closes [#3944](https://github.com/qTox/qTox/issues/3944))
- remove unnecessary variable when checking whether save is a TES ([231e1d74](https://github.com/qTox/qTox/commit/231e1d746a9b97bb5da952c7565c0d6f052f26de))
- Status reflection for friend calls ([917812da](https://github.com/qTox/qTox/commit/917812daaabc8b8118526691e67a3f6e2dd8fcb2))
- dataToString correctness regression ([a4bda265](https://github.com/qTox/qTox/commit/a4bda2652550f3b2c2c8275dc8f607498f1c06ff))
- **UI:**
  - make link in `About` setting page point to all contributors ([326d5219](https://github.com/qTox/qTox/commit/326d5219bc5bdabc87028e1040b98cde1e7c8950), closes [#3898](https://github.com/qTox/qTox/issues/3898))
  - remove unnecessary space between a word and colon ([fba5f8cc](https://github.com/qTox/qTox/commit/fba5f8ccc44b3227dc7d064f6c4d2307b616b839))
  - copy friend's non-truncated status message instead of truncated ([9d1275b3](https://github.com/qTox/qTox/commit/9d1275b3963bfa87a4c1321d2d590db994e87ff3))
  - Changed tab order to be same with visual order ([f7899d7b](https://github.com/qTox/qTox/commit/f7899d7b5044706d339537ff9658087a7ad22e37))
- **core:** don't emit idSet signal twice ([355fb2ae](https://github.com/qTox/qTox/commit/355fb2ae413cad8f680ce3e8b1615c9415715135))
- **l10n:** fix wrongly positioned `%` in translations ([e831ba94](https://github.com/qTox/qTox/commit/e831ba9427f356513eb66e1c4c26f889cc1a129d))
- **videoButton:** Added disabling button while call is active ([eff27f69](https://github.com/qTox/qTox/commit/eff27f690e8a8af2e8f1fe0cd80f3b21dea2bed7), closes [#4004](https://github.com/qTox/qTox/issues/4004))

<a name="v1.6.0"></a>

## v1.6.0 (2016-11-13)

### Release notes

qTox v1.6.0 release

More information in the CHANGELOG.md.

#### Bug Fixes

- Fixed crash on profile import ([a130a60e](https://github.com/qTox/qTox/commit/a130a60e9a09096de5e562d6bb89edf3e9984cca), closes [#3660](https://github.com/qTox/qTox/issues/3660))
- hide option to `Load automatically` for encrypted profiles ([b5efc2bb](https://github.com/qTox/qTox/commit/b5efc2bb7124ccbbd256792c524ffffdd02fab3f))
- use `qAbs()` instead of `abs()` for better platform compatibility ([79c249be](https://github.com/qTox/qTox/commit/79c249be55f084c1bf68dbe927c847a4ea45e045), closes [#3613](https://github.com/qTox/qTox/issues/3613))
- **UI:**
  - prevent text style from being reset during retranslation ([28c29157](https://github.com/qTox/qTox/commit/28c29157f8ea7f956bd76a3979becb41d6968bb3), closes [#3805](https://github.com/qTox/qTox/issues/3805))
  - bump default emoticon size, since otherwise it can be too small ([bbdd4f04](https://github.com/qTox/qTox/commit/bbdd4f044c2c6b70fcb8794c8e5f01f1380af168))
  - string for option auto-accepting files ([1d4ea1ec](https://github.com/qTox/qTox/commit/1d4ea1ec408c7c974f0a408835f599864320a15f))
- **audio:**
  - Clear audio buffer when ending audio loop ([244d1dc3](https://github.com/qTox/qTox/commit/244d1dc386f634a0a27e58e26a5f0fd22340081f))
  - keep the data pointed to by tmpDevName in scope ([af37fa7b](https://github.com/qTox/qTox/commit/af37fa7b20faf22129fe8d949ba28ca01b0a462f), closes [#3786](https://github.com/qTox/qTox/issues/3786))
- **avform:**
  - Fixed empty value in audio and video lists ([15181c43](https://github.com/qTox/qTox/commit/15181c431a800347bd7bf82fa2703ea05bfe0dd5))
  - Added shift screen region ([eaccbf3d](https://github.com/qTox/qTox/commit/eaccbf3da9a9e90897142370ec76defc85e7fe02), closes [#3552](https://github.com/qTox/qTox/issues/3552))
- **build:**
  - enable backports repository for Debian Jessie ([fb5639b2](https://github.com/qTox/qTox/commit/fb5639b292c99c176fe91ca844f7360820ce68ac), closes [#3679](https://github.com/qTox/qTox/issues/3679))
  - fix OpenAL build on Windows ([2f7076c1](https://github.com/qTox/qTox/commit/2f7076c1687dcf163ee4e63105e37bac9d9aebd3), closes [#3372](https://github.com/qTox/qTox/issues/3372))
- **chatform:** remove `std::dynamic_pointer_cast` in favor of static cast ([775b6a32](https://github.com/qTox/qTox/commit/775b6a325be9a19104e73a90c7836446bf429536), closes [#3801](https://github.com/qTox/qTox/issues/3801))
- **chatlog:** silence warning about unused parameters ([8ff0d28a](https://github.com/qTox/qTox/commit/8ff0d28abf8b11940b13fa3ae861db0bc5dce9e1))
- **i18n:**
  - Remove HTML tags from translation ([f5f7b5ab](https://github.com/qTox/qTox/commit/f5f7b5abba756860e3c2e3949b92f6abafc0c3aa))
  - remove HTML from translations ([c8bde5bc](https://github.com/qTox/qTox/commit/c8bde5bc452aaf68e88424bc448f86a16b282ffc), closes [#3781](https://github.com/qTox/qTox/issues/3781))
  - Removed extra tag from translation ([fde9ad53](https://github.com/qTox/qTox/commit/fde9ad535667e25869879fcde2dd24d8c7d8ca51))
- **l10n:** correct ru translation ([5c901d0e](https://github.com/qTox/qTox/commit/5c901d0ea5bd5fc32a05138f705023a316b0813b))
- **rawdatabase:** Added anonymizing SQL query in logs ([85ee69f0](https://github.com/qTox/qTox/commit/85ee69f0082105a6df187d85eb831ea5e7ce8c3e), closes [#3399](https://github.com/qTox/qTox/issues/3399))
- **settings:**
  - Changed grid layout on form layout ([9fd0dd79](https://github.com/qTox/qTox/commit/9fd0dd790b766355377cb53f11cfb7d4a099b1b6))
  - Properly update taskbar on disconnect ([6e165ceb](https://github.com/qTox/qTox/commit/6e165ceb475313655a80e166ea1d88577ee8afb0), closes [#3795](https://github.com/qTox/qTox/issues/3795))
  - Change test sound button to checkbox ([22b89bea](https://github.com/qTox/qTox/commit/22b89beaf14f82f623464967f1980e5d78040649), closes [#3800](https://github.com/qTox/qTox/issues/3800))
  - Play test sound when user enables test sound ([9b46cf64](https://github.com/qTox/qTox/commit/9b46cf64041a4c65155d3db75ed2d08630a673f0), closes [#3735](https://github.com/qTox/qTox/issues/3735))
  - correct empty listbox audio device ([84a95700](https://github.com/qTox/qTox/commit/84a95700398273129ff2e6d63ea884f64c7ee686), closes [#3709](https://github.com/qTox/qTox/issues/3709))
  - Use old settings instead of default values ([6f0431ae](https://github.com/qTox/qTox/commit/6f0431ae50499bc2a3a19ddc9e83f86f55e105d5), closes [#3706](https://github.com/qTox/qTox/issues/3706))
  - correct default value of dateformat ([e794acbc](https://github.com/qTox/qTox/commit/e794acbccc29ee7d1a262c283b838e8b297b5527), closes [#3707](https://github.com/qTox/qTox/issues/3707))
  - Fixed group to load makeToxPortable setting ([a3c201d3](https://github.com/qTox/qTox/commit/a3c201d3b2562c794450ba15117ff7a69408da57), closes [#3681](https://github.com/qTox/qTox/issues/3681))
  - Added loader for old settings ([740d91b6](https://github.com/qTox/qTox/commit/740d91b61a66f5a5929dafc5fa86095d02bcf35a))
  - const getters ([e2f9d2cf](https://github.com/qTox/qTox/commit/e2f9d2cfe873a16841277f32ed581cbcdcb8bddc))
- **settingsform:** Fix layout spacing ([f1975231](https://github.com/qTox/qTox/commit/f197523155000fe719947b73aa84bffdc9fff385))
- **text:** Change idealSize calculation ([c8512fa9](https://github.com/qTox/qTox/commit/c8512fa98461a54f3febc076fa1d7f99eae1d593), closes [#1887](https://github.com/qTox/qTox/issues/1887))
- **timeformat:**
  - Remove duplicate of timeformat ([661388d7](https://github.com/qTox/qTox/commit/661388d73195a539285fd6dc7acad4e86ba0e57b))
  - Make timestamp editable same as date format ([d6f323ce](https://github.com/qTox/qTox/commit/d6f323cefc15b477ec588a3e88a133188ecbac84))
  - Fixed list of available time formats ([35e7da85](https://github.com/qTox/qTox/commit/35e7da85d34fa8f5bd0c9ec6da10894c932af568))
- **uiform:** Added format constraints ([13baf932](https://github.com/qTox/qTox/commit/13baf9326d869d8b410a6a7f9c447766fcdea45e))
- **video:** fix scaling issues under HiDPI displays with desktop video ([ef157ca8](https://github.com/qTox/qTox/commit/ef157ca8af14862c1d69d62ab609fe44754c4b3f))
- **videomode:** Added ability to store negative coordinates ([26a27cf3](https://github.com/qTox/qTox/commit/26a27cf3f8de49b0dd49e56b55d1ad0303b93975))
- **widget:**
  - do not block logout on Linux desktop environments v2 ([6b00779d](https://github.com/qTox/qTox/commit/6b00779ddecb4a28c20ef3d00eadd0ffd5490c48), closes [#3699](https://github.com/qTox/qTox/issues/3699))
  - do not block logout on Linux desktop environments ([7ee883bc](https://github.com/qTox/qTox/commit/7ee883bcc7cf09fe3b87ab7b4594165b7cc08ba9), closes [#1485](https://github.com/qTox/qTox/issues/1485))

#### Features

- bump font size from undersized ([2d472004](https://github.com/qTox/qTox/commit/2d472004cd3a6afc73889f836e80c3c9fa6b29ac), closes [#2280](https://github.com/qTox/qTox/issues/2280))
- **autoAnswer:** add auto answer feature ([7fe8a86c](https://github.com/qTox/qTox/commit/7fe8a86c0329481cd34f5ed4e134d2db7a11d096))
- **avform:**
  - Made warning and debug info more useful ([94783778](https://github.com/qTox/qTox/commit/9478377868506559f7b126c501e24c8f9c844806))
  - Added automatic screen rescan ([08229735](https://github.com/qTox/qTox/commit/08229735b26cd849343810e1f84c2c3bf5807dc5))
- **dateformat:** Edit the date format string ([b3083250](https://github.com/qTox/qTox/commit/b3083250ff8f71936db56993a9d7b1216d8323f9))
- **l10n:**
  - update Slovak translation from Weblate ([dcfbdf8f](https://github.com/qTox/qTox/commit/dcfbdf8f95c865c8336ed5ffbc094b91fb2315b2)), ([287e2bcc](https://github.com/qTox/qTox/commit/287e2bcc38ee08d59f7877ee9aaef397fe53abca))
  - update Chinese translation from Weblate ([11b19e42](https://github.com/qTox/qTox/commit/11b19e42e8a45b53f8f0185a1073a36726de26a8)), ([4f20d032](https://github.com/qTox/qTox/commit/4f20d032d1cd67a7667f1403e816854d933507d4)), ([3539e840](https://github.com/qTox/qTox/commit/3539e8405159ab4b0f15b23ae88ca11b34f32c5c)), ([5f087194](https://github.com/qTox/qTox/commit/5f087194050bbb148cc3b9743e5fd3e0a4144e3a))
  - update Lithuanian translation from Weblate ([55bb35ac](https://github.com/qTox/qTox/commit/55bb35acaa32c5b75189cb6b909f8eb4a016641b)), ([7e6e3970](https://github.com/qTox/qTox/commit/7e6e39703b0fc9810f7aa0c0a84a173b092d46ea)), ([25e86e54](https://github.com/qTox/qTox/commit/25e86e5464fcf556bcd8d538cb07cc97acd83432))
  - update Dutch translation from Weblate ([2b3eaa1b](https://github.com/qTox/qTox/commit/2b3eaa1bc54727ae2820883fc393593461020472)), ([2c5a38cc](https://github.com/qTox/qTox/commit/2c5a38cc586ff811b5b039e073c4a60f48cc2880))
  - update French translation from Weblate ([55d1fbeb](https://github.com/qTox/qTox/commit/55d1fbeb42e0e83fe6ef5af0441eac4eb3d82592)), ([a24a6df5](https://github.com/qTox/qTox/commit/a24a6df55076d4b0cf2f3accfcba8065f45e8edb)), ([f5371032](https://github.com/qTox/qTox/commit/f5371032a4d40e528628159db996b0fbd12908cd))
  - update Japanese translation from Weblate ([f733f475](https://github.com/qTox/qTox/commit/f733f4755124dff89441c6e959dae888feb34c58))
  - update Spanish translation from Weblate ([8a8859b7](https://github.com/qTox/qTox/commit/8a8859b7a93a980e010b3e1081553142143afaf3))
  - update Czech translation from Weblate ([6b4fe28c](https://github.com/qTox/qTox/commit/6b4fe28c534fc12ce686240af682f5f31ef3f2f7)), ([037f2da8](https://github.com/qTox/qTox/commit/037f2da82bd4b573fa29e69d1e050dcb2458eac6)), ([3c8b8d63](https://github.com/qTox/qTox/commit/3c8b8d63449968904934ea791010d14ede146c6a)), ([a6d94e8f](https://github.com/qTox/qTox/commit/a6d94e8f839e34e73d7785ce2cf55f29d1713d7b))
  - add Slovak translation to interface and docs ([52ae7093](https://github.com/qTox/qTox/commit/52ae7093f7af9d26fde1cdb37ec7051bb6c16516))
  - add Slovak translation from Weblate ([ad176260](https://github.com/qTox/qTox/commit/ad176260d5d89412759a85755ef42b9b69d0fdaf))
  - update Belarusian translation from Weblate ([d898e7d4](https://github.com/qTox/qTox/commit/d898e7d40a4cac1b8e9c2cbbb8f7caa72c71d1a9)), ([226f4eaf](https://github.com/qTox/qTox/commit/226f4eaf4f98c9c53edd370f68b91c79c2acd895)), ([4296d377](https://github.com/qTox/qTox/commit/4296d377eec0f8a136b3d62eb06a5fb122c766a8)), ([60af778f](https://github.com/qTox/qTox/commit/60af778fb8d0fbd8373c4d994d56d3b21281212a)), ([0842b50b](https://github.com/qTox/qTox/commit/0842b50b760882014bfbf85c27583f22cf34f3b3))
  - update German translation from Weblate ([0a37f7ea](https://github.com/qTox/qTox/commit/0a37f7ea7151568e6bd02f5e57e40e3851310b61)), ([79a847f1](https://github.com/qTox/qTox/commit/79a847f14dd6d610ab88440f1ae200262e97de02)), ([1cb65ce4](https://github.com/qTox/qTox/commit/1cb65ce42249e36bfc6a33a5fb3d31bbde73369f)), ([a2e559d7](https://github.com/qTox/qTox/commit/a2e559d7597f63a2f519b90840f686834cd87a20)), ([7942206b](https://github.com/qTox/qTox/commit/7942206ba988eda20dbdcfa3f79d81fa1a5ef6c8)), ([00fa4d3b](https://github.com/qTox/qTox/commit/00fa4d3b6203e0d0355b86fc1a39215672ba9077))
  - update Hungarian translation from Weblate ([16e4198b](https://github.com/qTox/qTox/commit/16e4198b094d13a9be29ce9c806915e72a28ea15)), ([0e47ecda](https://github.com/qTox/qTox/commit/0e47ecdad5b40856dc32c82de75ef8a8cb02044a)), ([16b32e3b](https://github.com/qTox/qTox/commit/16b32e3bbf55b6b4b486a1e51444183da0c9423c))
  - update Russian translation from Weblate ([dd2a5dad](https://github.com/qTox/qTox/commit/dd2a5dadc57b13eba98e2dbb21893c3ec7d3550b)), ([628cfd23](https://github.com/qTox/qTox/commit/628cfd23222df3406ef0b39580565b75d94ae352)), ([0865217d](https://github.com/qTox/qTox/commit/0865217d468bf9084c4e3755ba11a42046eec5b4))
  - update Estonian translation from Weblate ([e0c71216](https://github.com/qTox/qTox/commit/e0c7121666570743674f949baf663eae1cc2905d)), ([ee2b93ae](https://github.com/qTox/qTox/commit/ee2b93ae21d3af9298a68a34adb0aeeaae9ccc10)), ([bed51e88](https://github.com/qTox/qTox/commit/bed51e884c1971c2487b4119dcb14584a9dfedd0)), ([018a88df](https://github.com/qTox/qTox/commit/018a88dfe6e4e0ef57b5c476f8ef06c7c720230c)), ([d8a525ac](https://github.com/qTox/qTox/commit/d8a525acb3ae68f98428a111ab366c08c97916ea)), ([f6aa3085](https://github.com/qTox/qTox/commit/f6aa3085ef5d47e1073bd5032d622e4244770dc9))
  - update Greek translation from Weblate ([ba2d62fe](https://github.com/qTox/qTox/commit/ba2d62feeddf7e054b9ec10f92ca997814d48b94))
  - update Portuguese translation from Weblate ([9e39708e](https://github.com/qTox/qTox/commit/9e39708e76bc583025491be245879c1034e98c0e))
  - update Polish translation ([7fc98a9e](https://github.com/qTox/qTox/commit/7fc98a9ea7292dea74adb38d533fb3f74992a261))
  - update Russian translation from weblate ([fb418c64](https://github.com/qTox/qTox/commit/fb418c64836e562a11f298319520add2cdc160ff))
  - update Danish translation from Weblate ([ab312318](https://github.com/qTox/qTox/commit/ab3123187df057357356891c7d51db1823cd7c94))
  - update Arabic translation from Weblate ([7d704e8d](https://github.com/qTox/qTox/commit/7d704e8dc8e4830e85dba6711e58261ff5124f1d))
  - update Norwegian (old code) translation from Weblate ([548df204](https://github.com/qTox/qTox/commit/548df204979a3805795e5aed661767d38041f0f7))
- **reconnect:** Made `Reconnect` button wider ([4a5f89bb](https://github.com/qTox/qTox/commit/4a5f89bb98fc8657514e35c5022713358a453d49))
- **settings:**
  - Export and copy debug log ([3c6bd043](https://github.com/qTox/qTox/commit/3c6bd043a04edb0ae76444f5d54ce8eaf5fb0b14), closes [#2890](https://github.com/qTox/qTox/issues/2890))
  - Moved proxy to personal settings ([4dfe3ec2](https://github.com/qTox/qTox/commit/4dfe3ec22636150074a0afc73644582e14f744ff), closes [#2465](https://github.com/qTox/qTox/issues/2465))
  - Added privacy block ([9499bdd4](https://github.com/qTox/qTox/commit/9499bdd45877f20bcb8f82606a5db7bc6e3a2db5))
  - Deleted settings header ([a1041ed1](https://github.com/qTox/qTox/commit/a1041ed11a27e9b9de617a7e11c907d83ba55952))
  - Added reset settings button ([9c9f1c11](https://github.com/qTox/qTox/commit/9c9f1c11d1fece24402384dbd6b41d1c05aec00b))
  - Extracted user interface settings on new tab ([fb4aa4c8](https://github.com/qTox/qTox/commit/fb4aa4c8f628b47083402221380172951ab41532))
  - add notification signals for changed settings values ([f00b9008](https://github.com/qTox/qTox/commit/f00b9008e669905e6178c84f4b2e81456ee9cb5e))
- **smileys:**
  - try to load smileys from `XDG_DATA_DIR` directories ([29da2210](https://github.com/qTox/qTox/commit/29da2210c98a2f1bf2dcaf6af18ba818ceef5d2c))
  - allow compiling qTox with minimal or no smileys ([ae769106](https://github.com/qTox/qTox/commit/ae76910631a9cc15dd9673d5dfc0392aeffeb459))
- **translation:** add update language Spanish ([a7c46fff](https://github.com/qTox/qTox/commit/a7c46fff2de152c77b54cd54aa8f9a209077b05e))
- **uiform:** Separated time and date format and examples ([d909d99b](https://github.com/qTox/qTox/commit/d909d99bd22e2ba35459c7bb3be0fa8724f97346))

<a name="v1.5.2"></a>

## v1.5.2 (2016-09-29)

### Release notes

Rebuilt binaries to fix a FFmpeg bug

<a name="v1.5.1"></a>

## v1.5.1 (2016-08-09)

### Release notes

No changes in qTox itself.

#### Bug Fixes

- **build:** allow deployment of OSX dmg binary ([4975ca9b](https://github.com/qTox/qTox/commit/4975ca9bb5d94955ada72939580e3652a6f36032))

<a name="v1.5.0"></a>

## v1.5.0 (2016-08-09)

### Release notes

The most important change is video improvements. Bored by waiting minutes for
video call to start? Fixed.

Among other things, qTox has been translated into 5 new languages.

More information on features / fixes / changes below.

#### Features

- **audio:**
  - add slider tickmarks, improving better visible orientation ([431a10f8](https://github.com/qTox/qTox/commit/431a10f82b6370153aa843856d109287078b0dc5))
  - add real gain control of the input device ([f72baa61](https://github.com/qTox/qTox/commit/f72baa613f6f15de454783eeb4e709c691aef4ad))
- **avform, screenshotgrabber:** Added custom screen region selection ([9cfd678c](https://github.com/qTox/qTox/commit/9cfd678c262b223413dab30d656aff50ae7ce470))
- **bootstrap.sh:** add an option to install sqlcipher ([66f270ec](https://github.com/qTox/qTox/commit/66f270ecad591fc6e5547af12753358a63c4b171))
- **cameradevice, avform:** Added ability of screen selection ([d781a4f7](https://github.com/qTox/qTox/commit/d781a4f762a75a5766d9ca534ef0f034bf332ea0))
- **camerasource:** Change default video mode to preferred ([c3de6238](https://github.com/qTox/qTox/commit/c3de6238ca5efa9e42b484a755934c986d0d4b6e))
- **capslock:** Added caps lock checker ([97f95e7e](https://github.com/qTox/qTox/commit/97f95e7e91a5dc8f5b1136238efc7c5cb10d55f4))
- **capslockindicator:** Added tooltip ([c7d485f5](https://github.com/qTox/qTox/commit/c7d485f515e3ce3660f04b095434dc37f2c33b23))
- **chat:**
  - add the ui settings to alter font and size for chat messages ([41c96eb1](https://github.com/qTox/qTox/commit/41c96eb15962306e1da02c69a1f515b5223fd270))
  - consistent use of the chat font settings ([67136e29](https://github.com/qTox/qTox/commit/67136e29ed0027fe8942033032bda437ecdc0e0b))
  - add settings to alter the chat view's base font ([8ba20541](https://github.com/qTox/qTox/commit/8ba205419048a039a8dbc84c87dfa4c6b2cb1252))
- **chatform:** Disable call buttons if friend is offline ([bbefe011](https://github.com/qTox/qTox/commit/bbefe0119d405cf7bcc0e7a90571b1c18b993817))
- **doxygen:** Created simple doxygen config file ([194c55a4](https://github.com/qTox/qTox/commit/194c55a4c5c217c060ca78e11eec8c3b349aeffe))
- **emoticons:** add ASCII-less version of emojione emoticons ([c4b4155a](https://github.com/qTox/qTox/commit/c4b4155a53d72fbcad7475590d860c94bea336b3), closes [#3398](https://github.com/qTox/qTox/issues/3398))
- **emoticonswidget:** Keep emoticon option open ([d0ea5bb4](https://github.com/qTox/qTox/commit/d0ea5bb4fd6e5ffdb2bb11c4e63d1c518af36b65))
- **genericchatform:** add "Quote selected text" feature to chat window ([40a805c2](https://github.com/qTox/qTox/commit/40a805c2fd66d7c5cd618fb4974dcd65bf7df650))
- **gui, setpassworddialog:** Added buttons translation ([58e503bb](https://github.com/qTox/qTox/commit/58e503bb14e747b16a30c22dffc7397999098bac), closes [#3479](https://github.com/qTox/qTox/issues/3479))
- **i10n:** update Hungarian translation from weblate ([c6938d6c](https://github.com/qTox/qTox/commit/c6938d6c4e809b7d99357641914c9f08dea3ffb6))
- **i18n:**
  - Make activity by time labels translated by locale ([f2aada8f](https://github.com/qTox/qTox/commit/f2aada8f4fd404947ca4fa2d34e00df45c25f76f))
  - make Markdown settings translatable ([3e22593a](https://github.com/qTox/qTox/commit/3e22593ae71f388972a59079ccf6d719f980d035))
- **importProfile:** Add way to import profile ([9ea25d1f](https://github.com/qTox/qTox/commit/9ea25d1fbd928657cfcc0e73e868783f866e5ea9))
- **l10n:**
  - update French translation from weblate ([525db227](https://github.com/qTox/qTox/commit/525db2276a49ec24a17b89966a626a1252ce2b13)), ([5a147646](https://github.com/qTox/qTox/commit/5a147646c59a21a27cc342e454a6c6b2078035ab)), ([f6f336a7](https://github.com/qTox/qTox/commit/f6f336a7a7a40eac4fc7210d60a020d9cecac4b4)), ([d9fc36db](https://github.com/qTox/qTox/commit/d9fc36db4bf097b02e37fa8874432ed44309cd8e))
  - update German translation from weblate ([2b65fac3](https://github.com/qTox/qTox/commit/2b65fac36f5c04878dded3a456832ebe9f4cf1ca)), ([962206db](https://github.com/qTox/qTox/commit/962206db769112c7e2ee35743d31081bfa3f42f8)), ([351c4166](https://github.com/qTox/qTox/commit/351c4166b26bd93877cea973212043f42513ad18)), ([65019117](https://github.com/qTox/qTox/commit/6501911730c1937f0b71116898e4023f4a6fc039))
  - update Estonian translation from weblate ([ba0d7ec7](https://github.com/qTox/qTox/commit/ba0d7ec76814ceb4b272cefddf15e427a94af7e2)), ([6d7d9c33](https://github.com/qTox/qTox/commit/6d7d9c33a5c3d2a2948832f2d9a5807865537c3d)), ([c6fba9c5](https://github.com/qTox/qTox/commit/c6fba9c54872a35f708f9344647a4258d206dbb6)), ([9c8335fa](https://github.com/qTox/qTox/commit/9c8335fa16eb8ab7e0a74b8585561816f0cc6285))
  - update Belarusian translation from weblate ([1b16466c](https://github.com/qTox/qTox/commit/1b16466cf924576e09ed92691eb82c7d6300b439)), ([97d8c7a1](https://github.com/qTox/qTox/commit/97d8c7a108f3f6541eacd6c8dda3eb11c0f0813a)), ([526f13aa](https://github.com/qTox/qTox/commit/526f13aa0df27eba245918b6b15f1f981e29a1e2)), ([44420953](https://github.com/qTox/qTox/commit/444209537828eb4aab20ed47f7ec304863c3e48d)), ([7c6ba752](https://github.com/qTox/qTox/commit/7c6ba75200e8e3cd5461d99d85002ce1ccd1e488)), ([daabda84](https://github.com/qTox/qTox/commit/daabda84c47c4ea5a29c45e045689b917fa9c750)), ([f2c19912](https://github.com/qTox/qTox/commit/f2c19912c46c89f212bb6c809d6a54619018252d))
  - update Ukrainian translation from weblate ([3a5e91a2](https://github.com/qTox/qTox/commit/3a5e91a20854f24a33e4be422054309f993f88b1))
  - add Lojban translation from weblate ([237351fd](https://github.com/qTox/qTox/commit/237351fdd28841a61fcc733d9f039471fcf05e40))
  - update Czech translation from weblate ([d951cb75](https://github.com/qTox/qTox/commit/d951cb7589ce9e3908b36deeac151797453576b1)), ([1e9efbfe](https://github.com/qTox/qTox/commit/1e9efbfe694f0eabdc42edb7901d4282c28c6498)), ([8d94ca92](https://github.com/qTox/qTox/commit/8d94ca92275b552e39cc91f8c48cf96f0e0fc0e5)), ([83f874e5](https://github.com/qTox/qTox/commit/83f874e5ceb1ac814974a927d415a18c6c98b95f))
  - update Bulgarian translation ([10d913ee](https://github.com/qTox/qTox/commit/10d913ee4ac80c54f79060915e37f32eb14ec385)), ([b6b149a7](https://github.com/qTox/qTox/commit/b6b149a756841ca901dee893992239161d1d7399))
  - update Russian translation from weblate ([1826e2ae](https://github.com/qTox/qTox/commit/1826e2aebb2458f90c91bdd8168a78c1b5569ee5)), ([31ecfd8b](https://github.com/qTox/qTox/commit/31ecfd8b12996cea6439e03dd97c54b952a6436f)), ([29dbd030](https://github.com/qTox/qTox/commit/29dbd030765a5f382ceb0e386fe381cfc51ee5de)), ([861cf7d9](https://github.com/qTox/qTox/commit/861cf7d93de8800224c145f6806dc986cd237874)), ([379aaa0f](https://github.com/qTox/qTox/commit/379aaa0fdb85a0fd384110a387e10aa7de49d17f))
  - add Uighur language to UI ([93015129](https://github.com/qTox/qTox/commit/9301512936ab9e96217b8b5e8869c348c0548972))
  - add Uighur translation from weblate ([3ee8f72a](https://github.com/qTox/qTox/commit/3ee8f72a6c79e53d19d0083d1d0d2c3f0b01d97c))
  - update Hungarian translation from weblate ([9bc642ee](https://github.com/qTox/qTox/commit/9bc642eee9def1a7c594216c1a6c23e09fc5c18f)), ([0c3f3817](https://github.com/qTox/qTox/commit/0c3f3817c3e9e18abc168761f5d60b6de850aac0))
  - update Polish translation from weblate ([88b839c1](https://github.com/qTox/qTox/commit/88b839c1af1e0b5fabb7a6ac8034d208620cc638))
  - update Lithuanian translation from weblate ([281d94ef](https://github.com/qTox/qTox/commit/281d94ef4e9db5edf1fe208a834f2a5648a8b4d9))
  - update Bulgarian translation from weblate ([6052364b](https://github.com/qTox/qTox/commit/6052364bca754ac3e49638737b76c597fe158e8b)), ([e0b41d57](https://github.com/qTox/qTox/commit/e0b41d5764d76c35e627f3b23a24fffd265b2d49))
  - update Lithuanian translation ([9d108840](https://github.com/qTox/qTox/commit/9d10884029f1dae69fdad3ba03f691acf2da5d18)), ([0bb416cd](https://github.com/qTox/qTox/commit/0bb416cd76b8191a57d8dcd24d135972f64c024c))
  - update Italian translation from weblate ([e6f870f4](https://github.com/qTox/qTox/commit/e6f870f4b408e4df3f95e0159adb1f5e3d0169e5))
  - update Dutch translation from weblate ([8ac47bf0](https://github.com/qTox/qTox/commit/8ac47bf06b038cd7decf62a114347579488559a7))
  - add initial Belarusian translation from weblate ([684835de](https://github.com/qTox/qTox/commit/684835de1b3a26aac83d1fa99f2bb5a78eab411e))
  - update Estonian translation from Weblate ([85a701f5](https://github.com/qTox/qTox/commit/85a701f5f820874d0f1c2993f8ffffc124bc030f))
  - add initial Esperanto translation from Weblate ([7971975c](https://github.com/qTox/qTox/commit/7971975cbe5f53ad328bb8af76b09170d341f869))
  - update Italian translation ([7d308f99](https://github.com/qTox/qTox/commit/7d308f99ce306333a1c285633070bfe8cab45bf7)), ([e7089a3d](https://github.com/qTox/qTox/commit/e7089a3d1a46c799c6c8e7b9f149306a8d59272d))
  - Update French translation ([402f9eb9](https://github.com/qTox/qTox/commit/402f9eb936ac3b1bad75ea35a6bdf9e8e9a0eac3))
  - update French translation ([2a368436](https://github.com/qTox/qTox/commit/2a368436dbed906d928d44771aa96d93e64951e4))
  - Update Arabic translation from Weblate ([91af5c95](https://github.com/qTox/qTox/commit/91af5c951a51e6398e74dd4c85065a942a0a5da4))
  - Update German translation from Weblate ([8a0a8f1f](https://github.com/qTox/qTox/commit/8a0a8f1f75ef80019165f208485e6f567a30cbed)), ([1107b642](https://github.com/qTox/qTox/commit/1107b6421be5a2ef3f945b2c78456fba42e31c27))
  - Update Lithuanian translation from Weblate ([e19f4c70](https://github.com/qTox/qTox/commit/e19f4c70092b6e9e022da45a6b594014db837a10))
  - Update Russian translation from Weblate ([d4ff03c8](https://github.com/qTox/qTox/commit/d4ff03c82b66a7efddcb453a53c5898f7625075b)), ([6beea2bd](https://github.com/qTox/qTox/commit/6beea2bda594ac9d32144765e4ba3873e102684b)), ([0856d4dd](https://github.com/qTox/qTox/commit/0856d4dd13dc61b7b4ff8c69b01eab4a6f56f430)), ([21b5cc3f](https://github.com/qTox/qTox/commit/21b5cc3f9db21068eef0b7138d362d7295b1e5ed))
  - Update French translation from Weblate ([774f3c16](https://github.com/qTox/qTox/commit/774f3c1641cdf0ece2e5a8193a1468bcc3229031)), ([4b42a6db](https://github.com/qTox/qTox/commit/4b42a6dba385da8e730f028291e06afae079e4d7))
  - Update Estonian translation from Weblate ([2cd35e17](https://github.com/qTox/qTox/commit/2cd35e17360dd1a9b66f0eb5d0ed163c1903895b)), ([4137a19f](https://github.com/qTox/qTox/commit/4137a19fbc1286f5c66e1814c3b0cde94fdd0617))
  - Update Polish translation ([e7c0159f](https://github.com/qTox/qTox/commit/e7c0159fce86df7b9c2f7627faea41425b432f55))
  - Update Japanese translation ([75d64dc6](https://github.com/qTox/qTox/commit/75d64dc68dc86fe8479e5c48dd54f375e4c63d34))
  - Update Polish translation from Weblate ([6f074061](https://github.com/qTox/qTox/commit/6f074061cb02c9013ab6ccb493755509485a73e6)), ([a49e7f27](https://github.com/qTox/qTox/commit/a49e7f276a9a528d34d29245c21a5886108ff2b8))
  - update Polish translation ([9a3ba021](https://github.com/qTox/qTox/commit/9a3ba02145a3357c1e4f3044372d03dfebcfb568))
  - Update Chinese translations from Weblate ([fe432dea](https://github.com/qTox/qTox/commit/fe432dead428cbdd9d18b93fd60a9744a1ea210d))
  - Add Hebrew translation from Weblate ([83b89f12](https://github.com/qTox/qTox/commit/83b89f1233c49e3c8d92a2368f4b0cb5df870937))
  - Add Danish translations from Weblate ([c8c7bda3](https://github.com/qTox/qTox/commit/c8c7bda38e7e9167c0eab084452df192b8485f59))
  - Update Ukrainian translation from Weblate ([2ab5af56](https://github.com/qTox/qTox/commit/2ab5af566f6030425b596b84af5274a5fa3d0e4d))
  - Update Norwegian translation from Weblate ([1466fbf5](https://github.com/qTox/qTox/commit/1466fbf554ba810b07305969a55a0f5425df488e))
  - update zh translation. ([f8ee4484](https://github.com/qTox/qTox/commit/f8ee448412b5bca7a97a106d9e2507fe63bce998))
  - add Japanese translation ([d06efd38](https://github.com/qTox/qTox/commit/d06efd387bdaa85143e6924352a0bcc038537a98), closes [#3223](https://github.com/qTox/qTox/issues/3223))
- **loginform:** Added caps lock indicator to newPass ([cbe8fb8e](https://github.com/qTox/qTox/commit/cbe8fb8ef989e6e178343f1d5d5e016a62ce1d70))
- **loginscreen:** Created new CapsLockIndicator class ([fb7fcaaa](https://github.com/qTox/qTox/commit/fb7fcaaa8ca98fe32ebdc3d4cab2834ac951b50a))
- **main:** Changed time in logs on UTC. ([4018c004](https://github.com/qTox/qTox/commit/4018c0041b179bbd03f152c43ab0e70dbf00a9f0), closes [#3554](https://github.com/qTox/qTox/issues/3554))
- **notificationscrollarea:** Add ability to delete widget from traced widgets list ([e3d74117](https://github.com/qTox/qTox/commit/e3d74117caa1e1289879388640797a1a9594c35a))
- **profile:**
  - add a dialog to indicate profile deletion error ([78fd245e](https://github.com/qTox/qTox/commit/78fd245e4cb0a73570b5d6a24d612ac099dd16ef))
  - show warning on failure to delete profile ([1dabbca9](https://github.com/qTox/qTox/commit/1dabbca94c172bf21f83e23400ef99e5be45c084))
- **profileform:** Added log toxme errors ([d2d5b230](https://github.com/qTox/qTox/commit/d2d5b2306450032ae485909dc0c480d3c02b3596))
- **settings:** add RecursiveSignalBlocker ([b71c919f](https://github.com/qTox/qTox/commit/b71c919f2a5cc92cdb8c0f7b0d3a76e268244549))
- **settings, generalform, widget:** Added setting for sound notification in busy status ([e23eb1c5](https://github.com/qTox/qTox/commit/e23eb1c5f770b84a41ab061c2e6163ea8c9337a9))
- **smileys:** add emojione emoji-pack and make it the default ([3f4a0abe](https://github.com/qTox/qTox/commit/3f4a0abe6b10ddf335c89a6220e079bb858c9fd6), closes [#3315](https://github.com/qTox/qTox/issues/3315))
- **status:**
  - make status in about dialog copyable ([1935d0c6](https://github.com/qTox/qTox/commit/1935d0c6ec781b7cdb14a427a670aef5910e4df0))
  - add ability to copy status messages ([57ce030f](https://github.com/qTox/qTox/commit/57ce030f1d19f4a16f309149addc80bed53b8b2d))
- **systemtray:** add "Show" action to context menu ([a851a5b1](https://github.com/qTox/qTox/commit/a851a5b18da8c295fba441c817920cf65db2cdbb))
- **textstyle:** Change markdown syntax to be more intuitive ([32e48a97](https://github.com/qTox/qTox/commit/32e48a979ca78717a212800547c95ca0f1e67b8f), closes [#3404](https://github.com/qTox/qTox/issues/3404))
- **translation:**
  - add update language Spanish ([090a715b](https://github.com/qTox/qTox/commit/090a715b4cf2fc731d864b7de7fdeeec47d33e05)), ([17f43668](https://github.com/qTox/qTox/commit/17f43668a417192155d9c41be5b6cb6550728d5e)), ([f81f20f0](https://github.com/qTox/qTox/commit/f81f20f0cfba9e0bdec7599e0ae9eca55bf14e5a))
  - Update for German translation file ([beca3a9c](https://github.com/qTox/qTox/commit/beca3a9c45a65f5085bd6d8c1dff91a73215619f))
  - add update language German ([750d1b50](https://github.com/qTox/qTox/commit/750d1b50cc5f05fce2348bf352d2565201cd5d3c))
- **video:**
  - add a isValid() function to ToxTUVFrame ([3e7d55e8](https://github.com/qTox/qTox/commit/3e7d55e8337fa6a3f97216a7c49379354526d95b))
  - add accessor functions for sourceID and frameID ([8952208e](https://github.com/qTox/qTox/commit/8952208e8acde6d3299192dd18123ceb01bf0383))
  - adds an ID parameter to the VideoSource class ([4ac20c7b](https://github.com/qTox/qTox/commit/4ac20c7b460b5207237bcfe0becd10b8b3be0f60))
  - redesign and improve VideoFrame class ([38b1a9b6](https://github.com/qTox/qTox/commit/38b1a9b63dabdd9b71e7824c602f9ff77e99eb1e))
  - add setting for 120p very-low-res video ([6045ced3](https://github.com/qTox/qTox/commit/6045ced3f8b538b0c3044370878d4feb76103480))
- **videomode:** Added possible video shift ([fd701df1](https://github.com/qTox/qTox/commit/fd701df1012763cba98bbfbbf7bf9ccd898f1c03))
- **widget:** Disable sound notification for `busy` status ([e7785ab4](https://github.com/qTox/qTox/commit/e7785ab4c2790b2c10b33c416bc78ad23a16cc63), closes [#2893](https://github.com/qTox/qTox/issues/2893))

#### Bug Fixes

- Added forgotten return statement ([8bc3c3c8](https://github.com/qTox/qTox/commit/8bc3c3c8d280df306127e6556c451d0159d55fe3))
- increase timer for checking offline messages timeout (again) ([a77afca1](https://github.com/qTox/qTox/commit/a77afca1ec8f86cda68fe7ba853f7e4d854c346f))
- correctly tab-complete nicks starting with `$` ([dbd16ae6](https://github.com/qTox/qTox/commit/dbd16ae6a362809e8fc2936968cd7f0677b7e6f0))
- **.gitattributes:** bootstrap.sh execution fails on MSYS ([ad828621](https://github.com/qTox/qTox/commit/ad828621359963e6eaadb07cf2b834eb9d53cf01))
- **about-qtox:** fix QString "missing argument" warning ([f2f48a8f](https://github.com/qTox/qTox/commit/f2f48a8f07106dd4c3828e202b184062ab39798f))
- **addfriendform:** Fixed problem with reading friend request ([7be8ad01](https://github.com/qTox/qTox/commit/7be8ad01da50c9f95312dea4a5e56ec2df8517a2), closes [#3421](https://github.com/qTox/qTox/issues/3421))
- **audio:** actually disable the audio in/out device in settings, when selected ([9694d6b6](https://github.com/qTox/qTox/commit/9694d6b6d434bb7557c9766e89dce5fb4cb89502))
- **avform:**
  - display true video height in video mode selection ([192c1e8f](https://github.com/qTox/qTox/commit/192c1e8ff53e6c29c92d4e99107c70c9f9ae00cc))
  - add missing "first" video mode back to video modes ([5324e768](https://github.com/qTox/qTox/commit/5324e768c31643fa1741bf94105c4604be482a7c), closes [#3588](https://github.com/qTox/qTox/issues/3588))
  - Add skipped camera open call ([1f9b7b13](https://github.com/qTox/qTox/commit/1f9b7b13de54668e0dbe185fbbc3b09c369b5f77), closes [#3476](https://github.com/qTox/qTox/issues/3476))
  - Added rounding height in mode name. ([c2e3358d](https://github.com/qTox/qTox/commit/c2e3358dd2eca3839a5f9fc858d8f4546dc842a2))
  - Changed "best modes" search algorithm. ([6e1ef706](https://github.com/qTox/qTox/commit/6e1ef70651647d01e55057d48bd5d482b8530777))
  - initialize slider value from settings ([c9dbfa5e](https://github.com/qTox/qTox/commit/c9dbfa5eacd07c58d234c73dc58d91d1ad5a8f12))
  - make "Screen" translatable ([24f0b11a](https://github.com/qTox/qTox/commit/24f0b11a4d9d9020f7146c0c94b0b65de89a9d0f))
  - Added restoring selected region ([1c515821](https://github.com/qTox/qTox/commit/1c5158213dd6f38acdcf202e660b6e383764c436))
  - Took default resolution from middle of list ([2d861ee2](https://github.com/qTox/qTox/commit/2d861ee25b2f6ac5607005ea89f6866d7321e66d))
- **bootstrap.sh:** add instructions for missing unzip & adjust path ([fa5ee5b1](https://github.com/qTox/qTox/commit/fa5ee5b1adbfa2cfde3595cdee1a42ca4cac8a11), closes [#3153](https://github.com/qTox/qTox/issues/3153))
- **build:**
  - Link qrencode statically on Jenkins ([0a976c7a](https://github.com/qTox/qTox/commit/0a976c7a50649ae0874bbf341a273b669901a183))
  - Jenkins ffmpeg link order ([9de833ad](https://github.com/qTox/qTox/commit/9de833ad396b5842431fdbb3908f4d3ead2522aa))
  - Fix jenkins static builds ([790f9ffc](https://github.com/qTox/qTox/commit/790f9ffc670997172fffad2e4a6c4eaf794aed71))
- **capslock_x11:** properly release the X display handle ([f9190734](https://github.com/qTox/qTox/commit/f9190734d727a2a1cb424629caf0fb3a77f3f7e9))
- **capslockindicator:**
  - also update indicator when the app gets focus ([2fe41071](https://github.com/qTox/qTox/commit/2fe41071bedb689ba1776affe97e50138b9d6984))
  - fix altering the line edit height ([653e0b5a](https://github.com/qTox/qTox/commit/653e0b5af2dc176e80eb617df89f32bf8a00427e), closes [#3379](https://github.com/qTox/qTox/issues/3379))
  - Fix position of capslock indicator ([12476901](https://github.com/qTox/qTox/commit/124769017d5c5c2a70cffd09ef9c5916bb98ef55))
  - Tooltip color was changed. Tooltip translation was added ([bbe158c7](https://github.com/qTox/qTox/commit/bbe158c7d99dff54f2f96cd1ed7f31b8cfc2816a))
- **chat:** cleanup chat css base style ([989b15e6](https://github.com/qTox/qTox/commit/989b15e6560c2bd8fbdfafcb4b9736a49af72774))
- **chat window:** prevent right click from opening chat window ([b9a392d5](https://github.com/qTox/qTox/commit/b9a392d59ee2760a68712b835e21375425bdaebe), closes [#3205](https://github.com/qTox/qTox/issues/3205))
- **chat-settings:** change order during chat-base-font UI initialization ([9ec5c11c](https://github.com/qTox/qTox/commit/9ec5c11caaf2d9ee5bce2ce120b5397a2bfd1be3))
- **chatform:**
  - Fixed call buttons ([dbe0a159](https://github.com/qTox/qTox/commit/dbe0a1596376a3edc4c1c10d604c9aaa54c8af73), closes [#3521](https://github.com/qTox/qTox/issues/3521))
  - Markdown after emojis ([998f0915](https://github.com/qTox/qTox/commit/998f0915db515851d92e32873e6ac4c528cd9a16), closes [#3260](https://github.com/qTox/qTox/issues/3260))
- **chatform, screenshotgrabber:** Fixed memory leak ([bf7c62d6](https://github.com/qTox/qTox/commit/bf7c62d6fa57b3f8b5078d55bf20372fe6aa47b7))
- **chatlog:** Don't delete active transfer widget ([abf7b423](https://github.com/qTox/qTox/commit/abf7b42324f85c00ca1105ac11a8c989e8488aa2), closes [#3275](https://github.com/qTox/qTox/issues/3275))
- **chattextedit.cpp:** fix drag-and-drop to be consistent across systems ([70fc247b](https://github.com/qTox/qTox/commit/70fc247b7092b0bd320b3a59bcaf57d896257da8), closes [#2847](https://github.com/qTox/qTox/issues/2847))
- **contentdialog, widget:** Remove "new message" bar after reading message ([b2c1f468](https://github.com/qTox/qTox/commit/b2c1f468946aff73b23fb9ac6539331f6c0c7fbe), closes [#2247](https://github.com/qTox/qTox/issues/2247))
- **core:** Added emit idSet after set NoSpam ([e087398f](https://github.com/qTox/qTox/commit/e087398f566eeb403b1b9f43b324e6bf1da42ec1), closes [#1680](https://github.com/qTox/qTox/issues/1680))
- **corevideosource:** Partial revert of ef641ce6d3398792c10b30bf24a81c5a6005fe06 ([b1adef2f](https://github.com/qTox/qTox/commit/b1adef2fd0b9aeef58cea34c6637b8cd3f6e16a8), closes [#3527](https://github.com/qTox/qTox/issues/3527))
- **directshow:** Fixed problem with crosses initialization ([504ad534](https://github.com/qTox/qTox/commit/504ad534e0e9d27077a49222f2c7f9e0d568b22d))
- **doc:** CONTRIBUTING.md typos ([4eed2549](https://github.com/qTox/qTox/commit/4eed2549aaef7c72f2e0ddf92696f35f209ae1a4))
- **friendlistwidget:** use nullptr instead of `0` ([f1543144](https://github.com/qTox/qTox/commit/f1543144be7726a9d2dbb6e04ca9b0a4c1000737))
- **friendwidget:**
  - check pointer before using it ([e24f3dbc](https://github.com/qTox/qTox/commit/e24f3dbc20e066606a7d4ad7fe76a0cc9a078d4f))
  - the limitation of the group's name in the shortcut menu ([d357fe1c](https://github.com/qTox/qTox/commit/d357fe1c650f57f4dad34294b544640f2d06eb88))
- **generalform:** call UI retranslation when date or time format changes ([d601599d](https://github.com/qTox/qTox/commit/d601599de8c2d18b98f5fabd1b8ac78468fada8e))
- **genericchatform:**
  - Fixed position of screenshot button ([86e44143](https://github.com/qTox/qTox/commit/86e44143ad7e473198ff6aa340be4136f2cca569), closes [#2427](https://github.com/qTox/qTox/issues/2427))
  - separate messages from different days ([8ebad59a](https://github.com/qTox/qTox/commit/8ebad59a3e3aa42324a5821760b0752546a1e5ac))
- **groupinviteform:**
  - escape HTML ([e4bc8570](https://github.com/qTox/qTox/commit/e4bc857037cf7cecd88929af8af453a800af2201))
  - consider dateTime format in group invites ([6030b083](https://github.com/qTox/qTox/commit/6030b083b131cb367971eb205403d80764c2564e), closes [#3058](https://github.com/qTox/qTox/issues/3058))
- **gui:** don't quit application before last event ([32c158bc](https://github.com/qTox/qTox/commit/32c158bc61f778ae464e93178a66f8e95b6af896))
- **i18n:** Divide getting and translating Toxme error message ([98a1f23b](https://github.com/qTox/qTox/commit/98a1f23bfbbe341c50ed9ee2cc52c1f9dfc15110))
- **l10n:**
  - remove unnecessary space in Czech translation ([47153b3d](https://github.com/qTox/qTox/commit/47153b3d77011c4a944bd597ad3a059e9668ed7c))
  - missing argument in German translation ([e6e666fa](https://github.com/qTox/qTox/commit/e6e666fa8ce36cd3792d4536a59ab2b65fd5b546))
  - incorrect/missing arguments in Arabic translation ([82bd897b](https://github.com/qTox/qTox/commit/82bd897ba46e388ecdb60c16484e056a607e45fa))
- **loginscreen.cpp:** fix password input focus after mouse click ([6e8ea15a](https://github.com/qTox/qTox/commit/6e8ea15a15c82ce29a5d0d1267d699af75a5f3cd))
- **main:** Closing file before removing ([29ab61ef](https://github.com/qTox/qTox/commit/29ab61efdf2e4c335c9f32d3552934bc59057830), closes [#3440](https://github.com/qTox/qTox/issues/3440))
- **markdown:** Remove spaces from markdown translation ([fca5f155](https://github.com/qTox/qTox/commit/fca5f15532ddbc21d70e6295d0742218cbc6fe1e))
- **passwordedit.cpp:** Fix build issue with Qt 5.3 ([f18db4fd](https://github.com/qTox/qTox/commit/f18db4fd508c170fb93ae1e04fc7f2e0b5486bb7), closes [#3416](https://github.com/qTox/qTox/issues/3416))
- **passwordfields:** use PasswordEdit widget for all password fields ([e3d0cc0e](https://github.com/qTox/qTox/commit/e3d0cc0e55ee91d85942b3beddac716435b78bee), closes [#3378](https://github.com/qTox/qTox/issues/3378))
- **persistence:** pass by reference to const ([e852ed92](https://github.com/qTox/qTox/commit/e852ed92f6a83d30d824cf3b35a8d3f68537fe9c))
- **platform:** Added checkCapsLock OSX implementation ([35a0e1fb](https://github.com/qTox/qTox/commit/35a0e1fb6f0e0417ddc24408904a334d95e05c65))
- **profile:**
  - Fix for opening file dialog using Nautilus file manager ([881409b9](https://github.com/qTox/qTox/commit/881409b91fe133ae53b2e0344a6d764fd0d0a6d7), closes [#3436](https://github.com/qTox/qTox/issues/3436), closes [#3443](https://github.com/qTox/qTox/issues/3443))
  - change password buttons behaviour ([f9edd39b](https://github.com/qTox/qTox/commit/f9edd39bba64a0fd92c6e9820f3265618d2057fc), closes [#3300](https://github.com/qTox/qTox/issues/3300))
- **profileform:** set parent for validator ([93c6aa8a](https://github.com/qTox/qTox/commit/93c6aa8ac0519110e4de249346811e60dbb006c6))
- **qtox.pro:** don't depend on GTK in order to build on Linux ([2d06b996](https://github.com/qTox/qTox/commit/2d06b9960c60f1fc886ae04c595bd9a921cff316))
- **screen-grabber:** fix crash ([780a0179](https://github.com/qTox/qTox/commit/780a017928cfa5613fddada6e787bd6f3965baa9))
- **settings:**
  - block signals during initialization ([de438f1c](https://github.com/qTox/qTox/commit/de438f1c2328b67a26c141fdfeae6049fc80580d))
  - Look for portable setting in module path, not CWD ([17e57982](https://github.com/qTox/qTox/commit/17e57982dffa0dabffb7839cfd34fb34ef51ec88))
  - correct ordering of languages ([7c63594a](https://github.com/qTox/qTox/commit/7c63594adf16d033c2ad40b369bb79fa98f8a95b))
  - make it clear that `Markdown` is about text formatting ([67d01a73](https://github.com/qTox/qTox/commit/67d01a73c440f6b4ffeb9d5f0dbf560c832cf41f))
- **simple_make.sh:**
  - add sqlite dependencies for Fedora ([5cb271b0](https://github.com/qTox/qTox/commit/5cb271b0c0ce2d3a1700d05559c7a0a3ce34af25))
  - add missing dependencies for Fedora ([5b51f71f](https://github.com/qTox/qTox/commit/5b51f71ff86a0784aa0e74e13face02eda5d6c47), closes [#2998](https://github.com/qTox/qTox/issues/2998))
- **status:** explictly refer to this within lambda ([63af0d75](https://github.com/qTox/qTox/commit/63af0d7500f98db4d7e2ada78166730ffc420249))
- **systemtray:** don't activate qTox widget on tray icon click in Unity backend ([2f0ffdd2](https://github.com/qTox/qTox/commit/2f0ffdd27e8750f1554d0d10174ce3e831e2ac9a))
- **systemtrayicon:**
  - don't set an invalid and useless icon on GTK ([a13c5667](https://github.com/qTox/qTox/commit/a13c566736b11880e43fe3af870b521175aeabe7), closes [#3154](https://github.com/qTox/qTox/issues/3154))
  - delete qtIcon in destructor ([f997d51b](https://github.com/qTox/qTox/commit/f997d51b663741e2577cd00d559aca2434172844))
- **text:** Using modified text, instead of raw ([f7018c42](https://github.com/qTox/qTox/commit/f7018c42c93bd823eedece87052dbb4aa565da1c), closes [#3568](https://github.com/qTox/qTox/issues/3568))
- **toxsave, profileimporter:** Added `remove` function call before overwrite file ([58ea0afe](https://github.com/qTox/qTox/commit/58ea0afed1c699badecb6310786b6b018e23c0e5), closes [#3558](https://github.com/qTox/qTox/issues/3558))
- **translator:** Added layout direction reset on translation. ([927d512f](https://github.com/qTox/qTox/commit/927d512fa2a9925db59a22e0a46a7ae992995924), closes [#3542](https://github.com/qTox/qTox/issues/3542))
- **ui:** Prevent suicide crash on logout ([2bdd9824](https://github.com/qTox/qTox/commit/2bdd9824c758a7ce3ff0cb06fa59ed7d8c81455a), closes [#2480](https://github.com/qTox/qTox/issues/2480))
- **updater:** Use module path, not working dir ([0a2e96ab](https://github.com/qTox/qTox/commit/0a2e96ab07a9c0427660124a2f1ff45bec0ba0b8), closes [#3255](https://github.com/qTox/qTox/issues/3255))
- **video:**
  - guard storeVideoFrame() against freeing in-use memory ([5b31b5db](https://github.com/qTox/qTox/commit/5b31b5db723e97daa7404d5c17ee9340d97198e8))
  - force the use of non-deprecated pixel formats for YUV ([df3345dc](https://github.com/qTox/qTox/commit/df3345dce5827672a4106b11575728690fcd38f2))
  - use a QReadWriteLock to manage camera access ([de6475f3](https://github.com/qTox/qTox/commit/de6475f3d362442122b574c6f5b3f84a60b90784))
  - specify color ranges for pixel formats that are not YUV ([00270ee4](https://github.com/qTox/qTox/commit/00270ee4d21ea5cc28591d568f1b68bd2c35bf73))
  - fix invalid VideoSource ID allocation ([707f7af2](https://github.com/qTox/qTox/commit/707f7af29ab55d75957cf62fc731797881c018ce))
  - added declaration for missing biglock in CameraSource ([c4f88df7](https://github.com/qTox/qTox/commit/c4f88df7c986b1fb61caead9ce58d6c9b37cfbfe))
  - fix a use-after-free with VideoFrame ([8487dcec](https://github.com/qTox/qTox/commit/8487dcecf846043733d1c77083169d9599bfc8a1))
  - fix slanted video when video size is not divisible by 8 ([904495d2](https://github.com/qTox/qTox/commit/904495d2bf2b85da4c2f381778a0bb09b4ce1bbf))
  - fix memory leak caused by unfreed buffers in CoreVideoSource ([3df6b990](https://github.com/qTox/qTox/commit/3df6b990ae4e137ae4069961131ebb39e1dbdf06))
  - fix CoreAV and VideoSurface to conform to new VideoFrame ([277ddc3d](https://github.com/qTox/qTox/commit/277ddc3d2f31078fdb4320ce834785c2120d02a1))
  - Changed minimum window size with video ([f8a45b40](https://github.com/qTox/qTox/commit/f8a45b40519de8f38cef80f5ffc41c19417e0139), closes [#3396](https://github.com/qTox/qTox/issues/3396))
  - do not list the same mode twice ([03c39236](https://github.com/qTox/qTox/commit/03c392369477870273cfc6cc69986d5ffad82bd3))
  - fix video resolution setting ([b4df3c8b](https://github.com/qTox/qTox/commit/b4df3c8b4aabf62f2170586a6a9a49d648d60040), closes [#1033](https://github.com/qTox/qTox/issues/1033))
- **videoframe:** Added correct image copy ([1ddc1371](https://github.com/qTox/qTox/commit/1ddc1371a0ea12de9621bd387b633ee8cdd575b3))
- **widget:**
  - change received files execution method ([def2e880](https://github.com/qTox/qTox/commit/def2e880c9a4190b697bc97edd6d775b15ea1978), closes [#3140](https://github.com/qTox/qTox/issues/3140))
  - Added saving window state before closing ([bfb5dae6](https://github.com/qTox/qTox/commit/bfb5dae6faa12fdcc09952d27e9904314e01762a), closes [#3397](https://github.com/qTox/qTox/issues/3397))
  - properly open chat window ([c17c3405](https://github.com/qTox/qTox/commit/c17c3405bfd14acd494fb81dd624740ea0f0ccd4), closes [#3386](https://github.com/qTox/qTox/issues/3386))
  - rename "Activate" to "Show" ([6173199a](https://github.com/qTox/qTox/commit/6173199a5b8eb81c9957044bc31edb2a77248a45))
  - delete icon in destructor ([f82f49da](https://github.com/qTox/qTox/commit/f82f49da4d88fa89349146f5083366b3773187c3))
  - open a chat window instead of contacts list in multi-window mode ([fdf0cbb1](https://github.com/qTox/qTox/commit/fdf0cbb1e1b82f4f4327bb0a1d6d64da340ffadd), closes [#3212](https://github.com/qTox/qTox/issues/3212))
  - show unread messages notification ([c81e6e2d](https://github.com/qTox/qTox/commit/c81e6e2dd1d499a0b14bf2a0f494f31cbfcc7329), closes [#3193](https://github.com/qTox/qTox/issues/3193))
  - properly show status messages ([dcb8c3f3](https://github.com/qTox/qTox/commit/dcb8c3f3232c5458d86e1595aa704ce3362cb41b), closes [#3123](https://github.com/qTox/qTox/issues/3123))
- **x11grab:** try and use the current display ([294bdab7](https://github.com/qTox/qTox/commit/294bdab77f8fb6c2b29917f5318ffe4ec2bc2ab6), closes [#3500](https://github.com/qTox/qTox/issues/3500))

<a name="v1.4.0"></a>

## v1.4.0 (2016-04-24)

### Release notes

Time flows, and with the flow come new features, new bugfixes, and hopefully no
new bugs.

With this release, a partial changelog of changes since 1.3.0 is added.

Next release will contain a changelog with all changes.

We are hoping that you'll enjoy the new stuff.

Cheers. :)

_PS._ If it's not clear from the changelog below, audio groupchats have been
~fixed.

#### Bug Fixes

- close groupcall if alone ([98d51399](https://github.com/qTox/qTox/commit/98d513990eabb0038112aa2242d45b22e58b2e43))
- disable netcamview if no peer left ([622b543d](https://github.com/qTox/qTox/commit/622b543d9a8d29fca004cf41d7701b3dbef4eebf))
- audiocall button disabled in groupchats ([db4f02a0](https://github.com/qTox/qTox/commit/db4f02a0c4065014bdbadab4a622aae14a112c76))
- Close logfile only after the disabling logging to file ([de487890](https://github.com/qTox/qTox/commit/de4878909d75d9d4312db025f8c0fc30f9f79f39))
- Make logMessageHandler thread-safe ([a7ffc08c](https://github.com/qTox/qTox/commit/a7ffc08cdb9521e1291e768d29aa39ee53a30dbe))
- Deadlock while rotating logs ([c1e2a3c5](https://github.com/qTox/qTox/commit/c1e2a3c5b670c246c1acc138a8698a1f54591173), closes [#2920](https://github.com/qTox/qTox/issues/2920))
- increase faux offline message timeout ([76d8e193](https://github.com/qTox/qTox/commit/76d8e19320d2f15aeb019f72ecaaac2d6a23feab))
- remove unnecessary qDebug call ([66f96019](https://github.com/qTox/qTox/commit/66f96019cb7389eb847b880a2e76bbd68a0ac288))
- **Widget::updateIcons:**
  - workaround QIcon fallback bug ([0b53c4fd](https://github.com/qTox/qTox/commit/0b53c4fd5c71f43a240f2d980bac4e0166fffd2a))
  - fix the way systray icons are loaded ([90874a47](https://github.com/qTox/qTox/commit/90874a478fb3cc6d953a0e37aeb110b95066eb19))
- **addfriendform:** Removed extra connect return press ([66bcfdae](https://github.com/qTox/qTox/commit/66bcfdae3c8ae1f9d0b4a4fb1b3bf8f53440d53d), closes [#3081](https://github.com/qTox/qTox/issues/3081))
- **addfriendform, widget:** Remove Accepted Request ([53071e95](https://github.com/qTox/qTox/commit/53071e952e6d04f97fc8b3e1ab82e0fff8dd5293), closes [#3066](https://github.com/qTox/qTox/issues/3066), closes [#3166](https://github.com/qTox/qTox/issues/3166))
- **chatform:** regression in detecting `tox:` type IDs ([48f3fb7d](https://github.com/qTox/qTox/commit/48f3fb7dcbf5ce990229114158f38098320ede97))
- **core, widget:** Added checks ([f28c3a16](https://github.com/qTox/qTox/commit/f28c3a16ae6d54ed66ce64fbe7c5605badd34e65), closes [#3133](https://github.com/qTox/qTox/issues/3133))
- **file transfer widget:** QPushButton allows image to overflow ([32d588a4](https://github.com/qTox/qTox/commit/32d588a499a76a37340e930e17cb096ec4c27f24), closes [#3042](https://github.com/qTox/qTox/issues/3042))
- **genericchatitemwidget, micfeedbackwidget:** Added members init in the constructor ([27faec91](https://github.com/qTox/qTox/commit/27faec918a023c5bba4b6ee67854438507220b35))
- **groupaudio:**
  - don't set button to green while call running ([6d355154](https://github.com/qTox/qTox/commit/6d3551548b2568d5587992be1ecb3b559dd827f5))
  - don't play audio while call is inactive ([5339ad97](https://github.com/qTox/qTox/commit/5339ad978bd2f962e0799e48452ac2549edf8bc7))
  - avoid deadlock when ending groupcall ([afcd146a](https://github.com/qTox/qTox/commit/afcd146a5bd78d10083a5bf3eb009face24f02b6))
- **groupinviteform:**
  - make list of groups scrollable ([b74ecd92](https://github.com/qTox/qTox/commit/b74ecd92d2f925ff5defee1a4ca0cd50ecf6a2e6), closes [#3085](https://github.com/qTox/qTox/issues/3085))
  - translation invite message ([24efaf05](https://github.com/qTox/qTox/commit/24efaf0594576ba4e761653c6d612f98b440c28a), closes [#3083](https://github.com/qTox/qTox/issues/3083))
  - remove deleted buttons from set ([f137ba71](https://github.com/qTox/qTox/commit/f137ba710cc823e920adf2976ee1061f5a61f9aa), closes [#3087](https://github.com/qTox/qTox/issues/3087))
- **l18n:** make typing notification & groupchat name translatable ([43e61041](https://github.com/qTox/qTox/commit/43e610415a094a649160bdfab1c571a8fc6fee1f))
- **login screen:** Change text on login tooltip ([4e065f13](https://github.com/qTox/qTox/commit/4e065f1395b8feb35431c7736e531b6a0a6fee45))
- **main:** Added check `sodium_init` result ([64a19d34](https://github.com/qTox/qTox/commit/64a19d34192e2ca9f864b1ccc3a320fb90bc780b))
- **profile:** Don't require .ini to load profile ([56a36e2e](https://github.com/qTox/qTox/commit/56a36e2e0a1023a77f0b047a7273295a35aa1833))
- **profileform:**
  - Add toxme username limitation ([132f87c0](https://github.com/qTox/qTox/commit/132f87c05e89e21824efa2c4dfef83d807e0f5bf))
  - Deleted extra check and extra url ([1f7e23d0](https://github.com/qTox/qTox/commit/1f7e23d007887a0e198b8643961f391a06faf36c))
  - Fixed very quick relogin segfault ([88de3a0a](https://github.com/qTox/qTox/commit/88de3a0a7a09b89b0a621452e53b4d6d4ec3bfe8))
  - Fixed segfault on logut ([2e9295f4](https://github.com/qTox/qTox/commit/2e9295f420fc6832091337813010319166450270))
  - Fix tab order, fix loop ([65ab1f4e](https://github.com/qTox/qTox/commit/65ab1f4e14d3ebc9dd13d4308988d999c83a5a47))
- **screenshot:** incorrect screenshot capture resolution under HiDPI ([a36248b5](https://github.com/qTox/qTox/commit/a36248b5013c41c332df3bc13cac428bb0d3b18e))
- **systemtrayicon:** only delete the systray backend that was used ([1d6f32c9](https://github.com/qTox/qTox/commit/1d6f32c9f9e481dc2fed445bb96ea2666f6d69d8))
- **systemtrayicon, widget:** Added deallocate memory ([cbb7eeca](https://github.com/qTox/qTox/commit/cbb7eeca62f9ec8ad2047b41a7cb914b27d6c618))
- **title:** Change title on initial startup on "Add friend" ([47d94045](https://github.com/qTox/qTox/commit/47d940455d35e4b0f76081a8877a881ebf843c86), closes [#3100](https://github.com/qTox/qTox/issues/3100))
- **toxme:**
  - Delete extra check ([d1b706a4](https://github.com/qTox/qTox/commit/d1b706a4b3926fc3bd1f7af115358cf171080021))
  - Fixed potential memory leaks ([8f4b6869](https://github.com/qTox/qTox/commit/8f4b6869f178c12a43d7d37336f32f8ecf7b1427))
  - Fix possible segfault ([11ec3947](https://github.com/qTox/qTox/commit/11ec3947f566e7083a6345ce2eea317f31219c5e))
  - Use format strings ([fc2a5723](https://github.com/qTox/qTox/commit/fc2a5723092ed1c2fe6b0991d9bd9def6ec24a98))
  - Translation fixs ([9565a817](https://github.com/qTox/qTox/commit/9565a8175558d64c9ffdc2ec6b64e69d9ecdc58d))
- **video:**
  - uses explicit default screen from QGuiApplication ([d2189f38](https://github.com/qTox/qTox/commit/d2189f3891b01ca9c4fa55b080e4457887c00f28))
  - usage of invalid file descriptors on error ([556a8750](https://github.com/qTox/qTox/commit/556a8750a1d4c57d02bfe3a4caaffceaf816c783))
  - incorrect desktop video resolution when using HiDPI ([75b40d0a](https://github.com/qTox/qTox/commit/75b40d0a6f82110aac62c864149210f8b491df4b))
- **widget:** Change focus after create group ([b111c509](https://github.com/qTox/qTox/commit/b111c509a7dcf3a0c3d7a72d92c080ff7fd92731), closes [#3128](https://github.com/qTox/qTox/issues/3128))
- **widget, contentdialog:** Added reset icon after activate chat window ([4edc5996](https://github.com/qTox/qTox/commit/4edc5996c74d8679c270c50328642665ed6b3aed), closes [#3139](https://github.com/qTox/qTox/issues/3139))

#### Features

- install icons with make install on unix ([218228b6](https://github.com/qTox/qTox/commit/218228b696367e688537a04afeb62055037afdc3))
- disable building with `filter_audio` by default ([116cc936](https://github.com/qTox/qTox/commit/116cc9366cd45eaff7262b92a8eb4d0fdbede096))
- **audio:** add (repair) support for group audio calls ([356543ca](https://github.com/qTox/qTox/commit/356543ca3ba9d084e9739bcd7b37c4597a245d1d))
- **chatform:** add support for non-local file and samba share links ([47764c03](https://github.com/qTox/qTox/commit/47764c0397a51da2662021a781cefe29af46bf25))
- **profileform:** Added ability to change toxme server ([41c5d4bf](https://github.com/qTox/qTox/commit/41c5d4bf14986348f04f9e60f538e79c1edc4a04))
- **toxme:**
  - Add save toxme info ([204fe1d3](https://github.com/qTox/qTox/commit/204fe1d3dec00c0f4408f8a98c100377759f1218))
  - Add ToxMe registration ([cb8bf134](https://github.com/qTox/qTox/commit/cb8bf134d2e8d0aaf60e136c51f79201b62f67f9))

#### Performance

- **camerasource:** Passed parameter by reference ([910c41f4](https://github.com/qTox/qTox/commit/910c41f4fab106e1c736685ffad33141c1528107))
- **contentdialog:** Delete redundant conditions ([904a1d49](https://github.com/qTox/qTox/commit/904a1d490973ed57327bc093aeaae1e7317b52e8))
