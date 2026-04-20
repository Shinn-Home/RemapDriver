#include "DrvComm.h"

#pragma warning(disable : 4005)

#include <ntstatus.h>

DrvComm& DrvComm::Get()
{
	static DrvComm Instance;
	return Instance;
}

DrvComm::DrvComm()
	: m_Ntdll(nullptr)
	, m_NtQuerySystemInformation(nullptr)
{
	m_Ntdll = ::LoadLibraryW(L"ntdll.dll");
	if (m_Ntdll != nullptr)
	{
		m_NtQuerySystemInformation = reinterpret_cast<fn_NtQuerySystemInformation>(::GetProcAddress(m_Ntdll, "NtQuerySystemInformation"));
	}
}

DrvComm::~DrvComm()
{
	if (m_Ntdll != nullptr)
	{
		::FreeLibrary(m_Ntdll);
		m_Ntdll = nullptr;
	}

	m_NtQuerySystemInformation = nullptr;
}

bool DrvComm::IsReady() const
{
	return (m_NtQuerySystemInformation != nullptr);
}

NTSTATUS DrvComm::Send(
	PVOID Buffer,
	ULONG BufferSize,
	PULONG ReturnLength
	) const
{
	if (!IsReady())
	{
		return STATUS_PROCEDURE_NOT_FOUND;
	}

	if (Buffer == nullptr || BufferSize == 0)
	{
		return STATUS_INVALID_PARAMETER;
	}

	return m_NtQuerySystemInformation(
		SystemCodeIntegrityInformation,
		Buffer,
		BufferSize,
		ReturnLength
	);
}
