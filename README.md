# Claws Archive Button Plugin

This plugin is supposed to ease transition from other mail
clients (e.g. Thunderbird), to claws. One big issue when 
switching mail clients is that some of them have the functionality
to archive mail instead of deleting it. This makes sense 
if mail is to be searched afterwards and a whole history of
mail is to be kept, without cluttering the inbox.

Claws already provides functionality to move mail to an archive
folder on button- or keypress, but no functionality to move
messages to a specific folder, depending on which account the
mail is stored in.

This plugin provides a toolbar item, which can be assigned to
a configurable button to archive marked mail. In the settings
of each account the archive folder for this account can then be
selected subsequently.

## Compiling

- Checkout the Claws mail source.

  `http://git.claws-mail.org/readonly/claws.git`

- Go to the Claws mail plugin source folder and checkout
  the archive button source

  `cd claws/src/plugins`

  `git clone https://github.com/meyerd/claws-archive_button.git archive_button`

- Add `archive_button` to the end of the `SUBDIRS` definition
  in `Makefile.am` in `claws/src/plugins`

- Build claws (`./autogen.sh`, `./configure`, `make`, `make install`)
  in the claws source root directory.

## Usage

- Load the plugin in Configuration → Plugins (`archive_button.so`).
- For each account, enable archiving and choose the archive folder in
  Configuration → Edit accounts → (account) → Plugins → ArchiveButton.
  Optionally enable "Mark read on archive."
- Add the "Archive" item to the main window and/or message window
  toolbar in Configuration → Preferences → Toolbars.

## Compatibility

Ported to Claws Mail 4.x (GTK3); tested against Claws Mail 4.4.0.
Building also requires adding the plugin to `configure.ac`
(`--enable-archive_button-plugin`, `BUILD_ARCHIVE_BUTTON_PLUGIN`
conditional and `src/plugins/archive_button/Makefile` in
`AC_CONFIG_FILES`) before running `./autogen.sh`.
