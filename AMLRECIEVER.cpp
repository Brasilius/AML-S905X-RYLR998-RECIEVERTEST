#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

int main() {
    // Open the serial port
    // /dev/ttyS1 is the standard UART on the Le Potato header
    int serial_port = open("/dev/ttyS1", O_RDWR);

    if (serial_port < 0) {
        std::cerr << "Error: Could not open serial port." << std::endl;
        return 1;
    }

    // Configure serial port settings
    struct termios tty;
    if(tcgetattr(serial_port, &tty) != 0) {
        std::cerr << "Error from tcgetattr." << std::endl;
        return 1;
    }

    cfsetispeed(&tty, B115200); // Set Baud Rate to 115200
    cfsetospeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;        // No parity
    tty.c_cflag &= ~CSTOPB;        // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;            // 8 data bits
    tty.c_iflag |= ICANON;          // Canonical mode (reads line by line)

    tcsetattr(serial_port, TCSANOW, &tty);

    std::cout << "Waiting for data from RYLR998..." << std::endl;

    char read_buf[256];
    
    while (true) {
        memset(&read_buf, '\0', sizeof(read_buf));
        int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

        if (num_bytes > 0) {
            std::string data(read_buf);
            // RYLR998 outputs received data in format: +RCV=ADDRESS,LENGTH,DATA,RSSI,SNR
            // We check if the data section contains the integer "1"
            if (data.find(",1,") != std::string::npos || data.find(",1\r") != std::string::npos) {
                std::cout << "Received integer: 1" << std::endl;
            }
        }
    }

    close(serial_port);
    return 0;
}