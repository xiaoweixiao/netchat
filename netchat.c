#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<signal.h>

#define ERR_EXIT(x) do{perror(x);exit(EXIT_FAILURE);}while(0)

pthread_mutex_t out_lock;

struct Node{
    int type;
    char str[1024];
};


void client(char* ip,int port)
{
    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        pthread_mutex_lock(&out_lock);
        fflush(stdout);
        ERR_EXIT("socket");
        pthread_mutex_unlock(&out_lock);
    }
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(ip);
    if(connect(sock,(struct sockaddr*)&peer,sizeof(peer)) < 0)
        ;//ERR_EXIT("connect");

    int ret = -1;
    struct Node node;
    node.type = 1;//类型1,表示该报文为通信报文
    int first = 0,sl=0;

    while(1)
    {
        if(first == 0)
            first = 1;
        else if(first == 1){
            pthread_mutex_lock(&out_lock);
            fflush(stdout);
            if(sl == 0)
            {
                sleep(25);
                sl = 1;
            }
            printf("[%s %d]your message:\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
            pthread_mutex_unlock(&out_lock);
        }
        memset(node.str,0,sizeof(node.str));
        if(fgets(node.str,sizeof(node.str),stdin) == NULL)
            ERR_EXIT("fgets");
        send(sock,(void*)&node,sizeof(node),0);
    }
    close(sock);
}

void server(int port)
{
    struct Node node;
    int con = 0;
    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
        ERR_EXIT("socket");

    struct sockaddr_in local;
    memset(&local,0,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
        ERR_EXIT("bind");
    int ret  = 0;
    struct sockaddr_in peer;
    socklen_t peerlen = sizeof(peer);

    while(1){
        memset((void*)&node,0,sizeof(node));
        ret = recvfrom(sock,(void*)&node,1024,0,(struct sockaddr*)&peer,&peerlen);
        if(ret < 0){
            if(errno == EINTR)//信号中断
                continue;
            ERR_EXIT("recvfrom");
        }
        else if(ret > 0){
            if(con == 0){
                con = 1;
                for(int i=0;i<3;i++){
                    pthread_mutex_lock(&out_lock);
                    fflush(stdout);
                    printf("session request from: %s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                    printf("#chat?:\n");
                    pthread_mutex_unlock(&out_lock);
                    sleep(5);
                }
                pthread_mutex_lock(&out_lock);
                fflush(stdout);
                //printf("accept request?(y/n)y\n");
                printf("success:%s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
            }
            else if(node.type == 1){
                char head[1024] = "sent msg:";
                strcat(head,node.str);
                pthread_mutex_lock(&out_lock);
                fflush(stdout);
                printf("[%s %d]%s",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port),head);
                pthread_mutex_unlock(&out_lock);
            }
        }
    }
    close(sock);
}


typedef void (*sighandler_t)(int signum);

void sigint(int num)
{
    pthread_mutex_lock(&out_lock);
    fflush(stdout);
    printf("terminating all sessions...\n");
    pthread_mutex_unlock(&out_lock);
    exit(0);
}

void sigquit(int num)
{
    pthread_mutex_lock(&out_lock);
    fflush(stdout);
    printf("#terminate session (for help <h>): 127.0.0.1 %d\n",getpid());
    pthread_mutex_unlock(&out_lock);
}


int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        pthread_mutex_lock(&out_lock);
        fflush(stdout);
        ERR_EXIT("Usage:netchat port");
        pthread_mutex_unlock(&out_lock);
    }

    pthread_mutex_init(&out_lock,NULL);
    //信号处理
    signal(SIGINT,sigint);

    struct sigaction act;
    act.sa_handler = sigquit;
    act.sa_flags = 0;
    struct sigaction old;
    sigaction(SIGQUIT,&act,&old);

    pid_t pro = fork();
    if(pro < 0 )
        ERR_EXIT("fork");
    else if(pro == 0)
    {
        pthread_mutex_lock(&out_lock);
        printf("chat?:\n");
        pthread_mutex_unlock(&out_lock);
        char ip[24] = {0};
        int port = 0;
        scanf("%s%d",ip,&port);
        client(ip,port);
    }
    else
    {
        //父进程
        server(atoi(argv[1]));
    }
    pthread_mutex_destroy(&out_lock);
    return 0;
}
