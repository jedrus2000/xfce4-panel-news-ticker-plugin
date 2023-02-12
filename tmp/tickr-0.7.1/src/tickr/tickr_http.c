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

/* Are these values OK ? */
#define	HTTP_REQUEST_MAXLEN		(8 * 1024 - 1)
#define HTTP_HEADER_FIELD_MAXLEN	(2 * 1024 - 1)
#define MAX_HTTP_REDIRECT		8
#define DEFAULT_HTTP_PORT_STR		"80"
#define DEFAULT_HTTPS_PORT_STR		"443"
#define TAB				'\x09'

void remove_chunk_info(char **response)
{
	char	*response2;
	int	i, j;

	response2 = l_str_new(*response);
	response2[0] = '\0';
	i = go_after_next_empty_line(*response);
	while ((j = (int)get_http_chunk_size(*response + i)) > 0) {
		i += go_after_next_cr_lf(*response + i);
		str_n_cat(response2, *response + i, j);
		i += j;
		i += go_after_next_cr_lf(*response + i);
	}
	free2(*response);
	*response = response2;
}

/*
 * 'Quickly check' feed format (xml/rss2.0/rss1.0/atom), get shift to begining
 * and cut off trailing part.
 * Return OK or, if error, RESOURCE_ENCODING_ERROR or FEED_FORMAT_ERROR.
 */
static int format_quick_check(char *response, int *shift)
{
	int i;

	i = 0;
	while (strncmp(response + i, "<rss", 4) != 0 && response[i] != '\0')	/* rss 2.0 */
		i++;
	if (response[i] != '\0') {
		*shift = i;
		while (strncmp(response + i, "/rss>", 5) != 0 && response[i] != '\0')
			i++;
		if (response[i] != '\0')
			response[i + 5] = '\0';
	 	else
			return FEED_FORMAT_ERROR;
	} else {
		i = 0;
		while (strncmp(response + i, "<rdf:RDF", 8) != 0 && response[i] != '\0')	/* rss 1.0 */
			i++;
		if (response[i] != '\0') {
			*shift = i;
			while (strncmp(response + i, "/rdf:RDF>", 9) != 0 && response[i] != '\0')
				i++;
			if (response[i] != '\0')
				response[i + 9] = '\0';
			else
				return FEED_FORMAT_ERROR;
		} else {
			i = 0;
			while (strncmp(response + i, "<feed", 5) != 0 && response[i] != '\0')	/* atom */
				i++;
			if (response[i] != '\0') {
				*shift = i;
				while (strncmp(response + i, "/feed>", 6) != 0 && response[i] != '\0')
					i++;
				if (response[i] != '\0')
					response[i + 6] = '\0';
				else
					return FEED_FORMAT_ERROR;
			} else
				return FEED_FORMAT_ERROR;
		}
	}
	i = 0;
	while (strncmp(response + i, "<?xml", 5) != 0 && response[i] != '\0')
		i++;
	if (response[i] != '\0')
		*shift = i;
	return OK;
}

/*
 * Try to connect and set TCP socket.
 * Get host (and port if specified) from URL but if connecting through proxy.
 * Supported schemes are 'http', 'https' and 'file'.
 */
