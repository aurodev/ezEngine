#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <Foundation/Utilities/ConversionUtils.h>

EZ_ENUMERABLE_CLASS_IMPLEMENTATION(ezCommandLineOption);

void ezCommandLineOption::GetSortingGroup(ezStringBuilder& ref_sOut) const
{
  ref_sOut = m_szSortingGroup;
}

void ezCommandLineOption::GetSplitOptions(ezStringBuilder& out_sAll, ezDynamicArray<ezStringView>& ref_splitOptions) const
{
  GetOptions(out_sAll);
  out_sAll.Split(false, ref_splitOptions, ";", "|");
}

bool ezCommandLineOption::IsHelpRequested(const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/)
{
  return pUtils->GetBoolOption("-help") || pUtils->GetBoolOption("--help") || pUtils->GetBoolOption("-h") || pUtils->GetBoolOption("-?");
}

ezResult ezCommandLineOption::RequireOptions(const char* szRequiredOptions, ezString* pMissingOption /*= nullptr*/, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/)
{
  ezStringBuilder tmp;
  ezStringBuilder allOpts = szRequiredOptions;
  ezHybridArray<ezStringView, 16> options;
  allOpts.Split(false, options, ";");

  for (auto opt : options)
  {
    opt.Trim(" ");

    if (pUtils->GetOptionIndex(opt.GetData(tmp)) < 0)
    {
      if (pMissingOption)
      {
        *pMissingOption = opt;
      }

      return EZ_FAILURE;
    }
  }

  if (pMissingOption)
  {
    pMissingOption->Clear();
  }

  return EZ_SUCCESS;
}

bool ezCommandLineOption::LogAvailableOptions(LogAvailableModes mode, const char* szGroupFilter /*= nullptr*/, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/)
{
  if (mode == LogAvailableModes::IfHelpRequested)
  {
    if (!IsHelpRequested(pUtils))
      return false;
  }

  ezMap<ezString, ezHybridArray<ezCommandLineOption*, 16>> sorted;

  ezStringBuilder sGroupFilter;
  if (!ezStringUtils::IsNullOrEmpty(szGroupFilter))
  {
    sGroupFilter.Set(";", szGroupFilter, ";");
  }

  for (ezCommandLineOption* pOpt = ezCommandLineOption::GetFirstInstance(); pOpt != nullptr; pOpt = pOpt->GetNextInstance())
  {
    ezStringBuilder sGroup;
    pOpt->GetSortingGroup(sGroup);
    sGroup.Prepend(";");
    sGroup.Append(";");

    if (!sGroupFilter.IsEmpty())
    {
      if (sGroupFilter.FindSubString_NoCase(sGroup) == nullptr)
        continue;
    }

    sorted[sGroup].PushBack(pOpt);
  }

  if (ezApplication::GetApplicationInstance())
  {
    ezLog::Info("");
    ezLog::Info("{} command line options:", ezApplication::GetApplicationInstance()->GetApplicationName());
  }

  if (sorted.IsEmpty())
  {
    ezLog::Info("This application has no documented command line options.");
    return true;
  }

  ezStringBuilder sLine;

  for (auto optIt : sorted)
  {
    for (auto pOpt : optIt.Value())
    {
      ezStringBuilder sOptions, sParamShort, sParamDefault, sLongDesc;

      sLine.Clear();

      pOpt->GetOptions(sOptions);
      pOpt->GetParamShortDesc(sParamShort);
      pOpt->GetParamDefaultValueDesc(sParamDefault);
      pOpt->GetLongDesc(sLongDesc);

      ezHybridArray<ezStringView, 4> lines;

      sOptions.Split(false, lines, ";", "|");

      for (auto o : lines)
      {
        sLine.AppendWithSeparator(", ", o);
      }

      if (!sParamShort.IsEmpty())
      {
        sLine.Append(" ", sParamShort);

        if (!sParamDefault.IsEmpty())
        {
          sLine.Append(" = ", sParamDefault);
        }
      }

      ezLog::Info("");
      ezLog::Info(sLine);

      sLongDesc.Trim(" \t\n\r");
      sLongDesc.Split(true, lines, "\n");

      for (auto o : lines)
      {
        sLine = o;
        sLine.Trim("\t\n\r");
        sLine.Prepend("    ");

        ezLog::Info(sLine);
      }
    }

    ezLog::Info("");
  }

  ezLog::Info("");

  return true;
}


