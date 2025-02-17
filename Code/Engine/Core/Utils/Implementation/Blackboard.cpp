#include <Core/CorePCH.h>

#include <Core/Utils/Blackboard.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_BITFLAGS(ezBlackboardEntryFlags, 1)
  EZ_BITFLAGS_CONSTANTS(ezBlackboardEntryFlags::Save, ezBlackboardEntryFlags::OnChangeEvent,
    ezBlackboardEntryFlags::UserFlag0, ezBlackboardEntryFlags::UserFlag1, ezBlackboardEntryFlags::UserFlag2, ezBlackboardEntryFlags::UserFlag3, ezBlackboardEntryFlags::UserFlag4, ezBlackboardEntryFlags::UserFlag5, ezBlackboardEntryFlags::UserFlag6, ezBlackboardEntryFlags::UserFlag7)
EZ_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
EZ_BEGIN_SUBSYSTEM_DECLARATION(Core, Blackboard)

  ON_CORESYSTEMS_SHUTDOWN
  {
    EZ_LOCK(ezBlackboard::s_GlobalBlackboardsMutex);
    ezBlackboard::s_GlobalBlackboards.Clear();
  }

EZ_END_SUBSYSTEM_DECLARATION;
// clang-format on

// static
ezMutex ezBlackboard::s_GlobalBlackboardsMutex;
ezHashTable<ezHashedString, ezSharedPtr<ezBlackboard>> ezBlackboard::s_GlobalBlackboards;

// static
ezSharedPtr<ezBlackboard> ezBlackboard::Create(ezAllocatorBase* pAllocator /*= ezFoundation::GetDefaultAllocator()*/)
{
  return EZ_NEW(pAllocator, ezBlackboard);
}

// static
ezSharedPtr<ezBlackboard> ezBlackboard::GetOrCreateGlobal(const ezHashedString& sBlackboardName, ezAllocatorBase* pAllocator /*= ezFoundation::GetDefaultAllocator()*/)
{
  EZ_LOCK(s_GlobalBlackboardsMutex);

  auto it = s_GlobalBlackboards.Find(sBlackboardName);

  if (it.IsValid())
  {
    return it.Value();
  }

  ezSharedPtr<ezBlackboard> pShrd = EZ_NEW(pAllocator, ezBlackboard);
  pShrd->m_sName = sBlackboardName;
  s_GlobalBlackboards.Insert(sBlackboardName, pShrd);

  return pShrd;
}

// static
ezSharedPtr<ezBlackboard> ezBlackboard::FindGlobal(const ezTempHashedString& sBlackboardName)
{
  EZ_LOCK(s_GlobalBlackboardsMutex);

  ezSharedPtr<ezBlackboard> pBlackboard;
  s_GlobalBlackboards.TryGetValue(sBlackboardName, pBlackboard);
  return pBlackboard;
}

ezBlackboard::ezBlackboard() = default;
ezBlackboard::~ezBlackboard() = default;

void ezBlackboard::SetName(ezStringView sName)
{
  EZ_LOCK(s_GlobalBlackboardsMutex);
  m_sName.Assign(sName);
}

void ezBlackboard::RegisterEntry(const ezHashedString& sName, const ezVariant& initialValue, ezBitflags<ezBlackboardEntryFlags> flags /*= ezBlackboardEntryFlags::None*/)
{
  EZ_ASSERT_ALWAYS(!flags.IsSet(ezBlackboardEntryFlags::Invalid), "The invalid flag is reserved for internal use.");

  bool bExisted = false;
  Entry& entry = m_Entries.FindOrAdd(sName, &bExisted);

  if (!bExisted || entry.m_Flags != flags)
  {
    ++m_uiBlackboardChangeCounter;
    entry.m_Flags |= flags;
  }

  if (!bExisted && entry.m_Value != initialValue)
  {
    // broadcasts the change event, in case we overwrite an existing entry
    SetEntryValue(sName, initialValue).IgnoreResult();
  }
}

void ezBlackboard::UnregisterEntry(const ezHashedString& sName)
{
  if (m_Entries.Remove(sName))
  {
    ++m_uiBlackboardChangeCounter;
  }
}

void ezBlackboard::UnregisterAllEntries()
{
  if (m_Entries.IsEmpty() == false)
  {
    ++m_uiBlackboardChangeCounter;
  }

  m_Entries.Clear();
}

