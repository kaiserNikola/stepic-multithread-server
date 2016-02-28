#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <thread>
       
using namespace std;

void log(const string& s, bool br=true){
		ofstream f("/tmp/stepic-task-srv.log", ofstream::out | ofstream::app);
		f << s;
		if (br) f << endl;
		f.close();
}

void send_reply_str(int sockfd, const char* msg, int sz=0){	
	int sent=0, total=sz ? sz : strlen(msg);
	
	log(msg, false);
	
	while (sent < total){
		int n = send(sockfd,msg+sent,total-sent, 0);
		if (n < 0) perror("ERROR writing to socket");		
		sent += n;		
	} 
}




void reply_send_http(int sockfd, int code, const string& msg){
	if (code == 200){
		ifstream f(msg.c_str());
		f.seekg (0, f.end);
		int length = f.tellg();
		f.seekg (0, f.beg);			
		
		send_reply_str(sockfd, "HTTP/1.0 200 OK\r\n");		
		//send_reply_str(sockfd, "Date: Fri, 20 Mar 1999 08:17:58 GMT\r\n");		
		send_reply_str(sockfd, "Server: Trololo/100500\r\n");		
		//send_reply_str(sockfd, "Last-modified: Mon, 17 Jun 1996 21:53:08 GMT\r\n");		
		send_reply_str(sockfd, "Content-type: text/html\r\n");	
		char tmp[64];	
		sprintf(tmp, "Content-lenght: %d\r\n", length);
		send_reply_str(sockfd, tmp);		
		send_reply_str(sockfd, "\r\n");	
		
		
		char buffer[512];
		int pos = 0;
		while (pos < length){
			int sz = min((int)sizeof(buffer), length-pos);
			f.read(buffer, sz);	
			pos += sz;	
			send_reply_str(sockfd, buffer, sz);	
		}

		f.close();		
			
	}
	else
	if (code == 404){
		send_reply_str(sockfd, "HTTP/1.0 404 NOT FOUND\r\n");	
		send_reply_str(sockfd, "Content-Type: text/html\r\n");	
		send_reply_str(sockfd, "\r\n");		 	
	}
	log("---\n");
}

string read_http_request(int sockfd){
	string out;
	char buffer[512];  
	
	while (1){
		bzero(buffer, sizeof(buffer));	
		int n = recv(sockfd, buffer, sizeof(buffer)-1, 0);
		if (n < 0) perror("ERROR reading from socket");
		
		out += buffer;
		if (out.rfind("\r\n\r\n") != std::string::npos)
			break;
	}
	
	return out;
}


void serve(int sockfd, const string &home){
	string request = read_http_request(sockfd);
	//cout << request << endl;
	log(request);
	
	// find url from: GET /index.html HTTP/1.0
	int splitter = request.find("\r\n");
	string query = request.substr(0, splitter-1);
	
	int sp_1 = query.find(" ");
	int sp_2 = query.find(" ", sp_1+1);
	string url = query.substr(sp_1+1, sp_2-sp_1-1);
	
	sp_1 = url.find("?");
	if (sp_1 != std::string::npos){
			url = url.substr(0, sp_1);
	}
	
	if (url == "/") url = "/index.html";
	//cout << "[" << url << "]"<<endl;
	
	url = home + url;
	log("\n>>> "+url);
	
	if (access( url.c_str(), 0 ) == 0){
		//cout << "file exists" <<endl;		
		reply_send_http(sockfd, 200, url);
	}
	else{
		//cout << "file ["<< url << "] not found" <<endl;	
		reply_send_http(sockfd, 404, "");	
	}
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}


int main(int argc, char ** argv){
	//cout << "Hello, desu" << endl;
	// -h <ip> -p <port> -d <directory>

	int port = 8080;	
	char directory[512] = "/home/kaiser/projects/courses/stepic_multiprocessing/git/files";
	char host[32] = "localhost";
	
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
	   default: // '?' 
		   fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
		   exit(-1);
	   }
	}
	int directory_sz = strlen(directory);
	if (directory[directory_sz-1] == '/')
		directory[directory_sz-1] = 0;

	pid_t parpid;
    if((parpid=fork())<0) {                
             printf("\ncan't fork"); //--если нам по какойлибо причине это сделать не удается выходим с ошибкой.
             exit(1);                //--здесь, кто не совсем понял нужно обратится к man fork
    }
    else if (parpid!=0){
		exit(0);
	} 
                   
    setsid();           //--перевод нашего дочернего процесса в новую сесию	


	//cout << "host=" << host <<"; port="<<port<<"; directory="<<directory<< endl;
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	  perror("ERROR opening socket");
	  
	int yes = 1;  
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) {
	  perror("setsockopt() error");
	  exit(1);
	} 	  
	
	sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, host, &serv_addr);
	
	if (bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		perror("ERROR on binding");
		
	listen(sockfd, 20);	
	sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);
	
	while(1){
		int newsockfd = accept(sockfd, (sockaddr *) &cli_addr, (socklen_t *)&clilen);
		if (newsockfd < 0)
		  perror("ERROR on accept");	
		//printf("\n[Client: %s:%d]\n\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port);  
		std::thread thr(serve, newsockfd, string(directory));
		thr.detach();
	}
	//cout << "Goodbye, desu" << endl;
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return 0;
}
