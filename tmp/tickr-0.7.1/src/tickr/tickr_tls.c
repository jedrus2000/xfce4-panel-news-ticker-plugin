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
#include "tickr_tls.h"

/* Global GnuTLS variables: session and credentials */
static gnutls_session_t				tls_s;
static gnutls_certificate_credentials_t		tls_c;

gnutls_session_t *get_tls_session()
{
	return &tls_s;
}

gnutls_certificate_credentials_t *get_tls_cred()
{
	return &tls_c;
}

/*
 * Retrieve available credentials and check X509 server certificate.
 */
int tls_connect(gnutls_session_t *s, gnutls_certificate_credentials_t *c,
		const sockt *sock, const char *url, const char *host)
{
	int i;

	if ((i = gnutls_certificate_allocate_credentials(c)) != GNUTLS_E_SUCCESS) {
		warning(M_S_MOD, "%s:\ngnutls_certificate_allocate_credentials() error: %s",
			url, gnutls_strerror(i));
		return TLS_ERROR;
	}
	if ((i = gnutls_certificate_set_x509_system_trust(*c)) < 0) {
		warning(M_S_MOD, "%s:\ngnutls_certificate_set_x509_system_trust() error: %s",
			url, gnutls_strerror(i));
		gnutls_certificate_free_credentials(*c);
		return TLS_ERROR;
	} else {
		DEBUG_INFO("gnutls_certificate_set_x509_system_trust(): %d certificates processed\n", i);
	}
	gnutls_certificate_set_verify_function(*c, verif_cert_callback);
	if ((i = gnutls_init(s, GNUTLS_CLIENT)) != GNUTLS_E_SUCCESS) {
		warning(M_S_MOD, "%s:\ngnutls_init() error: %s", url, gnutls_strerror(i));
		gnutls_certificate_free_credentials(*c);
		return TLS_ERROR;
	}
	gnutls_session_set_ptr(*s, (void *)host);
	gnutls_server_name_set(*s, GNUTLS_NAME_DNS, host, strlen(host));
	if ((i = gnutls_priority_set_direct(*s, "NORMAL:%COMPAT", NULL)) != GNUTLS_E_SUCCESS) {
		warning(M_S_MOD, "%s:\ngnutls_priority_set_direct() error: %s", url, gnutls_strerror(i));
		gnutls_certificate_free_credentials(*c);
		gnutls_deinit(*s);
		return TLS_ERROR;
	} else if ((i = gnutls_credentials_set(*s, GNUTLS_CRD_CERTIFICATE, *c)) != GNUTLS_E_SUCCESS) {
		warning(M_S_MOD, "%s:\ngnutls_credentials_set() error: %s", url, gnutls_strerror(i));
		gnutls_certificate_free_credentials(*c);
		gnutls_deinit(*s);
		return TLS_ERROR;
	}
	gnutls_transport_set_int(*s, *sock);
	gnutls_handshake_set_timeout(*s, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
	do {
		i = gnutls_handshake(*s);
		if (i < 0 && gnutls_error_is_fatal(i)) {
			warning(M_S_MOD, "%s:\ngnutls_handshake() error: %s", url, gnutls_strerror(i));
			gnutls_certificate_free_credentials(*c);
			gnutls_deinit(*s);
			return TLS_ERROR;
		}
	} while (i < 0);
	return OK;
}

void tls_disconnect(gnutls_session_t *s, gnutls_certificate_credentials_t *c, const char *url)
{
	int i, j;

	gnutls_certificate_free_credentials(*c);
	for (j = 0; j < 3; j++) {	/* 3 times */
		if ((i = gnutls_bye(*s, GNUTLS_SHUT_RDWR)) == GNUTLS_E_SUCCESS)
			break;
		else if (i == GNUTLS_E_AGAIN || i == GNUTLS_E_INTERRUPTED)
			continue;
		else {
			INFO_ERR("%s:\ngnutls_bye() error: %s\n", url, gnutls_strerror(i))
			break;
		}
	}
	gnutls_deinit(*s);
}

int verif_cert_callback(gnutls_session_t s)
{
	char		*host;
	unsigned int	status;
	int		type;
	gnutls_datum_t	output;
	int		i;

	host = gnutls_session_get_ptr(s);
	if ((i = gnutls_certificate_verify_peers3(s, host, &status)) < 0) {
		INFO_ERR("gnutls_certificate_verify_peers3() error: %s\n", gnutls_strerror(i))
		return GNUTLS_E_CERTIFICATE_ERROR;
	}
	if (status == 0) {
		DEBUG_INFO("Certificate is OK, yep !!\n");
		return 0;
	} else {
		/*if (question_win("WARNING: Certificate is not trusted. Continue anyways ?") == YES)
			return 0;
		else
			return GNUTLS_E_CERTIFICATE_ERROR;*/
		type = gnutls_certificate_type_get(s);
		if ((i = gnutls_certificate_verification_status_print(status, type, &output, 0)) < 0) {
			INFO_ERR("gnutls_certificate_verification_status_print() error: %s\n",
				gnutls_strerror(i))
			return GNUTLS_E_CERTIFICATE_ERROR;
		}
		warning(M_S_MOD, "Certificate is not trusted\n%s", output.data);
		gnutls_free(output.data);
		return GNUTLS_E_CERTIFICATE_ERROR;
	}
}

/*
 * Using GnuTLS session.
 * TODO: Re-check everything, especially errno vs gnutls error codes
 */
#define	FPRINTF_FFLUSH_2(a1, a2);		{fprintf(a1, a2); fflush(a1);}
#define	FPRINTF_FFLUSH_3(a1, a2, a3);		{fprintf(a1, a2, a3); fflush(a1);}

/*
 * Return n bytes sent or (if < 0) some GNUTLS_E_ error.
 */
int tcp_tls_send_full(sockt sock, gnutls_session_t s, const char *str)
{
	int len = strlen(str), i, j = 0;

	while (writable_data_is_available_on_tcp_socket(sock) == SELECT_TRUE) {
		if ((i = gnutls_record_send(s, str + j, len)) > 0) {
			j += i;
			len -= i;
			if (len == 0)
				break;
		} else if (i == 0) {
			if (len > 0)
				VERBOSE_INFO_ERR("tcp_tls_send_full(): Not all bytes sent ?")
			break;
		} else if (i == GNUTLS_E_AGAIN || i == GNUTLS_E_INTERRUPTED) {
			continue;
		} else if (i == GNUTLS_E_REHANDSHAKE) {
			/*INFO_ERR("Server requests TLS renegotiation\n")*/
			INFO_ERR("gnutls_record_send() error: %s\n", gnutls_strerror(i))
			j = i;
			break;
		} else {
			INFO_ERR("gnutls_record_send() error: %s\n", gnutls_strerror(i))
			if (gnutls_error_is_fatal(i)) {
				j = i;
				break;
			}
		}
	}
	return j;
}

/*
 * Return response = tcp_tls_recv_full(socket, gnutls_session, &bytes_received, &status)
 * or NULL if error.
 * -> status = OK or CONNECTION_CLOSED_BY_SERVER or some GNUTLS_E_ error.
 * -> allocate memory for response (must be freed afterwards with free2() if != NULL).
 */
char *tcp_tls_recv_full(sockt sock, gnutls_session_t s, int *bytes_received, int *status)
{
	char	*response, *full_response;
	int	i;

	*bytes_received = 0;
	response = malloc2(TLS_RECV_CHUNK_LEN + 1);
	response[0] = '\0';
	full_response = l_str_new(response);
	while (readable_data_is_available_on_tcp_socket(sock) == SELECT_TRUE) {
		if ((i = gnutls_record_recv(s, response, TLS_RECV_CHUNK_LEN)) > 0) {
			response[MIN(i, TLS_RECV_CHUNK_LEN)] = '\0';
			full_response = l_str_cat(full_response, response);
			*bytes_received += i;
			*status = OK;
		} else if (i == 0) {	/* EOF */
			if (gnutls_record_check_pending(s) == 0) {
				*status = OK;
				break;
			}
		} else if (i == GNUTLS_E_AGAIN || i == GNUTLS_E_INTERRUPTED) {
			continue;
		} else {
			INFO_ERR("gnutls_record_recv() error: %s\n",
				gnutls_strerror(i))
			if (gnutls_error_is_fatal(i)) {
				*status = i;
				l_str_free(full_response);
				full_response = NULL;
				break;
			}
		}
	}
	free2(response);
	return full_response;
}
