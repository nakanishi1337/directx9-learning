#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>


#define szClassName TEXT("start skelton code") //アプリケーション名

//メッセージ処理
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case(WM_DESTROY):
		PostQuitMessage(0);//メッセージキューにWM_QUITをポスト メッセージループを抜ける値をProcが返す
		return(0);

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	};
}


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//ウィンドウ生成////////////////
	 //ウィンドウの初期位置とサイズを決める
	int WindowInitialPosition_X = 100;
	int WindowInitialPosition_Y = 100;
	int WindowInitialSize_X = 300;
	int WindowInitialSize_Y = 300;

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, NULL,(HBRUSH)(COLOR_WINDOW + 1), NULL, szClassName, NULL };
	if (!RegisterClassEx(&wcex)) { MessageBox(NULL, TEXT("ウィンドウクラスの取得に失敗しました"), TEXT("エラー"), MB_OK); return 0; }

	HWND hWnd = CreateWindow(szClassName, szClassName, WS_OVERLAPPEDWINDOW, WindowInitialPosition_X, WindowInitialPosition_Y, WindowInitialSize_X, WindowInitialSize_Y, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) { MessageBox(NULL, TEXT("ウィンドウの生成に失敗しました"), TEXT("エラー"), MB_OK); return 0; }

	ShowWindow(hWnd, nCmdShow);


	// Direct3Dの初期化//////////////////
	LPDIRECT3D9 g_pD3D;
	if (!(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return 0;

	D3DPRESENT_PARAMETERS d3dpp = { WindowInitialSize_X,WindowInitialSize_Y,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,D3DSWAPEFFECT_DISCARD,NULL,TRUE,TRUE,D3DFMT_D24S8,0,0 };

	LPDIRECT3DDEVICE9 g_pD3DDev;
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
		if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
			if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
				if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev)))
				{
					g_pD3D->Release(); return 0;
				}


	//メッセージループ/////////////////
	MSG msg;
	while (TRUE) {
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;/*winprocから送られたWM_QUITで抜ける*/
			else DispatchMessage(&msg);
		}

		//direct3d描画開始
		g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(40, 40, 80), 1.0f, 0);//バッファのクリア
		g_pD3DDev->BeginScene();//描画開始



		//direct3d描画終了
		g_pD3DDev->EndScene();//描画終了
		g_pD3DDev->Present(NULL, NULL, NULL, NULL);//フロントバッファとバックバッファを交換
	}
	/////////////////////////////////

   //main関数の終了//////////////////////
	g_pD3DDev->Release();
	g_pD3D->Release();
	return (0);
}