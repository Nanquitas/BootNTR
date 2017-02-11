#include "main.h"
#include "config.h"

extern ntrConfig_t      *ntrConfig;
extern bootNtrConfig_t  *bnConfig;
extern u8               *tmpBuffer;
extern char             *g_error;

Result	bnInitParamsByFirmware(void)
{
	u32		kernelVersion = osGetKernelVersion();
	bool	isNew3DS = false;

	APT_CheckNew3DS(&isNew3DS);
	ntrConfig->isNew3DS = isNew3DS ? 1 : 0;
	ntrConfig->PMPid = 2;
	ntrConfig->HomeMenuPid = 0xF;
	bnConfig->SMPid = 3;
	bnConfig->FSPid = 0;
	if (!isNew3DS)
	{
		ntrConfig->IoBasePad = 0xfffc6000;
		ntrConfig->IoBaseLcd = 0xfffc8000;
		ntrConfig->IoBasePdc = 0xfffc0000;
		ntrConfig->KMMUHaxAddr = 0xfffbe000;
		ntrConfig->KMMUHaxSize = 0x00010000;
		ntrConfig->KProcessHandleDataOffset = 0xD4;
		ntrConfig->KProcessPIDOffset = 0xB4;
		ntrConfig->KProcessCodesetOffset = 0xB0;
		if (kernelVersion == SYSTEM_VERSION(2, 44, 6))
		{
			//TODO: add old3ds 8.0.0 firmware support 
			ntrConfig->firmVersion = SYSTEM_VERSION(8, 0, 0);
			bnConfig->SvcPatchAddr = 0xDFF82294;
            goto unsupported;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 46, 0))
		{
			// old3ds 9.0.0
			ntrConfig->firmVersion = SYSTEM_VERSION(9, 0, 0);
			ntrConfig->PMSvcRunAddr = 0x00102FC0;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff882cc;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff882d0;
			bnConfig->SvcPatchAddr = 0xDFF82290;
			bnConfig->FSPatchAddr = 0x0010ED64;
			bnConfig->SMPatchAddr = 0x00101838;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 50, 1))
		{
			// old3ds 9.6.0
			ntrConfig->firmVersion = SYSTEM_VERSION(9, 6, 0);
			ntrConfig->PMSvcRunAddr = 0x00103184;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff882D8;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff882DC;
			bnConfig->SvcPatchAddr = 0xDFF82284;
			bnConfig->FSPatchAddr = 0x0010EFAC;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 51, 0))
		{
			// old3ds 11.0.0
			ntrConfig->firmVersion = SYSTEM_VERSION(11, 0, 0);
			ntrConfig->PMSvcRunAddr = 0x00103154;
			ntrConfig->ControlMemoryPatchAddr1 = 0xDFF88468;
			ntrConfig->ControlMemoryPatchAddr2 = 0xDFF8846C;
			bnConfig->SvcPatchAddr = 0xDFF82288;
			bnConfig->FSPatchAddr = 0x0010EED4;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 51, 2))
		{
			// old3ds 11.1.0
			ntrConfig->firmVersion = SYSTEM_VERSION(11, 1, 0);
			ntrConfig->PMSvcRunAddr = 0x00103154;
			ntrConfig->ControlMemoryPatchAddr1 = 0xDFF88468;
			ntrConfig->ControlMemoryPatchAddr2 = 0xDFF8846C;
			bnConfig->SvcPatchAddr = 0xDFF82288;
			bnConfig->FSPatchAddr = 0x0010F024;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
        else if (kernelVersion == SYSTEM_VERSION(2, 52, 0))
        {
            // old3ds 11.2.0
            ntrConfig->firmVersion = SYSTEM_VERSION(11, 2, 0);
            ntrConfig->PMSvcRunAddr = 0x00103154;
            ntrConfig->ControlMemoryPatchAddr1 = 0xDFF88468;
            ntrConfig->ControlMemoryPatchAddr2 = 0xDFF8846C;
            bnConfig->SvcPatchAddr = 0xDFF82288;
            bnConfig->FSPatchAddr = 0x0010F024;
            bnConfig->SMPatchAddr = 0x0010189C;
        }
        else if (kernelVersion == SYSTEM_VERSION(2, 53, 0))
        {
            // old3ds 11.3.0
            ntrConfig->firmVersion = SYSTEM_VERSION(11, 3, 0);
            ntrConfig->PMSvcRunAddr = 0x00103154;
            ntrConfig->ControlMemoryPatchAddr1 = 0xDFF884E4;
            ntrConfig->ControlMemoryPatchAddr2 = 0xDFF884E8;
            bnConfig->SvcPatchAddr = 0xDFF82288;
            bnConfig->FSPatchAddr = 0x0010F024;
            bnConfig->SMPatchAddr = 0x0010189C;
        }
        else 
            goto unsupported;
	}
	else
	{
		ntrConfig->IoBasePad = 0xfffc2000;
		ntrConfig->IoBaseLcd = 0xfffc4000;
		ntrConfig->IoBasePdc = 0xfffbc000;
		ntrConfig->KMMUHaxAddr = 0xfffba000;
		ntrConfig->KMMUHaxSize = 0x00010000;
		ntrConfig->KProcessHandleDataOffset = 0xdc;
		ntrConfig->KProcessPIDOffset = 0xBC;
		ntrConfig->KProcessCodesetOffset = 0xB8;
		if (kernelVersion == SYSTEM_VERSION(2, 45, 5))
		{
			// new3ds 8.1
			ntrConfig->firmVersion = SYSTEM_VERSION(8, 1, 0);
			ntrConfig->PMSvcRunAddr = 0x0010308C;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff88158;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff8815C;
			bnConfig->SvcPatchAddr = 0xDFF82264;
			bnConfig->FSPatchAddr = 0x0010ED64;
			bnConfig->SMPatchAddr = 0x00101838;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 46, 0))
		{
			// new3ds 9.0
			ntrConfig->firmVersion = SYSTEM_VERSION(9, 0, 0);
			ntrConfig->PMSvcRunAddr = 0x00102FEC;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff884ec;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff884f0;
			bnConfig->SvcPatchAddr = 0xDFF82260;
			bnConfig->FSPatchAddr = 0x0010ED64;
			bnConfig->SMPatchAddr = 0x00101838;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 49, 0))
		{
			// new3ds 9.5
			ntrConfig->firmVersion = SYSTEM_VERSION(9, 5, 0);
			ntrConfig->PMSvcRunAddr = 0x001030F8;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff884F8;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff884FC;
			bnConfig->SvcPatchAddr = 0xDFF8226c;
			bnConfig->FSPatchAddr = 0x0010ED64;
			bnConfig->SMPatchAddr = 0x00101838;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 50, 1))
		{
			//new3ds 9.6
			ntrConfig->firmVersion = SYSTEM_VERSION(9, 6, 0);
			ntrConfig->PMSvcRunAddr = 0x001030D8;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff8850C;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff88510;
			bnConfig->SvcPatchAddr = 0xDFF82268;
			bnConfig->FSPatchAddr = 0x0010EFAC;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 50, 7))
		{
			// new3ds 10.0
			//TODO: add new3ds 10.0 firmware support
			ntrConfig->firmVersion = SYSTEM_VERSION(10, 0, 0);
            goto unsupported;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 50, 9))
		{
			// new3ds 10.2
			ntrConfig->firmVersion = SYSTEM_VERSION(10, 2, 0);
			ntrConfig->PMSvcRunAddr = 0x001031E4;
			ntrConfig->ControlMemoryPatchAddr1 = 0xdff884E4;
			ntrConfig->ControlMemoryPatchAddr2 = 0xdff884E8;
			bnConfig->SvcPatchAddr = 0xDFF82270;
			bnConfig->FSPatchAddr = 0x0010EED4;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 51, 0))
		{
			// new3ds 11.0
			ntrConfig->firmVersion = SYSTEM_VERSION(11, 0, 0);
			ntrConfig->PMSvcRunAddr = 0x00103150;
			ntrConfig->ControlMemoryPatchAddr1 = 0xDFF88598;
			ntrConfig->ControlMemoryPatchAddr2 = 0xDFF8859C;
			bnConfig->SvcPatchAddr = 0xDFF8226C;
			bnConfig->FSPatchAddr = 0x0010EED4;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
		else if (kernelVersion == SYSTEM_VERSION(2, 51, 2))
		{
			// new3ds 11.1
			ntrConfig->firmVersion = SYSTEM_VERSION(11, 1, 0);
			ntrConfig->PMSvcRunAddr = 0x00103150;
			ntrConfig->ControlMemoryPatchAddr1 = 0xDFF88598;
			ntrConfig->ControlMemoryPatchAddr2 = 0xDFF8859C;
			bnConfig->SvcPatchAddr = 0xDFF8226C;
			bnConfig->FSPatchAddr = 0x0010F024;
			bnConfig->SMPatchAddr = 0x0010189C;
		}
        else if (kernelVersion == SYSTEM_VERSION(2, 52, 0))
        {
            // new3ds 11.2
            ntrConfig->firmVersion = SYSTEM_VERSION(11, 2, 0);
            ntrConfig->PMSvcRunAddr = 0x00103150;
            ntrConfig->ControlMemoryPatchAddr1 = 0xDFF88598;
            ntrConfig->ControlMemoryPatchAddr2 = 0xDFF8859C;
            bnConfig->SvcPatchAddr = 0xDFF8226C;
            bnConfig->FSPatchAddr = 0x0010F024;
            bnConfig->SMPatchAddr = 0x0010189C;
        }
        else if (kernelVersion == SYSTEM_VERSION(2, 53, 0))
        {
            // new3ds 11.3
            ntrConfig->firmVersion = SYSTEM_VERSION(11, 3, 0);
            ntrConfig->PMSvcRunAddr = 0x00103150;
            ntrConfig->ControlMemoryPatchAddr1 = 0xDFF885FC; 
            ntrConfig->ControlMemoryPatchAddr2 = 0xDFF88600;
            bnConfig->SvcPatchAddr = 0xDFF8226C;
            bnConfig->FSPatchAddr = 0x0010F024; 
            bnConfig->SMPatchAddr = 0x0010189C;
        }
        else 
            goto unsupported;
	}
	bnConfig->requireKernelHax = 0;
    return (0);
unsupported:
    ntrConfig->firmVersion = 0;
    return (RESULT_ERROR);
}
