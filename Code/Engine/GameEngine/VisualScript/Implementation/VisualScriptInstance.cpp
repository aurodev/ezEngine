#include <GameEngine/GameEnginePCH.h>

#include <Core/Messages/EventMessage.h>
#include <Core/World/GameObject.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Strings/HashedString.h>
#include <GameEngine/VisualScript/Nodes/VisualScriptBasicNodes.h>
#include <GameEngine/VisualScript/VisualScriptInstance.h>
#include <GameEngine/VisualScript/VisualScriptNode.h>
#include <GameEngine/VisualScript/VisualScriptResource.h>

ezVisualScriptInstance::ezVisualScriptInstance()
{
  SetupPinDataTypeConversions();
}

ezVisualScriptInstance::~ezVisualScriptInstance()
{
  Clear();
}

void ezVisualScriptInstance::Clear()
{
  for (ezUInt32 i = 0; i < m_Nodes.GetCount(); ++i)
  {
    m_Nodes[i]->GetDynamicRTTI()->GetAllocator()->Deallocate(m_Nodes[i]);
  }

  m_pWorld = nullptr;
  m_Nodes.Clear();
  m_ExecutionConnections.Clear();
  m_DataConnections.Clear();
  m_LocalVariables.Clear();
  m_hScriptResource.Invalidate();
}


void ezVisualScriptInstance::ComputeNodeDependencies()
{
  m_NodeDependencies.SetCount(m_Nodes.GetCount());

  for (auto it = m_DataConnections.GetIterator(); it.IsValid(); ++it)
  {
    const ezVisualScriptPinConnectionID src = it.Key();

    const ezUInt16 uiSourceNode = (src >> 16) & 0xFFFF;

    if (m_Nodes[uiSourceNode]->IsManuallyStepped())
      continue;

    const ezHybridArray<DataPinConnection, 2>& dst = it.Value();

    for (const DataPinConnection& target : dst)
    {
      m_NodeDependencies[target.m_uiTargetNode].PushBack(uiSourceNode);
    }
  }
}


void ezVisualScriptInstance::ExecuteDependentNodes(ezUInt16 uiNode)
{
  const auto& dep = m_NodeDependencies[uiNode];
  for (ezUInt32 i = 0; i < dep.GetCount(); ++i)
  {
    const ezUInt16 uiDependency = dep[i];
    auto* pNode = m_Nodes[uiDependency];

    // recurse to the most dependent nodes first
    ExecuteDependentNodes(uiDependency);

    // only nodes that are not manually stepped are in the dependency list
    // so we do not need to filter those out here
    pNode->Execute(this, 0);
    pNode->m_bInputValuesChanged = false;
  }
}

void ezVisualScriptInstance::Configure(const ezVisualScriptResourceHandle& hScript, ezComponent* pOwnerComponent)
{
  Clear();

  ezResourceLock<ezVisualScriptResource> pScript(hScript, ezResourceAcquireMode::BlockTillLoaded);
  const auto& resource = pScript->GetDescriptor();
  m_pMessageHandlers = &resource.m_MessageHandlers;

  m_hScriptResource = hScript;

  if (pOwnerComponent)
  {
    m_hOwnerObject = pOwnerComponent->GetOwner()->GetHandle();
    m_hOwnerComponent = pOwnerComponent->GetHandle();
    m_pWorld = pOwnerComponent->GetWorld();
  }

  m_Nodes.Reserve(resource.m_Nodes.GetCount());

  for (ezUInt32 n = 0; n < resource.m_Nodes.GetCount(); ++n)
  {
    const auto& node = resource.m_Nodes[n];

    if (node.m_isFunctionCall)
    {
      CreateFunctionCallNode(n, resource);
    }
    else if (node.m_pType && node.m_pType->IsDerivedFrom<ezMessage>())
    {
      if (node.m_isMsgSender)
      {
        CreateMessageSenderNode(n, resource);
      }
      else if (node.m_isMsgHandler)
      {
        CreateMessageHandlerNode(n, resource);
      }
    }
    else if (node.m_pType && node.m_pType->IsDerivedFrom<ezVisualScriptNode>())
    {
      CreateVisualScriptNode(n, resource);
    }
    else
    {
      ezLog::Error("Invalid node type '{0}' in visual script", node.m_sTypeName);
      Clear();
      return;
    }
  }

  m_ExecutionConnections.Reserve(resource.m_ExecutionPaths.GetCount());

  for (const auto& con : resource.m_ExecutionPaths)
  {
    ConnectExecutionPins(con.m_uiSourceNode, con.m_uiOutputPin, con.m_uiTargetNode, con.m_uiInputPin);
  }

  for (const auto& con : resource.m_DataPaths)
  {
    ConnectDataPins(con.m_uiSourceNode, con.m_uiOutputPin, (ezVisualScriptDataPinType::Enum)con.m_uiOutputPinType, con.m_uiTargetNode,
      con.m_uiInputPin, (ezVisualScriptDataPinType::Enum)con.m_uiInputPinType);
  }

  ComputeNodeDependencies();

  // initialize local variables
  {
    for (const auto& p : resource.m_BoolParameters)
    {
      m_LocalVariables.StoreBool(p.m_sName, p.m_Value);
    }

    for (const auto& p : resource.m_NumberParameters)
    {
      m_LocalVariables.StoreDouble(p.m_sName, p.m_Value);
    }

    for (const auto& p : resource.m_StringParameters)
    {
      m_LocalVariables.StoreString(p.m_sName, p.m_sValue);
    }
  }
}