int connect_with_url(sockt *sock, char *url)
{
	char		scheme[URL_SCHEME_MAXLEN + 1];
	char		host[FILE_NAME_MAXLEN + 1];
	char		port_num[PORT_STR_MAXLEN + 1];
	static int	connect_fail_count = 0;

	str_n_cpy(scheme, get_scheme_from_url(url), URL_SCHEME_MAXLEN);
	if (!get_use_proxy()) {
		str_n_cpy(host, get_host_and_port_from_url(url, port_num), FILE_NAME_MAXLEN);
		/* Check first that port num, if any, is valid. */
		if (port_num[0] != 0 && !port_num_is_valid((const char *)port_num)) {
			warning(M_S_MOD, "%s:\nInvalid port number in URL: %s", url, port_num);
			return HTTP_INVALID_PORT_NUM;
		} else if (strcmp(scheme, "http") == 0) {
			if (port_num[0] == '\0')
				str_n_cpy(port_num, DEFAULT_HTTP_PORT_STR, PORT_STR_MAXLEN);
			else if (strcmp(port_num, DEFAULT_HTTP_PORT_STR) != 0)
				INFO_OUT("Will use non-standard HTTP port %s for this connection\n", port_num)
		} else if (strcmp(scheme, "https") == 0) {
			if (get_ticker_env()->https_support) {
				if (port_num[0] == '\0')
					str_n_cpy(port_num, DEFAULT_HTTPS_PORT_STR, PORT_STR_MAXLEN);
				else if (strcmp(port_num, DEFAULT_HTTPS_PORT_STR) != 0)
					INFO_OUT("Will use non-standard HTTPS port %s for this connection\n", port_num)
			} else {
				warning(M_S_MOD, "%s:\nNo HTTPS support (GnuTLS required version is %s",
					" but installed version is %s)", url, REQUIRED_GNUTLS_V_NUM_STR,
					gnutls_check_version(NULL));
				return HTTP_UNSUPPORTED_SCHEME;
			}
		} else if (strcmp(scheme, "file") == 0) {
			/* Do nothing here */
		} else if (scheme[0] == '\0') {
			warning(M_S_MOD, "No scheme found in URL: %s", url);
			return HTTP_UNSUPPORTED_SCHEME;
		} else {
			warning(M_S_MOD, "Unsupported or unknown scheme in URL: %s", scheme);
			return HTTP_UNSUPPORTED_SCHEME;
		}
	} else {
		/* Connect via proxy */
		str_n_cpy(host, get_proxy_host(), FILE_NAME_MAXLEN);
		str_n_cpy(port_num, get_proxy_port(), PORT_STR_MAXLEN);
	}
	if ((*sock = tcp_connect_to_host(host, port_num)) != TCP_SOCK_CREATE_ERROR) {
		connect_fail_count = 0;
		return OK;
	} else {
		if (get_use_proxy()) {
			warning(BLOCK, "Can't connect to proxy: %s:%s",
				get_proxy_host(), get_proxy_port());
			current_feed();
			connection_settings(PROXY_PAGE);
		} else
			warning(M_S_MOD, "Can't connect to host: %s", host);
		if (++connect_fail_count >= CONNECT_FAIL_MAX) {
			connect_fail_count = 0;
			return CONNECT_TOO_MANY_ERRORS;
		} else
			return TCP_SOCK_CANT_CONNECT;
	}
}

/* === TODO: Better document this func, still a bit confusing ===
 *
 * Do fetch resource specified by its id
 *
 * resrc_id = URL (including local URL) or full path/file name
 *
 * Here, resrc_id will be replaced by modified URL (in case of HTTP
 * moved-permanently redirects) and remote resource will be downloaded
 * and saved under file_name.
 *
 * === What about url ? ===
 *
 * Max length of resrc_id, file_name, and url = FILE_NAME_MAXLEN.
 */
