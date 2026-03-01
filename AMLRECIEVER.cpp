#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string>
#include <cstring>

int main() {
    // UART A on Le Potato is typically /dev/ttyAML6
    const char* port_name = "/dev/ttyAML6";
    int serial_port = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (serial_port < 0) {
        std::cerr << "Error: Unable to open serial port " << port_name << "\n";
        return 1;
    }

    // Configure the serial port
    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        std::cerr << "Error: Unable to get serial port attributes\n";
        close(serial_port);
        return 1;
    }

    // Set baud rate to 115200 (RYLR998 default)
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // 8N1 Mode (8 data bits, no parity, 1 stop bit)
    tty.c_cflag &= ~PARENB; 
    tty.c_cflag &= ~CSTOPB; 
    tty.c_cflag &= ~CSIZE; 
    tty.c_cflag |= CS8; 
    
    // Disable hardware flow control
    tty.c_cflag &= ~CRTSCTS; 
    // Enable receiver and ignore modem control lines
    tty.c_cflag |= CREAD | CLOCAL; 

    // Disable canonical mode, echo, and signals (raw mode)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;

    // Disable software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable special handling of bytes
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    // Raw output
    tty.c_oflag &= ~OPOST;

    // Wait for up to 1 second (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VTIME] = 10;    
    tty.c_cc[VMIN] = 0;

    // Save tty settings
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error: Unable to set serial port attributes\n";
        close(serial_port);
        return 1;
    }

    std::cout << "Listening on " << port_name << " for incoming data...\n";

    char read_buf [256];
    std::string received_message = "";

    // Continuous read loop
    while (true) {
        memset(&read_buf, '\0', sizeof(read_buf));
        int num_bytes = read(serial_port, &read_buf, sizeof(read_buf) - 1);

        if (num_bytes > 0) {
            for(int i = 0; i < num_bytes; i++) {
                char c = read_buf[i];
                received_message += c;
                
                // The RYLR998 ends messages with \r\n
                if (c == '\n') {
                    // Output the exact string received from the module
                    std::cout << "Data received: " << received_message;
                    
                    // Optional: Check if the message contains the integer '1'
                    // The format is usually +RCV=<Address>,<Length>,<Data>,<RSSI>,<SNR>
                    if (received_message.find(",1,") != std::string::npos || 
                        received_message.find(" 1\r") != std::string::npos) {
                        std::cout << "Detected target integer: 1\n";
                    }

                    received_message = ""; 
                }
            }
        }
    }

    close(serial_port);
    return 0;
}