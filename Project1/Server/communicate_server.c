/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "communicate.h"
#include <unistd.h>
#include "client.h"
#include "subscribe.h"
#include <netdb.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <list>




extern list<ClientInfo> g_clientList;


list<ClientInfo>::iterator find_client(string strIPAddress, int nPort)
{
	list<ClientInfo>::iterator it;

	for(it=g_clientList.begin();it!=g_clientList.end();it++)
    {
		if(strIPAddress.compare(it->m_szIPAddress) == 0 && nPort == it->m_nPort)
		{
			return it;
		}
	}
	return g_clientList.end();
}



bool_t *
join_1_svc(char *IP, int Port,  struct svc_req *rqstp)
{
	static bool_t result;
	// TODO: accept Client information of IP and PORT for UDP communication
	if (g_clientList.size() >= MAXCLIENT) 
	{
		fprintf(stderr, "The server is full\n");
		result = 1;
		return &result;
	}
    //check duplicate

	if (find_client(IP, Port) == g_clientList.end()) 
	{
		ClientInfo info;
		strcpy(info.m_szIPAddress, IP);
		info.m_nPort = Port;
		g_clientList.push_back(info);		
	}
	else{
		fprintf(stderr, "Client %s:%d is already registered to the server\n", IP,Port);
		result = 1;
		return &result;
	}
	printf("Client %s:%d has already joined to the group server\n",IP,Port);
	result = 0;
	return &result;
}

bool_t *
leave_1_svc(char *IP, int Port,  struct svc_req *rqstp)
{
	static bool_t result;

	/*
	 * insert server code here
	 */
	//search client
	list<ClientInfo>::iterator To_be_delete;
	if ((To_be_delete=find_client(IP, Port)) ==  g_clientList.end())
	{
		fprintf(stderr, "There is no such Client %s:%d\n", IP,Port);
		result = 1;
		return &result;
	}
	g_clientList.erase(To_be_delete);
	result = 0;
	printf("Client %s:%d has already left from the group server  \n",IP,Port);
	return &result;
}