int fetch_resource(char *resrc_id, const char *file_name, char *url)
{
	sockt		sock;
	int		status, recv_status;
	zboolean	http_moved_permamently;
	char		*response;
	char		*new_url;
	char		header_field[HTTP_HEADER_FIELD_MAXLEN + 1];
	FILE		*fp;
	int		content_length;
	char		orig_scheme[URL_SCHEME_MAXLEN + 1];
	char		orig_host[FILE_NAME_MAXLEN + 1];
	char		orig_port[PORT_STR_MAXLEN + 1];
	char		new_port[PORT_STR_MAXLEN + 1];
	GError		*error = NULL;
	char		*tmp = NULL;
	int		i, j;

	/* We first check the scheme */
	if (strncmp(resrc_id, "file://", strlen("file://")) == 0) {
		/* 'file' scheme */
		str_n_cpy(url, resrc_id + strlen("file://"), FILE_NAME_MAXLEN - strlen("file://"));
		if (strncmp(url, "localhost/", strlen("localhost/")) == 0)
			i = strlen("localhost/") - 1;
		else if (url[0] == '/')
			i = 0;
		else
			return RESOURCE_INVALID;

		if (g_file_get_contents(url + i, &response, NULL, &error)) {
			tmp = l_str_new(response);
			g_free(response);
			response = tmp;
			tmp = NULL;
			if ((j = format_quick_check(response, &i)) == OK) {
				if (g_file_set_contents(file_name, response + i, -1, &error)) {
					free2(response);	/* l_str_free() and free2() do the same things */
					return OK;
				} else {
					free2(response);
					if (error != NULL) {
						warning(BLOCK, error->message);	/* TODO: Should be more informative */
						g_error_free(error);
					} else
						warning(BLOCK, "Can't create file: '%s'", file_name);
					return CREATE_FILE_ERROR;
				}
			} else {
				free2(response);
				return j;
			}
		} else {
			if (error != NULL) {
				warning(M_S_MOD, error->message);	/* TODO: Should be more informative */
				g_error_free(error);
			} else
				warning(M_S_MOD, "Can't open file: '%s'", url + i);
			return OPEN_FILE_ERROR;
		}
	} else
		/* 'http' or 'https' scheme */
		str_n_cpy(url, resrc_id, FILE_NAME_MAXLEN);
	/* Connecting */
	if ((i = connect_with_url(&sock, url)) == OK) {
		if ((status = get_http_response(sock, "GET", "", url, &new_url,
				&response, &recv_status)) == TCP_SEND_ERROR || status == TCP_RECV_ERROR ||
				status == TLS_ERROR || status == TLS_SEND_ERROR || status == TLS_RECV_ERROR) {
			if (recv_status == CONNECTION_CLOSED_BY_SERVER || recv_status == TCP_SOCK_SHOULD_BE_CLOSED)
				CLOSE_SOCK(sock);
			return i;
		}
	} else
		return i;

	/* Keeping a copy of original URL in case of HTTP redirects. */
	str_n_cpy(get_resource()->orig_url, url, FILE_NAME_MAXLEN);	/* TODO: Is this OK here */

	while (1) {
		/*
		 * Status checked here are those returned by get_http_response()
		 * so we must make sure to catch all them.
		 * Beside HTTP status, they may also be error codes.
		 */
		if (status == HTTP_CONTINUE) {
			free2(response);
			status = get_http_response(sock, "GET", "", url, &new_url, &response, &recv_status);
			continue;
		} else if (status == HTTP_SWITCH_PROTO) {
			warning(M_S_MOD, "%s:\nRequest to switch protocol for this connection - Not supported", url);
			DEBUG_INFO("'Upgrade' header field in HTTP response: %s\n",
				get_http_header_value("Upgrade", response))
			free2(response);
			CLOSE_SOCK(sock);
			return status;
		} else if (status == OK) {
			if (strcmp(get_http_header_value("Transfer-Encoding", response), "chunked") == 0)
				/* Chunked transfer encoding */
				remove_chunk_info(&response);
			else if ((content_length = atoi(get_http_header_value("Content-Length", response))) > 0) {
				/*
				 * 'Content-Length' = length of entity body is mandatory
				 * but if 'Transfer-Encoding: chunked'.
				 */
				/* Do nothing */
			} else
				VERBOSE_INFO_ERR("No 'Transfer-Encoding' nor 'Content-Length' "
					"header field in HTTP response\n")
			/*
			 * Quickly 'check format' (xml and rss/atom stuff),
			 * shift to beginnig and cut off trailing part.
			 */
			if ((j = format_quick_check(response, &i)) != OK) {
				free2(response);
				return j;
			}
			/* If OK, save to dump file. */
			if ((fp = g_fopen(file_name, "wb")) != NULL) {
				fprintf(fp, "%s", response + i);
				fclose(fp);
				free2(response);
				CLOSE_SOCK(sock);
				return OK;
			} else {
				warning(BLOCK, "Can't create file: '%s'", file_name);
				fclose(fp);
				free2(response);
				CLOSE_SOCK(sock);
				return CREATE_FILE_ERROR;
			}
		} else if (status == HTTP_MOVED || status == HTTP_MOVED_PERMANENTLY) {
			if (status == HTTP_MOVED_PERMANENTLY)
				http_moved_permamently = TRUE;
			else
				http_moved_permamently = FALSE;
			str_n_cpy(get_resource()->orig_url, url, FILE_NAME_MAXLEN);
			str_n_cpy(orig_scheme, get_scheme_from_url(url), URL_SCHEME_MAXLEN);
			str_n_cpy(orig_host, get_host_and_port_from_url(url, orig_port), FILE_NAME_MAXLEN);
			i = 0;
			do {
				/*
				 * We test if URL scheme is present, to know whether new_url contains a full URL or a PATH.
				 * POSSIBLE FIXME: Could new_url contain host/path ?
				 */
				if (*get_scheme_from_url(new_url) != '\0')		/* Scheme -> full URL */
					str_n_cpy(url, new_url, FILE_NAME_MAXLEN);
				else							/* No scheme -> path */
					snprintf(url, FILE_NAME_MAXLEN + 1, "%s://%s%s",
						get_scheme_from_url(url),
						get_host_and_port_from_url(url, NULL),
						new_url);
				free2(response);
				/* No need to disconnect/reconnect if same scheme, host and port num. */
				if (strcmp(get_scheme_from_url(url), orig_scheme) != 0 ||
						strcmp(get_host_and_port_from_url(url, new_port), orig_host) != 0 ||
						strcmp(new_port, orig_port) != 0) {
					CLOSE_SOCK(sock);
					if ((j = connect_with_url(&sock, url)) != OK)
						return j;
				}
			} while (((status = get_http_response(sock, "GET", "", url,
				&new_url, &response, &recv_status)) == HTTP_MOVED ||
				status == HTTP_MOVED_PERMANENTLY) && ++i < MAX_HTTP_REDIRECT);
			if (status != HTTP_MOVED && status != HTTP_MOVED_PERMANENTLY && i <= MAX_HTTP_REDIRECT) {
				VERBOSE_INFO_OUT("Original URL: %s\nNew URL after HTTP redirect(s): %s\n",
					get_resource()->orig_url, url)
				DEBUG_INFO("status = %s\n", global_error_str(status))
				if (http_moved_permamently)
					str_n_cpy(resrc_id, url, FILE_NAME_MAXLEN);
				continue;
			} else {
				warning(M_S_MOD, "%s:\nToo many HTTP redirects", resrc_id);
				free2(response);
				CLOSE_SOCK(sock);
				return HTTP_TOO_MANY_REDIRECTS;
			}
		} else if (status == HTTP_USE_PROXY) {
			warning(BLOCK, "Resource: %s", "\nmust be accessed through proxy.\nProxy host: %s",
				url, new_url);
			str_n_cpy(get_proxy_host(), new_url, PROXY_HOST_MAXLEN);
			free2(response);
			CLOSE_SOCK(sock);
			connection_settings(PROXY_PAGE);
			current_feed();
			return status;
		} else if (status == HTTP_PROXY_AUTH_REQUIRED) {
			/* Proxy authentication */
			free2(response);
			CLOSE_SOCK(sock);
			if (get_use_proxy_auth()) {
				if ((i = connect_with_url(&sock, url)) != OK)
					return i;
				snprintf(header_field, HTTP_HEADER_FIELD_MAXLEN + 1,
					"Proxy-Authorization: Basic %s\r\n\r\n",
					get_proxy_auth_str());
				if ((status = get_http_response(sock, "GET", header_field, url, &new_url,
						&response, &recv_status)) == HTTP_PROXY_AUTH_REQUIRED) {
					warning(BLOCK, "Proxy authentication failed for: %s", get_proxy_host());
					free2(response);
					CLOSE_SOCK(sock);
					if (connection_settings(PROXY_PAGE) == GTK_RESPONSE_OK) {
						if ((i = connect_with_url(&sock, url)) != OK)
							return i;
						status = get_http_response(sock, "GET", header_field, url,
							&new_url, &response, &recv_status);
					} else
						return HTTP_NO_PROXY_AUTH_CREDENTIALS;
				}
				continue;
			} else {
				warning(BLOCK, "Proxy authentication required for: %s", get_proxy_host());
				if (connection_settings(PROXY_PAGE) == GTK_RESPONSE_OK && get_use_proxy_auth()) {
					if ((i = connect_with_url(&sock, url)) != OK)
						return i;
					snprintf(header_field, HTTP_HEADER_FIELD_MAXLEN + 1,
						"Proxy-Authorization: Basic %s\r\n\r\n",
						get_proxy_auth_str());
					status = get_http_response(sock, "GET", header_field, url,
						&new_url, &response, &recv_status);
					continue;
				} else
					return HTTP_NO_PROXY_AUTH_CREDENTIALS;
			}
		} else if (status == HTTP_UNAUTHORIZED) {
			/* HTTP authentication - only basic so far */
			free2(response);
			CLOSE_SOCK(sock);
			if (get_use_authentication()) {
				if ((i = connect_with_url(&sock, url)) != OK)
					return i;
				snprintf(header_field, HTTP_HEADER_FIELD_MAXLEN + 1,
					"Authorization: Basic %s\r\n\r\n",
					get_http_auth_str());
				if ((status = get_http_response(sock, "GET", header_field,
						url, &new_url, &response, &recv_status)) == HTTP_UNAUTHORIZED) {
					warning(BLOCK, "HTTP authentication failed for: %s", url);
					free2(response);
					CLOSE_SOCK(sock);
					if (connection_settings(AUTH_PAGE) == GTK_RESPONSE_OK) {
						if ((i = connect_with_url(&sock, url)) != OK)
							return i;
						status = get_http_response(sock, "GET", header_field,
							url, &new_url, &response, &recv_status);
					} else
						return HTTP_NO_AUTH_CREDENTIALS;
				}
				continue;
			} else {
				warning(BLOCK, "HTTP authentication required for: %s", url);
				if (connection_settings(AUTH_PAGE) == GTK_RESPONSE_OK && get_use_authentication()) {
					if ((i = connect_with_url(&sock, url)) != OK)
						return i;
					snprintf(header_field, HTTP_HEADER_FIELD_MAXLEN + 1,
						"Authorization: Basic %s\r\n\r\n",
						get_http_auth_str());
					status = get_http_response(sock, "GET", header_field,
						url, &new_url, &response, &recv_status);
					continue;
				} else
					return HTTP_NO_AUTH_CREDENTIALS;
			}
		} else if (	status == HTTP_BAD_REQUEST ||
				status == HTTP_FORBIDDEN ||
				status == HTTP_NOT_FOUND ||
				status == HTTP_GONE ||
				status == HTTP_INT_SERVER_ERROR ||
				status == HTTP_NO_STATUS_CODE) {
			warning(M_S_MOD, "%s:\n%s", url, global_error_str(status));
			free2(response);
			CLOSE_SOCK(sock);
			return status;
		} else {
			if (status > 10000)
				warning(M_S_MOD, "%s:\nResponse HTTP status code = %d",
					url, status - 10000);
			else
				DEBUG_INFO("%s\n", global_error_str(status))
			if (response != NULL)
				free2(response);
			CLOSE_SOCK(sock);
			return HTTP_ERROR;
		}
	}
}

