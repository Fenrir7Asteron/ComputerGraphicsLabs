#include "VertexPositionColor.h"
#include "GameFramework.h"


ID3D11InputLayout* VertexPositionColor::GetLayout(ID3DBlob* signature)
{
	// Layout from VertexShader input signature
	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};
	ID3D11InputLayout* geometryLayout;
	GameFramework::Instance->device->CreateInputLayout(inputElements, 2, signature->GetBufferPointer(), signature->GetBufferSize(), &geometryLayout);
	
	return geometryLayout;
}

//void VertexPositionColor::Convert(MeshVertex v)
//{
//	Position = new Vector4(v.Position, 1.0f);
//	Color = v.Color0.ToColor4();
//}