#pragma once

#include <RecastPlugin/Components/NpcComponent.h>
#include <RecastPlugin/RecastPluginDLL.h>

class ezRecastWorldModule;
class ezPhysicsWorldModuleInterface;
struct ezAgentSteeringEvent;

typedef ezComponentManagerSimple<class ezSoldierComponent, ezComponentUpdateType::WhenSimulating> ezSoldierComponentManager;

class EZ_RECASTPLUGIN_DLL ezSoldierComponent : public ezNpcComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezSoldierComponent, ezNpcComponent, ezSoldierComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

protected:
  virtual void Deinitialize() override;
  virtual void OnSimulationStarted() override;


  //////////////////////////////////////////////////////////////////////////
  // ezSoldierComponent

public:
  ezSoldierComponent();
  ~ezSoldierComponent();

protected:
  void Update();

  void SteeringEventHandler(const ezAgentSteeringEvent& e);

  enum class State
  {
    Idle,
    WaitingForPath,
    Walking,
    ErrorState,
  };

  State m_State = State::Idle;
  ezComponentHandle m_hSteeringComponent;
};
