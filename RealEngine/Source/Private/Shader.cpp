#include"Source/Public/Shader.h"

FShaderMapResourceCode* FShaderMapBase::GetResourceCode()
{
	if (!ResourceCode.IsValid())
	{
		ResourceCode = new FShaderMapResourceCode;
	}
    return ResourceCode;
}
