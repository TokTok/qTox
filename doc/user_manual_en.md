# qTox User Manual

## Index

- [Profile corner](#profile-corner)
- [Contact list](#contact-list)
- [User Profile](#user-profile)
- [Settings](#settings)
- [Groupchats](#groupchats)
- [Message Styling](#message-styling)
- [Quotes](#quotes)
- [Multi Window Mode](#multi-window-mode)
- [Keyboard Shortcuts](#keyboard-shortcuts)
- [Commandline Options](#commandline-options)
- [Emoji Packs](#emoji-packs)
- [Bootstrap nodes](#bootstrap-nodes)

## Profile corner

Located in the top left corner of the main window.

- **Avatar**: picture that is shown to your contacts. Clicking on it will open
  [user profile] where you can change it.
- **Name**: your name shown to your contacts. Clicking on it will open [user
  profile] where you can change it.
- **Status message**: your status message shown to your contacts. Click on it to
  change it.
- **Status orb**: colored orb button that shows your current status. Click on it
  to change it.

### Status

Status can be one of:

- `Online` – green
- `Away` – yellow
- `Busy` – red
- `Offline` – gray, set automatically by qTox when there is no connection to the
  Tox network.

## Contact list

Located on the left, below the [profile corner]. Can be sorted e.g.
`By Activity`.

`By Activity` sorting in qTox is updated whenever client receives something that
is directly aimed at you, and not sent to everyone, that is:

- audio/video call
- file transfer
- groupchat invite
- message

**Not** updated on:

- avatar change
- groupchat message
- name change
- [status](#status) change
- status message change

### Contact menu

Can be accessed by right-clicking on a contact or [circle](#circles). When
right-clicking on a contact a menu appears that has the following options:

- **Open chat in a new window:** opens a new window for the chosen contact.
- **Invite to group:** offers an option to create a new groupchat and
  automatically invite the friend to it or to an already existing groupchat.
- **Move to circle:** offers an option to move friend to a new
  [circle](#circles), or to an existing one.
- **Set alias:** set alias that will be displayed instead of contact's name.
- **Auto accept group invites:** if enabled, all group chat invites from this
  friend are automatically accepted.
- **Auto accept files from this friend:** option to automatically save files
  from the selected contact in a chosen directory.
- **Remove friend:** option to remove the contact. Confirmation is needed to
  remove the friend.
- **Show details:** show [details](#contact-details) of a friend.

#### Circles

Circles allow you to group contacts together and give this circle a name.
Contacts can be in one or in no circle.

#### Contact details

Contact details can be accessed by right-clicking on a contact and picking the
`Show details` option.

Some of the informations listed are:

- avatar
- name
- status message
- Public Key (PK) – note that PK is only a part of Tox ID and alone can't be
  used to add the contact e.g. on a different profile. Part not known by qTox is
  the [NoSpam](#nospam).

## User Profile

To access it, click on **Avatar**/**Name** in the [profile corner].

Your User Profile contains everything you share with other people on Tox. You
can open it by clicking the picture in the top left corner. It contains the
following settings:

### Public Information

- **Name:** This is your nickname which everyone who is on your contact list can
  see.
- **Status:** You can post a status message here, which again everyone on your
  contact list can see.

#### Avatar

Your profile picture that all your friends can see. To add or change, click on
the avatar. To remove, right-click.

### Tox ID

The long code in hexadecimal format is your Tox ID, share this with everyone you
want to talk to. Click it to copy it to your clipboard. Your Tox ID is also
shown as QR code to easily share it with friends over a smartphone.

The "Save image" button saves the QR code into a image file, while the "Copy
image" button copies into your clipboard.

### Register on ToxMe

An integration for ToxMe service providers that allows you to create a simple
alias for your Tox ID that will look like `user@example.com`. A default service
provider (toxme.io) is already listed, and you can add your own.

- **Username:** This will be used as an alias for your Tox ID.
- **Biography:** Optional. If you want, you can write something here.
- **Server:** Service address where your alias will be registered.

Note that by default aliases are public, but you can check the option to make a
private one, but given that it would be stored on a server that you don't
control, it's not actually private. At best you have a promise of privacy from
the owner of the server. For 100% privacy use Tox IDs.

After registration, you can give your new alias, e.g. `user@example.com` to your
friends instead of the long Tox ID.

### Profile

qTox allows you to use multiple Tox IDs with different profiles, each of which
can have different nicknames, status messages and friends.

- **Current profile:** Shows the filename which stores your information.
- **Current profile location:** Shows the path to the profile file.
- **Rename:** Allows you to rename your profile. Your nickname and profile name
  don't have to be the same.
- **Delete:** Deletes your profile and the corresponding chat history.
- **Export:** Allows you to export your profile in a format compatible with
  other Tox clients. You can also manually back up your \*.tox files.
- **Logout:** Close your current profile and show the login window.
- **Remove password:** Removes the existing password for your profile. If the
  profile already has no password, you will be notified.
- **Change password:** Allows you to either change an existing password, or
  create a new password if your profile does not have one.

## Friends' options

In the friend's window you can customize some options for this friend
specifically.

- _Auto answer:_ chooses the way to autoaccept audio and video calls. _
  *Manual:* All calls must be manually accepted. _ _Audio:_ Only audio calls
  will be automatically accepted. Video calls must be manually accepted. \*
  _Audio + video:_ All calls will be automatically accepted.

## Settings

### General

- **Language:** Changes which language the qTox interface uses.
- **Autostart:** If set, qTox will start when you login on your computer. qTox
  will also automatically open the profile which was active when you ticked the
  checkbox, but this only works if your profile isn't encrypted (has no password
  set).
- **Light icon:** If set, qTox will use a different icon, which is easier to see
  on black backgrounds.
- **Show system tray icon:** If set, qTox will show its icon in your system
  tray.
  - **Start in tray:** On start, qTox will only show its tray icon and no
    window.
  - **Minimize to tray:** The minimize button on the top right, will minimize
    qTox to its tray icon. There won't be a taskbar item.
  - **Close to tray**: The close button on the top right will minimize qTox to
    its tray icon. There won't be a taskbar item.
- **Play sound:** If checked, qTox will play a sound when you get a new message.
  - **Play sound while Busy**: If checked, qTox will play a sound even when your
    status is set to `Busy`.
- **Show contacts' status changes:** If set, qTox will show contact status
  changes in your chat window.
- **Faux offline messaging:** If enabled, qTox will attempt to send messages
  when a currently offline contact comes online again.
- **Auto away after (0 to disable):** After the specified amount of time, qTox
  will set your status to "Away". A setting of 0 will never change your status.
- **Default directory to save files:** Allows you to specify the default
  destination for incoming file transfers.
- **Autoaccept files:** If set, qTox will automatically accept file transfers
  and put them in the directory specified above.

### User Interface

#### Chat

- **Base font:** You can set a non-default font and its size for the chat. The
  new font setting will be used for new messages and all messages after qTox has
  been restarted.
- **Text Style format:** see [Message styling](#message-styling) section.

#### New message

- **Open window:** If checked, the qTox window will be opened when you receive a
  new message. If you use the multiple windows mode, see
  [Multi Window Mode](#multi-window-mode) for details.
  - **Focus window:** If checked, the qTox window will additionally be focused
    when you receive a new message.

#### Contact list

- **Group chats always notify:** If set, qTox will notify you on every new
  message in a groupchat.
- **Place groupchats at top of friend list:** If checked, your groupchats will
  be at the top of the contacts list instead of being sorted with your other
  contacts.
- **Compact contact list:** If set, qTox will use a contact list layout which
  takes up less screen space.
- **Multiple windows mode:** If enabled, the qTox user interface will be split
  into multiple independent windows. For details see
  [Multi Window Mode](#multi-window-mode).
  - **Open each chat in an individual window:** If checked, a new window will be
    opened for every chat you open. If you manually grouped the chat into
    another window, the window which hosts the chat will be focused.

#### Emoticons

- **Use emoticons:** If enabled, qTox will replace smileys ( e.g. `:-)` ) with
  corresponding graphical emoticons.
- **Smiley Pack:** Allows you to choose from different sets of shipped emoticon
  styles.
- **Emoticon size:** Allows you to change the size of the emoticons.

#### Theme

- **Style:** Changes the appearance of qTox.
- **Theme color:** Changes the colors qTox uses.
- **Timestamp format:** Changes the format in which qTox displays message
  timestamps.
- **Date format:** Same as above for the date.

### Privacy

- **Send typing notifications:** If enabled, notify your chat partner when you
  are currently typing.
- **Keep chat history:** If enabled, qTox will save your sent and received
  messages. Encrypt your profile, if you want to encrypt the chat history.
  **_Note_** that disabling history disables `Faux offline messaging`. With
  disabled history qTox doesn't store messages, so it can't try to re-send them.

#### NoSpam

NoSpam is a feature of Tox that prevents a malicious user from spamming you with
friend requests. If you get spammed, enter or generate a new NoSpam value. This
will alter your Tox ID. You don't need to tell your existing contacts your new
Tox ID, but you have to tell new contacts your new Tox ID. Your Tox ID can be
found in your [User Profile](#user-profile).

#### BlackList

BlackList is a feature of qTox that locally blocks a group member's messages
across all your joined groups, in case someone spams a group. You need to put a
members public key into the BlackList text box one per line to activate it.
Currently qTox doesn't have a method to get the public key from a group member,
this will be added in the future.

### Audio/Video

#### Audio Settings

- **Playback device:** Select the device qTox should use for all audio output
  (notifications, calls, etc).
- **Volume:** Here you can adjust the playback volume to your needs.
- **Capture device:** Select the device qTox should use for audio input in
  calls.
- **Gain:** Set the input volume of your microphone with this slider. When you
  are talking normally, the displayed volume indicator should be in the green
  range.

#### Video Settings

- **Video device:** Select the video device qTox should use for video calls.
  "None" will show a dummy picture to your chat partner. "Desktop" will stream
  the content of your screen.
- **Resolution:** You can select from the available resolutions and frame rates
  here. Higher resolutions provide more quality, but if the bandwidth of your
  connection is low, the video may get choppy.

If you set up everything correctly, you should see the preview of your video
device in the box below.

- **Rescan devices:** Use this button to search for newly attached devices, e.g.
  you plugged in a webcam.

### Advanced

#### Portable

- **Make Tox portable:** If enabled, qTox will load/save user data from the
  working directory, instead of `~/.config/tox/`.

#### Connection Settings

- **Enable IPv6 (recommended):** If enabled, qTox will use IPv4 and IPv6
  protocols, whichever is available. If disabled, qTox will only use IPv4.
- **Enable UDP (recommended):** If enabled, qTox will use TCP and UDP protocols.
  If disabled, qTox will only use TCP, which lowers the amount of open
  connections and slightly decreases required bandwidth, but is also slower and
  puts more load on other network participants.

Most users will want both options enabled, but if qTox negatively impacts your
router or connection, you can try to disable them.

- **Proxy type:** If you want to use a proxy, set the type here. "None" disables
  the proxy.
- **Address:** If you use a proxy, enter the address here.
- **Port:** If you use a proxy, enter the port here.
- **Reconnect:** Reconnect to the Tox network, e.g. if you changed the proxy
  settings.

---

- **Reset to default settings:** Use this button to revert any changes you made
  to the qTox settings. _Note that current implentation
  [is buggy](https://github.com/qTox/qTox/issues/3664) and aside from settings
  also friend [aliases] will be removed!_

### About

- **Version:** Shows the version of qTox and the libraries it depends on. Please
  append this information to every bug report.
- **License:** Shows the license under which the code of qTox is available.
- **Authors:** Lists the people who developed this shiny piece of software.
- **Known Issues:** Links to our list of known issues and improvements.

## Groupchats

Groupchats are a way to talk with multiple friends at the same time, like when
you are standing together in a group. To create a groupchat click the groupchat
icon in the bottom left corner and set a name. Now you can invite your contacts
by right-clicking on the contact and selecting "Invite to group". Currently, if
the last person leaves the chat, it is closed and you have to create a new one.
Videochats and file transfers are currently unsupported in groupchats.

## Message Styling

Similar to other messaging applications, qTox supports stylized text formatting.

- For **Bold**, surround text in single or double asterisks: `*text*` or
  `**text**`
- For **Italics**, surround text in single or double forward slashes: `/text/`
  or `//text//`
- For **Strikethrough**, surround text in single or double tilde's: `~text~` or
  `~~text~~`
- For **Underline**, surround text in single or double underscores: `_text_` or
  `__text__`
- For **Code**, surround your code in in single backticks: `` `text` ``

Additionally, qTox supports three modes of Markdown parsing:

- `Plaintext`: No text is stylized
- `Show Formatting Characters`: Stylize text while showing formatting characters
  (Default)
- `Don't Show Formatting Characters`: Stylize text without showing formatting
  characters

_Note that any change in Markdown preference will require a restart._

qTox also supports action messages by prefixing a message with `/me`, where
`/me` is replaced with your current username. For example `/me likes cats` turns
into _` _ qTox User likes cats`\*.

## Quotes

qTox has feature to quote selected text in chat window:

1. Select the text you want to quote.
2. Right-click on the selected text and choose "Quote selected text" in the
   context menu. You also can use `ALT` + `q` shortcut.
3. Selected text will be automatically quoted into the message input area in a
   pretty formatting.

## Friend- and Groupinvites

To invite a friend to a chat with you, you have to click the `+` button on the
bottom left of the qTox window. The "Add a friend" Tab allows you to enter the
Tox ID of your friend, or the username of a [ToxMe service] if your friend
registered there.

On the "Friend requests" tab you can see, friend requests you got from other Tox
users. You can then choose to either accept or decline these requests.

On the Groupinvites page, you can create a new groupchat and add users to it by
using the context menu in your contact list. Invites from your contacts are also
displayed here and you can accept and decline them.

## Multi Window Mode

In this mode, qTox will separate its main window into a single contact list and
one or multiple chat windows, which allows you to have multiple conversations on
your screen at the same time. Additionally you can manually group chats into a
window by dragging and dropping them onto each other. This mode can be activated
and configured in [settings](#settings).

## Keyboard Shortcuts

The following shortcuts are currently supported:

| Shortcut                 | Action                         |
| ------------------------ | ------------------------------ |
| `Arrow up`               | Paste last sent message        |
| `CTRL` + `SHIFT` + `L`   | Clear chat                     |
| `CTRL` + `q`             | Quit qTox                      |
| `CTRL` + `Page Down`     | Switch to the next contact     |
| `CTRL` + `Page Up`       | Switch to the previous contact |
| `CTRL` + `TAB`           | Switch to the next contact     |
| `CTRL` + `SHIFT` + `TAB` | Switch to the previous contact |
| `CTRL` + `p`             | [Push to talk](#push-to-talk)  |
| `ALT` + `q`              | Quote selected text            |
| `F11`                    | Toggle fullscreen mode         |

## Push to talk

In audio group chat microphone mute state will be changed while `Ctrl` + `p`
pressed and reverted on release.

## Commandline Options

| Option                             | Action                                                              |
| ---------------------------------- | ------------------------------------------------------------------- |
| `-p` `<profile>`                   | Use specified unencrypted profile                                   |
| `-l`                               | Start with loginscreen                                              |
| `-I` `<on/off>`                    | Sets IPv6 toggle [Default: ON]                                      |
| `-U` `<on/off>`                    | Sets UDP toggle [Default: ON]                                       |
| `-L` `<on/off>`                    | Sets LAN toggle [Default: ON]                                       |
| `-P` `<protocol>:<address>:<port>` | Applies [proxy options](#commandline-proxy-options) [Default: NONE] |

### Commandline Proxy Options

Protocol: NONE, HTTP or SOCKS5 <br> Address: Proxy address <br> Port: Proxy port
number (0-65535) <br> Example input: <br> `qtox -P SOCKS5:192.168.0.1:2121` <br>
`qtox -P none`

## Emoji Packs

qTox provides support for custom emoji packs. To install a new emoji pack put it
in `%LOCALAPPDATA%/emoticons` for Windows or `~/.local/share/emoticons` for
Linux. If these directories don't exist, you have to create them. The emoji
files have to be in a subfolder also containing `emoticon.xml`, see the
structure of https://github.com/qTox/qTox/tree/v1.5.2/smileys for further
information.

## Bootstrap Nodes

qTox uses bootstrap nodes to find its way in to the DHT. The list of nodes is
stored in `bootstrapNodes.json` and can be found and modified if wanted at
`~/.config/tox/` on Linux, `%APPDATA%\Roaming\tox` on Windows, and
`~/Library/Application Support/Tox` on macOS.

[toxme service]: #register-on-toxme
[user profile]: #user-profile
[profile corner]: #profile-corner
