#pragma once

#include <iostream>
#include "..\..\Block_Crusher_Server\protocol.h"
#include <ws2tcpip.h> 
#include <winsock2.h> 
#pragma comment(lib, "ws2_32")

using namespace std;

struct Pos {
	float x;
	float y;
	float z;
};

int NetworkInit();
void NetCleanup();
void send_login_packet();
void send_move_packet(float x, float y, float z);
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
void WINAPI do_recv();

bool GetGameState();
Pos GetStartPos();