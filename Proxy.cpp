/*
 * Proxy.cpp
 * Course: CPSC 441
 * Assignment 1: Clown Proxy
 * UCID: 30064647
 * Name: Laura Timm
 * Date: Jan 26 2022
 */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>

#define HTTP_PORT   80
#define PORT        19291
#define SIZE        2480

//use a fork mentioned in tutorial
int pid;
int data_socket;
//responses... to extract info or be altered
char client_response[SIZE], server_response[SIZE];


using namespace std;


int main() {

    /*
     * The following code is from the Socket Program tutorial given by Bardia Abhari
     * This code was used in both the sample code and the lecture slides
     * 2-Socket Programming and Server.cpp
     */
    //PROXY ACTING LIKE THE SERVER (RECEIVING)
    // Proxy Address Initialization
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    //prepare listening socket (create, bind, listen)
    //Creation
    int proxy_server_socket;

    proxy_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_server_socket == -1) {
        printf("Proxy_server_socket creation failed \n");
        exit(-1);
    } else {
        printf("Proxy_server_socket created!\n");
    }

    //Bind
    int status = bind(proxy_server_socket, (struct sockaddr *) &address, sizeof(struct sockaddr_in));
    if (status == -1) {
        printf("Bind failed \n");
        exit(-1);
    } else {
        printf("Bind was successful\n");
    }

    //Listening
    status = listen(proxy_server_socket, 5);
    if (status == -1) {
        printf("Listen failed \n");
        exit(-1);
    } else {
        printf("Listen successful listening on port %d...\n", PORT);
    }
    //create an infinite loop to continue listening
    while (1) {
        //accept connection from client on data socket
        // Accepting part -- Blocking -- Starting to listen.
        data_socket = accept(proxy_server_socket, NULL, NULL);
        if (data_socket == -1) {
            printf("Accept failed \n");
            exit(-1);

        } else {
            printf("Accept successful\n");
        }
//    ***End of code from 2-Socket Programming and Server.cpp***
/*
 * The Code containing fork was found here https://www.delftstack.com/howto/cpp/cpp-fork/
 * and from the bad-http-server.c (from William Black)
 */
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
//    ***End of code from website and bad-http-server.c***
/*
* The following code is from the HTTP-Proxy tutorial given by Bardia Abhari
* This code was used in both the sample code and the lecture slides
* 3-HTTP-Proxy and HttpClient.cpp as well as http-c-examples.c (from William Black)
*/
//Receive the HTTP request from Web Client and parse it to extract needed info
// receive the text.
            ssize_t c_bytes;
            //requests
            char web_client_http[SIZE], proxy_http[SIZE], clown_request[SIZE];

            memset(web_client_http, 0, SIZE + 1);
            c_bytes = recv(data_socket, web_client_http, SIZE, 0);
            if (c_bytes == -1) {
                printf("Web browser receive failed!\n");
                exit(-1);
            } else {
                printf("Received Client Request: %s\n", web_client_http);
            }

//Extract needed info (Host and Path from the Request)
/*
 * The following code was created using: https://stackoverflow.com/questions/726122/best-ways-of-parsing-a-url-using-c
 * and from from: https://cboard.cprogramming.com/c-programming/96391-domain-name-url.html
 */
            char url[SIZE], host[SIZE], c_request[SIZE];
            sscanf(web_client_http, "GET http://%s", url); // this gives pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/test1.html
            sscanf(url, "%[^/]", host);
            sscanf(web_client_http, "%[^\r\n]", c_request); //GET http://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/test2.html HTTP/1.1

//    ***End of code from above websites**

            char clowna[] = "GET /~carey/CPSC441/ass1/clown1.png HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n";
            char clownb[] = "GET /~carey/CPSC441/ass1/clown2.png HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n";
            char clownc[] = "GET /~carey/CPSC441/ass1/pansage.gif HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n";
            char beef[] = "GET /~carey/CPSC441/ass1/burger.gif HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n";

            char *jpg_checker = strstr(c_request, ".jpg");

            if (jpg_checker != NULL) {
                int randy = (rand() % 40) + 1;
                cout<<"This is randy: "<< randy << "\n";
                if (randy < 10 ) {
                    memcpy(proxy_http, clowna, sizeof(clowna));
                } else if (randy > 11 && randy <20 ) {
                    memcpy(proxy_http, clownb, sizeof(clownb));
                } else if (randy > 21 && randy <30 ) {
                    memcpy(proxy_http, beef, sizeof(beef));
                } else {
                    memcpy(proxy_http, clownc, sizeof(clownc));
                }

            } else {
            //if floppy.jpg not in request copy request to proxy request
                memcpy(proxy_http, web_client_http, sizeof(web_client_http));
            }


//PROXY ACTING LIKE THE CLIENT
//prepare a new socket for connecting to the web server (socket, connect)
            struct sockaddr_in server_address;
            memset(&server_address, 0, sizeof(server_address));
            server_address.sin_family = AF_INET;
            server_address.sin_port = htons(HTTP_PORT);

            struct hostent *server;
            server = gethostbyname(host);
            bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);

