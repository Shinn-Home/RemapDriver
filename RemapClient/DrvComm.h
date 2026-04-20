#pragma once

#include <Windows.h>
#include <winternl.h>

class DrvComm
{
public:
	using fn_NtQuerySystemInformation = NTSTATUS (NTAPI*)(
		SYSTEM_INFORMATION_CLASS SystemInformationClass,
		PVOID SystemInformation,
		ULONG SystemInformationLength,
		PULONG ReturnLength
		);

	static DrvComm& Get();

	DrvComm();
	virtual ~DrvComm();

	DrvComm(const DrvComm&) = delete;
	DrvComm& operator=(const DrvComm&) = delete;

	bool IsReady() const;
	NTSTATUS Send(PVOID Buffer, ULONG BufferSize, PULONG ReturnLength = nullptr) const;

private:
	HMODULE m_Ntdll;
	fn_NtQuerySystemInformation m_NtQuerySystemInformation;
};
