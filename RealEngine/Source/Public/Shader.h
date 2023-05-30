
#include"Core/Public/template_test.h"
#include"Core/Public/SmartPointer.h"
class DLLEXPORT FShaderMapResourceCode : public FThreadCountObject
{
public:


private:

};




class DLLEXPORT FShaderMapBase
{
public:
	virtual ~FShaderMapBase()
	{

	}

	FShaderMapResourceCode* GetResourceCode();


private:
	TRefCountPtr<FShaderMapResourceCode> ResourceCode;
};


template <typename ShaderType,typename PointerTableType>
class TShaderRefBase
{
public:
	TShaderRefBase();
	~TShaderRefBase();

private:
	ShaderType* ShaderContent;
	const FShaderMapBase* ShaderMap;
};
