#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 9002
#define BACKLOG 10
#define BUFFER_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

int server_socket = -1;
int client_socket = -1;
FILE *file = NULL;

void handle_signal(int signal) {
    (void)signal; // Mark the parameter as unused
    syslog(LOG_INFO, "Caught signal, exiting");
    if (client_socket != -1) {
        close(client_socket);
    }
    if (server_socket != -1) {
        close(server_socket);
    }
    if (file != NULL) {
        fclose(file);
        remove(FILE_PATH);
    }
    closelog();
    exit(EXIT_SUCCESS);
}

void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        // Parent process
        exit(EXIT_SUCCESS);
    }

    // Child process
    if (setsid() < 0) {
        syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        // Parent process
        exit(EXIT_SUCCESS);
    }

    umask(0);
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "chdir failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    ssize_t bytes_written;
    int daemon_mode = 0;

    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    openlog("aesdsocket", LOG_PID, LOG_USER);
    setup_signal_handler();

    syslog(LOG_INFO, "Starting server");

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        syslog(LOG_ERR, "Failed to create socket: %s", strerror(errno));
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        close(server_socket);
        return -1;
    }

    if (daemon_mode) {
        daemonize();
    }

    if (listen(server_socket, BACKLOG) == -1) {
        syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
        close(server_socket);
        return -1;
    }

    syslog(LOG_INFO, "Server listening on port %d", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            if (errno == EINTR) {
                // Interrupted by signal, continue to accept new connections
                continue;
            }
            syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
            break;
        }

        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(client_addr.sin_addr));

        file = fopen(FILE_PATH, "a+");
        if (file == NULL) {
            syslog(LOG_ERR, "Failed to open file: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            if (strchr(buffer, '\n') != NULL) {
                bytes_written = fwrite(buffer, sizeof(char), bytes_received, file);
                if (bytes_written < bytes_received) {
                    syslog(LOG_ERR, "Failed to write to file: %s", strerror(errno));
                }
                fflush(file);

                fseek(file, 0, SEEK_SET);
                while ((bytes_received = fread(buffer, sizeof(char), BUFFER_SIZE - 1, file)) > 0) {
                    buffer[bytes_received] = '\0';
                    if (send(client_socket, buffer, bytes_received, 0) == -1) {
                        syslog(LOG_ERR, "Failed to send data to client: %s", strerror(errno));
                    }
                }
                break;
            }
        }

        fclose(file);
        file = NULL;
        close(client_socket);
        client_socket = -1;
        syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(client_addr.sin_addr));
    }

    close(server_socket);
    closelog();
    return 0;
}