void ezVisualScriptInstance::CreateVisualScriptNode(ezUInt32 uiNodeIdx, const ezVisualScriptResourceDescriptor& resource)
{
  EZ_ASSERT_DEBUG(uiNodeIdx < ezMath::MaxValue<ezUInt16>(), "Max supported node index is 16 bit.");

  const auto& node = resource.m_Nodes[uiNodeIdx];

  ezVisualScriptNode* pNode = node.m_pType->GetAllocator()->Allocate<ezVisualScriptNode>();
  pNode->m_uiNodeID = static_cast<ezUInt16>(uiNodeIdx);

  resource.AssignNodeProperties(*pNode, node);

  m_Nodes.PushBack(pNode);
}

void ezVisualScriptInstance::CreateMessageSenderNode(ezUInt32 uiNodeIdx, const ezVisualScriptResourceDescriptor& resource)
{
  EZ_ASSERT_DEBUG(uiNodeIdx < ezMath::MaxValue<ezUInt16>(), "Max supported node index is 16 bit.");

  const auto& node = resource.m_Nodes[uiNodeIdx];

  ezVisualScriptNode_MessageSender* pNode = ezGetStaticRTTI<ezVisualScriptNode_MessageSender>()->GetAllocator()->Allocate<ezVisualScriptNode_MessageSender>();
  pNode->m_uiNodeID = static_cast<ezUInt16>(uiNodeIdx);

  auto pMessage = node.m_pType->GetAllocator()->Allocate<ezMessage>();

  // assign all property values
  {
    for (ezUInt32 i = 0; i < node.m_uiNumProperties; ++i)
    {
      const ezUInt32 uiProp = node.m_uiFirstProperty + i;
      const auto& prop = resource.m_Properties[uiProp];

      ezAbstractProperty* pAbstract = pMessage->GetDynamicRTTI()->FindPropertyByName(prop.m_sName);
      if (pAbstract == nullptr)
      {
        if (prop.m_sName == "Delay" && prop.m_Value.CanConvertTo<ezTime>())
        {
          pNode->m_Delay = prop.m_Value.ConvertTo<ezTime>();
        }
        if (prop.m_sName == "Recursive" && prop.m_Value.CanConvertTo<bool>())
        {
          pNode->m_bRecursive = prop.m_Value.ConvertTo<bool>();
        }

        continue;
      }

      if (pAbstract->GetCategory() != ezPropertyCategory::Member)
        continue;

      ezAbstractMemberProperty* pMember = static_cast<ezAbstractMemberProperty*>(pAbstract);
      ezReflectionUtils::SetMemberPropertyValue(pMember, pMessage, prop.m_Value);
    }
  }

  pNode->SetMessageToSend(pMessage);
  m_Nodes.PushBack(pNode);
}


void ezVisualScriptInstance::CreateMessageHandlerNode(ezUInt32 uiNodeIdx, const ezVisualScriptResourceDescriptor& resource)
{
  EZ_ASSERT_DEBUG(uiNodeIdx < ezMath::MaxValue<ezUInt16>(), "Max supported node index is 16 bit.");

  const auto& node = resource.m_Nodes[uiNodeIdx];

  ezVisualScriptNode_MessageHandler* pNode = ezGetStaticRTTI<ezVisualScriptNode_MessageHandler>()->GetAllocator()->Allocate<ezVisualScriptNode_MessageHandler>();
  pNode->m_uiNodeID = static_cast<ezUInt16>(uiNodeIdx);
  pNode->m_pMessageTypeToHandle = node.m_pType;

  m_Nodes.PushBack(pNode);
}

