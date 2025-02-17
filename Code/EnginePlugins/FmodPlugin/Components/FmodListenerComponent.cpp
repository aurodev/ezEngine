#include <FmodPlugin/FmodPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <FmodPlugin/Components/FmodListenerComponent.h>
#include <FmodPlugin/FmodIncludes.h>
#include <FmodPlugin/FmodSingleton.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezFmodListenerComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("ListenerIndex", m_uiListenerIndex),
  }
  EZ_END_PROPERTIES;
}
EZ_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

ezFmodListenerComponent::ezFmodListenerComponent() = default;
ezFmodListenerComponent::~ezFmodListenerComponent() = default;

void ezFmodListenerComponent::SerializeComponent(ezWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  auto& s = inout_stream.GetStream();

  s << m_uiListenerIndex;
}

void ezFmodListenerComponent::DeserializeComponent(ezWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = inout_stream.GetStream();

  s >> m_uiListenerIndex;
}

void ezFmodListenerComponent::Update()
{
  const auto pos = GetOwner()->GetGlobalPosition();
  const auto vel = GetOwner()->GetVelocity();
  const auto fwd = (GetOwner()->GetGlobalRotation() * ezVec3(1, 0, 0)).GetNormalized();
  const auto up = (GetOwner()->GetGlobalRotation() * ezVec3(0, 0, 1)).GetNormalized();

  ezFmod::GetSingleton()->SetListener(m_uiListenerIndex, pos, fwd, up, vel);
}

EZ_STATICLINK_FILE(FmodPlugin, FmodPlugin_Components_FmodListenerComponent);
