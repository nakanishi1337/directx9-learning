#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>

#define szClassName TEXT("Display Cube") //アプリケーション名

const int PolygonNumber = 12;
const int IndexNumber = PolygonNumber * 3;
//vertex position
struct CUSTOMVERTEX {
	float x, y, z; // 頂点座標 
};
#define FVF_CUSTOM ( D3DFVF_XYZ)
//CUSTOMVERTEX* MyVertex = new CUSTOMVERTEX[IndexNumber];
CUSTOMVERTEX MyVertex[IndexNumber] = {
{-1,-1,-1,},
{1,1,-1,},
{1,-1,-1,},
{1,1,-1,},
{-1,-1,-1,},
{-1,1,-1,},
{-1,1,1,},
{0.999999,-1,1,},
{1,0.999999,1,},
{0.999999,-1,1,},
{-1,1,1,},
{-1,-1,1,},
{1,0.999999,1,},
{1,-1,-1,},
{1,1,-1,},
{1,-1,-1,},
{1,0.999999,1,},
{0.999999,-1,1,},
{0.999999,-1,1,},
{-1,-1,-1,},
{1,-1,-1,},
{-1,-1,-1,},
{0.999999,-1,1,},
{-1,-1,1,},
{-1,1,1,},
{-1,-1,-1,},
{-1,-1,1,},
{-1,-1,-1,},
{-1,1,1,},
{-1,1,-1,},
{-1,1,1,},
{1,1,-1,},
{-1,1,-1,},
{1,1,-1,},
{-1,1,1,},
{1,0.999999,1,}
};


//メッセージ処理
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case(WM_DESTROY):
		PostQuitMessage(0);//メッセージキューにWM_QUITをポスト メッセージループを抜ける値をProcが返す
		return(0);

	default:return DefWindowProc(hWnd, msg, wParam, lParam);
	};
}


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//ウィンドウ生成////////////////
	 //ウィンドウの初期位置とサイズを決める
	int WindowInitialPosition_X = 100;
	int WindowInitialPosition_Y = 100;
	int WindowInitialSize_X = 500;
	int WindowInitialSize_Y = 500;

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

	// 頂点バッファの作成
	IDirect3DVertexBuffer9* pVertex;
	if (FAILED(g_pD3DDev->CreateVertexBuffer(sizeof(CUSTOMVERTEX) * IndexNumber, D3DUSAGE_WRITEONLY, FVF_CUSTOM, D3DPOOL_MANAGED, &pVertex, NULL))) {
		g_pD3DDev->Release(); g_pD3D->Release();
		return 0;
	}

	//メッセージループ/////////////////
	MSG msg;
	D3DXMATRIX World;          // 立方体ワールド変換行列
	D3DXMATRIX View;   // ビュー変換行列
	D3DXMATRIX Persp;   // 射影変換行列
	D3DXMATRIX Rot_X;
	D3DXMATRIX Rot_Y;
	FLOAT Ang = 0.0f;   // 回転角度
	while (TRUE) {
		Sleep(1);
		Ang += 1;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;/*winprocから送られたWM_QUITで抜ける*/
			else DispatchMessage(&msg);
		}


		//ビデオカードのメモリへのアクセス
		void* pData;
		pVertex->Lock(0, sizeof(CUSTOMVERTEX) * IndexNumber, (void**)&pData, 0);
		memcpy(pData, MyVertex, sizeof(CUSTOMVERTEX) * IndexNumber);
		pVertex->Unlock();

		//オフセット
		D3DXMatrixRotationX(&Rot_X, D3DXToRadian(Ang * 0.4f));
		D3DXMatrixRotationY(&Rot_Y, D3DXToRadian(Ang * 0.4f));

		D3DXMatrixIdentity(&World);                    // 単位行列化 行列同士の積のため
		D3DXMatrixScaling(&World, 1, 1, 1);      //倍数で拡大
		D3DXMatrixMultiply(&World, &World, &Rot_X);    // X軸回転後
		D3DXMatrixMultiply(&World, &World, &Rot_Y);    // Y軸回転後


		// ビュー変換
		D3DXVECTOR3 eye = D3DXVECTOR3(0, 0, 10);
		D3DXVECTOR3 at = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
		D3DXMatrixLookAtLH(&View, &eye, &at, &up);

		// 射影変換
		D3DXMatrixPerspectiveFovLH(&Persp, D3DXToRadian(45), 640.0f / 480.0f, 1.0f, 10000.0f);

		// 行列登録
		g_pD3DDev->SetTransform(D3DTS_WORLD, &World);
		g_pD3DDev->SetTransform(D3DTS_VIEW, &View);
		g_pD3DDev->SetTransform(D3DTS_PROJECTION, &Persp);


		// ライトオフ
		g_pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

		//direct3d描画開始
		g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 250), 1.0f, 0);//バッファのクリア
		g_pD3DDev->BeginScene();//描画開始

		g_pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

		// 描画
		g_pD3DDev->SetStreamSource(0, pVertex, 0, sizeof(CUSTOMVERTEX));
		g_pD3DDev->SetFVF(FVF_CUSTOM);
		g_pD3DDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, /*三角形総数*/ PolygonNumber);

		//direct3d描画終了
		g_pD3DDev->EndScene();//描画終了
		g_pD3DDev->Present(NULL, NULL, NULL, NULL);//フロントバッファとバックバッファを交換

	}
	/////////////////////////////////

   //main関数の終了//////////////////////
	pVertex->Release();
	g_pD3DDev->Release();
	g_pD3D->Release();
	return (0);
}