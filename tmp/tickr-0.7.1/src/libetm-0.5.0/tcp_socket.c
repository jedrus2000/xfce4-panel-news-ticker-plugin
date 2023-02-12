/*
 *	libetm / tcp_socket.c - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
 *
 *	- A few TCP (stream) sockets functions -
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#ifndef WIN32_V
#  include <sys/socket.h>
#  include <netdb.h>
#  include <arpa/inet.h>
#else
#  define _WIN32_WINNT	0x0501	/* Win version = XP (5.1) or higher */
#  include <ws2tcpip.h>
#endif
#include "libetm.h"

#define FPRINTF2(stream, ...) \
{\
	fprintf(stream, ## __VA_ARGS__);\
	fflush(stream);\
}

static int	send_recv_timeout_sec = SEND_RECV_TIMEOUT_SEC;
static int	send_recv_timeout_usec = SEND_RECV_TIMEOUT_USEC;
static int	connect_timeout_sec = CONNECT_TIMEOUT_SEC;
static int	connect_timeout_usec = CONNECT_TIMEOUT_USEC;

static zboolean	use_proxy = FALSE;

void set_send_recv_timeout_sec(int timeout)
{
	send_recv_timeout_sec = timeout;
}

int get_send_recv_timeout_sec()
{
	return send_recv_timeout_sec;
}

void set_send_recv_timeout_usec(int timeout)
{
	send_recv_timeout_usec = timeout;
}

int get_send_recv_timeout_usec()
{
	return send_recv_timeout_usec;
}

void set_connect_timeout_sec(int timeout)
{
	connect_timeout_sec = timeout;
}

int get_connect_timeout_sec()
{
	return connect_timeout_sec;
}

void set_connect_timeout_usec(int timeout)
{
	connect_timeout_usec = timeout;
}

int get_connect_timeout_usec()
{
	return connect_timeout_usec;
}

void libetm_socket_set_use_proxy(zboolean use_proxy2)
{
	use_proxy = use_proxy2;
}

zboolean libetm_socket_get_use_proxy()
{
	return use_proxy;
}

/*
 * Can use IPv4 or IPv6
 */
#ifndef WIN32_V
static void *get_in_addr(struct sockaddr *s_a)
{
	if (s_a->sa_family == AF_INET)
		return &(((struct sockaddr_in *)s_a)->sin_addr);
	else
		return &(((struct sockaddr_in6 *)s_a)->sin6_addr);
}
#endif

/*
 * Open stream socket in non-blocking mode and connect to host.
 * Return socket fd (> 0) / TCP_SOCK_CREATE_ERROR (-1 on Linux) if error.
 */
sockt tcp_connect_to_host(const char *host, const char *portnum_str)
{
	sockt			sock;
#ifndef WIN32_V
	char			ipa_str[INET6_ADDRSTRLEN + 1];
#else
/*#define WIN32_INET6_ADDRSTRLEN	46
	char			ipa_str[WIN32_INET6_ADDRSTRLEN + 1];*/
	u_long			i_mode = 1;	/* != 0 to enable non-blocking mode */
#endif
	struct addrinfo	hints, *server_info, *ptr;
	fd_set			read_set, write_set;
	struct timeval		timeout;
	int			s_opt_value;
	socklen_t		s_opt_len = sizeof(sockt);
	int			i;

	/* addrinfo stuff */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/*VERBOSE_INFO_OUT("Resolving host: %s ... ", host)*/
	if ((i = getaddrinfo(host, portnum_str, &hints, &server_info)) != 0) {
		if (libetm_socket_get_use_proxy()) {
#ifndef WIN32_V
			warning(FALSE, "getaddrinfo() error: %s: %s", host, gai_strerror(i));
#else
			warning(FALSE, "getaddrinfo() error: %s: %s", host, sock_error_message());
#endif
		} else {
#ifndef WIN32_V
			INFO_ERR("getaddrinfo() error: %s: %s\n", host, gai_strerror(i))
#else
			INFO_ERR("getaddrinfo() error: %s: %s\n", host, sock_error_message())
#endif
		}
		return -1;
	}
	/*VERBOSE_INFO_OUT("Done\n")*/
	/* We get a list */
	for (ptr = server_info; ptr != NULL; ptr = ptr->ai_next) {
		/* Create socket */
		if ((sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == TCP_SOCK_CREATE_ERROR) {
			INFO_ERR("Error: %s\n", sock_error_message())
			continue;
		}
		/* Set socket in non-blocking mode */
#ifndef WIN32_V
		if ((i = fcntl(sock, F_GETFL, 0)) == TCP_SOCK_FUNC_ERROR) {
			INFO_ERR("fcntl() error: %s\n", sock_error_message())
			CLOSE_SOCK(sock);
			break;
		} else if (fcntl(sock, F_SETFL, i | O_NONBLOCK) == TCP_SOCK_FUNC_ERROR) {
			INFO_ERR("fcntl() error: %s\n", sock_error_message())
			CLOSE_SOCK(sock);
			break;
		}
#else
		if (ioctlsocket(sock, FIONBIO, &i_mode) == TCP_SOCK_FUNC_ERROR) {
			INFO_ERR("ioctlsocket() error %s\n", sock_error_message())
			CLOSE_SOCK(sock);
			break;
		}
#endif
		/* Get IP addr from server_info */
#ifndef WIN32_V
		inet_ntop(ptr->ai_family, get_in_addr((struct sockaddr *)ptr->ai_addr), ipa_str, INET6_ADDRSTRLEN);
		VERBOSE_INFO_OUT("Connecting to %s (%s) on port %s ... ", ipa_str, host, portnum_str);
#else
		/*if (is_vista_or_higher()) {
			// Available only on Vista and above
			InetNtop(ptr->ai_family, get_in_addr((struct sockaddr *)ptr->ai_addr), ipa_str, WIN32_INET6_ADDRSTRLEN);
			VERBOSE_INFO_OUT"Connecting to %s (%s) on port %s ... ", ipa_str, host, portnum_str)
		} else*/
			VERBOSE_INFO_OUT("Connecting to %s on port %s ... ", host, portnum_str);
#endif
		/* Connect */
		if ((i = connect(sock, ptr->ai_addr, ptr->ai_addrlen)) == TCP_SOCK_FUNC_ERROR &&
#ifndef WIN32_V
		 		errno == EINPROGRESS) {
#else
				WSAGetLastError() == WSAEWOULDBLOCK) {
#endif
			/* As socket is in non-blocking mode, we must use select() */
			FD_ZERO(&read_set);
			FD_ZERO(&write_set);
			FD_SET(sock, &read_set);
			FD_SET(sock, &write_set);
			timeout.tv_sec = get_connect_timeout_sec();
			timeout.tv_usec = get_connect_timeout_usec();
			if ((i = select(sock + 1, &read_set, &write_set, NULL, &timeout)) == TCP_SOCK_FUNC_ERROR) {
				INFO_ERR("select() error: %s\n", sock_error_message())
			} else if (i == 0){
				INFO_ERR("Timed out\n")
			} else if (FD_ISSET(sock, &read_set) || FD_ISSET(sock, &write_set)) {
				if (getsockopt(sock, SOL_SOCKET, SO_ERROR,
						(void *)(&s_opt_value), &s_opt_len) == TCP_SOCK_FUNC_ERROR) {
					INFO_ERR("getsockopt() error: %s\n", sock_error_message())
				} else if (s_opt_value == 0) {
					VERBOSE_INFO_OUT("OK\n")
					freeaddrinfo(server_info);
					return sock;
				}
#ifndef WIN32_V
				INFO_ERR("getsockopt(): %s\n", strerror(s_opt_value))
#else
				INFO_ERR("getsockopt(): %s\n", win32_error_msg(s_opt_value))
#endif
				CLOSE_SOCK(sock);
				break;
			}
			CLOSE_SOCK(sock);
			break;
		} else if (i == 0) {
			VERBOSE_INFO_OUT("OK\n")
			freeaddrinfo(server_info);
			return sock;
		} else {
			INFO_ERR("connect() error: %s\n", sock_error_message())
			CLOSE_SOCK(sock);
		}
	}
	freeaddrinfo(server_info);
	return -1;
}

int writable_data_is_available_on_tcp_socket(sockt sock)
{
	fd_set		write_set;
	struct timeval	timeout;
	int		i;

	FD_ZERO(&write_set);
	FD_SET(sock, &write_set);
	timeout.tv_sec = get_send_recv_timeout_sec();
	timeout.tv_usec = get_send_recv_timeout_usec();
	if ((i = select(sock + 1, NULL, &write_set, NULL, &timeout)) == TCP_SOCK_FUNC_ERROR) {
		INFO_ERR("select() error: %s\n", sock_error_message())
		return SELECT_ERROR;
	} else if (i == 0) {
		return SELECT_TIMED_OUT;
	} else {
		if (FD_ISSET(sock, &write_set))
			return SELECT_TRUE;
		else
			return SELECT_FALSE;
	}
}

int readable_data_is_available_on_tcp_socket(sockt sock)
{
	fd_set		read_set;
	struct timeval	timeout;
	int		i;

	FD_ZERO(&read_set);
	FD_SET(sock, &read_set);
	timeout.tv_sec = get_send_recv_timeout_sec();
	timeout.tv_usec = get_send_recv_timeout_usec();
	if ((i = select(sock + 1, &read_set, NULL, NULL, &timeout)) == TCP_SOCK_FUNC_ERROR) {
		INFO_ERR("select() error: %s\n", sock_error_message())
		return SELECT_ERROR;
	} else if (i == 0) {
		return SELECT_TIMED_OUT;
	} else {
		if (FD_ISSET(sock, &read_set))
			return SELECT_TRUE;
		else
			return SELECT_FALSE;
	}
}

/*
 * Return n bytes sent or TCP_SOCK_FUNC_ERROR (-1 on Linux) if error (connection
 * closed by server or ?)
 */
int tcp_send_full(sockt sock, const char *str)
{
	int len = strlen(str), i, j = 0;

	while (writable_data_is_available_on_tcp_socket(sock) == SELECT_TRUE) {
		if ((i = send(sock, str + j, len, 0)) != TCP_SOCK_FUNC_ERROR) {
			if (i > 0) {
				j += i;
				len -= i;
				if (len == 0)
					break;
			} else {
				/* Something to do ? */
			}
		} else {
			j = i;	/* TCP_SOCK_FUNC_ERROR */
#ifndef WIN32_V
			if (errno == EPIPE) {
#else
			if ((i = WSAGetLastError()) == WSAECONNRESET || i == WSAECONNABORTED ||\
					i == WSAESHUTDOWN) {
#endif
				VERBOSE_INFO_ERR("Connection closed by server\n")
			} else {
				INFO_ERR("send() error: %s\n", sock_error_message())
			}
			break;
		}
	}
	return j;
}

/*
 * Return response = tcp_recv_full(socket, &bytes_received, &status) or NULL if error.
 * -> status = TCP_SOCK_OK, CONNECTION_CLOSED_BY_SERVER, TCP_SOCK_FUNC_ERROR or TCP_SOCK_SHOULD_BE_CLOSED.
 * -> allocate memory for response (must be freed afterwards with free2() if != NULL).
 */
char *tcp_recv_full(sockt sock, int *bytes_received, int *status)
{
	char	*response, *full_response;
	int	i;

	*bytes_received = 0;
	response = malloc2(RECV_CHUNK_LEN + 1);
	response[0] = '\0';
	full_response = l_str_new(response);
	while (readable_data_is_available_on_tcp_socket(sock) == SELECT_TRUE) {
		if ((i = recv(sock, response, RECV_CHUNK_LEN, 0)) != TCP_SOCK_FUNC_ERROR) {
			if (i > 0) {
				response[MIN(i, RECV_CHUNK_LEN)] = '\0';
				full_response = l_str_cat(full_response, response);
				*bytes_received += i;
				*status = TCP_SOCK_OK;
			} else if (i == 0) {
				VERBOSE_INFO_ERR("Connection closed by server\n")
				*status = CONNECTION_CLOSED_BY_SERVER;
				break;
			}
		} else {
			l_str_free(full_response);
			full_response = NULL;
			*status = TCP_SOCK_FUNC_ERROR;
#ifndef WIN32_V
			INFO_ERR("recv() error: %s\n", sock_error_message())
#else
			if ((i = WSAGetLastError()) == WSAECONNRESET || i == WSAECONNABORTED ||\
					i == WSAESHUTDOWN) {
				VERBOSE_INFO_ERR("Connection closed by server\n")
				*status = TCP_SOCK_SHOULD_BE_CLOSED;
			} else {
				INFO_ERR("recv() error: %s\n", win32_error_msg(i))
			}
#endif
			break;
		}
	}
	free2(response);
	return full_response;
}

const char *sock_error_message()
{
	static char	str[N_SIMULTANEOUS_CALLS][1024];
	static int	count = -1;

	count++;
	count &= N_SIMULTANEOUS_CALLS_MASK;

#ifndef WIN32_V
	str_n_cpy(str[count], strerror(errno), 1023);
#else
	str_n_cpy(str[count], win32_error_msg(WSAGetLastError()), 1023);
#endif
	return (const char *)str[count];
}
