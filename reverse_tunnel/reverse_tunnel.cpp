#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    ssh_session my_ssh_session;
    int rc;
    std::string pem_file = "/home/rganni/Extras/aws-key.pem";
    int local_port = 8080;
    int remote_port = 80;

    my_ssh_session = ssh_new();
    if (my_ssh_session == nullptr) {
        std::cerr << "Error creating SSH session" << std::endl;
        return -1;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "ec2-18-222-60-7.us-east-2.compute.amazonaws.com");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "ec2-user");

    ssh_key key;
    rc = ssh_pki_import_privkey_file(pem_file.c_str(), nullptr, nullptr, nullptr, &key);
    if (rc != SSH_OK) {
        std::cerr << "Error importing private key: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_free(my_ssh_session);
        return -1;
    }

    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        std::cerr << "Error connecting to host: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_free(my_ssh_session);
        return -1;
    }

    rc = ssh_userauth_publickey(my_ssh_session, nullptr, key);
    if (rc != SSH_AUTH_SUCCESS) {
        std::cerr << "Authentication failed: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    std::cout << "SSH connection established and authenticated." << std::endl;

    int socket_fd, client_sockfd;
    struct sockaddr_in local_address;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Cannot open socket" << std::endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    memset(&local_address, 0, sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = INADDR_ANY;
    local_address.sin_port = htons(local_port);

    if (bind(socket_fd, (struct sockaddr *) &local_address, sizeof(local_address)) < 0) {
        std::cerr << "Cannot bind socket" << std::endl;
        close(socket_fd);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    listen(socket_fd, 5);
    std::cout << "Listening on port " << local_port << "..." << std::endl;

    client_sockfd = accept(socket_fd, nullptr, nullptr);
    if (client_sockfd < 0) {
        std::cerr << "Cannot accept connections" << std::endl;
        close(socket_fd);
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    std::cout << "Connection accepted." << std::endl;

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t nbytes = read(client_sockfd, buffer, sizeof(buffer));
    if (nbytes < 0) {
        std::cerr << "Error reading from socket" << std::endl;
    } else if (nbytes == 0) {
        std::cout << "Connection closed by client." << std::endl;
    } else {
        std::cout << "Received message: " << buffer << std::endl;
        const char* reply = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world!";
        send(client_sockfd, reply, strlen(reply), 0);
    }

    close(client_sockfd);
    close(socket_fd);
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return 0;
}
