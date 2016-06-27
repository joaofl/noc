#include <termios.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/signal.h>
#include <sys/time.h>
//----------------------------------------------------
#define BAUDRATE B3000000
#define MODEMDEVICE "/dev/ttyUSB2"
#define _POSIX_SOURCE 1
#define O_BINARY 0x8000
#define FALSE 0
#define TRUE 1
//----------------------------------------------------
volatile int STOP = FALSE;
void signal_handler_IO(int status);
int wait_flag = TRUE;
int fd, res;
unsigned char buf[255];
int przer = 0;

struct sigaction saio;
struct termios oldtio, newtio;

timespec ti, tf;
//----------------------------------------------------

int main() {
    std::cout << "START" << std::endl;

    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror(MODEMDEVICE);
        exit(-1);
    }


    saio.sa_handler = signal_handler_IO;
    sigemptyset(&saio.sa_mask);
    saio.sa_flags = 0;
    saio.sa_flags = SA_NODEFER;
    saio.sa_restorer = NULL;
    sigaction(SIGIO, &saio, NULL);


    fcntl(fd, F_SETOWN, getpid());
    fcntl(fd, F_SETFL, FASYNC);
    tcgetattr(fd, &oldtio);
    bzero(&newtio, sizeof (newtio));


    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VINTR] = 0;
    newtio.c_cc[VQUIT] = 0;
    newtio.c_cc[VERASE] = 0;
    newtio.c_cc[VKILL] = 0;
    newtio.c_cc[VEOF] = 1;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VSWTC] = 0;
    newtio.c_cc[VSTART] = 0;
    newtio.c_cc[VSTOP] = 0;
    newtio.c_cc[VSUSP] = 0;
    newtio.c_cc[VEOL] = 0;
    newtio.c_cc[VREPRINT] = 0;
    newtio.c_cc[VDISCARD] = 0;
    newtio.c_cc[VWERASE] = 0;
    newtio.c_cc[VLNEXT] = 0;
    newtio.c_cc[VEOL2] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

//    while (STOP == FALSE) {
//        res = read(fd, buf, 1);
//        buf[res] = 0;
//        std::cout << " Data: " << (int) buf[0] << std::endl;
//        if (buf[0] == 'z') STOP = TRUE;
//    }

    unsigned char pck[] = "aaaaaaaaaaaaaaaa";
    
    
    unsigned int i = 0;
    while(i < 1){
        
        clock_gettime(CLOCK_REALTIME, &ti);
        write(fd,&pck,sizeof(pck));
        std::cout << "Data writen\n";
        sleep(1);
        i++;
        
        
    }
    
    double t = double(tf.tv_nsec - ti.tv_nsec) / 1000; //in us
    tcsetattr(fd, TCSANOW, &oldtio);
    std::cout << "END" << std::endl << "Time = " << t;

    

    return 0;
}

void signal_handler_IO(int status) {
    wait_flag = FALSE;
    clock_gettime(CLOCK_REALTIME, &tf);
    printf("signal recived.\n");
    przer++;
} 












//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <fcntl.h>
//#include <sys/signal.h>
//#include <errno.h>
//#include <termios.h>



//sigset_t mskvar_1;                  //Variable of signal bitfieldtype
//struct sigaction sigio_action;       //Structure which describes signal handler
//void sio_handler(void);             //Signal handler function
//
//int main()
//{
//  sigfillset(&mskvar_1);                    //set all mask bits of maskbit variable
//  sigprocmask(SIG_SETMASK,&mskvar_1,NULL);  //write the mask info present in mskvar_1 to the pd
//  sigdelset(&mskvar_1,SIGIO);               //Unmask SIGIO , to register for IO Interrupt Events
//
//  sigio_action.sa_handler = sio_handler;    //Configure Signal Handler
//  sigio_action.sa_flags  = 0;
//  sigfillset(&sigio_action.sa_mask);
//  sigaction(SIGIO,&sigio_action,NULL);      //Install Signal handler
//
//  // Serial port initializtion here
//  // Set Serial port parameters , Baud Rate and flags
//
//
//while(1);
//return 0;
//}
//
//
//void sio_handler()
//{
//  printf("\nSIGIO RECEIVED , I/O interrupt signalled?\n");
//  return;
//}
//
////////////////
//
//void signal_handler_IO (int status);   /* definition of signal handler */
//
//int n;
//int fd;
//int connected;
//struct termios termAttr;
//struct sigaction saio;
//
//int main(int argc, char *argv[])
//{
//    unsigned char port[] = "/dev/ttyUSB1";
//     fd = open(port , O_RDWR | O_NOCTTY | O_NDELAY);
//     if (fd == -1)
//     {
//        perror("open_port: Unable to open port");
//        exit(1);
//     }
//
//     saio.sa_handler = signal_handler_IO;
//     saio.sa_flags = 0;
//     saio.sa_restorer = NULL; 
//     sigaction(SIGIO,&saio,NULL);
//
//     fcntl(fd, F_SETFL, FNDELAY);
//     fcntl(fd, F_SETOWN, getpid());
//     fcntl(fd, F_SETFL,  O_ASYNC ); /**<<<<<<------This line made it work.**/
//
//     tcgetattr(fd,&termAttr);
//     //baudRate = B115200;          /* Not needed */
//     cfsetispeed(&termAttr,B115200);
//     cfsetospeed(&termAttr,B115200);
////     termAttr.c_cflag &= ~PARENB;
//     termAttr.c_cflag &= ~CSTOPB;
//     termAttr.c_cflag &= ~CSIZE;
//     termAttr.c_cflag |= CS8;
//     termAttr.c_cflag |= (CLOCAL | CREAD);
//     termAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
//     termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
//     termAttr.c_oflag &= ~OPOST;
//     tcsetattr(fd,TCSANOW,&termAttr);
//     printf("Port %s configured....\n", port);
//
//     connected = 1;
//     
//     char pck[] = "a";
//     unsigned int cycles = 10;
//     unsigned int i = 0;
//  
//    while(i < cycles){
//        write(fd, &pck, sizeof(pck));
//        printf("Data sent\n");
//        sleep(1);
//        
//        i++;
//    }
//
//
//     close(fd);
//     exit(0);             
//}
//
//void signal_handler_IO (int status)
//{
//     printf("Data received\n");
//     return;
//}