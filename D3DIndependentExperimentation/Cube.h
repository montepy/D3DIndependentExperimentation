#pragma once


#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <WICTextureLoader.h>
#include <d3d11_2.h>
struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
};


using namespace DirectX;
class Cube
{
private:
	Vertex vertices[24];
	XMMATRIX RotX, RotZ, RotY,trans;
	double scale;
	XMFLOAT3 loc;
	short indices[36];
	ID3D11DeviceContext* d3dDevCon;
	ID3D11Device*d3dDev;
	ID3D11ShaderResourceView*CubeTexture;
	XMMATRIX cubeWorld;

public:
	Cube();
	Cube(double,double,double, double, XMFLOAT3,ID3D11DeviceContext*,ID3D11Device*,std::string );
	~Cube();
	Vertex* getVertices();
	void Rotate(double,double,double);
	void Move(double,double,double); 
	void Draw();
	void Update();
	XMMATRIX getWorld();
	short* getIndices();
};

