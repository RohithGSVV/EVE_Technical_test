#include <libssh/libssh.h>
#include <iostream>

int main() {
    ssh_session my_ssh_session;
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

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return 0;
}