ezResult ezBlackboard::SetEntryValue(const ezTempHashedString& sName, const ezVariant& value, bool bForce /*= false*/)
{
  auto itEntry = m_Entries.Find(sName);

  if (!itEntry.IsValid())
  {
    return EZ_FAILURE;
  }

  Entry& entry = itEntry.Value();

  if (!bForce && entry.m_Value == value)
    return EZ_SUCCESS;

  ++m_uiBlackboardEntryChangeCounter;
  ++entry.m_uiChangeCounter;

  if (entry.m_Flags.IsSet(ezBlackboardEntryFlags::OnChangeEvent))
  {
    EntryEvent e;
    e.m_sName = itEntry.Key();
    e.m_OldValue = entry.m_Value;
    e.m_pEntry = &entry;

    entry.m_Value = value;

    m_EntryEvents.Broadcast(e, 1); // limited recursion is allowed
  }
  else
  {
    entry.m_Value = value;
  }

  return EZ_SUCCESS;
}

const ezBlackboard::Entry* ezBlackboard::GetEntry(const ezTempHashedString& sName) const
{
  auto itEntry = m_Entries.Find(sName);

  if (!itEntry.IsValid())
    return nullptr;

  return &itEntry.Value();
}

ezVariant ezBlackboard::GetEntryValue(const ezTempHashedString& sName, ezVariant fallback) const
{
  auto value = m_Entries.GetValue(sName);
  return value != nullptr ? value->m_Value : fallback;
}

ezBitflags<ezBlackboardEntryFlags> ezBlackboard::GetEntryFlags(const ezTempHashedString& sName) const
{
  auto itEntry = m_Entries.Find(sName);

  if (!itEntry.IsValid())
  {
    return ezBlackboardEntryFlags::Invalid;
  }

  return itEntry.Value().m_Flags;
}

ezResult ezBlackboard::Serialize(ezStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(1);

  ezUInt32 uiEntries = 0;

  for (auto it : m_Entries)
  {
    if (it.Value().m_Flags.IsSet(ezBlackboardEntryFlags::Save))
    {
      ++uiEntries;
    }
  }

  inout_stream << uiEntries;

  for (auto it : m_Entries)
  {
    const Entry& e = it.Value();

    if (e.m_Flags.IsSet(ezBlackboardEntryFlags::Save))
    {
      inout_stream << it.Key();
      inout_stream << e.m_Flags;
      inout_stream << e.m_Value;
    }
  }

  return EZ_SUCCESS;
}

ezResult ezBlackboard::Deserialize(ezStreamReader& inout_stream)
{
  inout_stream.ReadVersion(1);

  ezUInt32 uiEntries = 0;
  inout_stream >> uiEntries;

  for (ezUInt32 e = 0; e < uiEntries; ++e)
  {
    ezHashedString name;
    inout_stream >> name;

    ezBitflags<ezBlackboardEntryFlags> flags;
    inout_stream >> flags;

    ezVariant value;
    inout_stream >> value;

    RegisterEntry(name, value, flags);
  }

  return EZ_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
EZ_BEGIN_STATIC_REFLECTED_TYPE(ezBlackboardCondition, ezNoBase, 1, ezRTTIDefaultAllocator<ezBlackboardCondition>)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("EntryName", GetEntryName, SetEntryName),
    EZ_ENUM_MEMBER_PROPERTY("Operator", ezComparisonOperator, m_Operator),
    EZ_MEMBER_PROPERTY("ComparisonValue", m_fComparisonValue),
  }
  EZ_END_PROPERTIES;
}
EZ_END_STATIC_REFLECTED_TYPE;
// clang-format on

bool ezBlackboardCondition::IsConditionMet(const ezBlackboard& blackboard) const
{
  auto pEntry = blackboard.GetEntry(m_sEntryName);
  if (pEntry != nullptr && pEntry->m_Value.IsNumber())
  {
    double fEntryValue = pEntry->m_Value.ConvertTo<double>();
    return ezComparisonOperator::Compare(m_Operator, fEntryValue, m_fComparisonValue);
  }

  return false;
}

constexpr ezTypeVersion s_BlackboardConditionVersion = 1;

ezResult ezBlackboardCondition::Serialize(ezStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_BlackboardConditionVersion);

  inout_stream << m_sEntryName;
  inout_stream << m_Operator;
  inout_stream << m_fComparisonValue;
  return EZ_SUCCESS;
}

ezResult ezBlackboardCondition::Deserialize(ezStreamReader& inout_stream)
{
  const ezTypeVersion uiVersion = inout_stream.ReadVersion(s_BlackboardConditionVersion);

  inout_stream >> m_sEntryName;
  inout_stream >> m_Operator;
  inout_stream >> m_fComparisonValue;
  return EZ_SUCCESS;
}

EZ_STATICLINK_FILE(Core, Core_Utils_Implementation_Blackboard);
