﻿#include "framework.h"
//#pragma comment(lib, "winAPIEngine.lib")


#include "CDxEngine.h"

#include "DDSTextureLoader.h"

#include "ryubuffer.h"
#include "CMesh.h"

#include "CMtlTexture.h"

/*
    현재까지 발견된 문제점

    i) 텍스쳐가 제대로 매핑되지 않는다

        기존 코드는 정점의 위치 인덱스 정보 따로, uv좌표 인덱스 정보 따로, normal 인덱스 정보 따로 담고,
        최소한으로만 기록된 정점 위치 데이터를 기준으로 해서
        정점 데이터를 구축했다.
        하지만, 그것은 맞지 않다.

        왜냐하면,
        vertex(위치), uv, normal 이 세 가지 데이터의 조합이
        하나의 정점을 의미하는 것이다.

        그러므로, 이 세 조합까지 고려하여 index를 다시 계산하는 것이 필요하다.

        예를 들면
        위치, uv, normal을 각각 따로 담아 생각한다면
        4/1/1과 4/2/1은 크게 다르지 않다.
        정점의 위치 4
        uv 1, 2
        normal 1

        이지만

        만약 이 세 가지의 조합을 하나의 정점의 인덱스로 본다면
        이것은 그냥 4/1/1과 4/2/1 이렇게 두 가지의 경우가 나오는 것이다.

    ii) 좌우가 뒤집어져 있다
*/





//선언 
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);


class CRyuEngine : public CDxEngine
{   
    CMesh* mpMesh = nullptr;
    CMtlTexture* mpMtlTexture = nullptr;

    
    //상수 버퍼( 변환행렬 용 )
    ID3D11Buffer* mpCBTransform = nullptr;

