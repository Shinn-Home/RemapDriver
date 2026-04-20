#pragma once

#include <ntddk.h>
#include <ntimage.h>

namespace Remap
{
	PVOID
	RemapSelf (
		IN PDRIVER_OBJECT DrvObj
		);
}