/* 'rq_str' may contain header fields(s) separated and ended by "\r\n". */
const char *build_http_request(const char *method, const char *path,
	const char *host, const char* rq_str)
{
	static char str[HTTP_REQUEST_MAXLEN + 1];

	snprintf(str, HTTP_REQUEST_MAXLEN,
		"%s %s HTTP/1.1\r\n"	/* Start line with method and path */
		"Host: %s\r\n"		/* Mandatory host header field */
		"User-Agent: " APP_NAME "-" APP_V_NUM "\r\n"
		/* Seems totally useless -
		 * "Accept-Charset: utf-8\r\n"*/
		"%s\r\n",		/* Optional extra header field(s) */
		method, path, host, rq_str);
	return (const char *)str;
}

/*
 * 'rq_str' may contain header field(s) separated and ended by "\r\n".
 * Must 'free2' response afterwards.
 *
 * If returned value > 10000, it's an unprocessed HTTP status code + 10000.
 */
int get_http_response(sockt sock, const char *rq_method, const char *rq_str,
	const char *rq_url, char **new_rq_url, char **response, int *recv_status)
{
	char					*str;
	static char				location[FILE_NAME_MAXLEN + 1];
	int					bytes_sent, bytes_received, status_code;
	gnutls_session_t			*s;
	gnutls_certificate_credentials_t	*c;

	location[0] = '\0';
	*new_rq_url = (char *)location;
	*response = NULL;
	str = (char *)build_http_request(
		rq_method,
		(get_use_proxy() ? rq_url : get_path_from_url(rq_url)),	/* Path or full (absolute) URL if using proxy */
		/* is that correct when using proxy? */
		get_host_and_port_from_url(rq_url, NULL),
		rq_str);
	if (strcmp(get_scheme_from_url(rq_url), "http") == 0)
		bytes_sent = tcp_send_full(sock, (const char *)str);
	else {	/* https */
		s = get_tls_session();
		c = get_tls_cred();
		if (tls_connect(s, c, (const sockt *)&sock, (const char *)rq_url, get_host_and_port_from_url(rq_url, NULL)) == OK) {
			/*char *info;
			info = gnutls_session_get_desc(*s);
			DEBUG_INFO("tls_connect() = OK - TLS session info:\n%s\n", info)
			gnutls_free(info);*/
		} else
			return TLS_ERROR;
		bytes_sent = tcp_tls_send_full(sock, *get_tls_session(), (const char *)str);
	}
	if (bytes_sent >= 0) {
		if (strcmp(get_scheme_from_url(rq_url), "http") == 0)
			*response = tcp_recv_full(sock, &bytes_received, recv_status);
		else {	/* https */
			*response = tcp_tls_recv_full(sock, *get_tls_session(), &bytes_received, recv_status);
			tls_disconnect(get_tls_session(), get_tls_cred(), (const char *)rq_url);
		}
		if (*response != NULL) {
			if ((status_code = get_http_status_code(*response)) == 100)
				return HTTP_CONTINUE;
			else if (status_code == 101)
				return HTTP_SWITCH_PROTO;
			else if (status_code == 200)
				return OK;
			else if (	status_code == 300 ||	/* 'Multiple choices' */
					status_code == 302 ||	/* 'Found' */
					status_code == 303 ||	/* 'See other' */
					status_code == 304 ||	/* 'Not modified' */
					status_code == 307	/* 'Moved temporarily' */
				) {
				str_n_cpy(location, get_http_header_value("Location", *response),
					FILE_NAME_MAXLEN);
				return HTTP_MOVED;		/* Must use the new URL in 'Location' */
			} else if (status_code == 301) {	/* 'Moved permanently' */
				str_n_cpy(location, get_http_header_value("Location", *response),
					FILE_NAME_MAXLEN);
				return HTTP_MOVED_PERMANENTLY;	/* Must use the new URL in 'Location'
								 * and set resrc->id to new value */
			} else if (status_code == 305)
				return HTTP_USE_PROXY;
			else if (status_code == 400)
				return HTTP_BAD_REQUEST;
			else if (status_code == 401)
				return HTTP_UNAUTHORIZED;
			else if (status_code == 403)
				return HTTP_FORBIDDEN;
			else if (status_code ==  404)
				return HTTP_NOT_FOUND;
			else if (status_code == 407)
				return HTTP_PROXY_AUTH_REQUIRED;
			else if (status_code == 410)
				return HTTP_GONE;
			else if (status_code == 500)
				return HTTP_INT_SERVER_ERROR;
			else if (status_code == -1)
				return HTTP_NO_STATUS_CODE;
			else
				return status_code + 10000;
		} else {
			if (strcmp(get_scheme_from_url(rq_url), "http") == 0)
				return TCP_RECV_ERROR;
			else	/* https */
				return TLS_RECV_ERROR;
		}
	} else
		if (strcmp(get_scheme_from_url(rq_url), "http") == 0)
			return TCP_SEND_ERROR;
		else	/* https */
			return TLS_SEND_ERROR;
}

