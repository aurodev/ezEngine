#pragma once

#include <Core/World/World.h>

class AsteroidComponent;
typedef ezComponentManagerSimple<AsteroidComponent, ezComponentUpdateType::WhenSimulating> AsteroidComponentManager;

class AsteroidComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(AsteroidComponent, ezComponent, AsteroidComponentManager);

public:
  AsteroidComponent();

  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override {}
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override {}

  void Update();

  float m_fRotationSpeed;

protected:
  virtual void OnSimulationStarted() override;
};
