#pragma comment (lib,"libfbxsdk.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <fbxsdk.h>
#include <d3d9.h>
#include <d3dx9.h>

char msgbuf[100];

#define FVF_CUSTOM (D3DFVF_XYZB4|D3DFVF_LASTBETA_UBYTE4  )
#define szClassName TEXT("fbx animation")
char ImportFileName[] = "Res/panda_nomalized_polygon900.fbx";
LPCWSTR TextureName = L"panda_tex_2.jpg";

struct CUSTOMVERTEX {
	D3DXVECTOR3 coord;
	D3DXVECTOR3 weight;
	unsigned char matrixIndex[4];
};
int PositionNumber;
int PolygonNumber;
int IndexNumber;
int SkinNumber;
int BoneNumber;
FbxTime period;
FbxTime startTime;
FbxTime endTime;
int FrameNum;
CUSTOMVERTEX* MyVertex;
D3DXMATRIX* initMats;
D3DXMATRIX* INVinitMats;
D3DXMATRIX** FrameMat;

// シェーダ
//  重み係数と頂点を動かすためのワールド変換行列の配列を渡します
const char* vertexShaderStr =
"float4x4 view : register(c0);"
"float4x4 proj : register(c4);"
"float4x4 trs : register(c8);"
"float4x4 world[26] : register(c12);""  "
"struct VS_IN {"
"    float3 pos : POSITION;"
"    float3 blend : BLENDWEIGHT;"
"    int4 idx : BLENDINDICES;"
"};"
"struct VS_OUT {"
"    float4 pos : POSITION;"
"};"
"VS_OUT main( VS_IN In ) {"
"    VS_OUT Out = (VS_OUT)0;"
"    float w[3] = (float[3])In.blend;"
"    float4x4 comb = (float4x4)0;"
"    for ( int i = 0; i < 3; i++ )"
"        comb += world[In.idx[i]] * w[i];"
"    comb += world[In.idx[3]] * (1.0f - w[0] - w[1] - w[2]);"
"    "
"    Out.pos = mul( float4(In.pos, 1.0f), comb );"
"    Out.pos = mul( Out.pos, trs );"

"    Out.pos = mul( Out.pos, view );"
"    Out.pos = mul( Out.pos, proj );"
"    return Out;"
"}";

// ピクセルシェーダ
const char* pixelShaderStr =
"struct VS_OUT {"
"    float4 pos : POSITION;"
"};"
"float4 main( VS_OUT In ) : COLOR {"
"    return float4(1.0f, 1.0f, 1.0f, 1.0f);"
"}"
"";