/* Return -1 if error. */
int get_http_status_code(const char *response)
{
	char	status_code[4];
	int	i = 0;

	while (response[i] != ' ' && response[i] != TAB) {
		if (response[i] == '\n' || response[i] == '\r' || response[i] == '\0')
			return -1;
		else
			i++;
	}
	while (response[i] == ' ' || response[i] == TAB)
		i++;
	if (response[i] == '\n' || response[i] == '\r' || response[i] == '\0')
		status_code[0] = '\0';
	else
		str_n_cpy(status_code, response + i, 3);
	return atoi(status_code);
}

/* Get header_value as string (empty one if header name not found). */
const char *get_http_header_value(const char *header_name, const char *response)
{
	static char	header_value[1024];
	int		len = strlen(header_name), i = 0;

	while (strncasecmp(response + i, header_name, len) != 0 && response[i] != '\0')
		i++;
	if (response[i] != '\0') {
		i += len;
		if (response[i] != ':') {
				header_value[0] = '\0';
				return (const char *)header_value;
		} else
			i++;
		while (response[i] == ' ' || response[i] == TAB) {
			if (response[i] == '\n' || response[i] == '\r' ||
					response[i] == '\0') {
				header_value[0] = '\0';
				return (const char *)header_value;
			} else
				i++;
		}
		str_n_cpy(header_value, response + i, 1023);
		i = 0;
		while (header_value[i] != '\0' && header_value[i] != ' ' &&
			header_value[i] != TAB && header_value[i] != '\n' &&
			header_value[i] != '\r' && i < 1023)
			i++;
		header_value[i] = '\0';
	} else
		header_value[0] = '\0';
	return (const char *)header_value;
}

