#include "DrvClient.h"

#include <array>
#include <iomanip>
#include <iostream>

int main()
{
	auto& Client = DrvClient::Get();
	if (!Client.IsReady())
	{
		std::cout << "NtQuerySystemInformation failed." << std::endl;
		return 1;
	}

	std::array<UCHAR, 0x100> Buffer = {};
	ULONG RetValue = 0;

	const NTSTATUS Status = Client.SendCtl(
		0x1000,
		Buffer.data(),
		static_cast<ULONG>(Buffer.size()),
		&RetValue
	);

	std::cout << "NtQuerySystemInformation status = 0x"
		<< std::hex << std::uppercase << static_cast<ULONG>(Status)
		<< std::endl
		<< "ÄÚºË²ã·µ»Ø return = 0x"
		<< RetValue
		<< std::endl;

	system("pause");
	return 0;
}
