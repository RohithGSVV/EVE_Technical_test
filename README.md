# SSH Program and Reverse Tunnel Setup

This repository demonstrates the following:

- Setting up an AWS EC2 instance (free tier) and obtaining an SSH key (`<key>.pem`).
- Downloading and compiling `libssh` in both debug and release modes.
- Using CMake to create an SSH program that:
  - Loads the PEM key.
  - Establishes an SSH connection to the AWS server.
- Creating another program that:
  - Establishes a reverse tunnel to the AWS server.

## Contents

- [Step 1: Setup AWS EC2 Instance](#step-1-setup-aws-ec2-instance)
- [Step 2: Download and Compile libssh on EC2](#step-2-download-and-compile-libssh-on-ec2)
- [Step 3: Create a Simple SSH Program](#step-3-create-a-simple-ssh-program)
- [Step 4: Create a Reverse Tunnel Program](#step-4-create-a-reverse-tunnel-program)


## Step 1: Setup AWS EC2 Instance

1. **Create EC2 Instance**
    - Go to the [AWS Management Console](https://aws.amazon.com/console/).
    - Navigate to EC2 and launch a new instance.
    - Choose the Amazon Linux 2 AMI.
    - Select the `t2.micro` instance type
    - Create a new key pair and download the `.pem` file (eg., aws-key.pem).

2. **Configure Security Group:**
    - Go to the EC2 Dashboard on the AWS Management Console.
    - Select your running instance, then go to the Security tab.
    - Edit the Inbound Rules of the instance’s security group to allow traffic on the port you are forwarding (e.g., port 80):
        - Add a new rule:
            - Type: HTTP
            - Protocol: TCP
            - Port Range: 80
            - Source: Anywhere (0.0.0.0/0) or a specific IP range

3. **Connect to the EC2 Instance:**
    - Open your terminal and navigate to the directory where your `aws-key.pem` file is located.
    - Change the permissions of the `.pem` file:

      ```bash
      chmod 400 /<path>/aws-key.pem
      ```

    - Connect to the EC2 instance using SSH:

      ```bash
      ssh -i aws-key.pem ec2-user@<your-ec2-public-dns>
      ```

    ---
    > 💡 **Hint:** You can find the EC2 public DNS (IPv4 Public IP) in the AWS Management Console, under the "Instance Summary" section of your active EC2 instance.
    ---

## Step 2: Download and Compile libssh on EC2

In this step, we will download and compile the `libssh` library on the EC2 instance.

1. **Initial Setup on EC2:**

Install the necessary development tools and libraries, including CMake and OpenSSL development headers.

```bash
    sudo yum update -y
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y cmake openssl-devel
```

2. **Download libssh:**

    Download the `libssh` source code archive and extract it.

```bash
    wget https://www.libssh.org/files/0.9/libssh-0.9.5.tar.xz
    tar -xf libssh-0.9.5.tar.xz
    cd libssh-0.9.5
```

3. **Compile libssh:**

    Create a build directory, configure the build with CMake, and compile the library.

```bash
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
```

Now, you should have a running AWS server with libssh installed and ready to use.

## Step 3: Using CMake to Create an SSH Program with libssh

In this step, we will create a simple SSH program that uses the `libssh` library to connect to the AWS EC2 instance using the PEM key.

1. **Create a Directory for Your Program:**

    Create a directory named `ssh_program` and navigate into it.

    ```bash
    mkdir ~/ssh_program
    cd ~/ssh_program
    ```

2. **Ensure SSH Configuration on EC2 Instance Allows Port Forwarding:**

    Edit the SSH daemon configuration file on the EC2 instance to enable TCP forwarding and gateway ports.

    ```bash
    sudo nano /etc/ssh/sshd_config
    ```

    Ensure the following settings are enabled:

    ```plaintext
    AllowTcpForwarding yes
    GatewayPorts yes
    ```

    Restart the SSH daemon to apply the changes:

    ```bash
    sudo systemctl restart sshd
    ```

3. **Make pkg-config Recognize libssh:**
    Ensure that pkg-config can find the `libssh` package by updating the `PKG_CONFIG_PATH`.

    ```bash
    export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
    pkg-config --modversion libssh
    ```

4. **Build the Program:**

Create a build directory, configure the build with CMake, and compile the program.

```bash
    mkdir build
    cd build
    cmake ..
    make
```

This is how the build directory should look like:

```plaintext
ssh_program
├── CMakeLists.txt
├── main.cpp
└── build
    └── ssh_prog (binary)
```

To run the SSH program:

```bash
./ssh_prog <username> <hostname> <path_to_pem_key>
```

Example Usage:

```bash
./ssh_program ec2-user ec2-18-222-60-7.us-east-2.compute.amazonaws.com /home/rganni/Extras/aws-key.pem
```

---
> 💡 **:** Username for EC2 instance is by default ec2-user.
---

On successful execution, you will see Authentication successful in the log. You can also verify this by logging into the EC2 instance and checking the SSH log for recent connections:

```bash
sudo cat /var/log/secure | grep sshd
```

## Step 4: Create a Reverse Tunnel Program

In this step, we will create a reverse tunnel program that uses the `libssh` library to connect to the AWS EC2 instance and forward local traffic to a remote server.

1. **Create a Directory for Your Program:**

    ```bash
    mkdir ~/reverse_tunnel
    cd ~/reverse_tunnel
    ```

2. **Build the Program:**

Create a build directory within reverse_tunnel, configure the build with CMake, and compile the program.

```bash
    mkdir build
    cd build
    cmake ..
    make
```

This is how the build directory should look like:

```plaintext
reverse_tunnel
├── CMakeLists.txt
├── reverse_tunnel.cpp
└── build
    └── reverse_tunnel (binary)
```

Running the Reverse Tunnel Program:

```bash
./reverse_tunnel
```

3. **Checking the Reverse Tunnel**

To verify that your reverse tunnel is working correctly, follow these steps:

- **Run the Reverse Tunnel Program:**

   Navigate to the build directory and execute the `reverse_tunnel` program:

   ```bash
   cd ~/Extras/reverse_tunnel/build
   ./reverse_tunnel
   ```

   You should see the following output:

   ```plaintext
   SSH connection established and authenticated.
   Listening on port 8080...
   ```

   The program will wait for incoming connections.

- **Test the Connection:**

   Open another terminal and execute the following command to send a request to the local port:

   ```bash
   curl http://localhost:8080
   ```

   You should see the following response:

   ```plaintext
   Hello world!
   ```

- **Verify the Connection:**

   Switch back to the terminal running the `reverse_tunnel` program. You should see the following output indicating that a connection was accepted and a message was received:

   ```plaintext
   SSH connection established and authenticated.
   Listening on port 8080...
   Connection accepted.
   Received message: GET / HTTP/1.1
   Host: localhost:8080
   User-Agent: curl/7.81.0
   Accept: */*
   ```

This confirms a reverse SSH tunnel from your local machine to the AWS EC2 instance, allowing you to securely forward traffic.