void ezVisualScriptInstance::CreateFunctionCallNode(ezUInt32 uiNodeIdx, const ezVisualScriptResourceDescriptor& resource)
{
  EZ_ASSERT_DEBUG(uiNodeIdx < ezMath::MaxValue<ezUInt16>(), "Max supported node index is 16 bit.");

  const auto& node = resource.m_Nodes[uiNodeIdx];

  ezVisualScriptNode_FunctionCall* pNode =
    ezGetStaticRTTI<ezVisualScriptNode_FunctionCall>()->GetAllocator()->Allocate<ezVisualScriptNode_FunctionCall>();
  pNode->m_uiNodeID = static_cast<ezUInt16>(uiNodeIdx);

  pNode->m_pExpectedType = node.m_pType;

  if (pNode->m_pExpectedType != nullptr)
  {
    ezStringBuilder sFunc = node.m_sTypeName.FindSubString("::");
    sFunc.Shrink(2, 0);

    const ezScriptableFunctionAttribute* pSfAttr = nullptr;
    pNode->m_pFunctionToCall = SearchForScriptableFunctionOnType(pNode->m_pExpectedType, sFunc, pSfAttr);

    if (pNode->m_pFunctionToCall != nullptr)
    {
      pNode->m_ArgumentIsOutParamMask = 0;
      pNode->m_Arguments.SetCount(pNode->m_pFunctionToCall->GetArgumentCount());

      // initialize the variants to the proper type
      for (ezUInt32 arg = 0; arg < pNode->m_pFunctionToCall->GetArgumentCount(); ++arg)
      {
        pNode->m_Arguments[arg] = ezReflectionUtils::GetDefaultVariantFromType(pNode->m_pFunctionToCall->GetArgumentType(arg)->GetVariantType());
        ezVisualScriptDataPinType::EnforceSupportedType(pNode->m_Arguments[arg]);

        if (pSfAttr->GetArgumentType(arg) != ezScriptableFunctionAttribute::In) // out or inout
        {
          pNode->m_ArgumentIsOutParamMask |= EZ_BIT(arg);
        }
      }

      ezVariant tmpVal;

      // assign all property values
      for (ezUInt32 uiPropIdx = 0; uiPropIdx < node.m_uiNumProperties; ++uiPropIdx)
      {
        const ezUInt32 uiProp = node.m_uiFirstProperty + uiPropIdx;
        const auto& prop = resource.m_Properties[uiProp];

        if (prop.m_iMappingIndex >= 0 || prop.m_iMappingIndex < (ezInt32)pNode->m_Arguments.GetCount())
        {
          ezResult couldConvert = EZ_SUCCESS;
          const ezRTTI* pTargetType = pNode->m_pFunctionToCall->GetArgumentType(prop.m_iMappingIndex);
          if (pTargetType == ezGetStaticRTTI<ezVariant>())
          {
            tmpVal = prop.m_Value;
          }
          else
          {
            tmpVal = prop.m_Value.ConvertTo(pNode->m_Arguments[prop.m_iMappingIndex].GetType(), &couldConvert);
          }

          if (couldConvert.Succeeded())
          {
            pNode->m_Arguments[prop.m_iMappingIndex] = tmpVal;
          }
        }
      }
    }
    else
    {
      ezLog::Error("Function '{}' does not exist on type '{}'", sFunc, node.m_pType->GetTypeName());
    }
  }
  else
  {
    ezLog::Error("Expected target object type is null for vis script function call node '{}'", node.m_sTypeName);
  }

  m_Nodes.PushBack(pNode);
}

ezAbstractFunctionProperty* ezVisualScriptInstance::SearchForScriptableFunctionOnType(const ezRTTI* pObjectType, ezStringView sFuncName, const ezScriptableFunctionAttribute*& out_pSfAttr) const
{
  if (sFuncName.IsEmpty())
    return nullptr;

  while (pObjectType != nullptr)
  {
    for (auto pFunc : pObjectType->GetFunctions())
    {
      if (sFuncName != pFunc->GetPropertyName())
        continue;

      out_pSfAttr = pFunc->GetAttributeByType<ezScriptableFunctionAttribute>();

      if (out_pSfAttr == nullptr)
        continue;

      return pFunc;
    }

    pObjectType = pObjectType->GetParentType();
  }

  return nullptr;
}

void ezVisualScriptInstance::ExecuteScript(ezVisualScriptInstanceActivity* pActivity /*= nullptr*/)
{
  m_pActivity = pActivity;

  if (m_pActivity != nullptr)
  {
    m_pActivity->Clear();
  }

  const ezUInt16 uiNodeCount = static_cast<ezUInt16>(m_Nodes.GetCount());
  for (ezUInt16 i = 0; i < uiNodeCount; ++i)
  {
    auto* pNode = m_Nodes[i];

    if (pNode->m_bStepNode)
    {
      ExecuteDependentNodes(i);

      // node stepping is always executed, even if the node only 'wants' to be executed on input change
      pNode->m_bStepNode = false;
      pNode->Execute(this, 0);
      pNode->m_bInputValuesChanged = false;
    }
  }
}

