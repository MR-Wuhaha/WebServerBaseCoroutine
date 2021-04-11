#ifndef _OTHERFUN
#define _OTHERFUN
#include<sys/socket.h>
#include<unistd.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<iostream>
#include<cstring>
#include<memory>
#include"libco/co_routine.h"
class channel;
int Maccept(std::shared_ptr<channel> _channel,char *buff,int length);
int readn(std::shared_ptr<channel> _channel,char *buff,int length);
int writen(std::shared_ptr<channel> _channel,char *buff,int length);
int sysread(std::shared_ptr<channel> _channel,char *buff,int length);
int set_noblock(int);
#endif