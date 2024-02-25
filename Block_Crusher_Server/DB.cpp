#include "DB.h"

DB_EVENT::DB_EVENT()
{
}

DB_EVENT::DB_EVENT(int ob_id, std::chrono::system_clock::time_point time, DB_EVENT_TYPE et, int t_id)
{
	obj_id = ob_id;
	wakeup_time = time;
	event_id = et;
	target_id = t_id;
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

bool DB::Search_User(const char* id, const char* password)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR wstr[100];
	SQLWCHAR user_id[20];
	SQLWCHAR user_password[20];
	SQLINTEGER user_high_score;
	SQLLEN len[3];

	memset(wstr, 0, sizeof(wstr));
	wsprintf(wstr, L"EXEC Login %S, %S", id, password);
	//wsprintf(wstr, L"EXEC Nest_search_user %S", id);
	//cout << "이 스레드 들어옴?" << endl;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wstr, SQL_NTS);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &user_id, 20, &len[0]);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_password, 20, &len[1]);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &user_high_score, 4, &len[2]);

		retcode = SQLFetch(hstmt);
		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
			show_error(hstmt, SQL_FETCH_ABSOLUTE, retcode);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			std::cout << "성공" << std::endl;
		}
		else
		{
			std::cout << "실패" << std::endl;
		}
	}
	else
	{
		show_error(hstmt, SQL_HANDLE_STMT, retcode);
	}
	return true;
}

bool DB::Search_Id(const char* id)
{
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;

	SQLWCHAR wstr[100];
	SQLWCHAR user_id[20];

	memset(wstr, 0, sizeof(wstr));
	wsprintf(wstr, L"EXEC id_search %S", id);

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)wstr, SQL_NTS);
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