void GetMeshInfo(FbxNode* node) {

	FbxMesh* mesh = (FbxMesh*)node->GetNodeAttribute();
	PositionNumber = mesh->GetControlPointsCount();	// 頂点数
	PolygonNumber = mesh->GetPolygonCount();  //ポリゴン数
	IndexNumber = PolygonNumber * 3; //インデックス数
	SkinNumber = mesh->GetDeformerCount(); //スキン数


	// 頂点座標配列
	FbxVector4* position = mesh->GetControlPoints();	

	//インデックスリスト
	int* index = new int[IndexNumber];
	for (int p = 0; p < PolygonNumber; ++p)
	{
		for (int n = 0; n < 3; ++n)
		{
			index[3 * p + n] = mesh->GetPolygonVertex(p, n);
		}
	}

	//初期行列取り出し/////////////////////////////////
	for (int i = 0; i < SkinNumber; ++i) {
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
		BoneNumber = skin->GetClusterCount();
		
		initMats = new D3DXMATRIX[BoneNumber];

		for (int j = 0; j < BoneNumber; ++j) {
			FbxCluster* cluster = skin->GetCluster(j);
			FbxAMatrix initMat;
			cluster->GetTransformLinkMatrix(initMat);

			initMats[j]._11 = initMat[0][0];
			initMats[j]._12 = initMat[0][1];
			initMats[j]._13 = initMat[0][2];
			initMats[j]._14 = initMat[0][3];
			initMats[j]._21 = initMat[1][0];
			initMats[j]._22 = initMat[1][1];
			initMats[j]._23 = initMat[1][2];
			initMats[j]._24 = initMat[1][3];
			initMats[j]._31 = initMat[2][0];
			initMats[j]._32 = initMat[2][1];
			initMats[j]._33 = initMat[2][2];
			initMats[j]._34 = initMat[2][3];
			initMats[j]._41 = initMat[3][0];
			initMats[j]._42 = initMat[3][1];
			initMats[j]._43 = initMat[3][2];
			initMats[j]._44 = initMat[3][3];
		}
	}
	///////////////////////////////////////////////////

	//逆初期行列取り出し/////////////////////////////////
	for (int i = 0; i < SkinNumber; ++i) {
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
		int BoneNumber = skin->GetClusterCount();
		INVinitMats = new D3DXMATRIX[BoneNumber];
		for (int j = 0; j < BoneNumber; ++j) {
			FbxCluster* cluster = skin->GetCluster(j);
			FbxAMatrix initMat;
			cluster->GetTransformLinkMatrix(initMat);
			FbxAMatrix invMat = initMat.Inverse();

			INVinitMats[j]._11 = invMat[0][0];
			INVinitMats[j]._12 = invMat[0][1];
			INVinitMats[j]._13 = invMat[0][2];
			INVinitMats[j]._14 = invMat[0][3];
			INVinitMats[j]._21 = invMat[1][0];
			INVinitMats[j]._22 = invMat[1][1];
			INVinitMats[j]._23 = invMat[1][2];
			INVinitMats[j]._24 = invMat[1][3];
			INVinitMats[j]._31 = invMat[2][0];
			INVinitMats[j]._32 = invMat[2][1];
			INVinitMats[j]._33 = invMat[2][2];
			INVinitMats[j]._34 = invMat[2][3];
			INVinitMats[j]._41 = invMat[3][0];
			INVinitMats[j]._42 = invMat[3][1];
			INVinitMats[j]._43 = invMat[3][2];
			INVinitMats[j]._44 = invMat[3][3];

		}
	}
	///////////////////////////////////////////////////

	////フレームごとにボーンの行列を取得
	//時間取得　1フレーム分の時間を設定(1秒60フレーム)

	FrameMat = new D3DXMATRIX * [FrameNum];

	FbxNode* meshNode = node;
	FbxVector4 t0 = meshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r0 = meshNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s0 = meshNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);
	for (int i = 0; i < SkinNumber; ++i) {// スキンごとにループ
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
		BoneNumber = skin->GetClusterCount();

		for (int f = 0; f < FrameNum; f++) {
			FrameMat[f] = new D3DXMATRIX[BoneNumber];
		}

		for (int j = 0; j < FrameNum; ++j) {// フレームごとにループ
			for (int k = 0; k < BoneNumber; ++k) {// ボーンごとにループ
				FbxCluster* cluster = skin->GetCluster(k);
				FbxAMatrix mat;
				FbxTime time = startTime + period * j;
				FbxMatrix globalPosition = meshNode->EvaluateGlobalTransform(time);
				FbxMatrix vertexTransformMatrix;
				FbxAMatrix referenceGlobalInitPosition;
				FbxAMatrix clusterGlobalInitPosition;
				FbxMatrix clusterGlobalCurrentPosition;
				FbxMatrix clusterRelativeInitPosition;
				FbxMatrix clusterRelativeCurrentPositionInverse;
				cluster->GetTransformMatrix(referenceGlobalInitPosition);
				referenceGlobalInitPosition *= geometryOffset;
				cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
				clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(time);
				clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;
				clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;
				vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;

				FrameMat[j][k]._11 = vertexTransformMatrix[0][0];
				FrameMat[j][k]._12 = vertexTransformMatrix[0][1];
				FrameMat[j][k]._13 = vertexTransformMatrix[0][2];
				FrameMat[j][k]._14 = vertexTransformMatrix[0][3];
				FrameMat[j][k]._21 = vertexTransformMatrix[1][0];
				FrameMat[j][k]._22 = vertexTransformMatrix[1][1];
				FrameMat[j][k]._23 = vertexTransformMatrix[1][2];
				FrameMat[j][k]._24 = vertexTransformMatrix[1][3];
				FrameMat[j][k]._31 = vertexTransformMatrix[2][0];
				FrameMat[j][k]._32 = vertexTransformMatrix[2][1];
				FrameMat[j][k]._33 = vertexTransformMatrix[2][2];
				FrameMat[j][k]._34 = vertexTransformMatrix[2][3];
				FrameMat[j][k]._41 = vertexTransformMatrix[3][0];
				FrameMat[j][k]._42 = vertexTransformMatrix[3][1];
				FrameMat[j][k]._43 = vertexTransformMatrix[3][2];
				FrameMat[j][k]._44 = vertexTransformMatrix[3][3];
			}
		}
	}


	//スキンメッシュ用頂点取り出し//////////////////////////////////////////////////////////////////
	double** WEIGHT = new double* [PositionNumber];
	for (int i = 0; i < PositionNumber; i++) {
		WEIGHT[i] = new double[3];
	}
	for (int i = 0; i < PositionNumber; i++) {
		for (int k = 0; k < 3; k++) {
			WEIGHT[i][k] = 0;
		}
	}

	double** BONEID = new double* [PositionNumber];
	for (int i = 0; i < PositionNumber; i++) {
		BONEID[i] = new double[4];
	}
	for (int i = 0; i < PositionNumber; i++) {
		for (int k = 0; k < 4; k++) {
			BONEID[i][k] = 0;
		}
	}

	// スキン(アニメーションのこと？)の数を取得
	for (int i = 0; i < SkinNumber; ++i) {
		// i番目のスキンを取得
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));

		// クラスタ(ボーンのこと？)の数を取得
		int clusterNum = skin->GetClusterCount();

		for (int fff = 0; fff < PositionNumber; ++fff) {
			for (int j = 0; j < clusterNum; ++j) {
				// j番目のクラスタ(ボーン)を取得
				FbxCluster* cluster = skin->GetCluster(j);

				int	pointNum = cluster->GetControlPointIndicesCount();//影響する頂点数
				int* pointAry = cluster->GetControlPointIndices();    //影響する頂点インデックス
				double* weightAry = cluster->GetControlPointWeights();

				for (int k = 0; k < pointNum; ++k) {
					// 頂点インデックスとウェイトを取得
					int   index = pointAry[k];
					float weight = (float)weightAry[k];
					if (fff == index) {
						// std::cout << j << "番目のボーン" << weight << std::endl;

						 //WEIGHT補填
						if (WEIGHT[fff][0] == 0) {
							WEIGHT[fff][0] = weight;
						}
						else if (WEIGHT[fff][1] == 0) {
							WEIGHT[fff][1] = weight;
						}
						else if (WEIGHT[fff][2] == 0) {
							WEIGHT[fff][2] = weight;
						}

						//BONE補填
						if (BONEID[fff][0] == 0) {
							BONEID[fff][0] = j;
						}
						else if (BONEID[fff][1] == 0) {
							BONEID[fff][1] = j;
						}
						else if (BONEID[fff][2] == 0) {
							BONEID[fff][2] = j;
						}
						else if (BONEID[fff][3] == 0) {
							BONEID[fff][3] = j;
						}
					}
				}
			}
		}
	}


	
	////トライアングルリストの作成
	MyVertex = new CUSTOMVERTEX[IndexNumber];
	for (int i = 0; i < IndexNumber; ++i)
	{
		MyVertex[i] = { D3DXVECTOR3(position[index[i]][0],position[index[i]][1],position[index[i]][2]),
						D3DXVECTOR3(WEIGHT[index[i]][0],WEIGHT[index[i]][1],WEIGHT[index[i]][2]),
						{(unsigned char)BONEID[index[i]][0],(unsigned char)BONEID[index[i]][1],(unsigned char)BONEID[index[i]][2],(unsigned char)BONEID[index[i]][3]} };
	}
}