/* Return 0 if none found. */
int go_after_next_cr_lf(const char *response)
{
	int	i = 0;

	while (strncmp(response + i, "\r\n", 2) != 0 && response[i] != '\0')
		i++;
	if (response[i] != '\0')
		return i + 2;
	else
		return 0;
}

/* Return 0 if none found. */
int go_after_next_empty_line(const char *response)
{
	int	i = 0;

	while (strncmp(response + i, "\r\n\r\n", 4) != 0 && response[i] != '\0')
		i++;
	if (response[i] != '\0')
		return i + 4;
	else
		return 0;
}

/*
 * 'response' must point to chunk size hexa str or preceeding space(s).
 * Return -1 if invalid chunk size format (ie not an hexa str).
 */
int get_http_chunk_size(const char *response)
{
	char	size_str[32], *tailptr;
	int	size, i = 0;

	while (response[i] == ' ' || response[i] == TAB ||
			response[i] == '\n' || response[i] == '\r')
		i++;
	str_n_cpy(size_str, response + i, 31);
	i = 0;
	while (size_str[i] != ' ' && size_str[i] != TAB &&
			size_str[i] != '\n' && size_str[i] != '\r' &&
			size_str[i] != ';' && size_str[i] != '\0' &&
			i < 31)
		i++;
	size_str[i] = '\0';
	size = (int)strtoul(size_str, &tailptr, 16);
	if (tailptr == size_str) {
		INFO_ERR("Invalid hexadecimal value in HTTP chunk size: %s\n",
			size_str)
		return -1;
	} else
		return size;
}

