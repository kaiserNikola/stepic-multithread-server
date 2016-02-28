#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

       
using namespace std;


void serve(int sockfd){
	char buffer[1025];  
	bzero(buffer, sizeof(buffer));
	
	int n = recv(sockfd, buffer, sizeof(buffer)-1, 0);
	if (n < 0) perror("ERROR reading from socket");
	printf("Here is the message: %s",buffer);	
	
	n = send(sockfd,"I got your message",18, 0);
	if (n < 0) perror("ERROR writing to socket");	
}


int main(int argc, char ** argv){
	cout << "Hello, desu" << endl;
	// -h <ip> -p <port> -d <directory>

	int port = 8080;	
	char directory[255] = "/home/kaiser/";
	char host[32] = "0.0.0.0";
	
	int opt;
	while ((opt = getopt(argc, argv, "h:p:d:")) != -1) {
	   switch (opt) {
	   case 'h':
		   strcpy(host, optarg);
		   break;
	   case 'p':
		   port = atoi(optarg);
		   break;
	   case 'd':
		   strcpy(directory, optarg);
		   break;
	   default: /* '?' */
		   fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
		   exit(-1);
	   }
	}

	cout << "host=" << host <<"; port="<<port<<"; directory="<<directory<< endl;

	/*if (optind >= argc) {
	   fprintf(stderr, "Expected argument after options\n");
	   exit(-1);
	}*/	
	
	cout << "Goodbye, desu" << endl;
	return 0;
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	  perror("ERROR opening socket");
	  
	
	sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;  
	serv_addr.sin_port = htons(port);
	
	if (bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		perror("ERROR on binding");
		
	listen(sockfd, 20);	
	sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);
	
	while(1){
		int newsockfd = accept(sockfd, (sockaddr *) &cli_addr, (socklen_t *)&clilen);
		if (newsockfd < 0)
		  perror("ERROR on accept");	
		serve(newsockfd);		
		break;	  
	}
	cout << "Goodbye, desu" << endl;
	return 0;
}
