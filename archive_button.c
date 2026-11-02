/*
 * Claws Mail -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2015 the Claws Mail Team
 * Copyright (C) 2015 Dominik Meyer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#include "claws-features.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include "defs.h"

#include "version.h"
#include "claws.h"
#include "plugin.h"
#include "utils.h"
#include "folder.h"
#include "account.h"
#include "alertpanel.h"
#include "procmsg.h"
#include "prefs_common.h"

#include "main.h"
#include "mainwindow.h"
#include "summaryview.h"
#include "messageview.h"
#include "prefs_toolbar.h"
#include "menu.h"

#include "archive_button.h"
#include "archive_button_prefs.h"

#define PLUGIN_NAME (_("ArchiveButton"))
#define TOOLBAR_ITEM_NAME (_("Archive"))

static void toolbar_archive_button_mainwindow_cb (gpointer parent,
						  const gchar *item_name,
				 		  gpointer        data);

static void toolbar_archive_button_messagewindow_cb (gpointer parent,
						  const gchar *item_name,
				 		  gpointer        data);

static void archive_button_menu_cb(GtkAction *action, gpointer data);

/* A menu item makes the action bindable to a keyboard shortcut
 * (hover over it with "Enable keyboard shortcuts" on and press a key) */
static GtkActionEntry archive_button_main_menu[] = {{
	"Message/ArchiveButton",
	NULL, N_("_Archive"), NULL, NULL, G_CALLBACK(archive_button_menu_cb)
}};

static guint main_menu_id = 0;
static guint context_menu_id = 0;

static gboolean check_enabled(ArchiveButtonAccountConfig *ac_config) {
	if(!ac_config->enable) {
		alertpanel_error(_("Archive not enabled for this account."));
		return FALSE;
	}

	return TRUE;
}

/* Returns the account's archive config (caller frees with
 * archive_button_prefs_account_free_config()) and the resolved archive
 * folder, or NULL after telling the user what went wrong. */
static ArchiveButtonAccountConfig *get_archive_config(FolderItem *item,
						      FolderItem **dest_folder)
{
	PrefsAccount *ac = NULL;
	ArchiveButtonAccountConfig *ac_config = NULL;

	*dest_folder = NULL;

	if(NULL == (ac = account_find_from_item(item))) {
		alertpanel_error(_("ArchiveButton: move: could not retrieve account for message."));
		return NULL;
	}

	if(NULL == (ac_config = archive_button_prefs_account_get_config(ac))) {
		alertpanel_error(_("ArchiveButton: move: could not find archive folder from config."));
		return NULL;
	}

	if(!check_enabled(ac_config)) {
		archive_button_prefs_account_free_config(ac_config);
		return NULL;
	}

	if(ac_config->archive_folder)
		*dest_folder = folder_find_item_from_identifier(ac_config->archive_folder);

	if(!*dest_folder) {
		alertpanel_error(_("ArchiveButton: move: archive folder not found."));
		archive_button_prefs_account_free_config(ac_config);
		return NULL;
	}

	return ac_config;
}

static void move_message_to_archive_summaryview(SummaryView *summaryview)
{
	FolderItem *dest_folder = NULL;
	ArchiveButtonAccountConfig *ac_config = NULL;

	if(!summaryview->folder_item ||
	   FOLDER_TYPE(summaryview->folder_item->folder) == F_NEWS)
		return;

	if(NULL == (ac_config = get_archive_config(summaryview->folder_item, &dest_folder)))
		return;

	if(dest_folder == summaryview->folder_item) {
		archive_button_prefs_account_free_config(ac_config);
		return;
	}

	if(ac_config->mark_read_on_archive)
		summary_mark_as_read(summaryview);

	summary_move_selected_to(summaryview, dest_folder);

	archive_button_prefs_account_free_config(ac_config);
}

static void move_message_to_archive_messageview(MessageView *msgview)
{
	MsgInfo *msginfo = NULL;
	FolderItem *dest_folder = NULL;
	ArchiveButtonAccountConfig *ac_config = NULL;

	if (msgview->msginfo && msgview->mainwin && msgview->mainwin->summaryview)
		msginfo = summary_get_selected_msg(msgview->mainwin->summaryview);

	/* need a procmsg_msginfo_equal() */
	if (msginfo && msgview->msginfo &&
			msginfo->msgnum == msgview->msginfo->msgnum &&
			msginfo->folder == msgview->msginfo->folder) {
		move_message_to_archive_summaryview(msgview->mainwin->summaryview);
	} else {
		msginfo = msgview->msginfo;

		cm_return_if_fail(msginfo != NULL);
		cm_return_if_fail(msginfo->folder != NULL);

		if(NULL == (ac_config = get_archive_config(msginfo->folder, &dest_folder)))
			return;

		if(dest_folder == msginfo->folder) {
			archive_button_prefs_account_free_config(ac_config);
			return;
		}

		if(ac_config->mark_read_on_archive)
			procmsg_msginfo_unset_flags(msginfo, MSG_NEW | MSG_UNREAD, 0);

		archive_button_prefs_account_free_config(ac_config);

		if(folder_item_move_msg(dest_folder, msginfo) == -1) {
			alertpanel_error(_("ArchiveButton: move: could not move message."));
			return;
		}
	}

#ifdef GENERIC_UMPC
	if (msgview->window && !prefs_common.always_show_msg) {
		messageview_destroy(msgview);
	}
#endif
}

