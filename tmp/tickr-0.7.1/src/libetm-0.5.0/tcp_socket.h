/*
 *	libetm / tcp_socket.h - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
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

#ifndef INC_LIBETM_TCP_SOCKET_H
#define INC_LIBETM_TCP_SOCKET_H

#define TCP_SOCK_OK			LIBETM_OK

/* Timeout default values */
#define CONNECT_TIMEOUT_SEC		5
#define CONNECT_TIMEOUT_USEC		0

#define SEND_RECV_TIMEOUT_SEC		1
#define SEND_RECV_TIMEOUT_USEC		0

#define RECV_CHUNK_LEN			(16 * 1024 - 1)

#ifndef WIN32_V
typedef int sockt;
#  define TCP_SOCK_CREATE_ERROR		-1	/* socket() */
#  define TCP_SOCK_FUNC_ERROR		-1	/* setsockopt(), bind(), listen(), select(), connect(),
						 * send(), recv(), fnctl(), ioctlsocket() */
#  define CLOSE_SOCK(s)			close(s)
#else
#  include <windows.h>				/* Dirty fix. */
typedef SOCKET sockt;
#  define TCP_SOCK_CREATE_ERROR		INVALID_SOCKET
#  define TCP_SOCK_FUNC_ERROR		SOCKET_ERROR
#  define CLOSE_SOCK(s)			closesocket(s)
#endif

void		set_send_recv_timeout_sec(int);

int		get_send_recv_timeout_sec();

void		set_send_recv_timeout_usec(int);

int		get_send_recv_timeout_usec();

void		set_connect_timeout_sec(int);

int		get_connect_timeout_sec();

void		set_connect_timeout_usec(int);

int		get_connect_timeout_usec();

/* Default is not using proxy, otherwise must be set. */
void		libetm_socket_set_use_proxy(zboolean);

zboolean	libetm_socket_get_use_proxy();

/*
 * Open stream socket in non-blocking mode and connect to host.
 * Return socket fd (> 0) / TCP_SOCK_CREATE_ERROR (-1 on Linux) if error.
 */
sockt		tcp_connect_to_host(const char *, const char *);

int		writable_data_is_available_on_tcp_socket(sockt);

int		readable_data_is_available_on_tcp_socket(sockt);

/*
 * Return n bytes sent or TCP_SOCK_FUNC_ERROR (-1 on Linux) if error (connection
 * closed by server or ?)
 */
int		tcp_send_full(sockt, const char *);

/*
 * Return response = tcp_recv_full(socket, &bytes_received, &status) or NULL if error.
 * -> status = TCP_SOCK_OK, CONNECTION_CLOSED_BY_SERVER, TCP_SOCK_FUNC_ERROR or TCP_SOCK_SHOULD_BE_CLOSED.
 * -> allocate memory for response (must be freed afterwards with free2() if != NULL).
 */
char		*tcp_recv_full(sockt, int *, int *);

const char	*sock_error_message();
#endif /* INC_LIBETM_TCP_SOCKET_H */
