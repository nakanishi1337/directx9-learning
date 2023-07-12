
//アプリケーション名
#define szClassName TEXT("Animation import") 

#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <string>
#include "oxallocatehierarchy.h"

#include <Shlwapi.h>
#include <stdio.h>

#pragma comment(lib, "Shlwapi.lib")


OX::OXD3DXMESHCONTAINER* getMeshContainer(D3DXFRAME* frame) {
	if (frame->pMeshContainer)
		return (OX::OXD3DXMESHCONTAINER*)frame->pMeshContainer;
	if (frame->pFrameFirstChild) {
		OX::OXD3DXMESHCONTAINER* mesh = getMeshContainer(frame->pFrameFirstChild);
		if (mesh != 0)
			return mesh;
		if (frame->pFrameSibling)
			return getMeshContainer(frame->pFrameSibling);
		return 0;
	}
}

void setFrameId(OX::OXD3DXFRAME* frame, ID3DXSkinInfo* info) {
	std::map<std::string, DWORD> nameToIdMap;
	for (DWORD i = 0; i < info->GetNumBones(); i++)
		nameToIdMap[info->GetBoneName(i)] = i;

	struct create {
		static void f(std::map<std::string, DWORD> nameToIdMap, ID3DXSkinInfo* info, OX::OXD3DXFRAME* frame) {
			if (nameToIdMap.find(frame->Name) != nameToIdMap.end()) {
				frame->id = nameToIdMap[frame->Name];
				frame->offsetMatrix = *info->GetBoneOffsetMatrix(frame->id);
			}
			if (frame->pFrameFirstChild)
				f(nameToIdMap, info, (OX::OXD3DXFRAME*)frame->pFrameFirstChild);
			if (frame->pFrameSibling)
				f(nameToIdMap, info, (OX::OXD3DXFRAME*)frame->pFrameSibling);
		}
	};
	create::f(nameToIdMap, info, frame);
}

void updateCombMatrix(std::map<DWORD, D3DXMATRIX>& combMatrixMap, OX::OXD3DXFRAME* frame) {
	struct update {
		static void f(std::map<DWORD, D3DXMATRIX>& combMatrixMap, D3DXMATRIX& parentBoneMatrix, OX::OXD3DXFRAME* frame) {
			D3DXMATRIX& localBoneMatrix = frame->TransformationMatrix;
			D3DXMATRIX boneMatrix = localBoneMatrix * parentBoneMatrix;
			if (frame->id != 0xffffffff)
				combMatrixMap[frame->id] = frame->offsetMatrix * boneMatrix;
			if (frame->pFrameFirstChild)
				f(combMatrixMap, boneMatrix, (OX::OXD3DXFRAME*)frame->pFrameFirstChild);
			if (frame->pFrameSibling)
				f(combMatrixMap, parentBoneMatrix, (OX::OXD3DXFRAME*)frame->pFrameSibling);
		}
	};
	D3DXMATRIX iden;
	D3DXMatrixIdentity(&iden);
	update::f(combMatrixMap, iden, frame);
}




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