bool ezVisualScriptInstance::HandleMessage(ezMessage& ref_msg)
{
  if (m_pMessageHandlers == nullptr)
    return false;

  ezUInt32 uiFirstHandler = m_pMessageHandlers->LowerBound(ref_msg.GetId());

  bool bHandled = false;

  while (uiFirstHandler < m_pMessageHandlers->GetCount())
  {
    const auto& data = (*m_pMessageHandlers).GetPair(uiFirstHandler);
    if (data.key != ref_msg.GetId())
      break;

    const ezUInt32 uiNodeId = data.value;
    m_Nodes[uiNodeId]->HandleMessage(&ref_msg);

    bHandled = true;
    ++uiFirstHandler;
  }

  return bHandled;
}

void ezVisualScriptInstance::ConnectExecutionPins(ezUInt16 uiSourceNode, ezUInt8 uiOutputSlot, ezUInt16 uiTargetNode, ezUInt8 uiTargetPin)
{
  auto& con = m_ExecutionConnections[((ezUInt32)uiSourceNode << 16) | (ezUInt32)uiOutputSlot];
  con.m_uiTargetNode = uiTargetNode;
  con.m_uiTargetPin = uiTargetPin;
}

void ezVisualScriptInstance::ConnectDataPins(ezUInt16 uiSourceNode, ezUInt8 uiSourcePin, ezVisualScriptDataPinType::Enum sourcePinType,
  ezUInt16 uiTargetNode, ezUInt8 uiTargetPin, ezVisualScriptDataPinType::Enum targetPinType)
{
  DataPinConnection& con = m_DataConnections[((ezUInt32)uiSourceNode << 16) | (ezUInt32)uiSourcePin].ExpandAndGetRef();
  con.m_uiTargetNode = uiTargetNode;
  con.m_uiTargetPin = uiTargetPin;
  con.m_pTargetData = m_Nodes[uiTargetNode]->GetInputPinDataPointer(uiTargetPin);
  con.m_AssignFunc = FindDataPinAssignFunction(sourcePinType, targetPinType);
}

void ezVisualScriptInstance::SetOutputPinValue(const ezVisualScriptNode* pNode, ezUInt8 uiPin, const void* pValue)
{
  const ezUInt32 uiConnectionID = ((ezUInt32)pNode->m_uiNodeID << 16) | (ezUInt32)uiPin;

  ezHybridArray<DataPinConnection, 2>* TargetNodeAndPins;
  if (!m_DataConnections.TryGetValue(uiConnectionID, TargetNodeAndPins))
    return;

  for (const DataPinConnection& TargetNodeAndPin : *TargetNodeAndPins)
  {
    if (TargetNodeAndPin.m_AssignFunc)
    {
      if (TargetNodeAndPin.m_AssignFunc(pValue, TargetNodeAndPin.m_pTargetData))
      {
        m_Nodes[TargetNodeAndPin.m_uiTargetNode]->m_bInputValuesChanged = true;
      }
    }
  }

  if (m_pActivity != nullptr)
  {
    m_pActivity->m_ActiveDataConnections.PushBack(uiConnectionID);
  }
}

void ezVisualScriptInstance::ExecuteConnectedNodes(const ezVisualScriptNode* pNode, ezUInt16 uiNthTarget)
{
  EZ_ASSERT_DEBUG(pNode->IsManuallyStepped(), "Only visual script nodes that are flagged as manually stepped may call ExecuteConnectedNodes().\n\
Otherwise visual script graph execution can end up in an endless recursion with stack-overflow.\n\
Override ezVisualScriptNode::IsManuallyStepped() for type '{}' if necessary.",
    pNode->GetDynamicRTTI()->GetTypeName());

  const ezUInt32 uiConnectionID = ((ezUInt32)pNode->m_uiNodeID << 16) | (ezUInt32)uiNthTarget;

  ExecPinConnection TargetNode;
  if (!m_ExecutionConnections.TryGetValue(uiConnectionID, TargetNode))
    return;

  auto* pTargetNode = m_Nodes[TargetNode.m_uiTargetNode];

  ExecuteDependentNodes(TargetNode.m_uiTargetNode);

  pTargetNode->Execute(this, TargetNode.m_uiTargetPin);
  pTargetNode->m_bInputValuesChanged = false;

  if (m_pActivity != nullptr)
  {
    m_pActivity->m_ActiveExecutionConnections.PushBack(uiConnectionID);
  }
}

bool ezVisualScriptInstance::HandlesMessage(const ezMessage& msg) const
{
  if (m_pMessageHandlers == nullptr)
    return false;

  return m_pMessageHandlers->Contains(msg.GetId());
}



EZ_STATICLINK_FILE(GameEngine, GameEngine_VisualScript_Implementation_VisualScriptInstance);
