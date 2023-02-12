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

#ifndef INC_TICKR_TLS_H
#define INC_TICKR_TLS_H

#define REQUIRED_GNUTLS_V_NUM		0x030109
#define REQUIRED_GNUTLS_V_NUM_STR	"3.1.9"

#define TLS_RECV_CHUNK_LEN		RECV_CHUNK_LEN		/*  In libetm-0.5.0/tcp_socket.h */

gnutls_session_t			*get_tls_session();
gnutls_certificate_credentials_t	*get_tls_cred();
int					tls_connect(gnutls_session_t *, gnutls_certificate_credentials_t *,
						const sockt *, const char *, const char *);
void					tls_disconnect(gnutls_session_t *, gnutls_certificate_credentials_t *,
						const char *);
int					verif_cert_callback(gnutls_session_t);
int					tcp_tls_send_full(sockt, gnutls_session_t, const char *);
char					*tcp_tls_recv_full(sockt, gnutls_session_t, int *, int *);
#endif /* INC_TICKR_TLS_H */
