#include <pspsdk.h>
#include <pspkernel.h>

#include <stdio.h>
#include <string.h>
#include <pspctrl.h>
#include <systemctrl.h>

#include "pspmem.h"


#define TARGET_GAME "MAN2"
#define MODULE_NAME "MAN2STRA"

PSP_MODULE_INFO(MODULE_NAME, 0x1007, 1, 0);



int CWeapon_GetClass()
{
	return 3;
}

static unsigned int wepClassStub;

static unsigned int MakeSyscallStub(void *function) {
  SceUID block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, 2 * sizeof(unsigned int), NULL);
  unsigned int stub = (unsigned int)sceKernelGetBlockHeadAddr(block_id);
  _sw(0x03E00008, stub);
  _sw(0x0000000C | (sceKernelQuerySystemCall(function) << 6), stub + 4);
  return stub;
}

static STMOD_HANDLER previous;

void Init(unsigned int addr)
{
	PSPPatcher_Init(addr);
	
	// disable colour ramps
	Nop(0xD9924);
	
	// allow head decaps
	Nop(0x13BD74);
	
	// allow head explosions
	MakeCall(0x149A14, MakeSyscallStub(CWeapon_GetClass));

	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int OnModuleStart(SceModule2 *mod) {
	char* name = mod->modname;
	unsigned int baseAddr = mod->text_addr;


	if (strcmp(name, TARGET_GAME) == 0)
		Init(baseAddr);
	
	if (!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp)
{
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	/*if (sceIoDevctl("kemulator:", 0x00000003, NULL, 0, NULL, 0) == 0) {
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

				if (strcmp(info.name, TARGET_GAME) == 0)
					Init(info.text_addr);
			}
		}
	}*/
	return 0;
}