//main関数
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	////ウィンドウ生成///////////////

		//ウィンドウの初期位置とサイズを決める
	int WindowInitialPosition_X = 100;
	int WindowInitialPosition_Y = 100;
	int WindowInitialSize_X = 900;
	int WindowInitialSize_Y = 600;

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1) , NULL, szClassName, NULL };
	if (!RegisterClassEx(&wcex)) { MessageBox(NULL, TEXT("ウィンドウクラスの取得に失敗しました"), TEXT("エラー"), MB_OK); return 0; }

	HWND hWnd = CreateWindow(szClassName, szClassName, WS_OVERLAPPEDWINDOW, WindowInitialPosition_X, WindowInitialPosition_Y, WindowInitialSize_X, WindowInitialSize_Y, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) { MessageBox(NULL, TEXT("ウィンドウの生成に失敗しました"), TEXT("エラー"), MB_OK); return 0; }

	// ウィンドウ表示
	ShowWindow(hWnd, SW_SHOW);


	// Direct3Dの初期化/////////////
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


	// スキンメッシュ情報をXファイルから取得
	OX::AllocateHierarchy allocater;
	OX::OXD3DXFRAME* pRootFrame = 0;
	ID3DXAnimationController* controller = 0;
	D3DXLoadMeshHierarchyFromX(_T("Res/tiny.x"), D3DXMESH_MANAGED, g_pD3DDev, &allocater, 0, (D3DXFRAME**)&pRootFrame, &controller);

	OX::OXD3DXMESHCONTAINER* cont = getMeshContainer(pRootFrame);
	D3DXBONECOMBINATION* combs = (D3DXBONECOMBINATION*)cont->boneCombinationTable->GetBufferPointer();


	// フレーム内にボーンIDとオフセット行列を埋め込む
	setFrameId(pRootFrame, cont->pSkinInfo);

	// テクスチャ作成
	IDirect3DTexture9* tex = 0;
	D3DXCreateTextureFromFile(g_pD3DDev, _T("Res/Tiny_skin.dds"), &tex);

	// ライト///////////////////////////////////
	g_pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

   /////////////メッセージループ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	MSG msg;
	unsigned int i;
	FLOAT Ang = 0.0f;   // 回転角度

	D3DXMATRIX World;          // 立方体ワールド変換行列
	D3DXMATRIX Rot_X, Rot_Y;   // 立方体回転行列
	D3DXMATRIX Offset;         // 立方体オフセット行列

	D3DXMATRIX View;   // ビュー変換行列
	D3DXMATRIX Persp;   // 射影変換行列
	std::map<DWORD, D3DXMATRIX> combMatrixMap;

	while (TRUE) {

		Sleep(1);
		//終了処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;/*winprocから送られたWM_QUITで抜ける*/
			DispatchMessage(&msg);
		}

		// 時間を進めて姿勢更新
		controller->AdvanceTime(0.016f, 0);
		updateCombMatrix(combMatrixMap, pRootFrame);

	
	   //ワールド変換
		D3DXMatrixIdentity(&World);

		// ビュー変換
		D3DXVECTOR3 eye = D3DXVECTOR3(0, 0, -1000);
		D3DXVECTOR3 at = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
		D3DXMatrixLookAtLH(&View, &eye, &at, &up);


		// 射影変換
		D3DXMatrixPerspectiveFovLH(&Persp, D3DXToRadian(45), 640.0f / 480.0f, 1.0f, 10000.0f);

		// 行列登録 
		g_pD3DDev->SetTransform(D3DTS_WORLD, &World); 
		g_pD3DDev->SetTransform(D3DTS_VIEW, &View);
		g_pD3DDev->SetTransform(D3DTS_PROJECTION, &Persp);

		//direct3dの開始
		g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(40, 40, 80), 1.0f, 0);//バッファのクリア
		g_pD3DDev->BeginScene();//描画開始

		//描画部
		g_pD3DDev->SetTexture(0, tex);
		for (DWORD i = 0; i < cont->numBoneCombinations; i++) {
			DWORD boneNum = 0;
			for (DWORD j = 0; j < cont->maxFaceInfl; j++) {
				DWORD id = combs[i].BoneId[j];
				if (id != UINT_MAX) {
					g_pD3DDev->SetTransform(D3DTS_WORLDMATRIX(j), &combMatrixMap[id]);
					boneNum++;
				}
			}
			g_pD3DDev->SetRenderState(D3DRS_VERTEXBLEND, boneNum - 1);
			cont->MeshData.pMesh->DrawSubset(i);
		}


		//direct3dの終了
		g_pD3DDev->EndScene();//描画終了
		g_pD3DDev->Present(NULL, NULL, NULL, NULL);//バックバッファとフロントバッファの交代

	}
	//////////////////////////////////////////////////////////////////////////////////////////////////


	//main関数の終了
	allocater.DestroyFrame(pRootFrame);
	g_pD3DDev->Release();
	g_pD3D->Release();
	return (0);
}