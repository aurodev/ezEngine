#pragma once

#include <EditorFramework/Assets/AssetDocumentGenerator.h>
#include <EditorFramework/Assets/SimpleAssetDocument.h>
#include <EditorPluginPhysX/CollisionMeshAsset/CollisionMeshAssetObjects.h>

class ezPxMeshResourceDescriptor;
class ezGeometry;
class ezChunkStreamWriter;
struct ezPhysXCookingMesh;

class ezCollisionMeshAssetDocument : public ezSimpleAssetDocument<ezCollisionMeshAssetProperties>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezCollisionMeshAssetDocument, ezSimpleAssetDocument<ezCollisionMeshAssetProperties>);

public:
  ezCollisionMeshAssetDocument(const char* szDocumentPath, bool bConvexMesh);

  static ezStatus WriteToStream(ezChunkStreamWriter& inout_stream, const ezPhysXCookingMesh& mesh, const ezCollisionMeshAssetProperties* pProp);

protected:
  virtual void InitializeAfterLoading(bool bFirstTimeCreation) override;

  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const ezPlatformProfile* pAssetProfile, const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;

  ezStatus CreateMeshFromFile(ezPhysXCookingMesh& outMesh);
  ezStatus CreateMeshFromGeom(ezGeometry& geom, ezPhysXCookingMesh& outMesh);
  virtual ezTransformStatus InternalCreateThumbnail(const ThumbnailInfo& ThumbnailInfo) override;

  bool m_bIsConvexMesh = false;

  virtual void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
};

//////////////////////////////////////////////////////////////////////////


class ezCollisionMeshAssetDocumentGenerator : public ezAssetDocumentGenerator
{
  EZ_ADD_DYNAMIC_REFLECTION(ezCollisionMeshAssetDocumentGenerator, ezAssetDocumentGenerator);

public:
  ezCollisionMeshAssetDocumentGenerator();
  ~ezCollisionMeshAssetDocumentGenerator();

  virtual void GetImportModes(const char* szParentDirRelativePath, ezHybridArray<ezAssetDocumentGenerator::Info, 4>& out_modes) const override;
  virtual ezStatus Generate(const char* szDataDirRelativePath, const ezAssetDocumentGenerator::Info& info, ezDocument*& out_pGeneratedDocument) override;
  virtual const char* GetDocumentExtension() const override { return "ezCollisionMeshAsset"; }
  virtual const char* GetGeneratorGroup() const override { return "CollisionMeshes"; }
};

class ezConvexCollisionMeshAssetDocumentGenerator : public ezAssetDocumentGenerator
{
  EZ_ADD_DYNAMIC_REFLECTION(ezConvexCollisionMeshAssetDocumentGenerator, ezAssetDocumentGenerator);

public:
  ezConvexCollisionMeshAssetDocumentGenerator();
  ~ezConvexCollisionMeshAssetDocumentGenerator();

  virtual void GetImportModes(const char* szParentDirRelativePath, ezHybridArray<ezAssetDocumentGenerator::Info, 4>& out_modes) const override;
  virtual ezStatus Generate(const char* szDataDirRelativePath, const ezAssetDocumentGenerator::Info& info, ezDocument*& out_pGeneratedDocument) override;
  virtual const char* GetDocumentExtension() const override { return "ezConvexCollisionMeshAsset"; }
  virtual const char* GetGeneratorGroup() const override { return "CollisionMeshes"; }
};
