#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <libssh/keys.h>
#include <iostream>

void reverse_tunnel(ssh_session session, const char* remote_host, int remote_port, const char* local_host, int local_port) {
    ssh_channel channel = ssh_channel_new(session);
    if (channel == NULL) {
        std::cerr << "Error creating channel: " << ssh_get_error(session) << std::endl;
        return;
    }

    int rc = ssh_channel_open_forward(channel, remote_host, remote_port, local_host, local_port);
    if (rc != SSH_OK) {
        std::cerr << "Error opening forward channel: " << ssh_get_error(session) << std::endl;
        ssh_channel_free(channel);
        return;
    }

    // Keeping the tunnel open
    while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
        ssh_channel_poll(channel, 10);
    }

    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

int main() {
    ssh_session my_ssh_session;
    ssh_key key;
    int rc;

    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        return -1;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "ec2-18-222-60-7.us-east-2.compute.amazonaws.com");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "ec2-user");

    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        std::cerr << "Error connecting to localhost: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_free(my_ssh_session);
        return -1;
    }

    rc = ssh_pki_import_privkey_file("/home/ec2-user/key.pem", NULL, NULL, NULL, &key);
    if (rc != SSH_OK) {
        std::cerr << "Error loading private key: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    rc = ssh_userauth_publickey(my_ssh_session, NULL, key);
    ssh_key_free(key);
    if (rc != SSH_AUTH_SUCCESS) {
        std::cerr << "Authentication failed: " << ssh_get_error(my_ssh_session) << std::endl;
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return -1;
    }

    reverse_tunnel(my_ssh_session, "localhost", 8080, "localhost", 80);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return 0;
}
