/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"
#include <iostream>
#include <unistd.h>
#include <time.h>
#include "arpa/inet.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <bits/stdc++.h> 


#define BUFLEN 120	//Max length of buffer
#define UDP_PORT 5110
#define REGISTRY_PORT 5105

char IP_host[BUFLEN];  // store IP address
char REG_host[BUFLEN];  // store registry address
char REG_BUFFER[BUFLEN];
typedef enum {SUBSCRIBE=0,PUBLISH} ARTICLE_TYPE;
typedef enum {JOIN=0,LEAVE,SUBS,UNSUBS,PUB,PING} RPC_TYPE;

pthread_t ping_thread;

bool Is_client_live = true;
char* checkip()
{
    char* ip_address = new char[32];
    int fd;
    struct ifreq ifr;
     
    /*AF_INET - to define network interface IPv4*/
    /*Creating soket for it.*/
    fd = socket(AF_INET, SOCK_DGRAM, 0);
     
    /*AF_INET - to define IPv4 Address type.*/
    ifr.ifr_addr.sa_family = AF_INET;
     
    /*eth0 - define the ifr_name - port name
    where network attached.*/
    memcpy(ifr.ifr_name, "eno1", IFNAMSIZ-1);
     
    /*Accessing network interface information by
    passing address using ioctl.*/
    ioctl(fd, SIOCGIFADDR, &ifr);
    /*closing fd*/
    close(fd);
     
    /*Extract IP Address*/
    strcpy(ip_address,(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)));
    return ip_address;
}