static void toolbar_archive_button_mainwindow_cb(gpointer parent,
		const gchar *item_name, gpointer data)
{
	MainWindow *mainwindow = (MainWindow *)parent;

	debug_print("ArchiveButton: button mainwindow pressed\n");
	move_message_to_archive_summaryview(mainwindow->summaryview);
}

static void toolbar_archive_button_messagewindow_cb(gpointer parent,
		const gchar *item_name, gpointer data)
{
	MessageView *messageview = (MessageView *)parent;

	debug_print("ArchiveButton: button messagewindow pressed\n");
	move_message_to_archive_messageview(messageview);
}

static void archive_button_menu_cb(GtkAction *action, gpointer data)
{
	MainWindow *mainwindow = (MainWindow *)data;

	debug_print("ArchiveButton: menu item activated\n");
	move_message_to_archive_summaryview(mainwindow->summaryview);
}

gint plugin_init(gchar **error)
{
	MainWindow *mainwin = mainwindow_get_mainwindow();

	if (!check_plugin_version(MAKE_NUMERIC_VERSION(4,0,0,0),
				VERSION_NUMERIC, PLUGIN_NAME, error))
		return -1;

	debug_print("registering toolbar item\n");

	prefs_toolbar_register_plugin_item(TOOLBAR_MAIN, PLUGIN_NAME, TOOLBAR_ITEM_NAME,
			toolbar_archive_button_mainwindow_cb, NULL);
	prefs_toolbar_register_plugin_item(TOOLBAR_MSGVIEW, PLUGIN_NAME, TOOLBAR_ITEM_NAME,
			toolbar_archive_button_messagewindow_cb, NULL);

	gtk_action_group_add_actions(mainwin->action_group, archive_button_main_menu,
			1, (gpointer)mainwin);
	MENUITEM_ADDUI_ID_MANAGER(mainwin->ui_manager, "/Menu/Message/Trash", "ArchiveButton",
			  "Message/ArchiveButton", GTK_UI_MANAGER_MENUITEM,
			  main_menu_id)
	MENUITEM_ADDUI_ID_MANAGER(mainwin->ui_manager, "/Menus/SummaryViewPopup/Trash", "ArchiveButton",
			  "Message/ArchiveButton", GTK_UI_MANAGER_MENUITEM,
			  context_menu_id)

	archive_button_prefs_init();

	debug_print("ArchiveButton plugin loaded\n");

	return 0;
}

gboolean plugin_done(void)
{
	MainWindow *mainwin = mainwindow_get_mainwindow();

	if (mainwin != NULL) {
		MENUITEM_REMUI_MANAGER(mainwin->ui_manager, mainwin->action_group,
				"Message/ArchiveButton", main_menu_id);
		main_menu_id = 0;
		MENUITEM_REMUI_MANAGER(mainwin->ui_manager, mainwin->action_group,
				"Message/ArchiveButton", context_menu_id);
		context_menu_id = 0;
	}

	prefs_toolbar_unregister_plugin_item(TOOLBAR_MAIN, PLUGIN_NAME, TOOLBAR_ITEM_NAME);
	prefs_toolbar_unregister_plugin_item(TOOLBAR_MSGVIEW, PLUGIN_NAME, TOOLBAR_ITEM_NAME);

	archive_button_prefs_done();

	debug_print("ArchiveButton plugin unloaded\n");
	return TRUE;
}

const gchar *plugin_name(void)
{
	return PLUGIN_NAME;
}

const gchar *plugin_desc(void)
{
	return _("This Plugin adds an archive button to Claws Mail. "
	         "It allows moving selected mails to a predefined archive folder "
		 "for each account.\n\n"
		 "Add the \"Archive\" item to the main window and/or message "
		 "window toolbar in Configuration/Preferences/Toolbars, or use "
		 "Message/Archive (which can be given a keyboard shortcut), and "
		 "set the archive folder for each account in "
		 "Configuration/Edit accounts/Plugins/ArchiveButton.");
}

const gchar *plugin_type(void)
{
	return "GTK3";
}

const gchar *plugin_licence(void)
{
	return "GPL3+";
}

const gchar *plugin_version(void)
{
	return VERSION;
}

struct PluginFeature *plugin_provides(void)
{
	static struct PluginFeature features[] =
		{ {PLUGIN_UTILITY, N_("ArchiveButton")},
		  {PLUGIN_NOTHING, NULL}};
	return features;
}
