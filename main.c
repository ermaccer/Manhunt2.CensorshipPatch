#include <pspsdk.h>
#include <pspkernel.h>

#include <stdio.h>
#include <string.h>
#include <pspctrl.h>
#include <systemctrl.h>

#include "pspmem.h"


#define TARGET_GAME "MAN2"
#define MODULE_NAME "MAN2STRA"

PSP_MODULE_INFO(MODULE_NAME, 0, 1, 0);

PSP_MAIN_THREAD_ATTR(0);

u32 mod_text_addr;

int CWeapon_GetClass()
{
	return 3;
}

static STMOD_HANDLER previous;


// 0 - us
// 1 - europe

int GetGameVersion()
{
	int us_check = ReadInt(0x2EE430);
	int eu_check = ReadInt(0x2EE540);
	
	
	if (us_check == 'hnaM')
		return 0;
	else if (eu_check == 'hnaM')
		return 1;
	else
		return -1;
}


int Init(unsigned int addr)
{
	PSPPatcher_Init(addr);
	
	//ULUS10280
	if (GetGameVersion() == 0)
	{
		// disable colour ramps
		Nop(0xD9924);
		
		// allow head decaps
		Nop(0x13BD74);
		
		// allow head explosions
		MakeCall(0x149A14, (int)CWeapon_GetClass);
		
		// restore blood pools functionality like pc mh2/mh
		Nop(0x251F70); // fix alpha
		MakeJump(0x252CC4, GetBaseAddr() + 0x252D3C);
	
	}
	//ULES00756
	if (GetGameVersion() == 1)
	{
		// disable colour ramps
		Nop(0xD9924);
		
		// allow head decaps
		Nop(0x13BD74);
		
		// allow head explosions
		MakeCall(0x149A14, (int)CWeapon_GetClass);
		
		// restore blood pools functionality like pc mh2/mh 
		Nop(0x252038);
		MakeJump(0x252D8C, GetBaseAddr() + 0x252E04);
	}


	return 0;
}

int OnModuleStart(SceModule2 *mod) {
	char *modname = mod->modname;

	if (strcmp(modname, "MAN2") == 0) {
		mod_text_addr = mod->text_addr;

		int ret = Init(mod_text_addr);
		if (ret != 0)
			return -1;

		sceKernelDcacheWritebackAll();
	}
	if (!previous)
		return 0;

	return previous(mod);
}


int module_start(SceSize argc, void* argp) {

	if (sceIoDevctl("kemulator:", 0x00000003, NULL, 0, NULL, 0) == 0) { 
		SceUID modules[10];
		int count = 0;
		if (sceKernelGetModuleIdList(modules, sizeof(modules), &count) >= 0) {
			int i;
			SceKernelModuleInfo info;
			for (i = 0; i < count; ++i) {
				info.size = sizeof(SceKernelModuleInfo);
				if (sceKernelQueryModuleInfo(modules[i], &info) < 0) {
					continue;
				}
				if (strcmp(info.name, "MAN2") == 0) {

					mod_text_addr = info.text_addr;

					int ret = Init(mod_text_addr);
					if (ret != 0)
						return;

					sceKernelDcacheWritebackAll();
					return;
				}
			}
		}
	}
	else {
		previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	}


	return 0;
}

