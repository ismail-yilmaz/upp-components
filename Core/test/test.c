/*
 ============================================================================
 Name        : SmallSimpleSSH.c
 Author      : Ivan Tretyakov
 Version     : 0.1
 Copyright   : GPLv3
 Description : Libssh2 ssh client example
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libssh2.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>

#define EXIT_COMMAND	"exit\n"

/* Main function */
int main(int argc, char *argv[]) {

	const char *username;
	const char *password;
	const char *hostaddr;
	int rc;
	int sock;
	int written;
	struct sockaddr_in sin;
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *channel;
	char commandbuf[BUFSIZ];
	char inputbuf[BUFSIZ];
	const char numfds = 2;
	struct pollfd pfds[numfds];

	/* Get IP and authorization data */
	if (argv[1] != NULL && argv[2] != NULL && argv[3] != NULL) {
		hostaddr = argv[1];
		username = argv[2];
		password = argv[3];
	} else {
		fprintf(stderr, "Usage: %s <target ip> <username> <password>\n",
				argv[0]);
		return (EXIT_FAILURE);
	}

	/* Libss2 init block */
	rc = libssh2_init(0);
	if (rc) {
		fprintf(stderr, "Error: libssh_init()\n");
		return (EXIT_FAILURE);
	}

	/* Creating socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		return (EXIT_FAILURE);
	}

	/* Connect this socket to remote side */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(22);
	sin.sin_addr.s_addr = inet_addr(hostaddr);
	if (connect(sock, (struct sockaddr*)(&sin),
				sizeof(struct sockaddr_in)) != 0) {
		fprintf(stderr, "Failed to connect\n");
		return (EXIT_FAILURE);
	}

	/* Set socket non-blocking */
	rc = fcntl(sock, F_SETFL, O_NONBLOCK);
	if (rc == -1) {
		perror("fcntl");
		return (EXIT_FAILURE);
	}

	/* Set stdin non-blocking */
	rc = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	if (rc == -1) {
		perror("fcntl");
		return (EXIT_FAILURE);
	}

	/* Create a session instance and start it up. This will trade welcome
	 * banners, exchange keys, and setup crypto, compression, and MAC layers */
	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "SSH init failed\n");
		return (EXIT_FAILURE);
	}

	/* Handshake for session */
	rc = libssh2_session_handshake(session, sock);
	if (rc) {
		fprintf(stderr, "SSH handshake failed\n");
		return (EXIT_FAILURE);
	}

	/* Lets authenticate */
	rc = libssh2_userauth_password(session, username, password);
	if (rc) {
		printf("Authentication by password failed\n");
		return (EXIT_FAILURE);
	} else {
		printf("Authentication by password succeeded\n");
	}

	/* Request a shell */
	channel = libssh2_channel_open_session(session);
	if (!channel) {
		fprintf(stderr, "Unable to open a session\n");
		return (EXIT_FAILURE);
	}

	/* Request a terminal with 'vt100' terminal emulation */
	rc = libssh2_channel_request_pty(channel, "vt100");
	if (rc) {
		fprintf(stderr, "Failed requesting pty\n");
		return (EXIT_FAILURE);
	}

	/* Open a SHELL on that pty */
	rc = libssh2_channel_shell(channel);
	if (rc) {
		fprintf(stderr, "Unable to request shell on allocated pty\n");
		return (EXIT_FAILURE);
	}

	/* Set libssh2 to non-blocking mode */
	libssh2_channel_set_blocking(channel, 0);

	/* Prepare to use poll */
	memset(pfds, 0, sizeof(struct pollfd) * numfds);

	/* Main loop starts here.
	 * In it you will be requested to input a command
	 * command will be executed at remote side
	 * an you will get output from it */
	do {
		/* Declare that we neet to wait while
		 * socket or stdin not ready for reading */
		pfds[0].fd = sock;
		pfds[0].events = POLLIN;
		pfds[0].revents = 0;
		pfds[1].fd = STDIN_FILENO;
		pfds[1].events = POLLIN;
		pfds[1].revents = 0;

		/* Polling on socket and stdin while we are
		 * not ready to read from it */
		rc = poll(pfds, numfds, -1);
		if (-1 == rc) {
			perror("poll");
			break;
		}

		if (pfds[0].revents & POLLIN) {
			/* Read output from remote side */
			do {
				rc = libssh2_channel_read(channel, inputbuf, BUFSIZ);
				printf("%s", inputbuf);
				fflush(stdout);
				memset(inputbuf, 0, BUFSIZ);
			} while (LIBSSH2_ERROR_EAGAIN != rc && rc > 0);
		}
		if (rc < 0 && LIBSSH2_ERROR_EAGAIN != rc) {
			fprintf(stderr, "libssh2_channel_read error code %d\n", rc);
			return (EXIT_FAILURE);
		}

		if (pfds[1].revents & POLLIN) {
			/* Request for command input */
			fgets(commandbuf, BUFSIZ - 2, stdin);
			if (strcmp(commandbuf, EXIT_COMMAND) == 0)
				break;

			/* Adjust command format */
			commandbuf[strlen(commandbuf) - 1] = '\r';
			commandbuf[strlen(commandbuf)] = '\n';
			commandbuf[strlen(commandbuf) + 1] = '\0';

			/* Write command to stdin of remote shell */
			written = 0;
			do {
				rc = libssh2_channel_write(channel, commandbuf, strlen(commandbuf));
				written += rc;
			} while (LIBSSH2_ERROR_EAGAIN != rc
					&& rc > 0
					&& written != strlen(commandbuf));
			memset(commandbuf, 0, BUFSIZ);
		}
		if (rc < 0 && LIBSSH2_ERROR_EAGAIN != rc) {
			fprintf(stderr, "libssh2_channel_write error code %d\n", rc);
			return (EXIT_FAILURE);
		}

	} while (1);
	/* Main loop ends here */

	/* De-init and pre-exit actions */
	if (channel) {
		libssh2_channel_free(channel);
		channel = NULL;
	}

	/* Finish actions below */
	rc = libssh2_session_disconnect(session, "Normal Shutdown");
	if (rc) {
		fprintf(stderr, "Session disconnect error\n");
		return (EXIT_FAILURE);
	} else
		printf("Session finished successful\n");

	libssh2_session_free(session);

	close(sock);

	libssh2_exit();

	return (EXIT_SUCCESS);
}