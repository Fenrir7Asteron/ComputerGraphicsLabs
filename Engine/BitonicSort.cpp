#include "BitonicSort.h"
#include "GameFramework.h"
#include <d3dcompiler.h>
#include "magic_enum.hpp"
#include "MacrosParser.h"

BitonicSort::BitonicSort(GameFramework* game)
{
    this->game = game;

    LoadShaders();
    CreateBuffers();
}

void BitonicSort::Dispose()
{
    paramsCB->Release();
    buffer2->Release();
    uavBuf->Release();
    srvBuf->Release();

    std::vector<ComputeFlags> flags =
    {
        ComputeFlags::BITONIC_SORT,
        ComputeFlags::TRANSPOSE,
    };

    for (auto& flag : flags)
    {
        ComputeShaders[flag]->Release();
    }
}

void BitonicSort::Sort(ID3D11UnorderedAccessView* uav, ID3D11ShaderResourceView* srv)
{
    ID3D11DeviceContext* context = game->context;
    context->ClearState();
	game->RestoreTargets();

	UINT initialCount = -1;

    context->CSSetConstantBuffers(0, 1, &paramsCB);

    // Sort the data
    // First sort the rows for the levels <= to the block size
    for (UINT level = 2; level <= BitonicBlockSize; level <<= 1)
    {
        SetConstants(level, level, MatrixWidth, MatrixHeight);

        // Sort the row data
        UINT UAVInitialCounts = 0;
        context->CSSetUnorderedAccessViews(0, 1, &uav, &UAVInitialCounts);
        context->CSSetShader(ComputeShaders[ComputeFlags::BITONIC_SORT], nullptr, 0);
        context->Dispatch(NumberOfElements / BitonicBlockSize, 1, 1);
    }

    // Then sort the rows and columns for the levels > than the block size
    // Transpose. Sort the Columns. Transpose. Sort the Rows.
    for (UINT level = (BitonicBlockSize << 1); level <= NumberOfElements; level <<= 1)
    {
        SetConstants(level / BitonicBlockSize, (level & ~NumberOfElements, MatrixWidth) / BitonicBlockSize, MatrixWidth, MatrixHeight);

        // Transpose the data from buffer 1 into buffer 2
        ID3D11ShaderResourceView* pViewNULL = nullptr;
        UINT UAVInitialCounts = 0;
        context->CSSetShaderResources(0, 1, &pViewNULL);
        context->CSSetUnorderedAccessViews(0, 1, &uavBuf, &UAVInitialCounts);
        context->CSSetShaderResources(0, 1, &srv);
        context->CSSetShader(ComputeShaders[ComputeFlags::TRANSPOSE], nullptr, 0);
        context->Dispatch(MatrixWidth / TransposeBlockSize, MatrixHeight / TransposeBlockSize, 1);

        // Sort the transposed column data
        context->CSSetShader(ComputeShaders[ComputeFlags::BITONIC_SORT], nullptr, 0);
        context->Dispatch(NumberOfElements / BitonicBlockSize, 1, 1);

        SetConstants(BitonicBlockSize, level, MatrixWidth, MatrixHeight);

        // Transpose the data from buffer 2 back into buffer 1
        context->CSSetShaderResources(0, 1, &pViewNULL);
        context->CSSetUnorderedAccessViews(0, 1, &uav, &UAVInitialCounts);
        context->CSSetShaderResources(0, 1, &srvBuf);
        context->CSSetShader(ComputeShaders[ComputeFlags::TRANSPOSE], nullptr, 0);
        context->Dispatch(MatrixHeight / TransposeBlockSize, MatrixWidth / TransposeBlockSize, 1);

        // Sort the row data
        context->CSSetShader(ComputeShaders[ComputeFlags::BITONIC_SORT], nullptr, 0);
        context->Dispatch(NumberOfElements / BitonicBlockSize, 1, 1);
    }
}

void BitonicSort::LoadShaders()
{
    ID3DBlob *errorCode;
    const LPCWSTR shaderPath = L"./Shaders/BitonicSort.hlsl";

    std::vector<ComputeFlags> flags =
    {
        ComputeFlags::BITONIC_SORT,
        ComputeFlags::TRANSPOSE,
    };

    for (auto& flag : flags)
    {
        auto macros = MacrosParser::GetMacros(flag);

        ID3DBlob* computeBC = nullptr;

        HRESULT res = D3DCompileFromFile(shaderPath,
            &macros[0],
            nullptr /*include*/,
            "CSMain",
            "cs_5_0",
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            &computeBC,
            &errorCode);

        game->CheckShaderCreationSuccess(res, errorCode, shaderPath);

        game->device->CreateComputeShader(computeBC->GetBufferPointer(), computeBC->GetBufferSize(), nullptr, &ComputeShaders[flag]);
        computeBC->Release();
    }
}

void BitonicSort::CreateBuffers()
{
    D3D11_BUFFER_DESC constBufDesc;

    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.Usage = D3D11_USAGE_DEFAULT;
    constBufDesc.CPUAccessFlags = 0;
    constBufDesc.MiscFlags = 0;
    constBufDesc.ByteWidth = sizeof(Params);

    game->device->CreateBuffer(&constBufDesc, nullptr, &paramsCB);


    D3D11_BUFFER_DESC buffer2Desc;
    buffer2Desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    buffer2Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    buffer2Desc.Usage = D3D11_USAGE_DEFAULT;
    buffer2Desc.CPUAccessFlags = 0;
    buffer2Desc.StructureByteStride = sizeof(Particle);
    buffer2Desc.ByteWidth = MaxParticlesCount * sizeof(Particle);

    game->device->CreateBuffer(&buffer2Desc, nullptr, &buffer2);

    game->device->CreateShaderResourceView(buffer2, nullptr, &srvBuf);
    game->device->CreateUnorderedAccessView(buffer2, nullptr, &uavBuf);
}

void BitonicSort::SetConstants(const UINT& level, const UINT& levelMask, const UINT& width, const UINT& height)
{
    Params constants = { level, levelMask, width, height };
    game->context->UpdateSubresource(paramsCB, 0, nullptr, &constants, 0, 0);
}
