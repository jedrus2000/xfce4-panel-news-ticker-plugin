/*
 *	TICKR - GTK-based Feed Reader - Copyright (C) Emmanuel Thomas-Maurin 2009-2021
 *	<manutm007@gmail.com>
 *
 * 	This program is free software: you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tickr.h"

static const char *help_str0[] = {
	"USER INTERFACE MAIN FEATURES:\n\n",

	"- To open the main menu, right-click inside " APP_NAME " area.\n\n",

	"- You can import feed subscriptions with 'File > Import Feed List (OPML)',\n",
	"  for instance your Google Reader subscriptions.\n\n",

	"- To add a new feed, open 'File > Feed Organizer (RSS/Atom)', then look\n",
	"  for 'New Feed -> Enter URL' at the bottom of the window, click 'Clear'\n",
	"  and type or paste the feed URL.\n\n",

	"- To open a link in your browser, left-click on text.\n\n",

	"- Use mouse wheel to either adjust " APP_NAME " scrolling speed or open the\n",
	"  'Selected Feed Picker' window to quickly move between selected feeds\n",
	"  (use <ctrl> + mouse wheel for alternative action.) This behaviour is\n",
	"  set in the 'Preferences' window as 'Mouse Wheel acts on'.\n\n",

	"- Basically, use 'File > Feed Organizer (RSS|Atom)' to manage your feed\n",
	"  list, select feeds, subscribe to new ones, and 'Edit > Preferences'\n",
	"  to tweak " APP_NAME " appearance and behaviour.\n\n",

	"- 'Window - always on top' -> check this if you want " APP_NAME " to always stay\n",
	"  above your browser (and any other windows.)\n\n",

	"- 'Window - decorated' -> if you want " APP_NAME " to be'draggable'.\n\n",

	"If you don't import any feed list, there is a sample one that you can use.\n\n",

	"ONLY in case you're using " APP_NAME " inside a ***decorated*** window, you can\n",
	"use these keyboard shortcuts:\n\n",

	"- <ctrl> R to open the 'Feed Organizer (RSS|Atom)' window.\n\n",

	"- <ctrl> T to open a text file.\n\n",

	"- <ctrl> I to import (and merge) an URL list from an OPML file.\n\n",

	"- <ctrl> E to export the URL list to an OPML file.\n\n",

	"- <ctrl> P to open the 'Resource Properties' window.\n\n",

	"- <ctrl> Q to quit.\n\n",

	"- <ctrl> S to open the 'Preferences' window.\n\n",

	"- <ctrl> B to open the link displayed inside " APP_NAME " (middle area.)\n\n",

	"- <ctrl> J to play / <ctrl> K to pause / <ctrl> L to reload (current feed).\n\n",

	"- <ctrl> U (speed Up) / <ctrl> D (speed Down) to adjust scrolling speed on\n",
	"  the fly.\n\n",

	"- F1 to open the 'Quick Help' window.\n\n",

	"- <ctrl> H to launch the 'Online Help' (this very page.)\n\n",

	"- <ctrl> A to open the 'About' window.\n\n\n",

	"COMMAND LINE REFERENCE ('tickr -help' output):\n\n",

	NULL
};

static const char *help_str1[] = {
	APP_NAME "-" APP_V_NUM " - GTK-based highly graphically-customizable Feed Ticker\n",
	COPYRIGHT_STR " <" SUPPORT_EMAIL_ADDR ">\n\n",
	APP_NAME " is a GTK-based RSS/Atom Reader that displays feeds as a smooth\n"
	"scrolling line on your Desktop, as known from TV stations. Open feed\n"
	"links in your favourite Browser. Graphics are highly customizable.\n\n"
	"Usage:\n",
	"        " APP_CMD " [-help, -h, -? / -version, - v / -license, -l]\n",
	"              [-dumpconfig / -dumpfontlist / -dumperrorcodes]\n",
	"              [-instance-id=n / -no-ui]\n",
	"              [options (-name[=value])...]\n",
	"              [resource (URL or file name)]\n\n",
	"        help                           Get this help page\n\n",
	"        version                        Print out version number\n\n",
	"        license                        Print out license\n\n",
	"        dumpconfig                     Send content of config file to stdout\n\n",
	"        dumpfontlist                   Send list of available fonts to stdout\n\n",
	"        dumperrorcodes                 Send error codes and strings to stdout\n\n",
	"        instance-id=n                  n = 1 to 99 - Use this when launching\n",
	"                                       several instances simultaneously, each\n",
	"                                       instance using its own config and dump\n",
	"                                       files (to be effective, instance-id must\n",
	"                                       be the 1st argument)\n\n",
	"        no-ui                          Disable opening of UI elements which can\n",
	"                                       modify settings and/or URL list/\n",
	"                                       selection (to be effective, no-ui must\n",
	"                                       be the 1st or 2nd argument)\n\n",
	"Options:\n",
	"        delay=n                        Delay in milliseconds\n",
	"                                       (-> speed = K / delay)\n\n",
	"        shiftsize=n                    Shift size in pixels\n",
	"                                       (-> speed = K x shift size)\n\n",
	"        fgcolor=#rrggbbaa              Foreground 32-bit hexa color\n\n",
	/*"        highlightfgcolor=#rrggbbaa     Marked items foreground 32-bit hexa\n",
	"                                       color\n\n",*/
	"        bgcolor=#rrggbbaa              Background 32-bit hexa color\n\n",
	"        setgradientbg=[y/n]            Set gradient background\n\n",
	"        bgcolor2=#rrggbbaa             Background 32-bit hexa color2\n\n",
	"        fontname='str'                 Font name\n\n",
	"        fontsize=n                     Font size (can't be > 200)\n\n",	/* Check this is up to date */
	"        disablescreenlimits=[y/n]      Allow win_x, win_y and win_w to be\n"
	"                                       greater than screen dimensions\n\n",
	"        win_x=n                        Window position - x\n\n",
	"        win_y=n                        Window position - y\n\n",
	"        win_w=n                        Window width\n\n",
	"        win_h=n                        Window height (compute font size if > 0)\n\n",
	"        windec=[y/n]                   Window decoration\n\n",
	"        alwaysontop=[y/n]              Window always-on-top\n\n",
	"        wintransparency=n              Actually window opacity from 1 to 10\n",
	"                                       (0 = none -> 10 = full)\n\n",
	"        iconintaskbar=[y/n]            Icon in taskbar\n\n",
	"        winsticky=[y/n]                Visible on all user desktops\n\n",
	"        overrideredirect=[y/n]         Set top window override_redirect flag,\n",
	"                                       ie bypass window manager (experimental)\n\n",
	"        shadow=[y/n]                   Apply shadow to text\n\n",
	"        shadowoffset_x=n               Shadow x offset in pixels\n\n",
	"        shadowoffset_y=n               Shadow y offset in pixels\n\n",
	"        shadowfx=n                     Shadow effect (0 = none -> 10 = full)\n\n",
	"        linedelimiter='str'            String to be appended at end of line\n\n",
	"        specialchars=[y/n]             Enable or disable special characters.\n",
	"                                       This is only useful when resource is a\n",
	"                                       file, not an URL\n\n",
	"        newpgchar=c                    'New page' special character\n\n",
	"        tabchar=c                      'Tab' (8 spaces) special character\n\n",
	"        rssrefresh=n                   Refresh rate in minutes = delay before\n",
	"                                       reloading resource (URL or text file.)\n",
	"                                       0 = never force reload. Otherwise, apply\n",
	"                                       only if no TTL inside feed or if\n",
	"                                       resource is text file.\n",
	"                                       (Actually, in multiple selections mode,\n",
	"                                       all feeds are always reloaded\n",
	"                                       sequentially, because there is no\n",
	"                                       caching involved)\n\n",
	"        revsc=[y/n]                    Reverse scrolling (= L to R), for\n",
	"                                       languages written/read from R to L\n\n",
	"        feedtitle=[y/n]                Show or hide feed title\n\n",
	"        feedtitledelimiter='str'       String to be appended after feed title\n\n",
	"        itemtitle=[y/n]                Show or hide item title\n\n",
	"        itemtitledelimiter='str'       String to be appended after item title\n\n",
	"        itemdescription=[y/n]          Show or hide item description\n\n",
	"        itemdescriptiondelimiter='str' String to be appended after item\n",
	"                                       description\n\n",
	"        nitemsperfeed=n                Read N items max per feed (0 = no limit)\n\n",
	/*"        markitemaction=[h/c/n]         Mark item action: hide / color / none\n\n",*/
	"        rmtags=[y/n]                   Strip html tags\n\n",
	"        uppercasetext=[y/n]            Set all text to upper case\n\n",
	"        homefeed='str'                 Set URL as 'homefeed' = homepage\n",
	"                                       (from command line, not automatically\n",
	"                                       saved, so a little bit useless...)\n\n",
	"        openlinkcmd='str'              'Open in Browser' command line:\n",
	"                                       Application that will open active link\n",
	"                                       (may require path.) Most likely will\n",
	"                                       invoke your favourite browser\n\n",
	"        openlinkargs='str'             'Open in Browser' optional arguments\n\n",
	"        clock=[l/r/n]                  Clock location: left / right / none\n\n",
	"        clocksec=[y/n]                 Show seconds\n\n",
	"        clock12h=[y/n]                 12h time format\n\n",
	"        clockdate=[y/n]                Show date\n\n",
	"        clockaltdateform=[y/n]         Use alternative date format, ie\n"
	"                                       'Mon 01 Jan' instead of 'Mon Jan 01'\n\n",
	"        clockfontname='str'            Clock font name\n\n",
	"        clockfontsize=n                Clock font size (can't be > " APP_NAME "\n",
	"                                       height)\n\n",
	"        clockfgcolor=#rrggbbaa         Clock foreground 32-bit hexa color\n\n",
	"        clockbgcolor=#rrggbbaa         Clock background 32-bit hexa color\n\n",
	"        setclockgradientbg=[y/n]       Set clock gradient background\n\n",
	"        clockbgcolor2=#rrggbbaa        Clock background 32-bit hexa color2\n\n",
	"        disablepopups=[y/n]            Disable error/warning popup windows\n\n",
	"        pauseonmouseover=[y/n]         Pause " APP_NAME " on mouseover\n\n",
	"        disableleftclick=[y/n]         Disable left-click\n\n",
	"        mousewheelaction=[s/f/n]       Mouse wheel acts on:\n",
	"                                       (" APP_NAME "-)speed / feed(-in-list) / none\n",
	"                                       (use <ctrl> + mouse wheel for\n",
	"                                       alternative action)\n\n",
	"        sfeedpickerautoclose=[y/n]     Selected feed picker window closes when\n",
	"                                       pointer leaves area \n\n",
	"        enablefeedordering=[y/n]       Enable feed re-ordering (by user)\n\n",
	"        useauth=[y/n]                  Use HTTP basic authentication\n\n",
	"        user='str'                     User\n\n",
	"        psw='str'                      Password (never saved)\n\n",
	"        useproxy=[y/n]                 Connect through proxy\n\n",
	"        proxyhost='str'                Proxy host\n\n",
	"        proxyport='str'                Proxy port\n\n",
	"        useproxyauth=[y/n]             Use proxy authentication\n\n",
	"        proxyuser='str'                Proxy user\n\n",
	"        proxypsw='str'                 Proxy password (never saved)\n\n",
	"        connect-timeout=n              n = 1 to 60 (seconds) - Override default\n",
	"                                       connect timeout value (= 5 seconds),\n",	/* Check this is up to date */
	"                                       useful if proxy or slow internet link\n\n",
	"        sendrecv-timeout=n             Same as above for send/recv timeout\n",
	"                                       (default value = 1 second)\n\n",		/* Check this is up to date */
	"Mouse usage:\n"
	"        To open the main menu, right-click inside " APP_NAME " area.\n\n",
	"        You can import feed subscriptions from another feed reader with\n",
	"        'File > Import Feed List (OPML)'.\n\n",
	"        To add a new feed, open 'File > Feed Organizer (RSS/Atom)', then look\n",
	"        for 'New Feed -> Enter URL' at the bottom of the window, click 'Clear'\n",
	"        and type or paste the feed URL.\n\n",
	"        To open a link in your browser, left-click on text.\n\n",
	"        Use mouse wheel to either adjust " APP_NAME " scrolling speed or open the\n",
	"        'Selected Feed Picker' window to quickly move between selected feeds\n",
	"        (use <ctrl> + mouse wheel for alternative action.)\n\n",
	"        Basically, use 'File > Feed Organizer (RSS|Atom)' to manage your feed\n",
	"        list, select feeds, subscribe to new ones, and 'Edit > Preferences'\n",
	"        to tweak " APP_NAME " appearance and behaviour.\n\n",
	"Local resources:\n",
	"        file:///path/file_name is considered an URL and will be XML-parsed, ie\n",
	"        a RSS or Atom format is expected.\n\n",
	"        /path/file_name will be processed as a non-XML text file, ie the file\n",
	"        will be read 'as is'.\n\n",
	"You can set your favourite browser in the Full Settings window. Otherwise, the\n",
	"system default one will be looked for.\n\n",
	APP_NAME " parses command line arguments and looks for option(s) then for one\n",
	"resource, the rest of the line is ignored. It also reads configuration file\n",