void ExplorerNode(FbxNode* node)
{
	//DisplayMesh(node)に　nodeのタイプがMeshなら　渡す
	FbxNodeAttribute::EType lAttributeType = node->GetNodeAttribute()->GetAttributeType();
	switch (lAttributeType)
	{
	default:break;

	case FbxNodeAttribute::eMesh:
		GetMeshInfo(node);
		break;
	}

	//さらに子供を渡していく
	/*for (int i = 0; i < node->GetChildCount(); i++)
	{
		ExplorerNode(node->GetChild(i));
	}*/
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


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//fbx開始//////////////////////////////////////////////////////////////////////////
	FbxManager* manager = FbxManager::Create();

	//fbx出力設定
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	//fbxを取り込む(失敗処理も含む)
	const char* filename = ImportFileName;
	FbxImporter* importer = FbxImporter::Create(manager, "");
	if (!importer->Initialize(filename, -1, manager->GetIOSettings())) {
		OutputDebugStringA("Erro!!\n");
		exit(-1);
	}

	//sceneにfbxの情報を入れる
	FbxScene* scene = FbxScene::Create(manager, "scene");
	importer->Import(scene);

	//時間取得　1フレーム分の時間を設定(1秒60フレーム)
	period.SetTime(0, 0, 0, 1, 0, FbxTime::eFrames60);
	int animaStackCount = importer->GetAnimStackCount();//アニメの数
	FbxTakeInfo* takeInfo = importer->GetTakeInfo(0); //どのアニメーションを取るか
	startTime = takeInfo->mLocalTimeSpan.GetStart();
	endTime = takeInfo->mLocalTimeSpan.GetStop();
	FrameNum = (int)(endTime.Get() / period.Get()) - (int)(startTime.Get() / period.Get());

	importer->Destroy();

	//三角ポリゴン化
	FbxGeometryConverter geometryConverter(manager);
	geometryConverter.Triangulate(scene, true);
	
	//ノードをめぐる関数
	//DisplayContent(scene, importer);

	FbxNode* RootNode = scene->GetRootNode();
	if (RootNode) {
		for (int i = 0; i < RootNode->GetChildCount(); i++)
			ExplorerNode(RootNode->GetChild(i));
	}
	

	manager->Destroy();

	OutputDebugStringA("Hello, Debugging!!\n");
	sprintf(msgbuf, "PositionNumber is %d\n", PositionNumber); OutputDebugStringA(msgbuf);
	sprintf(msgbuf, "PolygonNumber is %d\n", PolygonNumber); OutputDebugStringA(msgbuf);
	sprintf(msgbuf, "IndexNumber is %d\n", IndexNumber); OutputDebugStringA(msgbuf);
	sprintf(msgbuf, "SkinNumber is %d\n", SkinNumber); OutputDebugStringA(msgbuf);
	sprintf(msgbuf, "BoneNumber is %d\n", BoneNumber); OutputDebugStringA(msgbuf);
	sprintf(msgbuf, "FrameNum is %d\n", FrameNum); OutputDebugStringA(msgbuf);
	for (int i = 0; i < 10; ++i) {
		sprintf(msgbuf, "MyVertex is D3DXVECTOR3(%f, %f, %f), D3DXVECTOR3(%f, %f, %f),{%hhu,%hhu,%hhu,%hhu}\n",
			MyVertex[i].coord.x, MyVertex[i].coord.y, MyVertex[i].coord.z,
			MyVertex[i].weight.x, MyVertex[i].weight.y, MyVertex[i].weight.z,
			MyVertex[i].matrixIndex[0], MyVertex[i].matrixIndex[1], MyVertex[i].matrixIndex[2], MyVertex[i].matrixIndex[3]);
		OutputDebugStringA(msgbuf);
	}
	for (int i = 0; i < 10; ++i) {
		sprintf(msgbuf, "initMats is %f, %f, %f\n",
			initMats[i]._11, initMats[i]._12, initMats[i]._13);
		OutputDebugStringA(msgbuf);
	}
	for (int i = 0; i < 10; ++i) {
		sprintf(msgbuf, "INVinitMats is %f, %f, %f\n",
			INVinitMats[i]._11, INVinitMats[i]._12, INVinitMats[i]._13);
		OutputDebugStringA(msgbuf);
	}
	for (int i = 0; i < 10; ++i) {
		sprintf(msgbuf, "FrameMat[0] is %f, %f, %f\n",
			FrameMat[0][i]._11, FrameMat[0][i]._12, FrameMat[0][i]._13);
		OutputDebugStringA(msgbuf);
	}
	for (int i = 0; i < 10; ++i) {
		sprintf(msgbuf, "FrameMat[1] is %f, %f, %f\n",
			FrameMat[0][i]._11, FrameMat[0][i]._12, FrameMat[0][i]._13);
		OutputDebugStringA(msgbuf);
	}

	//fbx end////////////////////////////////////////////////////////////////////////////////////////

	//ウィンドウ生成////////////////
	//ウィンドウの初期位置とサイズを決める
	int WindowInitialPosition_X = 100;
	int WindowInitialPosition_Y = 100;
	int WindowInitialSize_X = 1000;
	int WindowInitialSize_Y = 700;

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

	// 頂点宣言
	D3DVERTEXELEMENT9 declAry[] = {
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
		{0, 24, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
		D3DDECL_END()
	};
	IDirect3DVertexDeclaration9* decl = 0;
	g_pD3DDev->CreateVertexDeclaration(declAry, &decl);

	// シェーダのコンパイルとシェーダ作成
	ID3DXBuffer* shader, * error;
	IDirect3DVertexShader9* vertexShader;
	IDirect3DPixelShader9* pixelShader;
	HRESULT res = D3DXCompileShader(vertexShaderStr, (UINT)strlen(vertexShaderStr), 0, 0, "main", "vs_3_0", D3DXSHADER_PACKMATRIX_ROWMAJOR, &shader, &error, 0);
	if (FAILED(res)) {
		OutputDebugStringA((const char*)error->GetBufferPointer());
		return 0;
	};
	g_pD3DDev->CreateVertexShader((const DWORD*)shader->GetBufferPointer(), &vertexShader);
	shader->Release();
	res = D3DXCompileShader(pixelShaderStr, (UINT)strlen(pixelShaderStr), 0, 0, "main", "ps_3_0", D3DXSHADER_PACKMATRIX_ROWMAJOR, &shader, &error, 0);
	if (FAILED(res)) {
		OutputDebugStringA((const char*)error->GetBufferPointer());
		return 0;
	};
	g_pD3DDev->CreatePixelShader((const DWORD*)shader->GetBufferPointer(), &pixelShader);
	shader->Release();

	// 頂点バッファの作成
	IDirect3DVertexBuffer9* pVertex;
	if (FAILED(g_pD3DDev->CreateVertexBuffer(sizeof(CUSTOMVERTEX) * IndexNumber, D3DUSAGE_WRITEONLY, FVF_CUSTOM, D3DPOOL_MANAGED, &pVertex, NULL))) {
		g_pD3DDev->Release(); g_pD3D->Release();
		return -1;
	}

	D3DXMATRIX* combMat = new D3DXMATRIX[BoneNumber];//配列のそれぞれに行列を入れる
	D3DXMATRIX* combMatAry = combMat;

	//	テクスチャ
	//LPDIRECT3DTEXTURE9      g_pTexture = NULL;
	//D3DXCreateTextureFromFile(g_pD3DDev, TextureName, &g_pTexture);
	//g_pD3DDev->SetTexture(0, g_pTexture);

	//メッセージループ/////////////////
	MSG msg;
	D3DXMATRIX World;
	D3DXMATRIX View;   // ビュー変換行列
	D3DXMATRIX Persp;   // 射影変換行列
	D3DXMATRIX Rot_X;
	FLOAT Ang = 1.0f;   // 回転角度
	int NowFrame = 0;
	while (TRUE) {
		Sleep(1);
		Ang += 1;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;/*winprocから送られたWM_QUITで抜ける*/
			else DispatchMessage(&msg);
		}


		//ビデオカードのメモリへのアクセス　つまりロック関数
		void* pData;
		pVertex->Lock(0, sizeof(CUSTOMVERTEX) * IndexNumber, (void**)&pData, 0);
		memcpy(pData, MyVertex, sizeof(CUSTOMVERTEX) * IndexNumber);//頂点構造体を渡す
		pVertex->Unlock();


		if (NowFrame == FrameNum-1) { NowFrame = 0; }
		NowFrame += 1;
		for (int i = 0; i < BoneNumber; i++) {
			combMatAry[i] = /*INVinitMats[i]**/ FrameMat[NowFrame][i];//何フレーム目かの行列
		}

		//オフセット
		D3DXMatrixRotationY(&Rot_X, D3DXToRadian(Ang * 0.15f));

		D3DXMatrixIdentity(&World);                    // 単位行列化 行列同士の積のため
		D3DXMatrixScaling(&World, 1, 1, 1);      //倍数で拡大
		D3DXMatrixMultiply(&World, &World, &Rot_X);    // X軸回転後

		// ビュー変換
		D3DXVECTOR3 eye = D3DXVECTOR3(0, 0, -40.0f);
		D3DXVECTOR3 at = D3DXVECTOR3(0, 0, 0);
		D3DXVECTOR3 up = D3DXVECTOR3(0, 1, 0);
		D3DXMatrixLookAtLH(&View, &eye, &at, &up);

		// 射影変換
		D3DXMatrixPerspectiveFovLH(&Persp, D3DXToRadian(45), 640.0f / 480.0f, 1.0f, 10000.0f);


		// 行列登録
		g_pD3DDev->SetTransform(D3DTS_WORLD, &World);
		//g_pD3DDev->SetTransform(D3DTS_VIEW, &View);
		//g_pD3DDev->SetTransform(D3DTS_PROJECTION, &Persp);




		// 変数を書き込むレジスタ位置はシェーダに書いてありますよ。
		g_pD3DDev->SetVertexShader(vertexShader);
		g_pD3DDev->SetPixelShader(pixelShader);
		g_pD3DDev->SetVertexShaderConstantF(0, (const float*)&View, 4);
		g_pD3DDev->SetVertexShaderConstantF(4, (const float*)&Persp, 4);
		g_pD3DDev->SetVertexShaderConstantF(8, (const float*)&World, 4);
		g_pD3DDev->SetVertexShaderConstantF(12, (const float*)combMat, 4 * BoneNumber);
		// ワイヤーフレーム描画でカリング無しで
		//g_pD3DDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		g_pD3DDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		g_pD3DDev->SetTransform(D3DTS_WORLD, &World);

		//direct3d描画開始
		g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 250, 250), 1.0f, 0);//バッファのクリア
		g_pD3DDev->BeginScene();//描画開始

		g_pD3DDev->SetVertexDeclaration(decl);

		// ライトオフ
		g_pD3DDev->SetRenderState(D3DRS_LIGHTING, FALSE);

		// 描画
		g_pD3DDev->SetStreamSource(0, pVertex, 0, sizeof(CUSTOMVERTEX));
		g_pD3DDev->SetFVF(FVF_CUSTOM);
		g_pD3DDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, PolygonNumber);

		//direct3d描画終了
		g_pD3DDev->EndScene();//描画終了
		g_pD3DDev->Present(NULL, NULL, NULL, NULL);//フロントバッファとバックバッファを交換
	}
	return (0);
}