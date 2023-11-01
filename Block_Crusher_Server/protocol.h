#pragma once

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;

constexpr int MAX_USER = 2000;

constexpr int WORLD_WIDTH = 12000;		// �� = 60cm(px), ���� = �� 200�� 60 * 200
constexpr int WORLD_LENGHT = 12000;		// �� = 60cm(px), ���� = �� 200�� 60 * 200
constexpr int WORLD_HEIGHT = 3600;		// �� = 60cm(px), ���� = �Ʒ� 30��, �� 30�� 60 * 60

/*
	��Ŷ ���� ��Ģ
	��Ŷ�� �� ó���� unsigned char size�� ��Ŷ�� ����� ��Ÿ�� ������ ����� ��.
	�� ������ char type�� ��Ŷ�� Ÿ��, ��, ���� ������ ��Ŷ�� �������� ǥ���� �� -> ���̵� ���� ����

	CS -> 'Ŭ���̾�Ʈ'�� '����'����
	SC -> '����'�� 'Ŭ���̾�Ʈ'����
*/

// packet id
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

struct CS_MOVE_PACKET {
	unsigned char		size;
	char				type;
	char				direction;		// 0 = Up, 1 = Down, 2 = Left, 3 = Right
	short				x, y;			// �̵��ϴ� ��ǥ, ���߿� �ٲ� ��
};