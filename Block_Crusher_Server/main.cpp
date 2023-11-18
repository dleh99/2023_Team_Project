#include "header.h"
#include "protocol.h"
#include "Overlapped.h"
#include "User_Interface.h"

using namespace std;

HANDLE iocp_h;
SOCKET g_s_socket, g_c_socket;
Overlapped g_over;

array<User_Interface, MAX_USER> clients;

int set_client_id()
{
	for (int i{}; i < MAX_USER; ++i) {
		if (clients[i]._state == US_EMPTY)
			return i;
	}
	return -1;
}

void disconnect(int c_id)
{
	closesocket(clients[c_id]._socket);
	clients[c_id]._state = US_EMPTY;
}

void packet_process(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		clients[c_id].send_login_info_packet();
	}
	case CS_MOVE: {
			CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
			// ��Ŷ���� ������ ���⿡ ���� ��ġ �̵� �ڵ�
			short x = clients[c_id].x;
			short y = clients[c_id].y;
			short z = clients[c_id].z;

			// ���⿡ ���� �̵�
			switch (p->direction) {
				case 0: if (y < WORLD_LENGHT) y++; break;
				case 1: if (y > 0) y--; break;
				case 2: if (x > 0) x--; break;
				case 3: if (x < WORLD_WIDTH) x++; break;
			}
			clients[c_id].x = x;
			clients[c_id].y = y;
			break;
	}
	}
}

void worker_thread(HANDLE iocp_h)
{
	while (true) {
		DWORD byte_size;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;

		BOOL ret = GetQueuedCompletionStatus(iocp_h, &byte_size, &key, &over, INFINITE);
		Overlapped* ex_over = reinterpret_cast<Overlapped*>(over);

		// ������ ���� �� ó��
		if (FALSE == ret) {
			if (ex_over->_overlapped_type == OT_ACCEPT) std::cout << "Error of Accept";
			else {
				std::cout << "Error on client [" << key << "] in GQCS" << std::endl;
				disconnect(key);
				if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
				continue;
			}
		}

		// �� �����Ͱ� ���� ��
		if ((byte_size == 0) && ((ex_over->_overlapped_type == OT_RECV) || (ex_over->_overlapped_type == OT_SEND))) {
			disconnect(key);
			if (ex_over->_overlapped_type == OT_SEND) delete ex_over;
			continue;
		}

		// ���� ������ Ÿ�Կ� ���� ó��
		switch (ex_over->_overlapped_type)
		{
			case OT_ACCEPT:
			{
				int client_id = set_client_id();
				if (client_id != -1)
				{
					// ���� ��ȯ(������)
					clients[client_id]._state = US_CONNECTING;

					// �ʱⰪ ����
					clients[client_id].x = 0;
					clients[client_id].y = 0;
					clients[client_id]._id = client_id;
					clients[client_id]._prev_remain = 0;
					clients[client_id]._socket = g_c_socket;

					CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), iocp_h, client_id, 0);
					clients[client_id].do_recv();
					g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				}
				else
				{
					std::cout << "Max User in Game" << std::endl;
				}
				ZeroMemory(&g_over._over, sizeof(g_over._over));
				int addr_size = sizeof(SOCKADDR_IN);
				AcceptEx(g_s_socket, g_c_socket, g_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_over._over);
				break;
			}
			case OT_RECV:
			{
				int remain_data = byte_size + clients[key]._prev_remain;
				char* p = ex_over->_send_buf;
				// ��Ŷ ������
				while (remain_data > 0) {
					int packet_size = p[0];
					if (packet_size <= remain_data) {
						packet_process(static_cast<int>(key), p);
						p = p + packet_size;
						remain_data = remain_data - packet_size;
					}
					else break;
				}
				// ������ ���ٸ�
				clients[key]._prev_remain = remain_data;
				if (remain_data > 0) {
					memcpy(ex_over->_send_buf, p, remain_data);
				}
				clients[key].do_recv();
				break;
			}
			case OT_SEND:
			{
				delete ex_over;
				break;
			}
		}
	}
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	iocp_h = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), iocp_h, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_over._overlapped_type = OT_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_over._over);

	vector<thread> worker_threads;
	int thread_num = thread::hardware_concurrency();
	for (int i{}; i < thread_num; ++i)
		worker_threads.emplace_back(worker_thread, iocp_h);
	
	// Ÿ�̸ӷ� ó���� �͵� ó��
	//thread timer_thread{};
	
	for (auto& thread : worker_threads)
		thread.join();

	closesocket(g_s_socket);
	WSACleanup();
}