//create the socket
            int web_server_socket;
            web_server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (web_server_socket == -1) {
                printf("Web_server_socket creation failed\n");
                exit(-1);
            } else {
                printf("Web_server_socket created!\n");
            }

            int connect_status = connect(web_server_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr_in));
            if (connect_status == -1) {
                printf("connect failed!\n");
                exit(-1);
            } else { printf("Connect successful \n"); }

//send the HTTP request to web server
            int proxy_send_status;
            //do not strlen for binary data
            proxy_send_status = send(web_server_socket, proxy_http, strlen(proxy_http), 0);
            if (proxy_send_status == -1) {
                printf("send to webserver failed!\n");
                exit(-1);
            } else {
                printf("Sending to Web Server: %s\n", proxy_http);
            }

//receive the HTTP response from the web server (receive, close)
//edit this junk
            /*
            * The following code is modified from the bad-http-server.c (from William Black)
            */
            //while loop continuous reading of the receive and send calls

            ssize_t proxy_bytes;
            int first = 0;

            do {
                proxy_bytes = recv(web_server_socket, server_response, SIZE, 0);
                if (proxy_bytes < 0) {
                    printf("Proxy Receive Failed\n");
                } else {
                    printf("proxy_bytes= %d \n", proxy_bytes);
                }

                char copy_response[proxy_bytes];

                char *content_type;
                //Edit Response here (check the Content type)
                string server_response_string = server_response;
                memcpy(copy_response, server_response, proxy_bytes);
                if (first == 0) {
                    content_type = strstr(server_response, "Content-Type: ");
                    sscanf(content_type, "Content-Type:%s", content_type);
                    //cout << "_____" << content_type << "____\n";
                } else {
                    strcpy(content_type, "foo");
                }
                first = 1;

                //Reference for using strcmp https://www.codegrepper.com/code-examples/cpp/c%2B%2B+compare+char
                if (strcmp(content_type, "text/html;") == 0) {

                    //Reference for using on how to replace a string  https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
                    string replace = "Silly";
                    size_t start_pos = 0;
                    while ((start_pos = server_response_string.find("Happy")) != std::string::npos) {
                        server_response_string.replace(start_pos, replace.length(), replace);
                        start_pos += replace.length();
                    }
                    bcopy(server_response_string.c_str(), client_response, proxy_bytes);
                }
//                if (strcmp(content_type, "image/jpeg") == 0) {
//                    char clown[]= "GET /~carey/CPSC441/ass1/clown1.png HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n";
//                    //use this to build new request for Bonus
//        //            strcat(clown, "\r\n");
//        //            strcat(clown, "Host: ");
//        //            strcat(clown, host);
//        //            strcat(clown, "\r\n\r\n");
//
//                    //memcpy(clown_request, clown, sizeof(clown));
//                    //memcpy(clown_request, clown, sizeof(clown));
//                    proxy_send_status = send(web_server_socket, clown, sizeof(clown), 0);
//                    if (proxy_send_status == -1) {
//                        printf("Sending clown failed!\n");
//                        exit(-1);
//                    } else {
//                        printf("Sending clown to Web Server: '%s'\n", clown);
//                    }
//
//                    proxy_bytes = recv(web_server_socket, server_response, SIZE, 0);
//                    if (proxy_bytes < 0) {
//                        printf("Failed receive Clown\n");
//                    }
//                    else {
//                        printf("Revived clown %.50s\n", server_response);
//                    }
//                    memcpy(client_response, server_response, proxy_bytes);
//                }

                else {
                    memcpy(client_response, copy_response, proxy_bytes);
                    //write(1, client_response, proxy_bytes);   //this writes to the terminal
                }


//PROXY ACTING LIKE THE SERVER (RETURNING)
//sending size of data structure

                int results = send(data_socket, client_response, proxy_bytes, 0);
                if (results < 0) {
                    printf("Send to Client Failed\n");
                } else {
                    printf("send bytes %d \n", results);
                }
            } while (proxy_bytes > 0);
            /*
            * end of code from the bad-http-server.c (from William Black)
            */

            close(web_server_socket);
            memset(client_response, 0, SIZE);
            memset(server_response, 0, SIZE);


        }// end of fork
    } //end infinite while loop

//** end of code used from 3-HTTP-Proxy and HttpClient.cpp as well as http-c-examples.c (from William Black)***
//send the HTTP response to the client
    close(data_socket);
    close(proxy_server_socket);
    return 0;
}
