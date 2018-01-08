#include "stdafx.h"
#include "Cube.h"

using namespace DirectX;

Cube::Cube()
{
	RotX = XMMatrixRotationAxis(XMVectorSet(1.0, 0.0, 0.0, 0.0), 0);
	RotY = XMMatrixRotationAxis(XMVectorSet(0.0, 1.0, 0.0, 0.0), 0);
	RotZ = XMMatrixRotationAxis(XMVectorSet(0.0, 0.0, 1.0, 0.0), 0);
	scale = 1;
	loc = XMFLOAT3(0.0, 0.0, 0.0);

	Vertex v[] = 
	{ //remember that structs do not have constructors unless defined!
			// Front Face
			{ { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ -1.0f, -1.0f, -1.0f } },
			{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f },{ -1.0f,  1.0f, -1.0f } },
			{ { 1.0f,  1.0f, -1.0f },{ 1.0f, 0.0f } ,{ 1.0f,  1.0f, -1.0f } },
			{ { 1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f } ,{ 1.0f, -1.0f, -1.0f } },

			// Back Face
			{ { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f } ,{ -1.0f, -1.0f, 1.0f } },
			{ { 1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f } ,{ 1.0f, -1.0f, 1.0f } },
			{ { 1.0f,  1.0f, 1.0f },{ 0.0f, 0.0f } ,{ 1.0f,  1.0f, 1.0f } },
			{ { -1.0f,  1.0f, 1.0f },{ 1.0f, 0.0f },{ -1.0f,  1.0f, 1.0f } },

			// Top Face
			{ { -1.0f, 1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, 1.0f, -1.0f } },
			{ { -1.0f, 1.0f,  1.0f },{ 0.0f, 1.0f },{ -1.0f, 1.0f,  1.0f } },
			{ { 1.0f, 1.0f,  1.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f,  1.0f } },
			{ { 1.0f, 1.0f, -1.0f },{ 1.0f, 0.0f },{ 1.0f, 1.0f, -1.0f } },

			// Bottom Face
			{ { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, -1.0f, -1.0f } },
			{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, -1.0f, -1.0f } },
			{ { 1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f },{ 1.0f, -1.0f,  1.0f } },
			{ { -1.0f, -1.0f,  1.0f },{ 1.0f, 0.0f },{ -1.0f, -1.0f,  1.0f } },

			// Left Face
			{ { -1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f },{ -1.0f, -1.0f,  1.0f } },
			{ { -1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f },{ -1.0f,  1.0f,  1.0f } },
			{ { -1.0f,  1.0f, -1.0f },{ 1.0f, 0.0f },{ -1.0f,  1.0f, -1.0f } },
			{ { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, -1.0f, -1.0f } },

			// Right Face
			{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, -1.0f, -1.0f } },
			{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f,  1.0f, -1.0f } },
			{ { 1.0f,  1.0f,  1.0f },{ 1.0f, 0.0f },{ 1.0f,  1.0f,  1.0f } },
			{ { 1.0f, -1.0f,  1.0f },{ 1.0f, 1.0f },{ 1.0f, -1.0f,  1.0f } }

	};
	for (int i = 0; i < 24; i++) {
		vertices[i] = v[i];
	}
	short ind[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};
	for (int s = 0; s < 36; s++) {
		indices[s] = ind[s];
	}
}

