
#include<Windows.h>
#include<iostream>

#include <fstream>

using namespace std;

#define SERVICE_NAME TEXT("New Service")

SERVICE_STATUS          gSvcStatus = { 0 }; //status stucture
SERVICE_STATUS_HANDLE   gSvcStatusHandle = NULL; //hServiceStatusHandle
HANDLE                  ghSvcStopEvent = NULL; //hserviceEvent

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv);
void SvcInstall(void);
void SvcDelete(void);
void SvcStart(void);
void SvcStop(void);



int main(int argc, char *argv[]) {

	freopen("log.txt", "a", stdout);



	cout << endl <<"MAIN FUNCTION START" << endl;
	bool serviceCtrlDispatcher = FALSE;

	if (lstrcmpA(argv[1], "install" ) == 0) {
		SvcInstall();
		cout << "Installation Success" << endl;
	}
	else if (lstrcmpA(argv[1], "start") == 0) {
		SvcStart();
		cout << "Service Start Success" << endl;
	}
	else if (lstrcmpA(argv[1], "stop") == 0) {
		SvcStop();
		cout << "Service Stop Success" << endl;
	}
	else if (lstrcmpA(argv[1], "delete") == 0) {
		SvcDelete();
		cout << "Service Delete Success" << endl;
		cout << "------------------------------------------------------------------------------------";
	}
	else {

		SERVICE_TABLE_ENTRY DispatchTable [] = 
		{
			{(LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) SvcMain},
			{NULL,NULL}
		};

		serviceCtrlDispatcher = StartServiceCtrlDispatcher(DispatchTable);

		if (!serviceCtrlDispatcher) {
			cout << "Service dispatcher failed" << GetLastError() << endl;
		}
		else {
			cout << "Service dispatcher Success" << endl;
		}
	}
	//system("PAUSE");

	return 0;
}

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv){
	cout << "Service start" << endl;
	bool serviceStatus = FALSE;

	gSvcStatusHandle = RegisterServiceCtrlHandler(
		SERVICE_NAME,
		ServiceCtrlHandler
	);

	if(gSvcStatusHandle == NULL){
		cout << "Handler Failed" << GetLastError() << endl;
	}
	else {
		cout << "Handler Success" << endl;
	}
	
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;

	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	serviceStatus = SetServiceStatus(gSvcStatusHandle, &gSvcStatus);

	if(!serviceStatus) {
		cout << "INTIAL SETUP FAILED." << GetLastError() << endl;
	}
	else {
		cout << "INITIAL SETUP SUCCESS." << endl;
	}

	SvcInit(dwArgc, lpszArgv);

	cout << "ServiceMain End" << endl;

}

//service control Handler

VOID WINAPI ServiceCtrlHandler(DWORD dwControl) {
	cout << "Service Control Handler start" << endl;

	switch (dwControl) {
		case SERVICE_CONTROL_STOP:
		{
			ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
			cout << "Service Stopped" << endl;
		}

		default: break;
	}
	cout << "Service Control Handler end" << endl;

}


//Service Init

VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv) {
	cout << "Service Init Started" << endl;

	//create Event
	ghSvcStopEvent = CreateEvent(
		NULL, TRUE, // Manual
		FALSE, NULL);


	if (ghSvcStopEvent == NULL) {
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	else {
		ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
	}



	while (1) {
		WaitForSingleObject(ghSvcStopEvent, INFINITE);
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}

	cout << "Service Init End" << endl;
}

VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint) {
	cout << "Report Service Start" << endl;

	static DWORD dwCheckPoint = 1;
	BOOL setServiceStatus = FALSE;

	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;


	if (dwCurrentState == SERVICE_START_PENDING) //about to start
	{
		gSvcStatus.dwControlsAccepted = 0;
	}
	else {
		gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
	{
		gSvcStatus.dwCheckPoint = 0;
	}
	else {
		gSvcStatus.dwCheckPoint = dwCheckPoint++;
		 
	}

	setServiceStatus = SetServiceStatus(gSvcStatusHandle, &gSvcStatus);


	if (!setServiceStatus) {
		cout << "Service Status Failed" << GetLastError() << endl;
	}
	else {
		cout << "Service Status Success" << endl;
	}

	cout << "Report Service End" << endl;
}

void SvcInstall(void) {
	cout << "SERVICE INSTALL START" << endl;

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	TCHAR szPath[MAX_PATH];
	DWORD dwGetModuleFileName = 0;
	dwGetModuleFileName = GetModuleFileName(NULL, szPath, MAX_PATH);

	if (dwGetModuleFileName == 0) {
		cout << "Service Installation failed" << GetLastError() << endl;
	}
	else {

		cout << "Service Installation SuccessFull" << endl;
	}
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (schSCManager == NULL) {
		cout << "Open SC Manager failed" << GetLastError() << endl;
	}
	else {
		cout << "Open SC Manager Success" << endl;
	}

	schService = CreateService(schSCManager, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		szPath,
		NULL, NULL, NULL, NULL, NULL);

	if (schService == NULL) {
		cout << "Create Service Failed" << GetLastError() << endl;
		CloseServiceHandle(schSCManager);
	}
	else {
		cout << "Create Service Success" << endl;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);


	cout << "SERVICE INSTALL END" << endl;

}

void SvcDelete(void) {
	cout << "SERVICE DELETE START" << endl;

	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	BOOL bDeleteService = FALSE;
	SERVICE_STATUS ssStatus;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		cout << "OpenSCManager failed" <<  GetLastError() << endl;
	}
	else {
		cout << "Open SC Manager Success" << endl;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,       // SCM database 
		SERVICE_NAME,          // name of service 
		SERVICE_ALL_ACCESS);            // need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else {
		cout << "Open Service Success" << endl;
	}

	// Delete the service.

	if (!DeleteService(schService))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
	}
	else printf("Service deleted successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	cout << "SERVICE DELETE END" << endl;

}
void SvcStart(void) {
	cout << "SERVICE-START START" << endl;
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dwbytesNeeded;

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		cout << "OpenSCManager failed" << GetLastError() << endl;
	}
	else {
		cout << "Open SC Manager Success" << endl;
	}


	schService = OpenService(
		schSCManager,       // SCM database 
		SERVICE_NAME,          // name of service 
		SC_MANAGER_ALL_ACCESS);            // need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else {
		cout << "Open Service Success" << endl;
	}

	bQueryServiceStatus = QueryServiceStatusEx(
		schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwbytesNeeded
	);
	if (!bQueryServiceStatus) {
		cout << "QueryService Failed" << GetLastError() << endl;
	}
	else {
		cout << "QueryService Success" << endl;
	}

	if ((SvcStatusProcess.dwCurrentState != SERVICE_STOPPED) && (SvcStatusProcess.dwCurrentState != SERVICE_STOP_PENDING)) {
		cout << "Service is Running" << endl;
	}
	else {
		cout << "Service Stopped" << endl;
	}

	while (SvcStatusProcess.dwCurrentState == SERVICE_STOP_PENDING) {
		bQueryServiceStatus = QueryServiceStatusEx(
			schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&SvcStatusProcess,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwbytesNeeded
		);

		if (!bQueryServiceStatus) {
			cout << "QueryService failed" << GetLastError() << endl;
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
		}
		else {
			cout << "QueryService Success" << endl;
		}

	}

	bStartService = StartService(
		schService, NULL, NULL);
	if (!bStartService) {
		cout << "Startservice failed" << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}
	else {
		cout << "Start Service Success" << endl;
	}

	bQueryServiceStatus = QueryServiceStatusEx(
		schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwbytesNeeded
	);

	if (!bQueryServiceStatus) {
		cout << "QueryService failed" << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}
	else {
		cout << "QueryService Success" << endl;
	}

	if (SvcStatusProcess.dwCurrentState == SERVICE_RUNNING) {
		cout << "Service Running" << endl;
	}
	else {
		cout << "Service Running Failed" << GetLastError() <<  endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);



	cout << "SERVICE-START END" << endl;

}
void SvcStop(void) {
	cout << "SERVICE-STOP START" << endl;


	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	BOOL bQueryServiceStatus = TRUE;
	BOOL bControlService = TRUE;
	DWORD dwbytesNeeded;

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		cout << "OpenSCManager failed" << GetLastError() << endl;
	}
	else {
		cout << "Open SC Manager Success" << endl;
	}


	schService = OpenService(
		schSCManager,       // SCM database 
		SERVICE_NAME,          // name of service 
		SC_MANAGER_ALL_ACCESS);            // need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else {
		cout << "Open Service Success" << endl;
	}


	bQueryServiceStatus = QueryServiceStatusEx(
		schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwbytesNeeded
	);

	if (!bQueryServiceStatus) {
		cout << "QueryService failed" << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}
	else {
		cout << "QueryService Success" << endl;
	}


	bControlService = ControlService(
		schService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&SvcStatusProcess
	);

	if (bControlService) {
		cout << "Control Service succes" << endl;
	}
	else {
		cout << "Control Service Failed" << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);

	}

	while (SvcStatusProcess.dwCurrentState != SERVICE_STOPPED) {
		bQueryServiceStatus = QueryServiceStatusEx(
			schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&SvcStatusProcess,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwbytesNeeded
		);

		if (bQueryServiceStatus) {
			cout << "QueryService failed" << GetLastError() << endl;
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
		}
		else {
			cout << "QueryService Success" << endl;
		}

		if (SvcStatusProcess.dwCurrentState == SERVICE_STOPPED) {
			cout << "Service Stopped Successfully" << endl;
			break;
		}
		else {
			cout << "Service Stopped Failed" << GetLastError() << endl;

			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
		}

	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	cout << "SERVICE-STOP END" << endl;

}

