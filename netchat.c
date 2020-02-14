#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>

int main(int argc,char* argv[])
{
    if(argc != 2){
        printf("Usage:./netchat port\n");
        return -1;
    }
    int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sock < 0){
        perror("scoket error!");
        return -1;
    }
    //bind addr of oneself
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(atoi(argv[1]));
    local.sin_addr.s_addr = INADDR_ANY;

    socklen_t sock_len = sizeof(struct sockaddr_in);
    int ret = bind(sock,(struct sockaddr*)&local,sock_len);
    if(ret < 0){
        perror("bind error");
        return -1;
    }

    //connect peer
    printf("chat with?");
    char ip[24] = {0};
    char port[10] = {0};
    scanf("%s%s",ip,port);

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(atoi(port));
    peer.sin_addr.s_addr = inet_addr(ip);

    while(1){
        char mess[1024] = {0};
        printf("your message:");
        scanf("%s",mess);
        sendto(sock,mess,sizeof(mess),0,(struct sockaddr*)&peer,sock_len);
        memset(mess,0x00,1024);
        ret = recvfrom(sock,mess,sizeof(mess),0,(struct sockaddr*)&peer,&sock_len);
        if(ret < 0){
            perror("recv error");
            continue;
        }
        printf("recv message:%s\n",mess);
    }
    close(sock);
}