#ifndef G_OS_WIN32
	"'" APP_CMD "-conf' located in /home/<user_name>/" TICKR_DIR_NAME "/ if it exists (or\n",
	"'" APP_CMD "-conf<n>' if an instance id has been set to n.)\n\n",
#else
	"'" APP_CMD "-conf' located in <Application Data directory>" TICKR_DIR_NAME "/ if it exists (or\n",
	"'" APP_CMD "-conf<n>' if an instance id has been set to n.)\n\n",
#endif
	"Command line options override configuration file ones which override default\n",
	"ones.\n\n",
	"Compiled with GTK+2, Libxml2, GnuTLS and Libfribidi.\n\n",/*" on " __DATE__ " - " __TIME__ "\n\n",*/	/* Fix non-reproducible builds */
	"Visit " WEBSITE_URL " for more info.\n",
	NULL
};

static const char *license_str1[] = {
	APP_NAME " version " APP_V_NUM " - " COPYRIGHT_STR "\n\n",
	APP_NAME " is free software: you can redistribute it and/or modify\n",
	"it under the terms of the GNU General Public License as published by\n",
	"the Free Software Foundation, either version 3 of the License, or\n",
	"(at your option) any later version.\n\n",
	APP_NAME " is distributed in the hope that it will be useful,\n",
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n",
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n",
	"GNU General Public License for more details.\n\n",
	"You should have received a copy of the GNU General Public License\n",
	"along with this program. If not, see ",
	NULL
};

static const char *license_str2 =
	"http://www.gnu.org/licenses/\n";

const char** get_help_str0()
{
	return help_str0;
}

const char** get_help_str1()
{
	return help_str1;
}

const char** get_license_str1()
{
	return license_str1;
}

const char* get_license_str2()
{
	return license_str2;
}
