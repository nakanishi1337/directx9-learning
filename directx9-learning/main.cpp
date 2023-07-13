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
char ImportFileName[] = "Res/panda_nomalized_vertex900.fbx";
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
int FrameNum;
CUSTOMVERTEX* MyVertex = new CUSTOMVERTEX[IndexNumber];
D3DXMATRIX* initMats = new D3DXMATRIX[BoneNumber];
D3DXMATRIX* INVinitMats = new D3DXMATRIX[BoneNumber];
D3DXMATRIX** FrameMat = new D3DXMATRIX * [FrameNum];

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
	//for (int i = 0; i < SkinNumber; ++i) {
	//	FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
	//	int clusterNum = skin->GetClusterCount();
	//	for (int j = 0; j < clusterNum; ++j) {
	//		FbxCluster* cluster = skin->GetCluster(j);
	//		FbxAMatrix initMat;
	//		&cluster->GetTransformLinkMatrix(initMat);

	//		initMats[j]._11 = initMat[0][0];
	//		initMats[j]._12 = initMat[0][1];
	//		initMats[j]._13 = initMat[0][2];
	//		initMats[j]._14 = initMat[0][3];
	//		initMats[j]._21 = initMat[1][0];
	//		initMats[j]._22 = initMat[1][1];
	//		initMats[j]._23 = initMat[1][2];
	//		initMats[j]._24 = initMat[1][3];
	//		initMats[j]._31 = initMat[2][0];
	//		initMats[j]._32 = initMat[2][1];
	//		initMats[j]._33 = initMat[2][2];
	//		initMats[j]._34 = initMat[2][3];
	//		initMats[j]._41 = initMat[3][0];
	//		initMats[j]._42 = initMat[3][1];
	//		initMats[j]._43 = initMat[3][2];
	//		initMats[j]._44 = initMat[3][3];
	//	}
	//}
	///////////////////////////////////////////////////



	//逆初期行列取り出し/////////////////////////////////
	//for (int i = 0; i < SkinNumber; ++i) {
	//	FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
	//	int clusterNum = skin->GetClusterCount();
	//	for (int j = 0; j < clusterNum; ++j) {
	//		FbxCluster* cluster = skin->GetCluster(j);
	//		FbxAMatrix initMat;
	//		&cluster->GetTransformLinkMatrix(initMat);
	//		FbxAMatrix invMat = initMat.Inverse();

	//		INVinitMats[j]._11 = invMat[0][0];
	//		INVinitMats[j]._12 = invMat[0][1];
	//		INVinitMats[j]._13 = invMat[0][2];
	//		INVinitMats[j]._14 = invMat[0][3];
	//		INVinitMats[j]._21 = invMat[1][0];
	//		INVinitMats[j]._22 = invMat[1][1];
	//		INVinitMats[j]._23 = invMat[1][2];
	//		INVinitMats[j]._24 = invMat[1][3];
	//		INVinitMats[j]._31 = invMat[2][0];
	//		INVinitMats[j]._32 = invMat[2][1];
	//		INVinitMats[j]._33 = invMat[2][2];
	//		INVinitMats[j]._34 = invMat[2][3];
	//		INVinitMats[j]._41 = invMat[3][0];
	//		INVinitMats[j]._42 = invMat[3][1];
	//		INVinitMats[j]._43 = invMat[3][2];
	//		INVinitMats[j]._44 = invMat[3][3];

	//	}
	//}
	///////////////////////////////////////////////////




	////フレームごとにボーンの行列を取得
	//時間取得　1フレーム分の時間を設定(1秒60フレーム)
	//FbxTime period;
	//period.SetTime(0, 0, 0, 1, 0, FbxTime::eFrames60);

	//int animaStackCount = importer->GetAnimStackCount();//3dビューアで見れるアニメの数
	//FbxTakeInfo* takeInfo = importer->GetTakeInfo(0); //１行上の内どのアニメーションを取るか
	//FbxTime startTime = takeInfo->mLocalTimeSpan.GetStart();
	//FbxTime endTime = takeInfo->mLocalTimeSpan.GetStop();
	//int startFrame = (int)(startTime.Get() / period.Get());
	//int endFrame = (int)(endTime.Get() / period.Get());
	//int FrameNum = endFrame - startFrame;


	//FbxNode* meshNode = node;
	//FbxVector4 t0 = meshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	//FbxVector4 r0 = meshNode->GetGeometricRotation(FbxNode::eSourcePivot);
	//FbxVector4 s0 = meshNode->GetGeometricScaling(FbxNode::eSourcePivot);
	//FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);
	//for (int i = 0; i < SkinNumber; ++i) {// スキンごとにループ
	//	FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));
	//	BoneNumber = skin->GetClusterCount();

	//	for (int f = 0; f < FrameNum; f++) {
	//		FrameMat[f] = new D3DXMATRIX[BoneNumber];
	//	}

	//	for (int j = 0; j < FrameNum; ++j) {// フレームごとにループ
	//		for (int k = 0; k < BoneNumber; ++k) {// ボーンごとにループ
	//			FbxCluster* cluster = skin->GetCluster(k);
	//			FbxAMatrix mat;
	//			FbxTime time = startTime + period * j;
	//			FbxMatrix globalPosition = meshNode->EvaluateGlobalTransform(time);
	//			FbxMatrix vertexTransformMatrix;
	//			FbxAMatrix referenceGlobalInitPosition;
	//			FbxAMatrix clusterGlobalInitPosition;
	//			FbxMatrix clusterGlobalCurrentPosition;
	//			FbxMatrix clusterRelativeInitPosition;
	//			FbxMatrix clusterRelativeCurrentPositionInverse;
	//			cluster->GetTransformMatrix(referenceGlobalInitPosition);
	//			referenceGlobalInitPosition *= geometryOffset;
	//			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
	//			clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(time);
	//			clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;
	//			clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;
	//			vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;

	//			FrameMat[j][k]._11 = vertexTransformMatrix[0][0];
	//			FrameMat[j][k]._12 = vertexTransformMatrix[0][1];
	//			FrameMat[j][k]._13 = vertexTransformMatrix[0][2];
	//			FrameMat[j][k]._14 = vertexTransformMatrix[0][3];
	//			FrameMat[j][k]._21 = vertexTransformMatrix[1][0];
	//			FrameMat[j][k]._22 = vertexTransformMatrix[1][1];
	//			FrameMat[j][k]._23 = vertexTransformMatrix[1][2];
	//			FrameMat[j][k]._24 = vertexTransformMatrix[1][3];
	//			FrameMat[j][k]._31 = vertexTransformMatrix[2][0];
	//			FrameMat[j][k]._32 = vertexTransformMatrix[2][1];
	//			FrameMat[j][k]._33 = vertexTransformMatrix[2][2];
	//			FrameMat[j][k]._34 = vertexTransformMatrix[2][3];
	//			FrameMat[j][k]._41 = vertexTransformMatrix[3][0];
	//			FrameMat[j][k]._42 = vertexTransformMatrix[3][1];
	//			FrameMat[j][k]._43 = vertexTransformMatrix[3][2];
	//			FrameMat[j][k]._44 = vertexTransformMatrix[3][3];
	//		}
	//	}
	//}


	//スキンメッシュ用頂点情報取り出し//////////////////////////////////////////////////////////////////
	//double** WEIGHT = new double* [PositionNumber];
	//for (int i = 0; i < PositionNumber; i++) {
	//	WEIGHT[i] = new double[3];
	//}
	//for (int i = 0; i < PositionNumber; i++) {
	//	for (int k = 0; k < 3; k++) {
	//		WEIGHT[i][k] = 0;
	//	}
	//}

	//double** BONEID = new double* [PositionNumber];
	//for (int i = 0; i < PositionNumber; i++) {
	//	BONEID[i] = new double[4];
	//}
	//for (int i = 0; i < PositionNumber; i++) {
	//	for (int k = 0; k < 4; k++) {
	//		BONEID[i][k] = 0;
	//	}
	//}

	////トライアングルリストの作成
	//for (int i = 0; i < IndexNumber; ++i)
	//{
	//	MyVertex[i] = { D3DXVECTOR3(position[index[i]][0],position[index[i]][1],position[index[i]][2]),
	//					D3DXVECTOR3(WEIGHT[index[i]][0],WEIGHT[index[i]][1],WEIGHT[index[i]][2]),
	//					{(unsigned char)BONEID[index[i]][0],(unsigned char)BONEID[index[i]][1],(unsigned char)BONEID[index[i]][2],(unsigned char)BONEID[index[i]][3]} };
	//}
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
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		ExplorerNode(node->GetChild(i));
	}
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


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow){


	//fbx開始
	FbxManager* manager = FbxManager::Create();

	//fbx出力設定
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	//fbxを取り込む(失敗処理も含む)
	const char* filename = ImportFileName;
	FbxImporter* importer = FbxImporter::Create(manager, "");
	if (!importer->Initialize(filename, -1, manager->GetIOSettings())) {
		exit(-1);
	}


	//sceneにfbxの情報を入れる
	FbxScene* scene = FbxScene::Create(manager, "scene");
	importer->Import(scene);

	//時間取得　1フレーム分の時間を設定(1秒60フレーム)
	FbxTime period;
	period.SetTime(0, 0, 0, 1, 0, FbxTime::eFrames60);
	int animaStackCount = importer->GetAnimStackCount();//3dビューアで見れるアニメの数
	FbxTakeInfo* takeInfo = importer->GetTakeInfo(0); //１行上の内どのアニメーションを取るか
	FbxTime startTime = takeInfo->mLocalTimeSpan.GetStart();
	FbxTime endTime = takeInfo->mLocalTimeSpan.GetStop();
	int startFrame = (int)(startTime.Get() / period.Get());
	int endFrame = (int)(endTime.Get() / period.Get());
	int FrameNum = endFrame - startFrame;

	importer->Destroy();

	//三角ポリゴン化
	//FbxGeometryConverter geometryConverter(manager);
	//geometryConverter.Triangulate(scene, true);
	
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
	//sprintf(msgbuf, "SkinNumber is %d\n", SkinNumber); OutputDebugStringA(msgbuf);
	//sprintf(msgbuf, "BoneNumber is %d\n", BoneNumber); OutputDebugStringA(msgbuf);
	sprintf(msgbuf, "FrameNum is %d\n", FrameNum); OutputDebugStringA(msgbuf);
	//for (int i = 0; i < 10; ++i) {
	//	sprintf(msgbuf, "MyVertex is D3DXVECTOR3(%f, %f, %f), D3DXVECTOR3(%f, %f, %f),{%hhu,%hhu,%hhu,%hhu}\n",
	//		MyVertex[i].coord.x, MyVertex[i].coord.y, MyVertex[i].coord.z,
	//		MyVertex[i].weight.x, MyVertex[i].weight.y, MyVertex[i].weight.z, 
	//		MyVertex[i].matrixIndex[0], MyVertex[i].matrixIndex[1], MyVertex[i].matrixIndex[2], MyVertex[i].matrixIndex[3]);
	//	OutputDebugStringA(msgbuf);
	//}
	//for (int i = 0; i < 10; ++i) {
	//	sprintf(msgbuf, "initMats is %f, %f, %f\n",
	//		initMats[i]._11, initMats[i]._12, initMats[i]._13);
	//	OutputDebugStringA(msgbuf);
	//}
	//for (int i = 0; i < 10; ++i) {
	//	sprintf(msgbuf, "INVinitMats is %f, %f, %f\n",
	//		INVinitMats[i]._11, INVinitMats[i]._12, INVinitMats[i]._13);
	//	OutputDebugStringA(msgbuf);
	//}
	//for (int i = 0; i < 10; ++i) {
 //		sprintf(msgbuf, "FrameMat[0] is %f, %f, %f\n",
	//		FrameMat[0][i]._11, FrameMat[0][i]._12, FrameMat[0][i]._13);
	//	OutputDebugStringA(msgbuf);
	//}
	//for (int i = 0; i < 10; ++i) {
	//	sprintf(msgbuf, "FrameMat[1] is %f, %f, %f\n",
	//		FrameMat[0][i]._11, FrameMat[0][i]._12, FrameMat[0][i]._13);
	//	OutputDebugStringA(msgbuf);
	//}



	return (0);
}