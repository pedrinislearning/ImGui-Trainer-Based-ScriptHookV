#pragma once
#include <fstream>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <vector>
#include <d3d11.h>
#include "natives.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

uintptr_t FindPattern(uintptr_t pModuleBaseAddress, const char* szSignature, size_t nSelectResultIndex = NULL) {
	auto PatternToBytes = [](const char* szpattern) {
		auto       m_iBytes = std::vector<int>{};
		const auto szStartAddr = const_cast<char*>(szpattern);
		const auto szEndAddr = const_cast<char*>(szpattern) + strlen(szpattern);

		for (auto szCurrentAddr = szStartAddr; szCurrentAddr < szEndAddr; ++szCurrentAddr) {
			if (*szCurrentAddr == '?') {
				++szCurrentAddr;
				if (*szCurrentAddr == '?') ++szCurrentAddr;
				m_iBytes.push_back(-1);
			}
			else m_iBytes.push_back(strtoul(szCurrentAddr, &szCurrentAddr, 16));
		}
		return m_iBytes;
	};

	const auto pDosHeader = (PIMAGE_DOS_HEADER)pModuleBaseAddress;
	const auto pNTHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)pModuleBaseAddress + pDosHeader->e_lfanew);
	const auto dwSizeOfImage = pNTHeaders->OptionalHeader.SizeOfImage;
	auto       m_iPatternBytes = PatternToBytes(szSignature);
	const auto pScanBytes = reinterpret_cast<std::uint8_t*>(pModuleBaseAddress);
	const auto m_iPatternBytesSize = m_iPatternBytes.size();
	const auto m_iPatternBytesData = m_iPatternBytes.data();
	size_t nFoundResults = 0;

	for (auto i = 0ul; i < dwSizeOfImage - m_iPatternBytesSize; ++i) {
		bool bFound = true;

		for (auto j = 0ul; j < m_iPatternBytesSize; ++j) {
			if (pScanBytes[i + j] != m_iPatternBytesData[j] && m_iPatternBytesData[j] != -1) {
				bFound = false;
				break;
			}
		}

		if (bFound) {
			if (nSelectResultIndex != 0) {
				if (nFoundResults < nSelectResultIndex) {
					nFoundResults++;
					bFound = false;
				}
				else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
			}
			else return reinterpret_cast<uintptr_t>(&pScanBytes[i]);
		}
	}
	return NULL;
}

uintptr_t GetSteamModule() {
	uintptr_t ov = (uintptr_t)GetModuleHandleA("GameOverlayRenderer64.dll");
	if (ov == NULL) {
		MessageBox(NULL, L"Couldn't find Steam Overlay.\nPlease enable GameOverlay in Steam APP !", L"Pedrin's Simple Trainer", MB_ICONERROR);
		exit(-1);
	}
	else {
		return ov;
	}
}

void CreateHook(__int64 iAddr, __int64 iFunction, __int64* iOriginal) {
	static uintptr_t pHookAddr;
	if (!pHookAddr) pHookAddr = FindPattern(GetSteamModule(), ("48 ? ? ? ? 57 48 83 EC 30 33 C0"));
	auto hook = ((__int64(__fastcall*)(__int64 addr, __int64 func, __int64* orig, __int64 smthng))(pHookAddr));
	hook((__int64)iAddr, (__int64)iFunction, iOriginal, (__int64)1);
}

namespace cfg {
	int MenuKey;
	float VehAcc = 0;
}

typedef HRESULT(*Present)(IDXGISwapChain*, UINT, UINT);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);