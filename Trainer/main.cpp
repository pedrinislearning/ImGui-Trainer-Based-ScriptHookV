#include "include.h"

HWND hWnd = NULL;
Present oPresent = NULL;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* pRenderTargetView;
WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool ImGuiInit = false;
bool menu = false;
bool SHenabled = false;

HRESULT hkPresent(IDXGISwapChain* pSwapchain, UINT SyncInterval, UINT Flags) {
	if (!ImGuiInit) {
		if (SUCCEEDED(pSwapchain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapchain->GetDesc(&sd);
			hWnd = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 14);
			ImGui::GetIO().WantCaptureMouse = menu;
			ImGui::GetIO().WantTextInput = menu;
			ImGui::GetIO().WantCaptureKeyboard = menu;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX11_Init(pDevice, pContext);
			ImGuiInit = true;
		}
		else return oPresent(pSwapchain, SyncInterval, Flags);
	}
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_DELETE) & 1) {
		menu = !menu;
		ImGui::GetIO().MouseDrawCursor = menu;
	}

	if (menu) {
		ImGui::GetMouseCursor();
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
		ImGui::GetStyle().FrameRounding = 4.0f;
		ImGui::GetStyle().GrabRounding = 4.0f;
		RECT screen_rect;
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		auto x = float(screen_rect.right - 520.f) / 2.f;
		auto y = float(screen_rect.bottom - 350.f) / 2.f;
		ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(520.f, 350.f));
		ImGui::Begin("", &menu, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		{

			if (ImGui::BeginTabBar(" ", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
				if (ImGui::BeginTabItem("Player")) {
					if (ImGui::Button("Heal")) {

					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Vehicle")) {
					ImGui::SliderFloat("Acelleration Multplier", &cfg::VehAcc, 0.f, 2000.f, "% .2f");
					if (ImGui::Button("Fix Vehicle")) {
						SET_VEHICLE_FIXED(GET_VEHICLE_PED_IS_IN(PLAYER_PED_ID(), false));
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	
	ImGui::Render();
	pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapchain, SyncInterval, Flags);
}

void FunctionsLoop()
{
	while (true) 
	{

		Vehicle vehicle = GET_PLAYERS_LAST_VEHICLE();
		if (vehicle != NULL)
		_SET_VEHICLE_ENGINE_POWER_MULTIPLIER(vehicle, cfg::VehAcc);

		Sleep(1000);
	}
}

void STDMETHODCALLTYPE MainThread()
{
	uintptr_t pPresentAddr = FindPattern(GetSteamModule(), "48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B E8");
	CreateHook(pPresentAddr, (uintptr_t)hkPresent, (long long*)&oPresent);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		MainThread();
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FunctionsLoop, 0, 0, 0);

	}
	return TRUE;
}
