#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
 
#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define CMD 0xff
#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31
 
void negotiate(int sock, unsigned char *buf, int len) {
    int i;
     
    if (buf[1] == DO && buf[2] == CMD_WINDOW_SIZE) {
        unsigned char tmp1[10] = {255, 251, 31};
        if (send(sock, tmp1, 3 , 0) < 0)
            exit(1);
         
        unsigned char tmp2[10] = {255, 250, 31, 0, 80, 0, 24, 255, 240};
        if (send(sock, tmp2, 9, 0) < 0)
            exit(1);
        return;
    }
     
    for (i = 0; i < len; i++) {
        if (buf[i] == DO)
            buf[i] = WONT;
        else if (buf[i] == WILL)
            buf[i] = DO;
    }
 
    if (send(sock, buf, len , 0) < 0)
        exit(1);
}
 
static struct termios tin;
 
static void terminal_set(void) {
    // save terminal configuration
    tcgetattr(STDIN_FILENO, &tin);
     
    static struct termios tlocal;
    memcpy(&tlocal, &tin, sizeof(tin));
    cfmakeraw(&tlocal);
    tcsetattr(STDIN_FILENO,TCSANOW,&tlocal);
}
 
static void terminal_reset(void) {
    // restore terminal upon exit
    tcsetattr(STDIN_FILENO,TCSANOW,&tin);
}
 
#define BUFLEN 20
int main(int argc , char *argv[]) {
    int sock;
    struct sockaddr_in server;
    unsigned char buf[BUFLEN + 1];
    int len;
    int i, j;
 
 	/*
    if (argc < 2 || argc > 3) {
        printf("Usage: %s address [port]\n", argv[0]);
        return 1;
    }
    int port = 23;
    if (argc == 3)
        port = atoi(argv[2]);
    */
 
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        perror("Could not create socket. Error");
        return 1;
    }
 
    server.sin_addr.s_addr = inet_addr("140.123.20.230");
    server.sin_family = AF_INET;
    server.sin_port = htons(443);
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected...\n");
 
    // set terminal
    //terminal_set();
    //atexit(terminal_reset);
     
    struct timeval ts;
    ts.tv_sec = 1; // 1 second
    ts.tv_usec = 0;
    fd_set fds;

    unsigned char temp[300];
    unsigned char test[3];
    int stringIndex = 0;
    int specialChar = 0;
    int big5Check = 0;
    int big5Special = 0;
    int login = 0;

    while (1) {
        // select setup
        FD_ZERO(&fds);
        if (sock != 0)
            FD_SET(sock, &fds);
        FD_SET(0, &fds);
 
        // wait for data
        int nready = select(sock + 1, &fds, (fd_set *) 0, (fd_set *) 0, &ts);
        if (nready < 0) {
            perror("select. Error");
            return 1;
        }
        else if (nready == 0) {
            ts.tv_sec = 1; // 1 second
            ts.tv_usec = 0;
        }
        else if (sock != 0 && FD_ISSET(sock, &fds)) {
            // start by reading a single byte
            int rv;
            if ((rv = recv(sock , buf , 1 , 0)) < 0)
                return 1;
            else if (rv == 0) {
                printf("Connection closed by the remote end\n\r");
                return 0;
            }
 
            if (buf[0] == CMD) {
                // read 2 more bytes
                len = recv(sock , buf + 1 , 2 , 0);
                if (len  < 0)
                    return 1;
                else if (len == 0) {
                    printf("Connection closed by the remote end\n\r");
                    return 0;
                }
                negotiate(sock, buf, 3);
            }
            else {
            	if (specialChar == 1) {
            		/* end of special character */
            		if (buf[0] == 0x6D)
            			specialChar = 0;
            		continue;
            	}
                if (big5Check == 1) {
                    /* Big-5 higher */
                    test[1] = buf[0];
                    test[2] = '\0';

                    /* Special char? */
                    if( ((test[0] == 0xA1 && test[1] >= 0x40) && (test[0] == 0xA1 && test[1] <= 0xFE)) ||
                    ((test[0] == 0xA2 && test[1] >= 0x40) && (test[0] == 0xA2 && test[1] <= 0xCC)) || 
                	((test[0] == 0xA3 && test[1] >= 0x46) && (test[0] == 0xA3 && test[1] <= 0xBF)) || 
                	((test[0] == 0xF9 && test[1] >= 0xD6) && (test[0] == 0xF9 && test[1] <= 0xFE)) ) {
                    	big5Check = 0;
                    	continue;
                    }
                    big5Check = 0;
                    temp[stringIndex] = test[0];
                    stringIndex++; 
                    temp[stringIndex] = test[1];
                    stringIndex++; 
                }
                else if (buf[0] >= 0x81 && buf[0] <= 0xFE) {

                    /* Big-5 lower */
                    test[0] = buf[0];
                    big5Check = 1;
                }
                else if (buf[0] == 0x5B) {
                	/* special characater */
                	specialChar = 1;
                	continue;
                }
                else if(buf[0] == 0x20 || buf[0] == 0x8 || buf[0] == 0xD || buf[0] == 0xA){
                    /* special bit */
                    continue;
                }
                else if(buf[0] == 0x1B) {
                    /* break */
                    if (stringIndex > 0) {
	                    temp[stringIndex] = '\0';
	                    
	                    /*for(j = 0; j < stringIndex; j++) {
	                    	printf("\\0x%x\\", temp[j]);
	                    }*/
	                    printf("\n[%s]\n", temp);

	                    /*

	                    */
	                    stringIndex = 0;
	                    memset(temp, '\0', 300);
	                }
                }
                else {
                    temp[stringIndex] = buf[0];
                    stringIndex++; 
                }

                len = 1;
                buf[len] = '\0';
                fflush(0);
            }
        }
        else if (FD_ISSET(0, &fds)) {
            buf[0] = getc(stdin); //fgets(buf, 1, stdin);
            if (send(sock, buf, 1, 0) < 0)
                return 1;
            if (buf[0] == '\n') // with the terminal in raw mode we need to force a LF
                send(sock, "\r", 1, 0);
        }
    }
    close(sock);
    return 0;
}