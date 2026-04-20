#pragma once

#include "DrvComm.h"
#include "..\RemapDrv\Comm\DrvCommDef.h"

class DrvClient : public DrvComm
{
public:
	static DrvClient& Get();

	DrvClient() = default;
	~DrvClient() override = default;

	DrvClient(const DrvClient&) = delete;
	DrvClient& operator=(const DrvClient&) = delete;

	NTSTATUS SendCtl(
		ULONG CtlCode,
		PVOID Buffer,
		ULONG BufferSize,
		PULONG RetValue = nullptr
		) const;
};
