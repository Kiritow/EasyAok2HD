// EasyAok2HD: Tiny cheat engine on all resource.
// Created by Kiritow (https://github.com/kiritow)
// Just for playing with NPCs easier, not for any other purpose.
// ATTENTION: NEVER USE THIS IN ONLINE GAMES.
// NOTE:
// This program should be built in x86 mode.
// This cheat engine has only been tested with Age of Empires II: HD Edition (Steam version).
// This cheat engine does not require administrator permission.

#include <iostream>
#include <windows.h>
#include <vector>
using namespace std;

int SearchMemoryInPage(HANDLE process, const int& target, vector<long long>& vec, long long begin_address)
{
	float pagebuff[1024];
	SIZE_T retRead;
	ReadProcessMemory(process, (const void*)begin_address, pagebuff, sizeof(float) * 1024, &retRead);
	if (retRead != sizeof(float) * 1024)
	{
		//cout << "Failed to read some data in this page. Get " << retRead << ", need 4096. ErrorCode:" << GetLastError() << endl;
		//system("pause");
	}
	int cnt = retRead / sizeof(float);
	for (int i = 0; i < cnt; i++)
	{
		if (floor(pagebuff[i]) == target)
		{
			long long addr = begin_address + sizeof(float)*i;
			// cout << "Adding address " << (const void*)(addr) << "\t" << pagebuff[i] << endl;
			vec.push_back(addr);
		}
	}
	return cnt;
}

vector<long long> SearchMemory(HANDLE process, const int& target)
{
	vector<long long> vec;
	long long end = 0xFFFFFFFF;
	long long total_pages = end / (4 * 1024); // 4K page size
	for (long long current_page = 0; current_page < total_pages; current_page++)
	{
		long long begin_address = current_page * 4 * 1024;
		MEMORY_BASIC_INFORMATION info;
		SIZE_T ret=VirtualQueryEx(process, (const void*)begin_address, &info, sizeof(info));
		if (ret == 0)
		{
			//cout << "VirtualQueryEx call failed. ErrorCode:" << GetLastError() << endl;
			//cout << "Call parameter: VirtualQueryEx(" << process << "," << (const void*)begin_address << "," << &info << "," << sizeof(info) << ")" << endl;
			//system("pause");
		}
		if (info.State == MEM_COMMIT && info.AllocationProtect == PAGE_READWRITE)
		{
			// Commited page, readwrite. Let's search it.
			SearchMemoryInPage(process, target, vec, begin_address);
		}
	}
	cout << vec.size() << " results in set." << endl;
	return vec;
}

void ScanResultSet(HANDLE process, vector<long long>& vec,const int& target)
{
	for (auto iter = vec.begin(); iter != vec.end(); )
	{
		float temp;
		ReadProcessMemory(process, (const void*)(*iter), &temp, sizeof(temp), 0);
		if (floor(temp) != target)
		{
			// cout << "Removing address " << (const void*)(*iter) << endl;
			iter = vec.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void ListResultSet(HANDLE process, const vector<long long>& vec)
{
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
	{
		float temp;
		ReadProcessMemory(process, (const void*)(*iter), &temp, sizeof(temp), 0);
		cout << (const void*)(*iter) << "\t" << temp << endl;
	}
}

void ChangeResource(HANDLE process, const long long addr)
{
	float target = 1000000;
	WriteProcessMemory(process, (void*)(addr), &target, sizeof(target), NULL); // Food
	WriteProcessMemory(process, (void*)(addr + 4), &target, sizeof(target), NULL); // Wood
	WriteProcessMemory(process, (void*)(addr + 8), &target, sizeof(target), NULL); // Stone
	WriteProcessMemory(process, (void*)(addr + 12), &target, sizeof(target), NULL); // Gold
}

int main()
{
	cout << "Finding window..." << endl;
	HWND hwnd = FindWindowA(NULL, "Age of Empires II: HD Edition");
	if (hwnd == NULL)
	{
		cout << "Failed to find window." << endl;
		return 1;
	}
	DWORD processID;
	GetWindowThreadProcessId(hwnd, &processID);
	cout << "Opening process..." << endl;
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (process == NULL)
	{
		cout << "Failed to open process." << endl;
		return 1;
	}

	cout << "Ready" << endl;

	cout << "Please input current food amount:" << endl;
	int food_now;
	cin >> food_now;
	cout << "Searching memory..." << endl;
	vector<long long> vec = SearchMemory(process, food_now);
	while (vec.size() > 3)
	{
		cout << "Run the game and gain some food. Then type the value:" << endl;
		cin >> food_now;
		ScanResultSet(process, vec, food_now);
		cout << "Have " << vec.size() << " results in set." << endl;
	}
	ListResultSet(process, vec);
	cout << "Are we cool?" << endl;
	system("pause");
	cout << "Giving resource..." << endl;
	ChangeResource(process, vec[1]);
	ChangeResource(process, vec[2]);
	cout << "Switch back to game and have fun with it!" << endl;
	
	cout << "Program Finished." << endl;
    return 0;
}
