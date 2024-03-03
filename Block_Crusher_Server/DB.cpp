#include "DB.h"

DB_EVENT::DB_EVENT()
{
}

DB_EVENT::DB_EVENT(int ob_id, std::chrono::system_clock::time_point time, DB_EVENT_TYPE et, std::wstring id, std::wstring password)
{
	obj_id = ob_id;
	wakeup_time = time;
	event_id = et;
	_id = id;
	_password = password;
}

void DB::InitDB()
{
	SQLRETURN retcode;

	setlocale(LC_ALL, "korean");
	std::wcout.imbue(std::locale("korean"));

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2024_WINTER_ODBC", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					std::cout << "ODBC 연결. DB 초기화 완료" << std::endl;
				}
			}
		}
	}
}

void DB::ReleaseDB()
{
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

int DB::Search_User(std::wstring id, std::wstring password)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR wstr[100];
	SQLWCHAR user_id[20];
	SQLWCHAR user_password[20];
	SQLINTEGER user_high_score;
	SQLINTEGER user_login_state = -1;
	SQLLEN len[4];

	memset(wstr, 0, sizeof(wstr));
	//std::wcout << "아이디 : " << id << ", 비밀번호 : " << password << std::endl;
	wsprintf(wstr, L"EXEC Login %ls, %ls", id.c_str(), password.c_str());
	//std::wcout << wstr << std::endl;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wstr, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &user_id, 20, &len[0]);
		retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, &user_password, 20, &len[1]);
		retcode = SQLBindCol(hstmt, 3, SQL_INTEGER, &user_high_score, 4, &len[2]);
		retcode = SQLBindCol(hstmt, 4, SQL_INTEGER, &user_login_state, 4, &len[3]);

		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
			std::cout << "오류 발생 1" << std::endl;
			show_error(hstmt, SQL_HANDLE_STMT, retcode);
		}
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			/*std::wcout << user_id << ", " << user_password << ", ";
			std::cout << "점수 : " << user_high_score << ", 로그인 : " << user_login_state << std::endl;
			if (user_login_state == DB_SIGN_UP) {
				std::cout << "새로운 id 감지. 회원가입 합니다" << std::endl;
			}
			else if (user_login_state == DB_LOGIN_SUCCESS) {
				std::cout << "로그인 성공" << std::endl;
			}
			else if (user_login_state == DB_LOGIN_FAIL) {
				std::cout << "로그인 실패" << std::endl;
			}
			else if (user_login_state == DB_ALREADY_INGAME) {
				std::cout << "이미 접속중인 계정" << std::endl;
			}
			else if (user_login_state == -1) {
				std::cout << "여기도 들어옴" << std::endl;
			}*/
			SQLCancel(hstmt);
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		}
		else {
			std::cout << "오류 발생 2" << std::endl;
			show_error(hstmt, SQL_HANDLE_STMT, retcode);
		}
	}
	else
	{
		std::cout << "오류 발생 3" << std::endl;
		show_error(hstmt, SQL_HANDLE_STMT, retcode);
	}

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return user_login_state;
}

void DB::Disconnect_User(std::wstring id)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR wstr[100];
	SQLWCHAR user_id[20];
	SQLLEN len[1];

	memset(wstr, 0, sizeof(wstr));
	wsprintf(wstr, L"EXEC id_disconnect %ls", id.c_str());

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wstr, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &user_id, 20, &len[0]);

		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) 
			show_error(hstmt, SQL_HANDLE_STMT, retcode);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			std::cout << "id DB에서 삭제 완" << std::endl;
			SQLCancel(hstmt);
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		}
		else {
			show_error(hstmt, SQL_HANDLE_STMT, retcode);
		}
	}

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
}

void DB::show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}