#pragma once

#include <Core/World/Component.h>
#include <Core/World/World.h>
#include <RecastPlugin/RecastPluginDLL.h>

/// \brief Base class for all components that implement 'non player character' behavior.
/// Ie, game logic for how an AI controlled character should behave (state machines etc)
class EZ_RECASTPLUGIN_DLL ezNpcComponent : public ezComponent
{
  EZ_DECLARE_ABSTRACT_COMPONENT_TYPE(ezNpcComponent, ezComponent);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(ezWorldReader& inout_stream) override;

  //////////////////////////////////////////////////////////////////////////
  // ezNpcComponent

public:
  ezNpcComponent();
  ~ezNpcComponent();
};
