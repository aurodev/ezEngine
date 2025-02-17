#include <PhysXPlugin/PhysXPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <PhysXPlugin/Shapes/PxShapeComponent.h>
#include <PhysXPlugin/Utilities/PxConversionUtils.h>
#include <PhysXPlugin/WorldModule/Implementation/PhysX.h>
#include <PhysXPlugin/WorldModule/PhysXWorldModule.h>

// clang-format off
EZ_BEGIN_ABSTRACT_COMPONENT_TYPE(ezPxShapeComponent, 5)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("Surface", GetSurfaceFile, SetSurfaceFile)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Surface")),
    EZ_MEMBER_PROPERTY("CollisionLayer", m_uiCollisionLayer)->AddAttributes(new ezDynamicEnumAttribute("PhysicsCollisionLayer")),
    EZ_BITFLAGS_MEMBER_PROPERTY("OnContact", ezOnPhysXContact, m_OnContact),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Physics/PhysX/Shapes"),
  }
  EZ_END_ATTRIBUTES;
  EZ_BEGIN_FUNCTIONS
  {
    EZ_SCRIPT_FUNCTION_PROPERTY(GetShapeId),
  }
  EZ_END_FUNCTIONS;
}
EZ_END_ABSTRACT_COMPONENT_TYPE
// clang-format on

ezPxShapeComponent::ezPxShapeComponent() = default;
ezPxShapeComponent::~ezPxShapeComponent() = default;

void ezPxShapeComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  auto& s = inout_stream.GetStream();

  s << m_hSurface;
  s << m_uiCollisionLayer;

  // version 5
  s << m_OnContact;
}


void ezPxShapeComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const ezUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = inout_stream.GetStream();

  s >> m_hSurface;
  s >> m_uiCollisionLayer;

  if (uiVersion >= 2 && uiVersion <= 4)
  {
    bool m_bReportContact;
    s >> m_bReportContact;

    if (m_bReportContact)
      m_OnContact.Add(ezOnPhysXContact::SendReportMsg);
  }

  if (uiVersion == 3)
  {
    ezStringBuilder tmp1;
    float tmp2;
    s >> tmp2;
    s >> tmp1;
  }

  if (uiVersion == 4)
  {
    bool m_bSurfaceInteractions;
    s >> m_bSurfaceInteractions;

    if (m_bSurfaceInteractions)
      m_OnContact.Add(ezOnPhysXContact::ImpactReactions | ezOnPhysXContact::SlideReactions);
  }

  if (uiVersion >= 5)
  {
    s >> m_OnContact;
  }
}


void ezPxShapeComponent::Initialize()
{
  if (IsActive())
  {
    GetOwner()->UpdateLocalBounds();
  }
}

void ezPxShapeComponent::OnDeactivated()
{
  if (ezPhysXWorldModule* pModule = GetWorld()->GetModule<ezPhysXWorldModule>())
  {
    pModule->DeleteShapeId(m_uiShapeId);
    pModule->DeallocateUserData(m_uiUserDataIndex);
  }

  SUPER::OnDeactivated();
}

void ezPxShapeComponent::SetSurfaceFile(const char* szFile)
{
  if (!ezStringUtils::IsNullOrEmpty(szFile))
  {
    m_hSurface = ezResourceManager::LoadResource<ezSurfaceResource>(szFile);
  }

  if (m_hSurface.IsValid())
    ezResourceManager::PreloadResource(m_hSurface);
}

const char* ezPxShapeComponent::GetSurfaceFile() const
{
  if (!m_hSurface.IsValid())
    return "";

  return m_hSurface.GetResourceID();
}

void ezPxShapeComponent::SetInitialShapeId(ezUInt32 uiId)
{
  EZ_ASSERT_DEV(m_uiShapeId == ezInvalidIndex, "ezPxShapeComponent::SetInitialShapeId() can only be called before the component has been activated.");
  m_uiShapeId = uiId;
}

void ezPxShapeComponent::AddToActor(PxRigidActor* pActor, const ezSimdTransform& parentTransform)
{
  PxTransform shapeTransform(PxIdentity);

  ezHybridArray<physx::PxShape*, 4> shapes;
  CreateShapes(shapes, pActor, shapeTransform);
  EZ_ASSERT_DEBUG(!shapes.IsEmpty(), "PhysX shape creation failed");

  const ezSimdTransform& ownerTransform = GetOwner()->GetGlobalTransformSimd();

  ezSimdTransform localTransform;
  localTransform.SetLocalTransform(parentTransform, ownerTransform);

  ezPhysXWorldModule* pModule = GetWorld()->GetOrCreateModule<ezPhysXWorldModule>();

  if (m_uiShapeId == ezInvalidIndex)
  {
    m_uiShapeId = pModule->CreateShapeId();
  }

  PxFilterData filter = CreateFilterData();

  ezPxUserData* pUserData = nullptr;
  m_uiUserDataIndex = pModule->AllocateUserData(pUserData);
  pUserData->Init(this);

  const PxTransform t = ezPxConversionUtils::ToTransform(localTransform);

  for (auto pShape : shapes)
  {
    if (pShape == nullptr)
      continue;

    pShape->setLocalPose(t * shapeTransform);
    pShape->setSimulationFilterData(filter);
    pShape->setQueryFilterData(filter);
    pShape->userData = pUserData;
  }
}

void ezPxShapeComponent::ExtractGeometry(ezMsgExtractGeometry& ref_msg) const {}

PxMaterial* ezPxShapeComponent::GetPxMaterial()
{
  if (m_hSurface.IsValid())
  {
    ezResourceLock<ezSurfaceResource> pSurface(m_hSurface, ezResourceAcquireMode::BlockTillLoaded);

    if (pSurface->m_pPhysicsMaterialPhysX != nullptr)
    {
      return static_cast<PxMaterial*>(pSurface->m_pPhysicsMaterialPhysX);
    }
  }

  return ezPhysX::GetSingleton()->GetDefaultMaterial();
}

PxFilterData ezPxShapeComponent::CreateFilterData()
{
  return ezPhysX::CreateFilterData(m_uiCollisionLayer, m_uiShapeId, m_OnContact);
}



EZ_STATICLINK_FILE(PhysXPlugin, PhysXPlugin_Shapes_Implementation_PxShapeComponent);