Cube::Cube(double rotx,double roty,double rotz, double scale, XMFLOAT3 loc,ID3D11DeviceContext*devcon, ID3D11Device*dev,std::string name ) {
	RotX = XMMatrixRotationAxis(XMVectorSet(1.0, 0.0, 0.0, 0.0), rotx);
	RotY = XMMatrixRotationAxis(XMVectorSet(0.0, 1.0, 0.0, 0.0), roty);
	RotZ = XMMatrixRotationAxis(XMVectorSet(0.0, 0.0, 1.0, 0.0), rotz);	
	this->scale = scale;
	this->loc = loc;

	d3dDevCon = devcon;
	d3dDev = dev;

	CreateWICTextureFromFile(d3dDev, L"gray.jpg", NULL, &CubeTexture, 0);
	Vertex v[] =
	{ //remember that structs do not have constructors unless defined!
	  // Front Face
		{ { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ -1.0f, -1.0f, -1.0f } },
		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f },{ -1.0f,  1.0f, -1.0f } },
		{ { 1.0f,  1.0f, -1.0f },{ 1.0f, 0.0f } ,{ 1.0f,  1.0f, -1.0f } },
		{ { 1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f } ,{ 1.0f, -1.0f, -1.0f } },

		// Back Face
		{ { -1.0f, -1.0f, 1.0f },{ 1.0f, 1.0f } ,{ -1.0f, -1.0f, 1.0f } },
		{ { 1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f } ,{ 1.0f, -1.0f, 1.0f } },
		{ { 1.0f,  1.0f, 1.0f },{ 0.0f, 0.0f } ,{ 1.0f,  1.0f, 1.0f } },
		{ { -1.0f,  1.0f, 1.0f },{ 1.0f, 0.0f },{ -1.0f,  1.0f, 1.0f } },

		// Top Face
		{ { -1.0f, 1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, 1.0f, -1.0f } },
		{ { -1.0f, 1.0f,  1.0f },{ 0.0f, 1.0f },{ -1.0f, 1.0f,  1.0f } },
		{ { 1.0f, 1.0f,  1.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f,  1.0f } },
		{ { 1.0f, 1.0f, -1.0f },{ 1.0f, 0.0f },{ 1.0f, 1.0f, -1.0f } },

		// Bottom Face
		{ { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, -1.0f, -1.0f } },
		{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, -1.0f, -1.0f } },
		{ { 1.0f, -1.0f,  1.0f },{ 0.0f, 0.0f },{ 1.0f, -1.0f,  1.0f } },
		{ { -1.0f, -1.0f,  1.0f },{ 1.0f, 0.0f },{ -1.0f, -1.0f,  1.0f } },

		// Left Face
		{ { -1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f },{ -1.0f, -1.0f,  1.0f } },
		{ { -1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f },{ -1.0f,  1.0f,  1.0f } },
		{ { -1.0f,  1.0f, -1.0f },{ 1.0f, 0.0f },{ -1.0f,  1.0f, -1.0f } },
		{ { -1.0f, -1.0f, -1.0f },{ 1.0f, 1.0f },{ -1.0f, -1.0f, -1.0f } },

		// Right Face
		{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f },{ 1.0f, -1.0f, -1.0f } },
		{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f },{ 1.0f,  1.0f, -1.0f } },
		{ { 1.0f,  1.0f,  1.0f },{ 1.0f, 0.0f },{ 1.0f,  1.0f,  1.0f } },
		{ { 1.0f, -1.0f,  1.0f },{ 1.0f, 1.0f },{ 1.0f, -1.0f,  1.0f } }

	};
	for (int i = 0; i < 24; i++) {
		vertices[i] = v[i];
	}
	short ind[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};
	for (int s = 0; s < 36; s++) {
		indices[s] = ind[s];
	}
}


Cube::~Cube()
{
}

void Cube::Rotate(double rotx, double roty, double rotz) {
	RotX = XMMatrixRotationAxis(XMVectorSet(1.0, 0.0, 0.0, 0.0), rotx);
	RotY = XMMatrixRotationAxis(XMVectorSet(0.0, 1.0, 0.0, 0.0), roty);
	RotZ = XMMatrixRotationAxis(XMVectorSet(0.0, 0.0, 1.0, 0.0), rotz);
}

void Cube::Move(double x,double y, double z) {
	trans = XMMatrixTranslation(x, y, z);
}

void Cube::Update() {
	cubeWorld = XMMatrixIdentity();
	cubeWorld = RotX*RotY*RotZ*trans;
}

void Cube::Draw() {

}

XMMATRIX Cube::getWorld() {
	return cubeWorld;
}

Vertex* Cube::getVertices() {
	Vertex*out = vertices;
	return out;
}

short* Cube::getIndices() {
	short*out = indices;
	return out;
}