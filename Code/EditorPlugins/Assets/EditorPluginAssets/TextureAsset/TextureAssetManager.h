#pragma once

#include <Core/Configuration/PlatformProfile.h>
#include <EditorFramework/Assets/AssetDocumentManager.h>
#include <Foundation/Types/Status.h>

class ezTextureAssetProfileConfig : public ezProfileConfigData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTextureAssetProfileConfig, ezProfileConfigData);

public:
  ezUInt16 m_uiMaxResolution = 1024 * 16;
};

class ezTextureAssetDocumentManager : public ezAssetDocumentManager
{
  EZ_ADD_DYNAMIC_REFLECTION(ezTextureAssetDocumentManager, ezAssetDocumentManager);

public:
  ezTextureAssetDocumentManager();
  ~ezTextureAssetDocumentManager();

private:
  void OnDocumentManagerEvent(const ezDocumentManager::Event& e);

  virtual ezUInt64 ComputeAssetProfileHashImpl(const ezPlatformProfile* pAssetProfile) const override;

  virtual void InternalCreateDocument(const char* szDocumentTypeName, const char* szPath, bool bCreateNewDocument, ezDocument*& out_pDocument, const ezDocumentObject* pOpenContext) override;
  virtual void InternalGetSupportedDocumentTypes(ezDynamicArray<const ezDocumentTypeDescriptor*>& inout_DocumentTypes) const override;

  virtual bool GeneratesProfileSpecificAssets() const override { return true; }

private:
  ezAssetDocumentTypeDescriptor m_DocTypeDesc;
  ezAssetDocumentTypeDescriptor m_DocTypeDesc2;
};
