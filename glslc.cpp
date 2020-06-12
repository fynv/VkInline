#include <vector>
#include <string>
#include <string.h>
#include <unordered_map>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

const TBuiltInResource DefaultTBuiltInResource =
{
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */0,
	/* .maxMeshOutputPrimitivesNV = */0,
	/* .maxMeshWorkGroupSizeX_NV = */0,
	/* .maxMeshWorkGroupSizeY_NV = */0,
	/* .maxMeshWorkGroupSizeZ_NV = */0,
	/* .maxTaskWorkGroupSizeX_NV = */0,
	/* .maxTaskWorkGroupSizeY_NV = */0,
	/* .maxTaskWorkGroupSizeZ_NV = */0,
	/* .maxMeshViewCountNV = */0,
	/* .maxDualSourceDrawBuffersEXT = */ 0,
	/* .limits = */ 
	{
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

class BuiltInIncluder : public glslang::TShader::Includer
{
	const std::unordered_map<std::string, const char*>* m_headers;
public:
	BuiltInIncluder(const std::unordered_map<std::string, const char*>* headers = nullptr)
	{
		m_headers = headers;
	}

	virtual IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) 
	{
		if (m_headers != nullptr)
		{
			auto iter = m_headers->find(headerName);
			if (iter != m_headers->end())
			{
				IncludeResult* result = new IncludeResult(headerName, iter->second, strlen(iter->second), nullptr);
				return result;
			}
		}
		return nullptr;
	}

	virtual void releaseInclude(IncludeResult* result) override
	{ 
		if (result!=nullptr) delete result;
	}
};

bool GLSL2SPV(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV, EShLanguage ShaderType)
{
	glslang::InitializeProcess();
	glslang::TShader Shader(ShaderType);
	Shader.setStrings(&InputCString, 1);

	int ClientInputSemanticsVersion = 110;
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_1;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_1;

	Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

	TBuiltInResource Resources;
	Resources = DefaultTBuiltInResource;
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	const int DefaultVersion = 110;

	std::string PreprocessedGLSL;

	BuiltInIncluder Includer(headers);
	if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		puts("GLSL Preprocessing Failed for: ");
		puts(Shader.getInfoLog());
		puts(Shader.getInfoDebugLog());
	}
	
	const char* PreprocessedCStr = PreprocessedGLSL.c_str();
	Shader.setStrings(&PreprocessedCStr, 1);
	if (!Shader.parse(&Resources, 110, false, messages))
	{
		puts("GLSL Parsing Failed for: ");
		puts(Shader.getInfoLog());
		puts(Shader.getInfoDebugLog());
		return false;
	}

	glslang::TProgram Program;
	Program.addShader(&Shader);
	if (!Program.link(messages))
	{
		puts("GLSL Linking Failed for:");
		puts(Shader.getInfoLog());
		puts(Shader.getInfoDebugLog());
		return false;
	}
	   	  
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;

	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);

	if (logger.getAllMessages().length() > 0)
	{
		puts(logger.getAllMessages().c_str());
		return false;
	}

	return true;

}

bool GLSL2SPV_Compute(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV)
{
	return GLSL2SPV(InputCString, headers, SpirV, EShLangCompute);
}

bool GLSL2SPV_Vertex(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV)
{
	return GLSL2SPV(InputCString, headers, SpirV, EShLangVertex);
}

bool GLSL2SPV_Fragment(const char* InputCString, const std::unordered_map<std::string, const char*>* headers, std::vector<unsigned int>& SpirV)
{
	return GLSL2SPV(InputCString, headers, SpirV, EShLangFragment);
}