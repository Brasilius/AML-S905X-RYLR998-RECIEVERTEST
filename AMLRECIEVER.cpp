#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>

int main() {
    // Try /dev/ttyS1 (Header Pins 8/10). If it fails, try /dev/ttyAML1
    const char* port = "/dev/ttyS1";
    int serial_port = open(port, O_RDWR | O_NOCTTY);

    if (serial_port < 0) {
        perror("Error opening serial port");
        return 1;
    }

    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        perror("Error from tcgetattr");
        close(serial_port);
        return 1;
    }

    // Set Baud Rate
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Control Modes (c_cflag)
    tty.c_cflag |= (CLOCAL | CREAD);    // Ignore modem lines, enable receiver
    tty.c_cflag &= ~PARENB;             // No parity
    tty.c_cflag &= ~CSTOPB;             // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                 // 8 data bits
    tty.c_cflag &= ~CRTSCTS;            // No hardware flow control

    // Local Modes (c_lflag)
    tty.c_lflag |= ICANON;              // Canonical mode (line-based)
    tty.c_lflag &= ~ECHO;               // Disable echo
    tty.c_lflag &= ~ISIG;               // Disable interpretation of INTR, QUIT, SUSP

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
        return 1;
    }

    std::cout << "Listening on " << port << " at 115200 baud..." << std::endl;

    char buffer[256];
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        int n = read(serial_port, buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            std::string msg(buffer);
            // RYLR998 output: +RCV=ADDRESS,LENGTH,DATA,RSSI,SNR
            // Looking for "1" in the DATA field
            if (msg.find(",1,") != std::string::npos) {
                std::cout << "Confirmed: Received 1" << std::endl;
            }
        }
    }

    close(serial_port);
    return 0;
}