bool_t *
subscribe_1_svc(char *IP, int Port, char *Article,  struct svc_req *rqstp)
{
	static bool_t result;
	std::string article = std::string(Article);

	// prevalidate article format
	int num = count(article.begin(),article.end(),';');
	if (num != 3){
		fprintf(stderr, "Invalid Article format from Client %s:%d\n", IP,Port);
		result = 1;
		return &result;
	}

	// iterate all to find subs
	list<ClientInfo>:: iterator it = find_client(IP, Port);
	if (it == g_clientList.end()) {
		// should not be this case: client must join first
		fprintf(stderr,"Client %s:%d should have joined in first\n",IP,Port);
		result = 1;
		return &result;

	} else {
		// add filter in back

		list<string>::iterator f_it;

		for (f_it = it->filter.begin();f_it != it->filter.end();f_it++){
			if (*f_it == article){
				fprintf(stderr,"duplicate subscription: ");
				cout << article << endl;
				result = 1;
				return &result;
			}
		}
		
		it->filter.push_back(article);
		printf("Client %s:%d ",IP,Port);
		cout << "add new subscription: " << article << endl;
	}
	result = 0;
	return &result;
}
void unicast(list<ClientInfo>::iterator it,const string article)
{
	int sockfd = 0;
	char szReceivedData[32];
    struct sockaddr_in serv_addr;
	//const string strHeartBeat = "heartbeat";
	string strIPAddress;
	int nPort = 0;
	
    int recvLen, servLen;
    struct hostent *pHostent = NULL;
 
	//Send heartBeat to all other servers.
	//list<ClientInfo>::iterator it;
	socklen_t nClientAddr = 0;

	//wait list 2 sec.
	timeval tv;
	tv.tv_sec  = 2;
	tv.tv_usec = 0;

	// for(it=g_clientList.begin();it!=g_clientList.end();it++)
	// {
		// sock create
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		perror("sock create error. Can't send heartbeat msg to other server.\n");
		return;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(it->m_nPort);
	pHostent = gethostbyname(it->m_szIPAddress);

	memcpy((char *)&serv_addr.sin_addr.s_addr, pHostent->h_addr_list[0], pHostent->h_length);\
	//printf("sending article to %s........\n",it->m_szIPAddress);
	// send
	sendto(sockfd, article.c_str(), article.length(), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	close(sockfd);
	// }

	//pthread_exit(NULL);
	return;
}

bool_t *
unsubscribe_1_svc(char *IP, int Port, char *Article,  struct svc_req *rqstp)
{
	static bool_t result;
	std::string article = std::string(Article);

	// prevalidate article format
	int num = count(article.begin(),article.end(),';');
	if (num != 3){
		fprintf(stderr, "Invalid Article format from Client %s:%d\n", IP,Port);
		result = 1;
		return &result;
	}

	// iterate all to find subs
	list<ClientInfo>:: iterator it = find_client(IP, Port);
	if (it == g_clientList.end()) {
		// should not be this case: client must join first
		fprintf(stderr,"Client %s:%d should have joined in first\n",IP,Port);
		result = 1;
		return &result;

	} else {
		// add filter in back
		list<string>:: iterator f_it;
		for(f_it = it->filter.begin();f_it != it->filter.end();f_it++){
			if(*f_it == article){
				// delete this subscrption
				it->filter.erase(f_it);
				printf("Client %s:%d Unsubscribed article: ", IP,Port);
				cout << article << endl;
				result = 0;
				return &result;
			} 
		} 
		fprintf(stderr,"Client %s:%d has no subscription matches this unsubscription: ",IP,Port);	
		cout << article << endl;
	}

	result = 0;
	return &result;
}


bool_t *
publish_1_svc(char *Article, char *IP, int Port,  struct svc_req *rqstp)
{
	static bool_t  result;

	std::string article = std::string(Article);

	int num = count(article.begin(),article.end(),';');
	if (num != 3){
		fprintf(stderr, "Invalid Article format from Client %s:%d\n", IP,Port);
		result = 1;
		return &result;
	}


	// read article into buffer
	string buffer[4];
	char spliter = ';';
	stringstream ss(article);
	string item;

	int i = 0;
	while(getline(ss,item,spliter)){
		buffer[i] = item;
		i++;
	}	

	// print article buffer
	cout << "Read article content: " << endl;
	for(int k = 0;k<4;k++){
		cout << "-" << buffer[k];
	}
	cout << "-\n";

	// iterate all subscription 
	list<ClientInfo>::iterator it;
	for(it=g_clientList.begin();it!=g_clientList.end();it++)
    {

		list<string>::iterator f_it;
		for(f_it = it->filter.begin(); f_it != it->filter.end(); f_it++){
			// validate every subscription
			string f_article = *f_it;

			// read article into buffer
			string f_buffer[4];
			char f_spliter = ';';
			stringstream ss(f_article);
			string f_item;

			int i = 0;
			while(getline(ss,f_item,f_spliter)){
				f_buffer[i] = f_item;
				i++;
			}	

			bool filter_match = false;
			bool all_match = true;

			// compare with set 
			for(int j = 0;j<4;j++){
				if(f_buffer[j] == ""){  //filter "" <- buffer ANY
					continue;
				} else if(f_buffer[j] == buffer[j]){  //filter "A" <- buffer "A"
					filter_match = true;
				} else if(f_buffer[j] != buffer[j]){
					all_match = false;
				}
			} 
			if (filter_match && all_match){
				printf("Send Article to Client %s:%d\n",it->m_szIPAddress,it->m_nPort);
				// TODO: send article to this client via UDP
				unicast(it,article);
				break;  // go to next client
			}
		}

		// go here if not matched publish
		//printf("No match to Client %s:%d\n",it->m_szIPAddress,it->m_nPort);
	}
	
	result = 0;
	return &result;
}

bool_t *
ping_1_svc(struct svc_req *rqstp)
{
	static bool_t  result;

	// if success, result will be good.
	result = 0;
	return &result;
}