void
communicate_prog_1(char *host, char* article, RPC_TYPE type)
{
	//printf("Here! The content of host is: %s\n", host);
	CLIENT *clnt;
	bool_t  *result;
	

#ifndef	DEBUG
	clnt = clnt_create (host, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	std::string RPCname[] = {"Join","Leave","subscribe","unsubscribe","publish","ping"};

	std::cout << "Client: Start RPC Procedure " << RPCname[type] << std::endl; 
	char* localIP_address = checkip();
	switch(type){
		case JOIN:{
			result = join_1(localIP_address, UDP_PORT, clnt);
			break;
		}
		case LEAVE:{
			result = leave_1(localIP_address, UDP_PORT,clnt);
			break;
		}
		case SUBS:{
			result = subscribe_1(localIP_address, UDP_PORT, article, clnt);
			break;
		}
		case UNSUBS:{
			result = unsubscribe_1(localIP_address, UDP_PORT, article, clnt);
			break;
		}
		case PUB:{
			result = publish_1(article, localIP_address, UDP_PORT, clnt);
			break;
		}
		case PING:{
			result = ping_1(clnt);  //wont use this
		}
	}

	if (result == (bool_t *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	std::cout << "Client: Succeed calling RPC Procedure " << RPCname[type] << std::endl; 

#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}



bool isValidField(std::string field){
	if(field == ""){
		return false;
	} else {
		return true;
	}
}

bool isValidType(std::string field){
	std::string validType[] = {"Sports","Lifestyle","Entertainment","Busniess","Technology","Science","Politics","Health"};
	for(int i = 0; i<8;i++){
		if (field == validType[i]){
			return true;
		}
	}
	return false;
}

bool isValidArticle(std::string s_article, ARTICLE_TYPE type){


	std::vector<std::string> article;
	char spliter = ';';
	std::stringstream ss(s_article);
	std::string item;

	int num = std::count(s_article.begin(),s_article.end(),spliter);
	if (num != 3){
		//printf("Error: number of spliter(;) is not correct\n");
		return false;
	}

	while(std::getline(ss,item,spliter)){
		article.push_back(item);
		//std::cout<<item<<std::endl;
	}
	//std::cout<<"size: " << article.size()<<std::endl;

	
	if(article.size() == 3){ //end problem
		article.push_back("");
	}
	/*
	for (int i = 0;i < 3;i++){
		std::cout << article[i] << "-";
	}
	std::cout << article[3] << std::endl;
	*/

	bool result;

	// first field requirement
	if(isValidField(article[0]) && !isValidType(article[0])){
		printf("Error: First field not match");
		return false;
	}

	// subscrible judegment
	if(type == SUBSCRIBE){
		result = (isValidField(article[0]) || isValidField(article[1]) || isValidField(article[2])) && !(isValidField(article[3]));
	} else if (type == PUBLISH){
		result = (isValidField(article[0]) || isValidField(article[1]) || isValidField(article[2])) && isValidField(article[3]);
	} else {
		printf("Error: Article type not correctly assigned\n");
		return false;
	}

	return result;

}

void printServerList(char* buffer){
	char* p_buffer;  //pointer token
	p_buffer = strtok(buffer,";");
	int counter = 0;

	printf("------ Available Server List ------\n");
	while(p_buffer != NULL){
		if(counter == 0){
			printf("IP: %s		",p_buffer);
		} else if(counter == 1){
			printf("Program ID: %s	", p_buffer);
		} else if(counter == 2){
			printf("Version: %s	\n", p_buffer);
		}
		counter = (counter + 1) % 3;
		p_buffer = strtok(NULL,";");
	}
}

char* getIPfromList(char* buffer){  // get first IP
	char* p_buffer;  //pointer token
	p_buffer = strtok(buffer,";");   // first item is IP
	return p_buffer;
}

void getServerList(char *host, int PORT){

	printf("	----Set Up Connection...\n");

	// setup UDP connection
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);   //AF_INET -> Ipv4
	if(sockfd < 0){
		perror("Error: Fail to create socket\n");
		exit(1);
	}
	printf("	Client: created UDP socket\n");

	// Fill server infor
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	//server_addr.sin_addr.s_addr = INADDR_ANY;
	if (inet_aton(host , &server_addr.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	//server_addr.sin_addr.s_addr = inet_pton(AF_INET,host, &(server_addr.sin_addr));
	printf("	Client: filled UDP information\n");

	char* localIP_address = checkip();
	// Set up command content
	char command[127];
	snprintf(command,sizeof(command),"%s;%s;%s;%d","GetList","RPC",localIP_address,PORT);
	printf("	Client: Sent Command to Server. \n	(Command Content: %s)\n",command);  	

	sendto(sockfd, (const char *)command, strlen(command), 
		MSG_CONFIRM, (const struct sockaddr *) &server_addr, 
			sizeof(server_addr)); 
	// Receive server list info
	
	int n;
	socklen_t nServerAddr;
	//wait list 3 sec.
	timeval tv;
	tv.tv_sec  = 3;
	tv.tv_usec = 0;
	//Set Timeout for recv call
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv), sizeof(timeval));
	n = recvfrom(sockfd, (char *)REG_BUFFER, BUFLEN, 
				MSG_WAITALL, (struct sockaddr *) &server_addr, &nServerAddr);
	if(n==-1)
	{
		perror("Error: Fail to find register server\n");
		exit(1);
	} 
	REG_BUFFER[n] = '\0'; 
	printf("	Client: Received Feedback from Server: %s\n", REG_BUFFER); 
	// display these message into user readable data
	//printServerList(buffer);



	close(sockfd); 
	printf("	----Connection Finished and Closed.\n\n");
	//return REG_BUFFER;

}


void * listen(void * pData)//for receiving the new article
{
	//bool * Is_live = (bool*) pData;
	struct sockaddr_in si_me, si_other;
	int s,  recv_len;
	socklen_t slen= sizeof(si_other);
	char buf[BUFLEN];
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror("socket");
		exit(1);
	}
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(UDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		perror("bind");
		exit(1);
	}
	//keep listening for data
	//wait list 5 sec.
	timeval tv;
	tv.tv_sec  = 5;
	tv.tv_usec = 0;
	while(1)
	{
		//Set Timeout for recv call
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv), sizeof(timeval));
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			if(Is_client_live)
			{
				continue;
			}
			else
			{
				//perror("recvfrom()");
				exit(1);
			}
		}

		printf("receiving article ........\n");
		buf[recv_len] = '\0';
		printf("%s\n",buf);

	}
	close(s);
	pthread_exit(NULL);
}


