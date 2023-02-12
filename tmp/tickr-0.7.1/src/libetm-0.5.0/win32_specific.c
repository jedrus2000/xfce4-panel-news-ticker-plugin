/*
 *	libetm / win32_specific.c - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
 *
 *	- Win32 specific functions -
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef WIN32_V

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <rpcdce.h>
#include <Iphlpapi.h>
#include <winreg.h>
#include <shlobj.h>
#include "libetm.h"

#define APP_WIN32REG_KEYPATH	"app_win32_registry_keypath"	/* Or whatever */

FILE *std_out, *std_err;

/*
 * On Linux, STD_OUT = stdout and STD_ERR = stderr / on win32, we must
 * first create specific text files then pass them to this function.
 */
void libetm_init_win32_stdout_stderr(FILE *std_out2, FILE *std_err2)
{
	std_out = std_out2;
	std_err = std_err2;
}

void libetm_init_win32_sockets()
{
	WSADATA		wsadata;
	int		i;

	if ((i = WSAStartup(MAKEWORD(2, 2), &wsadata)) != 0)
		big_error/*_in_lib*/(WIN32_ERROR, "WSAStartup() error ", itoa2(i));
	else if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
		WSACleanup();
		big_error/*_in_lib*/(WIN32_ERROR, "Couldn't find a usable version of Winsock.dll");
	}
}

void libetm_cleanup_win32_sockets()
{
	WSACleanup();
}

/* Return NULL if error */
const char *get_appdata_dir()
{
	static TCHAR	appdata_dir[MAX_PATH + 1];
	static int	i = 0;

	if (i == 0) {
		i++;
		if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata_dir) != S_OK)
			i++;
	}
	if (i == 1)
		return (const char *)appdata_dir;
	else
		return NULL;
}

/* Return NULL if error */
const char *get_appdata_dir_w()
{
	static WCHAR	appdata_dir[MAX_PATH + 1];
	static int	i = 0;

	if (i == 0) {
		i++;
		if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdata_dir) != S_OK)
			i++;
	}
	if (i == 1)
		return (const char *)appdata_dir;
	else
		return NULL;
}

/* Return NULL if error */
const char *get_progfiles_dir()
{
	static TCHAR	progfiles_dir[MAX_PATH + 1];
	static int	i = 0;

	if (i == 0) {
		i++;
		if (SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, progfiles_dir) != S_OK)
			i++;
	}
	if (i == 1)
		return (const char *)progfiles_dir;
	else
		return NULL;
}

/* key_value must be able to store 255 chars */
int get_key_value_from_win32registry(const char *key_name, char *key_value)
{
	char	app_win32regkey_full[128];
	HKEY	hkey;
	DWORD	type = REG_SZ, buf_size = 256;
	LONG	result;

	str_n_cpy(app_win32regkey_full, APP_WIN32REG_KEYPATH, 64);
	str_n_cat(app_win32regkey_full, key_name, 63);

	if ((result = RegOpenKeyEx(HKEY_CURRENT_USER, app_win32regkey_full,
			0L, KEY_QUERY_VALUE, &hkey)) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hkey, NULL, NULL, &type, (unsigned char*)key_value,
				&buf_size) == ERROR_SUCCESS) {
			RegCloseKey(hkey);
			return LIBETM_OK;
		} else {
			RegCloseKey(hkey);
			return WIN32REGKEY_NOT_FOUND;
		}
	} else {
		if (result == ERROR_FILE_NOT_FOUND)
			return WIN32REGKEY_NOT_FOUND;
		else
			return WIN32REGKEY_OTHER_ERROR;
	}
}

int save_key_value_into_win32registry(const char *key_name, const char *key_value)
{
	char	app_win32regkey_full[128];
	HKEY	hkey;
	DWORD	disp = 0;

	str_n_cpy(app_win32regkey_full, APP_WIN32REG_KEYPATH, 64);
	str_n_cat(app_win32regkey_full, key_name, 63);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, app_win32regkey_full, 0L,
			NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, &disp)
			== ERROR_SUCCESS) {
		if (RegSetValueEx(hkey, NULL, 0L, REG_SZ, (unsigned char *)TEXT(key_value),
				256) == ERROR_SUCCESS) {
			RegCloseKey(hkey);
			return LIBETM_OK;
		} else {
			RegCloseKey(hkey);
			return  WIN32REGKEY_SAVE_ERROR;
		}
	} else
		return WIN32REGKEY_CREATE_ERROR;
}

