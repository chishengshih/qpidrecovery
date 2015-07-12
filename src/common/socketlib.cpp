/*
** QpidR - Qpid recovery tool
** newslab.csie.ntu.edu.tw, Taiwan
** See Copyright Notice in common.h
*/

#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>

#include<netdb.h>
#define MAX_ADDRESS_LENGTH INET_ADDRSTRLEN

/*
int getLocalIP(char*ip){
	struct ifaddrs*i,*j;
	if(getifaddrs(&i)<0)
		return -1;
ip[0]='\0';printf("ooooooo\n");fflush(stdout);
	for(j=i; j!=NULL; j=j->ifa_next){
		struct sockaddr_in* addr=(struct sockaddr_in*)j->ifa_addr;
		if(addr->sin_family!=AF_INET)
			continue;
		strcpy(ip, inet_ntoa(addr->sin_addr));puts(ip);fflush(stdout);
		if(strcmp(ip,"127.0.0.1")!=0)continue;
//			break;
	}
	freeifaddrs(i);
	return j==NULL?-1:0;
}
*/

int nametoaddr(const char *hostname, char *address){
	struct addrinfo *addrlist, *addrptr, hint;
	int ret = -1;

	hint.ai_family = AF_INET;//AF_UNSPEC;
	hint.ai_socktype = 0;
	hint.ai_flags = 0;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_addr = NULL;
	hint.ai_canonname = NULL;
	hint.ai_next = NULL;

	if(getaddrinfo(hostname, NULL/*port number*/, &hint, &addrlist) != 0){
		return ret;
	}
	for(addrptr = addrlist; addrptr != NULL; addrptr = addrptr->ai_next){
		if(addrptr->ai_family != AF_INET ||
		addrptr->ai_addrlen != sizeof(struct sockaddr_in)){
			continue;
		}
		if(inet_ntop(AF_INET,
		&(((struct sockaddr_in*)(addrptr->ai_addr))->sin_addr),
		address, INET_ADDRSTRLEN
		) != NULL){
			ret = 0;
			break;
		}
	}
	freeaddrinfo(addrlist);
	return ret;
}

static int fillsockaddr_in(struct sockaddr_in* addr, const char* ipaddr, const uint16_t port){
    addr->sin_family = AF_INET;
    if(ipaddr != NULL){
        if(inet_pton(AF_INET, ipaddr, &(addr->sin_addr)) <= 0)
            return -1;
    }
    else
        addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(port);
    return 0;
}

static int createsocket(unsigned port, int sockettype){
    int sfd;
    struct sockaddr_in addr;
    if(fillsockaddr_in(&addr, NULL, (uint16_t)port) == -1)
        return -1;

    sfd = socket(AF_INET, sockettype, 0);
    if(sfd == -1)
        return -1;
    if(bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        close(sfd);
        return -1;
    }
    return sfd;
}

int udpsocket(unsigned port){
	return createsocket(port, SOCK_DGRAM);
}

int udpreceivefrom(int sfd, char* from, void* buf, int bufsize){
    struct sockaddr_in addr;
    socklen_t addrsize = sizeof(addr);

    if(recvfrom(sfd, buf, bufsize, 0, (struct sockaddr*)&addr, &addrsize) == -1)
        return -1;
    if(inet_ntop(AF_INET, &(addr.sin_addr), from, MAX_ADDRESS_LENGTH/*caller specify?*/)==NULL)
        return -1;
    return 0;
}

int udpsendto(int sfd, const char* ipaddress, unsigned port, void* buf, int bufsize){
    struct sockaddr_in addr;
    if(fillsockaddr_in(&addr, ipaddress, (uint16_t)port) == -1)
        return -1;
    if(sendto(sfd, buf, bufsize, 0, (struct sockaddr*)&addr, sizeof(addr)) != bufsize)
        return -1;
    return 0;
}

int tcpserversocket(unsigned port){
	int sfd=createsocket(port, SOCK_STREAM);
	if(sfd<0)
		return -1;
	if(listen(sfd, 128)<0)
		return -1;
	return sfd;
}

int tcpaccept(int sfd, char* from){
	struct sockaddr_in addr;
	socklen_t addrsize=sizeof(struct sockaddr_in);
	int newsfd;

	newsfd=accept(sfd, (struct sockaddr*)&addr, &addrsize);
	if(newsfd<0)
		return -1;
	if(from!=NULL){/*don't care about address*/
		if(inet_ntop(AF_INET, &(addr.sin_addr), from, MAX_ADDRESS_LENGTH)==NULL){
			close(newsfd);
			return -1;
		}
	}
	return newsfd;
}

int tcpconnect(const char*ipaddr, unsigned port){
	int sfd;
	struct sockaddr_in addr;
	if(fillsockaddr_in(&addr, ipaddr, port)==-1)
		return -1;
	sfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0)
		return -1;
	if(connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))<0){
		close(sfd);
		return -1;
	}
	return sfd;
}

static void sigpipehandler(int s){
	printf("sigpipe %d\n", s);
}

void replaceSIGPIPE(){
	static int first = 1;
	if(first == 0)
		return;
	first = 0;
	signal(SIGPIPE, sigpipehandler);
}



