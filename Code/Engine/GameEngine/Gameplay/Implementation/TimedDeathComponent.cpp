#include <GameEngine/GameEnginePCH.h>

#include <Core/Messages/TriggerMessage.h>
#include <Core/Prefabs/PrefabResource.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <GameEngine/Gameplay/TimedDeathComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezTimedDeathComponent, 2, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("MinDelay", m_MinDelay)->AddAttributes(new ezClampValueAttribute(ezTime(), ezVariant()), new ezDefaultValueAttribute(ezTime::Seconds(1.0))),
    EZ_MEMBER_PROPERTY("DelayRange", m_DelayRange)->AddAttributes(new ezClampValueAttribute(ezTime(), ezVariant())),
    EZ_ACCESSOR_PROPERTY("TimeoutPrefab", GetTimeoutPrefab, SetTimeoutPrefab)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Prefab")),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgComponentInternalTrigger, OnTriggered),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("Gameplay"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezTimedDeathComponent::ezTimedDeathComponent() = default;
ezTimedDeathComponent::~ezTimedDeathComponent() = default;

void ezTimedDeathComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_MinDelay;
  s << m_DelayRange;
  s << m_hTimeoutPrefab;
}

void ezTimedDeathComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = inout_stream.GetStream();

  s >> m_MinDelay;
  s >> m_DelayRange;
  s >> m_hTimeoutPrefab;
}

void ezTimedDeathComponent::OnSimulationStarted()
{
  ezMsgComponentInternalTrigger msg;
  msg.m_sMessage.Assign("Suicide");

  ezWorld* pWorld = GetWorld();

  const ezTime tKill = ezTime::Seconds(pWorld->GetRandomNumberGenerator().DoubleInRange(m_MinDelay.GetSeconds(), m_DelayRange.GetSeconds()));

  PostMessage(msg, tKill);

  // make sure the prefab is available when the component dies
  if (m_hTimeoutPrefab.IsValid())
  {
    ezResourceManager::PreloadResource(m_hTimeoutPrefab);
  }
}

void ezTimedDeathComponent::OnTriggered(ezMsgComponentInternalTrigger& msg)
{
  if (msg.m_sMessage != ezTempHashedString("Suicide"))
    return;

  if (m_hTimeoutPrefab.IsValid())
  {
    ezResourceLock<ezPrefabResource> pPrefab(m_hTimeoutPrefab, ezResourceAcquireMode::AllowLoadingFallback);

    ezPrefabInstantiationOptions options;
    options.m_pOverrideTeamID = &GetOwner()->GetTeamID();

    pPrefab->InstantiatePrefab(*GetWorld(), GetOwner()->GetGlobalTransform(), options);
  }

  GetWorld()->DeleteObjectDelayed(GetOwner()->GetHandle());
}

void ezTimedDeathComponent::SetTimeoutPrefab(const char* szPrefab)
{
  ezPrefabResourceHandle hPrefab;

  if (!ezStringUtils::IsNullOrEmpty(szPrefab))
  {
    hPrefab = ezResourceManager::LoadResource<ezPrefabResource>(szPrefab);
  }

  m_hTimeoutPrefab = hPrefab;
}


const char* ezTimedDeathComponent::GetTimeoutPrefab() const
{
  if (!m_hTimeoutPrefab.IsValid())
    return "";

  return m_hTimeoutPrefab.GetResourceID();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>

class ezTimedDeathComponentPatch_1_2 : public ezGraphPatch
{
public:
  ezTimedDeathComponentPatch_1_2()
    : ezGraphPatch("ezTimedDeathComponent", 2)
  {
  }

  virtual void Patch(ezGraphPatchContext& ref_context, ezAbstractObjectGraph* pGraph, ezAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("Min Delay", "MinDelay");
    pNode->RenameProperty("Delay Range", "DelayRange");
    pNode->RenameProperty("Timeout Prefab", "TimeoutPrefab");
  }
};

ezTimedDeathComponentPatch_1_2 g_ezTimedDeathComponentPatch_1_2;



EZ_STATICLINK_FILE(GameEngine, GameEngine_Gameplay_Implementation_TimedDeathComponent);
