#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

const static u32 KERNELMEM_START = 0xDFF00000;
const static u32 KERNELMEM_SIZE  = 0x00100000;
const static u32 CHUNK_SIZE      = 0x400; // 1kB
const static char* FILENAME      = "/kernelmem_dump.bin";


static u8 tmpbuf[0x400];
static size_t mem_ctr;

s32 __attribute__ ((noinline)) dump_chunk()
{
    memcpy(tmpbuf, (void*)(KERNELMEM_START + mem_ctr), CHUNK_SIZE);
    return 0;
}

s32 __attribute__ ((naked)) dump_chunk_wrapper()
{
    __asm__ __volatile__ ("cpsid aif");
    dump_chunk();
}

int main(int argc, char** argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

    FILE* out_file = fopen(FILENAME, "wb");
    printf("Dumping kernel memory...\n");

    for (mem_ctr = 0; mem_ctr < KERNELMEM_SIZE; mem_ctr += CHUNK_SIZE) {
        svcBackdoor(dump_chunk_wrapper);
        printf("Dumping 1kb @ 0x%08lX...\n", KERNELMEM_START + mem_ctr);
        fwrite(tmpbuf, 1, CHUNK_SIZE, out_file);
    }

    fclose(out_file);
    printf("Dumped kernel memory to '%s'\n", FILENAME);

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}
