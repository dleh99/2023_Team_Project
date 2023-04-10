#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "protocol.h"

using namespace std;

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

}