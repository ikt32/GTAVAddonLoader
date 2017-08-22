#pragma once

#include <cstdint>
#include <vector>
#include "inc/natives.h"

namespace rage {
	class grcTexture {
	public:
		void* VTable; // 0x0000
		char _0x0008[0x20]; // 0x0008
		char* name; // 0x0028
		char _0x0030[0x14]; // 0x0030
		uint32_t unk_0x0044; // 0x0044
		char _0x0048[0x8]; // 0x0048
		uint16_t resolutionX; // 0x0050
		uint16_t resolutionY; // 0x0052
		char _0x0054[0xC]; // 0x0054
		grcTexture* previous; // 0x0060
		grcTexture* next; // 0x0068
		char _0x0070[0x20]; // 0x0070
	};

	class pgDictionary {
	public:
		char _0x0000[0x30]; // 0x0000
		grcTexture** textures; // 0x0030
		uint16_t textureCount; // 0x0038
	};
}


struct ScriptHeader
{
	char padding1[16];					//0x0
	unsigned char** codeBlocksOffset;	//0x10
	char padding2[4];					//0x18
	int codeLength;						//0x1C
	char padding3[4];					//0x20
	int localCount;						//0x24
	char padding4[4];					//0x28
	int nativeCount;					//0x2C
	__int64* localOffset;				//0x30
	char padding5[8];					//0x38
	__int64* nativeOffset;				//0x40
	char padding6[16];					//0x48
	int nameHash;						//0x58
	char padding7[4];					//0x5C
	char* name;							//0x60
	char** stringsOffset;				//0x68
	int stringSize;						//0x70
	char padding8[12];					//0x74
										//END_OF_HEADER

	bool IsValid() const { return codeLength > 0; }
	int CodePageCount() const { return (codeLength + 0x3FFF) >> 14; }
	int GetCodePageSize(int page) const
	{
		return (page < 0 || page >= CodePageCount() ? 0 : (page == CodePageCount() - 1) ? codeLength & 0x3FFF : 0x4000);
	}
	unsigned char* GetCodePageAddress(int page) const { return codeBlocksOffset[page]; }
	unsigned char* GetCodePositionAddress(int codePosition) const
	{
		return codePosition < 0 || codePosition >= codeLength ? NULL : &codeBlocksOffset[codePosition >> 14][codePosition & 0x3FFF];
	}
	char* GetString(int stringPosition)const
	{
		return stringPosition < 0 || stringPosition >= stringSize ? NULL : &stringsOffset[stringPosition >> 14][stringPosition & 0x3FFF];
	}

};
struct ScriptTableItem
{
	ScriptHeader* Header;
	char padding[4];
	int hash;

	inline bool IsLoaded() const
	{
		return Header != NULL;
	}
};

struct ScriptTable
{
	ScriptTableItem* TablePtr;
	char padding[16];
	int count;
	ScriptTableItem* FindScript(int hash)
	{
		if (TablePtr == NULL)
		{
			return NULL;//table initialisation hasnt happened yet
		}
		for (int i = 0; i<count; i++)
		{
			if (TablePtr[i].hash == hash)
			{
				return &TablePtr[i];
			}
		}
		return NULL;
	}
};

struct GlobalTable
{
	__int64** GlobalBasePtr;
	__int64* AddressOf(int index) const { return &GlobalBasePtr[index >> 18 & 0x3F][index & 0x3FFFF]; }
	bool IsInitialised()const { return *GlobalBasePtr != NULL; }
};

class MemoryAccess {
public:
	static uintptr_t FindPattern(const char* pattern, const char* mask);
	static uintptr_t FindPattern(const char *pattern, const char *mask, const char *startAddress, size_t size);
	static void initTxdStore();
	static std::vector<rage::grcTexture *> GetTexturesFromTxd(Hash txdHash);
	static std::array<std::vector<int>, 0x20> GenerateVehicleModelList();
	static std::vector<uint8_t> GetVehicleModKits(int modelHash);
	static char *GetVehicleGameName(int modelHash);
	static char *GetVehicleMakeName(int modelHash);
	static void enableCarsGlobal();
	static bool findPatterns();
};