/* Return NULL if error */
const char *get_default_browser_from_win32registry()
{
	static char	browser_cmd[512];
	HKEY		hkey;
	DWORD		type = REG_SZ, buf_size = 512;
	LONG		result;

	if ((result = RegOpenKeyEx(HKEY_CLASSES_ROOT, "http\\shell\\open\\command",
			0L, KEY_QUERY_VALUE, &hkey)) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hkey, NULL, NULL, &type, (unsigned char*)browser_cmd,
				&buf_size) == ERROR_SUCCESS) {
			RegCloseKey(hkey);
			return (const char *)browser_cmd;
		} else {
			RegCloseKey(hkey);
			return NULL;
		}
	} else
		return NULL;
}

/* Return -1 if error */
int get_win32_taskbar_height()
{
	HWND	hwnd;
	RECT	r;
	LPRECT	lpRect = &r;

	if ((hwnd = FindWindow("Shell_traywnd", "")) != NULL) {
		if (GetWindowRect(hwnd, lpRect))
			return (int) (lpRect->bottom - lpRect->top);
	}
	return -1;
}

/* Unused so commented out */
/*zboolean is_vista_or_higher()
{
	OSVERSIONINFO os_v_info;

	memset(&os_v_info, 0, sizeof(OSVERSIONINFO));
	os_v_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os_v_info);
	return (os_v_info.dwMajorVersion >= 6 ? TRUE : FALSE);	// Vista = 6.0
}*/

/* Unused so commented out */
/*
 * Find up to 15 mac addresses for this computer
 * Return NULL if error
 */
/*const char **find_mac_addresses()
{
	IP_ADAPTER_INFO		*adapter_info;
	ULONG			buf_len = sizeof(adapter_info);
	PIP_ADAPTER_INFO	p_adapter_info;
	static char		macaddr[16][256], tmp[3];
	static char		*p_macaddr[16];
	unsigned int		i, j = 0;

	adapter_info = (IP_ADAPTER_INFO *)malloc2(sizeof(IP_ADAPTER_INFO));
	buf_len = sizeof(IP_ADAPTER_INFO);
	// initial call is supposed to fail
	if (GetAdaptersInfo(adapter_info, &buf_len) != ERROR_SUCCESS) {
		free2(adapter_info);
		adapter_info = (IP_ADAPTER_INFO *)malloc2(buf_len);
	}
	// 2nd call
	if (GetAdaptersInfo(adapter_info, &buf_len) != ERROR_SUCCESS) {
		free2(adapter_info);
		return NULL;
	} else {
		p_adapter_info = (PIP_ADAPTER_INFO)adapter_info;
		while (p_adapter_info && j < 15) {
			macaddr[j][0] = '\0';
			for (i = 0; i < 127 && i < p_adapter_info->AddressLength; i++) {
				snprintf(tmp, 3, "%02X", p_adapter_info->Address[i]);
				str_n_cat(macaddr[j], tmp, 2);
			}
			p_macaddr[j] = macaddr[j];
			j++;
			p_adapter_info = p_adapter_info->Next;
		}
		free2(adapter_info);
		p_macaddr[j] = NULL;
	}
	return (const char **)p_macaddr;
}*/

const char *win32_error_msg(int i)
{
	static char	str[N_SIMULTANEOUS_CALLS][1024];
	LPVOID		msg_buf;
	static int	count = -1;

	count++;
	count &= N_SIMULTANEOUS_CALLS_MASK;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, (DWORD)i, MAKELANGID(LANG_NEUTRAL,
		SUBLANG_DEFAULT), (LPTSTR)&msg_buf, 0, NULL);
	str_n_cpy(str[count], (const char *)msg_buf, 1023);
	LocalFree(msg_buf);
	return (const char *)str[count];
}
#endif
