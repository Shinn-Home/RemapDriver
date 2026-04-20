#include "DrvClient.h"

DrvClient& DrvClient::Get()
{
	static DrvClient Instance;
	return Instance;
}

NTSTATUS DrvClient::SendCtl(
	ULONG CtlCode,
	PVOID Buffer,
	ULONG BufferSize,
	PULONG RetValue
	) const
{
	if (Buffer == nullptr && BufferSize != 0)
	{
		return STATUS_INVALID_PARAMETER;
	}

	DRV_COMM_PACKAGE Package = {};
	Package.Magic = DRV_COMM_MAGIC;
	Package.CtlCode = CtlCode;
	Package.Buffer = Buffer;
	Package.BufferSize = BufferSize;
	Package.RetValue = 0;

	const NTSTATUS Status = Send(
		&Package,
		static_cast<ULONG>(sizeof(Package))
	);

	if (RetValue != nullptr)
	{
		*RetValue = Package.RetValue;
	}

	return Status;
}
