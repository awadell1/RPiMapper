#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void main() {
    int file;
    char filename[40];
    const char *buffer;
    int addr = 0x05;        // The I2C address of the ADC

    sprintf(filename,"/dev/i2c-1");
    if ((file = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    if (ioctl(file,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    char buf[1028] = {0};
    // Using I2C Read
    if (read(file, buf, sizeof(buf)) <0) {
        /* ERROR HANDLING: i2c transaction failed */
        printf("Failed to read from the i2c bus.\n");
        printf(buffer);
        printf("\n\n");
    } else {
        // Clear chars after newline
        //buf[strcspn(buf, "\n")] = 0;

        // Type case as unsigned long
        printf("Msg: %s\n",buf);
    }
}
