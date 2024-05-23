#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <iostream>
#include <fstream>

void error(ssh_session session) {
    std::cerr << "Error: " << ssh_get_error(session) << std::endl;
    ssh_free(session);
    exit(-1);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <username> <hostname> <pem_key_path>" << std::endl;
        return -1;
    }

    const char* username = argv[1];
    const char* hostname = argv[2];
    const char* pem_key_path = argv[3];

    ssh_session session = ssh_new();
    if (session == nullptr) {
        std::cerr << "Error creating SSH session" << std::endl;
        return -1;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(session, SSH_OPTIONS_USER, username);
    int verbosity = SSH_LOG_PROTOCOL;
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

    int rc = ssh_connect(session);
    if (rc != SSH_OK) {
        error(session);
    }

    ssh_key private_key;
    rc = ssh_pki_import_privkey_file(pem_key_path, nullptr, nullptr, nullptr, &private_key);
    if (rc != SSH_OK) {
        std::cerr << "Error loading private key: " << ssh_get_error(session) << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    rc = ssh_userauth_publickey(session, nullptr, private_key);
    ssh_key_free(private_key);
    if (rc != SSH_AUTH_SUCCESS) {
        std::cerr << "Authentication failed: " << ssh_get_error(session) << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    std::cout << "Authentication successful!" << std::endl;

    ssh_channel channel = ssh_channel_new(session);
    if (channel == nullptr) {
        error(session);
    }
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        error(session);
    }
    rc = ssh_channel_request_exec(channel, "uptime");
    if (rc != SSH_OK) {
        error(session);
    }

    char buffer[256];
    int nbytes;
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
        std::cout.write(buffer, nbytes);
    }

    if (nbytes < 0) {
        error(session);
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}
