#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Reflection/Reflection.h>

class ezChunkStreamWriter;
class ezChunkStreamReader;

struct ezProfileTargetPlatform
{
  enum Enum
  {
    PC,
    UWP,
    Android,

    Default = PC
  };

  typedef ezUInt8 StorageType;
};

EZ_DECLARE_REFLECTABLE_TYPE(EZ_CORE_DLL, ezProfileTargetPlatform);

//////////////////////////////////////////////////////////////////////////

/// \brief Base class for configuration objects that store e.g. asset transform settings or runtime configuration information
class EZ_CORE_DLL ezProfileConfigData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezProfileConfigData, ezReflectedClass);

public:
  ezProfileConfigData();
  ~ezProfileConfigData();

  virtual void SaveRuntimeData(ezChunkStreamWriter& inout_stream) const;
  virtual void LoadRuntimeData(ezChunkStreamReader& inout_stream);
};

//////////////////////////////////////////////////////////////////////////

class EZ_CORE_DLL ezPlatformProfile : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezPlatformProfile, ezReflectedClass);

public:
  ezPlatformProfile();
  ~ezPlatformProfile();

  ezStringView GetConfigName() const { return m_sName; }

  void Clear();
  void AddMissingConfigs();

  template <typename TYPE>
  const TYPE* GetTypeConfig() const
  {
    return static_cast<const TYPE*>(GetTypeConfig(ezGetStaticRTTI<TYPE>()));
  }

  template <typename TYPE>
  TYPE* GetTypeConfig()
  {
    return static_cast<TYPE*>(GetTypeConfig(ezGetStaticRTTI<TYPE>()));
  }

  const ezProfileConfigData* GetTypeConfig(const ezRTTI* pRtti) const;
  ezProfileConfigData* GetTypeConfig(const ezRTTI* pRtti);

  ezResult SaveForRuntime(ezStringView sFile) const;
  ezResult LoadForRuntime(ezStringView sFile);

  ezString m_sName;
  ezEnum<ezProfileTargetPlatform> m_TargetPlatform;
  ezDynamicArray<ezProfileConfigData*> m_Configs;
};

//////////////////////////////////////////////////////////////////////////
