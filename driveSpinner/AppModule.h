#pragma once
class AppModule
{
	static SERVICE_TABLE_ENTRY ServiceTable[1];
public:
	static WinServiceHelper svc;
	static void ServiceMain(int argc, char** argv);
	static bool initialize();
	static int doService(int argc, char** argv);
};