    //응용프로그램 수준에서 다룰 변환행렬들
    XMMATRIX mMatWorld_0;            //월드변환 행렬 <--- 첫번째 정육면체 용
    XMMATRIX mMatWorld_1;       //월드변환 행렬 <--- 두번째 정육면체 용
    XMMATRIX mMatView;              //뷰변환 행렬
    XMMATRIX mMatProjection;       //투영변환 행렬 

  
    //ID3D11ShaderResourceView* mpTexture = nullptr;
  
public:
    CRyuEngine() {};
    virtual ~CRyuEngine() {};

private:
    CRyuEngine(const CRyuEngine& t) = delete;
    CRyuEngine& operator=(const CRyuEngine& t) = delete;

public:
    virtual void OnCreate() override
    {
        CDxEngine::OnCreate();




        mpMtlTexture = new CMtlTexture();
        mpMtlTexture->Create(mpD3D->GetD3DDevice());
        mpMtlTexture->SetEngine(this);


        mpMesh = new CMesh();
        //mpMesh->Create(mpD3D->GetD3DDevice(), L"resources/testUV256by256.dds");
        mpMesh->Create(mpD3D->GetD3DDevice(), L"resources/slimeby.dds");
        mpMesh->SetEngine(this);

        

        //텍스처 이미지 데이터를 파일에서 로드하여  mpTexture에 담음
        //CreateDDSTextureFromFile(mpD3D->GetD3DDevice(), L"resources/testUV256by256.dds", nullptr, &mpTexture);



        //상수 버퍼 생성
        //주설명
        D3D11_BUFFER_DESC tBd = {};
        tBd.Usage = D3D11_USAGE_DEFAULT;            //기본용도다 
        tBd.ByteWidth = sizeof(CBTransform) * 1;    //해당 상수버퍼의 메모리 크기를 알린다
        tBd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//constant buffer용도다
        tBd.CPUAccessFlags = 0;     //CPU의 접근은 불허한다

        //부설명 은 필요없다. 
        // UpdateSubResources을 이용하여 매 프레임마다 데이터의 값을 갱신할 것이기 때문이다.
        // 
        //D3D11_SUBRESOURCE_DATA InitData = {};
        //InitData.pSysMem = tVertices;       //해당 시스템 메모리의 데이터의 내용을 넘겨준다

        mpD3D->GetD3DDevice()->CreateBuffer(&tBd, nullptr, &mpCBTransform);



        //변환행렬들 설정 
        //월드변환 행렬
        mMatWorld_0 = XMMatrixIdentity();     //단위행렬로 초기화

        mMatWorld_1 = XMMatrixIdentity();     //단위행렬로 초기화

        //뷰변환 행렬
        mMatView = XMMatrixIdentity();

        //XMVECTOR tEye = XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f); //카메라의 위치
        XMVECTOR tEye = XMVectorSet(0.0f, 6.0f, 0.0f, 1.0f); //카메라의 위치
        XMVECTOR tAt = XMVectorSet(0.0f, 5.0f, 0.0f, 1.0f); //바라보는 응시점위치
        XMVECTOR tUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //월드에 상방벡터

        //뷰변환 행렬을 구한다. ~LH Left Hand 왼손좌표계
        mMatView = XMMatrixLookAtLH(tEye, tAt, tUp);


        //투영변환 행렬 
        mMatProjection = XMMatrixIdentity();

        RECT rc = { 0 };
        GetClientRect(this->mhWnd, &rc);    //window api
        unsigned int tWidth = rc.right - rc.left;
        unsigned int tHeight = rc.bottom - rc.top;//윈도우 좌표계 상에서 계산

        mMatProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (float)tWidth / (float)tHeight, 0.01f, 100.0f);
    }
    virtual void OnDestroy() override
    {        
        if (mpMtlTexture)
        {
            mpMtlTexture->Destroy();

            delete mpMtlTexture;
            mpMtlTexture = nullptr;
        }

        if (mpMesh)
        {
            mpMesh->Destroy();

            delete mpMesh;
            mpMesh = nullptr;
        }

    

        CDxEngine::OnDestroy();
    }
    virtual void OnUpdate(float tDeltaTime) override
    {
        CDxEngine::OnUpdate(tDeltaTime);

        //각도 증가
        static float tTheta = 0.0f;
        tTheta = tTheta + 2.5f * tDeltaTime;
        //y축을 회전축으로 회전하는 회전변환행렬
        mMatWorld_0 = XMMatrixRotationY(tTheta);


        XMMATRIX tRotation = XMMatrixRotationY(tTheta * 0.3f);
        //XMMATRIX tTranslation = XMMatrixTranslation(3.0f, 0.0f, 0.0f);  //x축으로 3만큼 이동
        XMMATRIX tTranslation = XMMatrixTranslation(0.0f, 0.0f, 20.0f);  //z축으로 20만큼 이동

        //mMatWorld_1 = XMMatrixRotationY(tTheta*3.0f);

        //DirectX의 응용프로그램 수준의 코드는 '행벡터'를 기준으로 한다.( 그러므로 행렬의 앞에 벡터가 곱해지는 것이다 )
        //mMatWorld_1 = tRotation * tTranslation;     //행벡터 * 회전변환행렬*이동변환행렬
        mMatWorld_0 = tRotation * tTranslation;     //행벡터 * 회전변환행렬*이동변환행렬

        //회전하고 나서 그리고나서 이동한다. 회전의 축은 월드좌표계 상의 y축이다. (정점단위로 적용된다)

        //mMatWorld_1 = tTranslation* tRotation;     //행벡터 *이동변환행렬* 회전변환행렬

        //이동하고 나서 그리고 회전한다. 회전의 축은 월드좌표계 상의 y축이다. (정점단위로 적용된다)


        //빛 정보 생성
        XMFLOAT4 tLightDir = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);//xyzw 빛의 방향은 전방(z+)
        XMFLOAT4 tLightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//rgba


        // XMFLOAT4 --> XMVECTOR --> XMFLOAT4
        //XMMATRIX tRotateLight = XMMatrixRotationX(0.5f * tTheta);//x축 회전축으로 회전변환행렬
        //XMVECTOR tVectorLight = XMLoadFloat4(&tLightDir);

        //tVectorLight = XMVector3Transform(tVectorLight, tRotateLight);

        //XMStoreFloat4(&tLightDir, tVectorLight);

        //XMVECTOR는  GPU연산에 최적화되어 만들어진 타입이다.
        //그래서 컴파일 후에 16바이트 단위로 정렬되도록 만들어진다.
        //그런데 이 정렬은 지역변수, 매개변수, 전역변수 등에만 적용되고 멤버변수에는 적용되지 않는다고 한다.
        // ------> 그래서, 응용프로그램 코드에 멤버변수는 주로 XMFLOAT계열이 사용되었다. <----- 
        /*
            //XMLoadFloat4:
                                        XMFLOAT4 ---> XMVECTOR

            //XMStoreFloat4:
                                        XMVECTOR ----> XMFLOAT4
        */





        this->Clear(Colors::DarkOrchid);

      
        mpD3D->GetImmediateDevice()->VSSetConstantBuffers(0, 1, &mpCBTransform);
        mpD3D->GetImmediateDevice()->PSSetConstantBuffers(0, 1, &mpCBTransform);

        //첫번째 육면체 랜더링==================================
        
        //directX의 응용프로그램 에서 시스템 스택에 마련된 상수버퍼 데이터 
        // 
        //directX 응용프로그램에서는 행렬연산을 '행벡터'를 기준으로 한다
        //hlsl셰이더 프로그램에서는 행렬연산을 '열벡터'를 기준으로 한다
        CBTransform tCB;
        tCB.mWorld = XMMatrixTranspose(mMatWorld_0);
        tCB.mView = XMMatrixTranspose(mMatView);
        tCB.mProjection = XMMatrixTranspose(mMatProjection);
        //빛 정보를 상수버퍼를 매개로 셰이더 측에 전달하기 위해 설정
        tCB.mLightDir = tLightDir;
        tCB.mLightColor = tLightColor;
        

        //셰이더 측에 상수버퍼에 값 갱신
        mpD3D->GetImmediateDevice()->UpdateSubresource(mpCBTransform, 0, nullptr, &tCB, 0, 0);

    


        mpMesh->Render();
        mpMtlTexture->Render(mpMesh->GetCountIndex(), mpMesh->GetTexture());


                
        this->Present();
    }
};




int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    
    CRyuEngine tEngine;
    tEngine.Create(hInstance, nCmdShow);

    MSG msg = { 0 };
    msg = tEngine.Run();

    return (int)msg.wParam;
}


//정의
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}