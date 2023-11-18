#pragma once

#include <iostream>
#include "..\..\Block_Crusher_Server\protocol.h"
#include <ws2tcpip.h> 
#include <winsock2.h> 
#pragma comment(lib, "ws2_32")

using namespace std;

int NetworkInit();
void send_login_packet();
void send_keyboard_packet(int direction);
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
DWORD WINAPI do_recv();