/*
 * 'http://www.sth1.org/sth2/sth3.xml' -> 'http'
 * (expect one scheme in URL, return empty str if none).
 */
const char *get_scheme_from_url(const char *url)
{
	static char	str[URL_SCHEME_MAXLEN + 1];
	int		i = 0;

	while (strncmp(url + i, "://", 3) != 0 && url[i] != '\0')
		i++;
	if (url[i] != '\0')
		str_n_cpy(str, url, MIN(i, URL_SCHEME_MAXLEN));
	else
		str[0] = '\0';
	return (const char *)str;
}

/*
 * 'http://www.sth1.org/sth2/sth3.xml' -> 'www.sth1.org', ''
 * 'http://www.sth1.org:80/sth2/sth3.xml' -> 'www.sth1.org', '80'
 * (expect one scheme in URL, return empty str if none).
 * port_num may be NULL. If port_num != NULL, MAKE SURE that it can handle
 * PORT_STR_MAXLEN chars.
 */
const char *get_host_and_port_from_url(const char *url, char *port_num)
{
	static char	str[FILE_NAME_MAXLEN + 1];
	int		i = 0, j = 0;

	while (strncmp(url + i, "://", 3) != 0 && url[i] != '\0')
		i++;
	if (url[i] != '\0') {
		str_n_cpy(str, url + i + 3, FILE_NAME_MAXLEN);
		i = 0;
		while (str[i] != '\0' && str[i] != '/' && i < FILE_NAME_MAXLEN) {
			if (str[i] == ':') {
				j = i;
				break;
			} else
				i++;
		}
		str[i] = '\0';
		if (port_num != NULL) {
			port_num[0] = '\0';
			if (j != 0) {
				j++;
				if (url[j] != '\0') {
					while (str[j] != '\0' && str[j] != '/' && j < FILE_NAME_MAXLEN)
						j++;
					str[j] = '\0';
					str_n_cpy(port_num, str + i + 1, PORT_STR_MAXLEN);
				}
			}
		}
	} else
		str[0] = '\0';
	return (const char *)str;
}

zboolean port_num_is_valid(const char *port_num)
{
	if (str_is_num(port_num) && atoi(port_num) > 0 && atoi(port_num) < 65536)
		return TRUE;
	else
		return FALSE;
}

/*
 * 'http://www.sth1.org/sth2/sth3.xml' -> '/sth2/sth3.xml'
 * (expect one scheme in URL, return empty str if none).
 */
const char *get_path_from_url(const char *url)
{
	static char	str[FILE_NAME_MAXLEN + 1];
	int		i = 0;

	while (strncmp(url + i, "://", 3) != 0 && url[i] != '\0')
		i++;
	if (url[i] != '\0') {
		i += 3;
		while (url[i] != '\0' && url[i] != '/')
			i++;
		if (url[i] != '\0')
			str_n_cpy(str, url + i, FILE_NAME_MAXLEN);
		else
			str[0] = '\0';
	} else
		str[0] = '\0';
	return (const char *)str;
}