void * ping(void * pData){

	// First try: clnt-ping...ping-close
	CLIENT *clnt;
	bool_t *result_6;
	char *host = (char*) pData;
	
#ifndef	DEBUG

	clnt = clnt_create (host, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	// use while to loop and pint every 5s

	while(Is_client_live){
		sleep(5);
		//printf("Client: Start to ping...\n");
		result_6 = ping_1(clnt);

		if (result_6 == (bool_t *) NULL) {
			clnt_perror (clnt, "call failed");

			// get new IP and join
			printf("Group Server %s failed. Looking for new server...\n",IP_host);

			sleep(6);

			//printf("Registry host: %s\n",REG_host);
			memset(REG_BUFFER,'\0',BUFLEN); // clear buffer

			//printf("Buffer content before: %s\n",REG_BUFFER);
			getServerList(REG_host,REGISTRY_PORT);
			//printf("Buffer content now: %s\n",REG_BUFFER);

			char* newIP = getIPfromList(REG_BUFFER);
			if(newIP == NULL){
				fprintf(stderr,"No available server could be used. Client exits.\n");
				exit(1);
			}
			printf("new IP content: %s\n",newIP);

			memset(IP_host,'\0',BUFLEN);
			memcpy(IP_host,newIP,BUFLEN);
			printf("New IP: %s has been found\n",IP_host); //validate

			// join again 
			communicate_prog_1(IP_host,NULL,JOIN);
			printf("------\nCommand:\njoin / publish / subs / unsubs / leave\n");
			pthread_create(&ping_thread, NULL, ping, (void*)&IP_host);
			//clnt_destroy (clnt);
			break;  
		}
	}
}


//Check the sting whether it is vaild or not.
bool isValidIP(const std::string ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}


int main (int argc, char *argv[])
{
	//char *host;
	char *registry_host;
	
	pthread_t listenThread;

	//char a_buffer[3];
	if (argc < 2) {
		printf ("usage: %s registry_host\n", argv[0]);
		exit (1);
	}

	printf("Client Server Starts.\n");
	//host = argv[1];
	registry_host = argv[1];


	memcpy(REG_host,registry_host,BUFLEN);
	printf("Designated Registry host: %s\n",REG_host);
	
	//registry_host = argv[1];
	
	/*
	
	std::cout << isValidArticle(";Someone;UMN;contents",PUBLISH) << std::endl; 
	std::cout << isValidArticle("Sports;;;contents",PUBLISH) << std::endl; 
	std::cout << isValidArticle("Science;Someone;UMN;contents",PUBLISH) << std::endl; 
	std::cout << isValidArticle("Science;;UMN;",PUBLISH) << std::endl; 
	std::cout << isValidArticle(";;UMN;",PUBLISH) << std::endl; 
	std::cout << isValidArticle("AA;;;",PUBLISH) << std::endl; 
	std::cout << isValidArticle(";;;contents",SUBSCRIBE) << std::endl; 
	std::cout << isValidArticle(";;contents",SUBSCRIBE) << std::endl; 
	std::cout << isValidArticle("a;a;a",SUBSCRIBE) << std::endl; 
	std::cout << isValidArticle(";;;",SUBSCRIBE) << std::endl; 
	*/
	
	char buffer[BUFLEN]; //buffer for input || article || 
	getServerList(registry_host, REGISTRY_PORT);
	printServerList(REG_BUFFER);  //TODO: default PORT num

	pthread_create(&listenThread, NULL, listen, NULL);//(void*)&Is_servers_live
	printf("\nInstructions:\n"); 
    printf("join: join in a group server with IP address\n");
	printf("publish: publish an article\n");
	printf("subs: subscribe with a filter article\n");
	printf("unsubs: unsubscribe a filter article\n");
	printf("leave: leave a group server\n");
	// cancel while loop
	// TODO: error state	
	while(1){
		printf("------\nCommand:\njoin / publish / subs / unsubs / leave\n");
		std::cin.getline(buffer, BUFLEN);

		// join
		if(strcmp(buffer,"join") == 0){
			printf("Please input address of server you would like to join in:\n");
			std::cin.getline(buffer, BUFLEN);
			if(isValidIP(std::string(buffer))){
				memcpy(IP_host,buffer,sizeof(buffer)); // save this ip address for future use
				communicate_prog_1(IP_host,buffer,JOIN);
				pthread_create(&ping_thread, NULL, ping, (void*)&IP_host);
			} else {
				printf("Error: not valid IP address\n");
			}

			

		// publish
		} else if (strcmp(buffer,"publish") == 0){

			printf("Please Enter Publish content:\n");
			std::cin.getline(buffer, BUFLEN);
			if(!isValidArticle(std::string(buffer),PUBLISH)){
				printf("Error: not valid article for publish\n");
			} else {
				communicate_prog_1(IP_host,buffer,PUB);	
			}

		// subscribe
		} else if(strcmp(buffer,"subs") == 0){
			printf("Please Enter Subscribe content:\n");
			std::cin.getline(buffer, BUFLEN);
			if(!isValidArticle(std::string(buffer),SUBSCRIBE)){
				printf("Error: not valid article for subscribe\n");
			} else {
				communicate_prog_1(IP_host,buffer,SUBS);	
			}

		// unsubscribe
		} else if(strcmp(buffer,"unsubs") == 0){
			printf("Please Enter Unsubscribe content:\n");
			std::cin.getline(buffer, BUFLEN);
			if(!isValidArticle(std::string(buffer),SUBSCRIBE)){
				printf("Error: not valid article for Unsubscribe\n");
			} else {
				communicate_prog_1(IP_host,buffer,UNSUBS);	
			}

		// leave
		} else if (strcmp(buffer,"leave") == 0){
			communicate_prog_1(IP_host,buffer,LEAVE);
			Is_client_live = false;
			printf("Shutting down......\n");
			break;
		} else {
			system(buffer);
		}

	}
	int nRes = pthread_join(listenThread, NULL);
	nRes = pthread_join(ping_thread, NULL);
	exit(0);
}