bool ezCommandLineOption::LogAvailableOptionsToBuffer(ezStringBuilder& out_sBuffer, LogAvailableModes mode, const char* szGroupFilter /*= nullptr*/, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/)
{
  ezLogSystemToBuffer log;
  ezLogSystemScope ls(&log);

  const bool res = ezCommandLineOption::LogAvailableOptions(mode, szGroupFilter, pUtils);

  out_sBuffer = log.m_sBuffer;

  return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCommandLineOptionDoc::ezCommandLineOptionDoc(const char* szSortingGroup, const char* szArgument, const char* szParamShortDesc, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive /*= false*/)
  : ezCommandLineOption(szSortingGroup)
{
  m_szArgument = szArgument;
  m_szParamShortDesc = szParamShortDesc;
  m_szParamDefaultValue = szDefaultValue;
  m_szLongDesc = szLongDesc;
  m_bCaseSensitive = bCaseSensitive;
}

void ezCommandLineOptionDoc::GetOptions(ezStringBuilder& ref_sOut) const
{
  ref_sOut = m_szArgument;
}

void ezCommandLineOptionDoc::GetParamShortDesc(ezStringBuilder& ref_sOut) const
{
  ref_sOut = m_szParamShortDesc;
}

void ezCommandLineOptionDoc::GetParamDefaultValueDesc(ezStringBuilder& ref_sOut) const
{
  ref_sOut = m_szParamDefaultValue;
}

void ezCommandLineOptionDoc::GetLongDesc(ezStringBuilder& ref_sOut) const
{
  ref_sOut = m_szLongDesc;
}

bool ezCommandLineOptionDoc::IsOptionSpecified(ezStringBuilder* out_pWhich, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  ezStringBuilder sOptions, tmp;
  ezHybridArray<ezStringView, 4> eachOption;
  GetSplitOptions(sOptions, eachOption);

  for (auto o : eachOption)
  {
    if (pUtils->GetOptionIndex(o.GetData(tmp), m_bCaseSensitive) >= 0)
    {
      if (out_pWhich)
      {
        *out_pWhich = tmp;
      }

      return true;
    }
  }

  if (out_pWhich)
  {
    *out_pWhich = m_szArgument;
  }

  return false;
}


bool ezCommandLineOptionDoc::ShouldLog(LogMode mode, bool bWasSpecified) const
{
  if (mode == LogMode::Never)
    return false;

  if (m_bLoggedOnce && (mode == LogMode::FirstTime || mode == LogMode::FirstTimeIfSpecified))
    return false;

  if (!bWasSpecified && (mode == LogMode::FirstTimeIfSpecified || mode == LogMode::AlwaysIfSpecified))
    return false;

  return true;
}

void ezCommandLineOptionDoc::LogOption(const char* szOption, const char* szValue, bool bWasSpecified) const
{
  m_bLoggedOnce = true;

  if (bWasSpecified)
  {
    ezLog::Info("Option '{}' is set to '{}'", szOption, szValue);
  }
  else
  {
    ezLog::Info("Option '{}' is not set, default value is '{}'", szOption, szValue);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCommandLineOptionBool::ezCommandLineOptionBool(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, bool bDefaultValue, bool bCaseSensitive /*= false*/)
  : ezCommandLineOptionDoc(szSortingGroup, szArgument, "<bool>", szLongDesc, bDefaultValue ? "true" : "false", bCaseSensitive)
{
  m_bDefaultValue = bDefaultValue;
}

bool ezCommandLineOptionBool::GetOptionValue(LogMode logMode, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  bool result = m_bDefaultValue;

  ezStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetBoolOption(sOption, m_bDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result ? "true" : "false", bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCommandLineOptionInt::ezCommandLineOptionInt(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, int iDefaultValue, int iMinValue /*= ezMath::MinValue<int>()*/, int iMaxValue /*= ezMath::MaxValue<int>()*/, bool bCaseSensitive /*= false*/)
  : ezCommandLineOptionDoc(szSortingGroup, szArgument, "<int>", szLongDesc, "0", bCaseSensitive)
{
  m_iDefaultValue = iDefaultValue;
  m_iMinValue = iMinValue;
  m_iMaxValue = iMaxValue;

  EZ_ASSERT_DEV(m_iMinValue < m_iMaxValue, "Invalid min/max value");
}

void ezCommandLineOptionInt::GetParamDefaultValueDesc(ezStringBuilder& ref_sOut) const
{
  ref_sOut.Format("{}", m_iDefaultValue);
}


void ezCommandLineOptionInt::GetParamShortDesc(ezStringBuilder& ref_sOut) const
{
  if (m_iMinValue == ezMath::MinValue<int>() && m_iMaxValue == ezMath::MaxValue<int>())
  {
    ref_sOut = "<int>";
  }
  else
  {
    ref_sOut.Format("<int> [{} .. {}]", m_iMinValue, m_iMaxValue);
  }
}

int ezCommandLineOptionInt::GetOptionValue(LogMode logMode, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  int result = m_iDefaultValue;

  ezStringBuilder sOption, tmp;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetIntOption(sOption, m_iDefaultValue, m_bCaseSensitive);

    if (result < m_iMinValue || result > m_iMaxValue)
    {
      if (ShouldLog(logMode, bSpecified))
      {
        ezLog::Warning("Option '{}' selected value '{}' is outside valid range [{} .. {}]. Using default value instead.", sOption, result, m_iMinValue, m_iMaxValue);
      }

      result = m_iDefaultValue;
    }
  }

  if (ShouldLog(logMode, bSpecified))
  {
    tmp.Format("{}", result);
    LogOption(sOption, tmp, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCommandLineOptionFloat::ezCommandLineOptionFloat(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, float fDefaultValue, float fMinValue /*= ezMath::MinValue<float>()*/, float fMaxValue /*= ezMath::MaxValue<float>()*/, bool bCaseSensitive /*= false*/)
  : ezCommandLineOptionDoc(szSortingGroup, szArgument, "<float>", szLongDesc, "0", bCaseSensitive)
{
  m_fDefaultValue = fDefaultValue;
  m_fMinValue = fMinValue;
  m_fMaxValue = fMaxValue;

  EZ_ASSERT_DEV(m_fMinValue < m_fMaxValue, "Invalid min/max value");
}

void ezCommandLineOptionFloat::GetParamDefaultValueDesc(ezStringBuilder& ref_sOut) const
{
  ref_sOut.Format("{}", m_fDefaultValue);
}

void ezCommandLineOptionFloat::GetParamShortDesc(ezStringBuilder& ref_sOut) const
{
  if (m_fMinValue == ezMath::MinValue<float>() && m_fMaxValue == ezMath::MaxValue<float>())
  {
    ref_sOut = "<float>";
  }
  else
  {
    ref_sOut.Format("<float> [{} .. {}]", m_fMinValue, m_fMaxValue);
  }
}

float ezCommandLineOptionFloat::GetOptionValue(LogMode logMode, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  float result = m_fDefaultValue;

  ezStringBuilder sOption, tmp;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = static_cast<float>(pUtils->GetFloatOption(sOption, m_fDefaultValue, m_bCaseSensitive));

    if (result < m_fMinValue || result > m_fMaxValue)
    {
      if (ShouldLog(logMode, bSpecified))
      {
        ezLog::Warning("Option '{}' selected value '{}' is outside valid range [{} .. {}]. Using default value instead.", sOption, result, m_fMinValue, m_fMaxValue);
      }

      result = m_fDefaultValue;
    }
  }

  if (ShouldLog(logMode, bSpecified))
  {
    tmp.Format("{}", result);
    LogOption(sOption, tmp, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCommandLineOptionString::ezCommandLineOptionString(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive /*= false*/)
  : ezCommandLineOptionDoc(szSortingGroup, szArgument, "<string>", szLongDesc, szDefaultValue, bCaseSensitive)
{
  m_szDefaultValue = szDefaultValue;
}

const char* ezCommandLineOptionString::GetOptionValue(LogMode logMode, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  const char* result = m_szDefaultValue;

  ezStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetStringOption(sOption, 0, m_szDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result, bSpecified);
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCommandLineOptionPath::ezCommandLineOptionPath(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szDefaultValue, bool bCaseSensitive /*= false*/)
  : ezCommandLineOptionDoc(szSortingGroup, szArgument, "<path>", szLongDesc, szDefaultValue, bCaseSensitive)
{
  m_szDefaultValue = szDefaultValue;
}

ezString ezCommandLineOptionPath::GetOptionValue(LogMode logMode, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  ezString result = m_szDefaultValue;

  ezStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  if (bSpecified)
  {
    result = pUtils->GetAbsolutePathOption(sOption, 0, m_szDefaultValue, m_bCaseSensitive);
  }

  if (ShouldLog(logMode, bSpecified))
  {
    LogOption(sOption, result, bSpecified);
  }

  return result;
}

ezCommandLineOptionEnum::ezCommandLineOptionEnum(const char* szSortingGroup, const char* szArgument, const char* szLongDesc, const char* szEnumKeysAndValues, ezInt32 iDefaultValue, bool bCaseSensitive /*= false*/)
  : ezCommandLineOptionDoc(szSortingGroup, szArgument, "<enum>", szLongDesc, "", bCaseSensitive)
{
  m_iDefaultValue = iDefaultValue;
  m_szEnumKeysAndValues = szEnumKeysAndValues;
}

ezInt32 ezCommandLineOptionEnum::GetOptionValue(LogMode logMode, const ezCommandLineUtils* pUtils /*= ezCommandLineUtils::GetGlobalInstance()*/) const
{
  ezInt32 result = m_iDefaultValue;

  ezStringBuilder sOption;
  const bool bSpecified = IsOptionSpecified(&sOption, pUtils);

  ezHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  if (bSpecified)
  {
    const char* selected = pUtils->GetStringOption(sOption, 0, "", m_bCaseSensitive);

    for (const auto& e : keysAndValues)
    {
      if (e.m_Key.IsEqual_NoCase(selected))
      {
        result = e.m_iValue;
        goto found;
      }
    }

    if (ShouldLog(logMode, bSpecified))
    {
      ezLog::Warning("Option '{}' selected value '{}' is unknown. Using default value instead.", sOption, selected);
    }
  }

found:

  if (ShouldLog(logMode, bSpecified))
  {
    ezStringBuilder opt;

    for (const auto& e : keysAndValues)
    {
      if (e.m_iValue == result)
      {
        opt = e.m_Key;
        break;
      }
    }

    LogOption(sOption, opt, bSpecified);
  }

  return result;
}

void ezCommandLineOptionEnum::GetParamShortDesc(ezStringBuilder& ref_sOut) const
{
  ezHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  for (const auto& e : keysAndValues)
  {
    ref_sOut.AppendWithSeparator(" | ", e.m_Key);
  }

  ref_sOut.Prepend("<");
  ref_sOut.Append(">");
}

void ezCommandLineOptionEnum::GetParamDefaultValueDesc(ezStringBuilder& ref_sOut) const
{
  ezHybridArray<EnumKeyValue, 16> keysAndValues;
  GetEnumKeysAndValues(keysAndValues);

  for (const auto& e : keysAndValues)
  {
    if (m_iDefaultValue == e.m_iValue)
    {
      ref_sOut = e.m_Key;
      return;
    }
  }
}

void ezCommandLineOptionEnum::GetEnumKeysAndValues(ezDynamicArray<EnumKeyValue>& out_keysAndValues) const
{
  ezStringBuilder tmp = m_szEnumKeysAndValues;

  ezHybridArray<ezStringView, 16> enums;
  tmp.Split(false, enums, ";", "|");

  out_keysAndValues.SetCount(enums.GetCount());

  ezInt32 eVal = 0;
  for (ezUInt32 e = 0; e < enums.GetCount(); ++e)
  {
    ezStringView eName;

    if (const char* eq = enums[e].FindSubString("="))
    {
      eName = ezStringView(enums[e].GetStartPointer(), eq);

      EZ_VERIFY(ezConversionUtils::StringToInt(eq + 1, eVal).Succeeded(), "Invalid enum declaration");
    }
    else
    {
      eName = enums[e];
    }

    eName.Trim(" \n\r\t=");

    const char* pStart = m_szEnumKeysAndValues;
    pStart += (ezInt64)eName.GetStartPointer();
    pStart -= (ezInt64)tmp.GetData();

    out_keysAndValues[e].m_iValue = eVal;
    out_keysAndValues[e].m_Key = ezStringView(pStart, eName.GetElementCount());

    eVal++;
  }
}


EZ_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_CommandLineOptions);
