#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/RTTI.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Uuid.h>
#include <Foundation/Types/Variant.h>
#include <Foundation/Types/VariantTypeRegistry.h>

ezResult ezOpenDdlUtils::ConvertToColor(const ezOpenDdlReaderElement* pElement, ezColor& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = pValues[3];

      return EZ_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = 1.0f;

      return EZ_SUCCESS;
    }
  }
  else if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::UInt8)
  {
    const ezUInt8* pValues = pElement->GetPrimitivesUInt8();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result = ezColorGammaUB(pValues[0], pValues[1], pValues[2], pValues[3]);

      return EZ_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result = ezColorGammaUB(pValues[0], pValues[1], pValues[2]);

      return EZ_SUCCESS;
    }
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToColorGamma(const ezOpenDdlReaderElement* pElement, ezColorGammaUB& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result = ezColor(pValues[0], pValues[1], pValues[2], pValues[3]);

      return EZ_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result = ezColor(pValues[0], pValues[1], pValues[2]);

      return EZ_SUCCESS;
    }
  }
  else if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::UInt8)
  {
    const ezUInt8* pValues = pElement->GetPrimitivesUInt8();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = pValues[3];

      return EZ_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = 255;

      return EZ_SUCCESS;
    }
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToTime(const ezOpenDdlReaderElement* pElement, ezTime& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_result = ezTime::Seconds(pValues[0]);

    return EZ_SUCCESS;
  }

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Double)
  {
    const double* pValues = pElement->GetPrimitivesDouble();

    out_result = ezTime::Seconds(pValues[0]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec2(const ezOpenDdlReaderElement* pElement, ezVec2& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec3(const ezOpenDdlReaderElement* pElement, ezVec3& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec4(const ezOpenDdlReaderElement* pElement, ezVec4& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec2I(const ezOpenDdlReaderElement* pElement, ezVec2I32& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Int32)
  {
    const ezInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec3I(const ezOpenDdlReaderElement* pElement, ezVec3I32& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Int32)
  {
    const ezInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec4I(const ezOpenDdlReaderElement* pElement, ezVec4I32& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Int32)
  {
    const ezInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}


ezResult ezOpenDdlUtils::ConvertToVec2U(const ezOpenDdlReaderElement* pElement, ezVec2U32& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::UInt32)
  {
    const ezUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec3U(const ezOpenDdlReaderElement* pElement, ezVec3U32& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::UInt32)
  {
    const ezUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVec4U(const ezOpenDdlReaderElement* pElement, ezVec4U32& out_vResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::UInt32)
  {
    const ezUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}



ezResult ezOpenDdlUtils::ConvertToMat3(const ezOpenDdlReaderElement* pElement, ezMat3& out_mResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 9)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_mResult.SetFromArray(pValues, ezMatrixLayout::ColumnMajor);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToMat4(const ezOpenDdlReaderElement* pElement, ezMat4& out_mResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 16)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_mResult.SetFromArray(pValues, ezMatrixLayout::ColumnMajor);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}


ezResult ezOpenDdlUtils::ConvertToTransform(const ezOpenDdlReaderElement* pElement, ezTransform& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 10)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_result.m_vPosition.x = pValues[0];
    out_result.m_vPosition.y = pValues[1];
    out_result.m_vPosition.z = pValues[2];
    out_result.m_qRotation.v.x = pValues[3];
    out_result.m_qRotation.v.y = pValues[4];
    out_result.m_qRotation.v.z = pValues[5];
    out_result.m_qRotation.w = pValues[6];
    out_result.m_vScale.x = pValues[7];
    out_result.m_vScale.y = pValues[8];
    out_result.m_vScale.z = pValues[9];

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToQuat(const ezOpenDdlReaderElement* pElement, ezQuat& out_qResult)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_qResult.SetElements(pValues[0], pValues[1], pValues[2], pValues[3]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToUuid(const ezOpenDdlReaderElement* pElement, ezUuid& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::UInt64)
  {
    const ezUInt64* pValues = pElement->GetPrimitivesUInt64();

    out_result = ezUuid(pValues[0], pValues[1]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToAngle(const ezOpenDdlReaderElement* pElement, ezAngle& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return EZ_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return EZ_FAILURE;

  if (pElement->GetPrimitivesType() == ezOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    // have to use radians to prevent precision loss
    out_result = ezAngle::Radian(pValues[0]);

    return EZ_SUCCESS;
  }

  return EZ_FAILURE;
}

ezResult ezOpenDdlUtils::ConvertToVariant(const ezOpenDdlReaderElement* pElement, ezVariant& out_result)
{
  if (pElement == nullptr)
    return EZ_FAILURE;

  // expect a custom type
  if (pElement->IsCustomType())
  {
    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "VarArray"))
    {
      ezVariantArray value;
      ezVariant varChild;

      /// \test This is just quickly hacked
      /// \todo Store array size for reserving var array length

      for (const ezOpenDdlReaderElement* pChild = pElement->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
      {
        if (ConvertToVariant(pChild, varChild).Failed())
          return EZ_FAILURE;

        value.PushBack(varChild);
      }

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "VarDict"))
    {
      ezVariantDictionary value;
      ezVariant varChild;

      /// \test This is just quickly hacked
      /// \todo Store array size for reserving var array length

      for (const ezOpenDdlReaderElement* pChild = pElement->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
      {
        // no name -> invalid dictionary entry
        if (!pChild->HasName())
          continue;

        if (ConvertToVariant(pChild, varChild).Failed())
          return EZ_FAILURE;

        value[pChild->GetName()] = varChild;
      }

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "VarDataBuffer"))
    {
      /// \test This is just quickly hacked

      ezDataBuffer value;

      const ezOpenDdlReaderElement* pString = pElement->GetFirstChild();

      if (!pString->HasPrimitives(ezOpenDdlPrimitiveType::String))
        return EZ_FAILURE;

      const ezStringView* pValues = pString->GetPrimitivesString();

      value.SetCountUninitialized(pValues[0].GetElementCount() / 2);
      ezConversionUtils::ConvertHexToBinary(pValues[0].GetStartPointer(), value.GetData(), value.GetCount());

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Color"))
    {
      ezColor value;
      if (ConvertToColor(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "ColorGamma"))
    {
      ezColorGammaUB value;
      if (ConvertToColorGamma(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Time"))
    {
      ezTime value;
      if (ConvertToTime(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec2"))
    {
      ezVec2 value;
      if (ConvertToVec2(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec3"))
    {
      ezVec3 value;
      if (ConvertToVec3(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec4"))
    {
      ezVec4 value;
      if (ConvertToVec4(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec2i"))
    {
      ezVec2I32 value;
      if (ConvertToVec2I(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec3i"))
    {
      ezVec3I32 value;
      if (ConvertToVec3I(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec4i"))
    {
      ezVec4I32 value;
      if (ConvertToVec4I(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec2u"))
    {
      ezVec2U32 value;
      if (ConvertToVec2U(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec3u"))
    {
      ezVec3U32 value;
      if (ConvertToVec3U(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Vec4u"))
    {
      ezVec4U32 value;
      if (ConvertToVec4U(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Mat3"))
    {
      ezMat3 value;
      if (ConvertToMat3(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Mat4"))
    {
      ezMat4 value;
      if (ConvertToMat4(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Transform"))
    {
      ezTransform value;
      if (ConvertToTransform(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Quat"))
    {
      ezQuat value;
      if (ConvertToQuat(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Uuid"))
    {
      ezUuid value;
      if (ConvertToUuid(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (ezStringUtils::IsEqual(pElement->GetCustomType(), "Angle"))
    {
      ezAngle value;
      if (ConvertToAngle(pElement, value).Failed())
        return EZ_FAILURE;

      out_result = value;
      return EZ_SUCCESS;
    }

    if (const ezRTTI* pRTTI = ezRTTI::FindTypeByName(pElement->GetCustomType()))
    {
      if (ezVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pRTTI))
      {
        if (pElement == nullptr)
          return EZ_FAILURE;

        void* pObject = pRTTI->GetAllocator()->Allocate<void>();

        for (const ezOpenDdlReaderElement* pChildElement = pElement->GetFirstChild(); pChildElement != nullptr; pChildElement = pChildElement->GetSibling())
        {
          if (!pChildElement->HasName())
            continue;

          if (ezAbstractProperty* pProp = pRTTI->FindPropertyByName(pChildElement->GetName()))
          {
            // Custom types should be POD and only consist of member properties.
            if (pProp->GetCategory() == ezPropertyCategory::Member)
            {
              ezVariant subValue;
              if (ConvertToVariant(pChildElement, subValue).Succeeded())
              {
                ezReflectionUtils::SetMemberPropertyValue(static_cast<ezAbstractMemberProperty*>(pProp), pObject, subValue);
              }
            }
          }
        }
        out_result.MoveTypedObject(pObject, pRTTI);
        return EZ_SUCCESS;
      }
      else
      {
        ezLog::Error("The type '{0}' was declared but not defined, add EZ_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", pElement->GetCustomType());
      }
    }
    else
    {
      ezLog::Error("The type '{0}' is unknown.", pElement->GetCustomType());
    }
  }
  else
  {
    // always expect exactly one value
    if (pElement->GetNumPrimitives() != 1)
      return EZ_FAILURE;

    switch (pElement->GetPrimitivesType())
    {
      case ezOpenDdlPrimitiveType::Bool:
        out_result = pElement->GetPrimitivesBool()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::Int8:
        out_result = pElement->GetPrimitivesInt8()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::Int16:
        out_result = pElement->GetPrimitivesInt16()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::Int32:
        out_result = pElement->GetPrimitivesInt32()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::Int64:
        out_result = pElement->GetPrimitivesInt64()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::UInt8:
        out_result = pElement->GetPrimitivesUInt8()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::UInt16:
        out_result = pElement->GetPrimitivesUInt16()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::UInt32:
        out_result = pElement->GetPrimitivesUInt32()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::UInt64:
        out_result = pElement->GetPrimitivesUInt64()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::Float:
        out_result = pElement->GetPrimitivesFloat()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::Double:
        out_result = pElement->GetPrimitivesDouble()[0];
        return EZ_SUCCESS;

      case ezOpenDdlPrimitiveType::String:
        out_result = ezString(pElement->GetPrimitivesString()[0]); // make sure this isn't stored as a string view by copying to to an ezString first
        return EZ_SUCCESS;

      default:
        EZ_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }

  return EZ_FAILURE;
}

void ezOpenDdlUtils::StoreColor(ezOpenDdlWriter& ref_writer, const ezColor& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Color", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreColorGamma(
  ezOpenDdlWriter& ref_writer, const ezColorGammaUB& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("ColorGamma", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt8);
    ref_writer.WriteUInt8(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreTime(ezOpenDdlWriter& ref_writer, const ezTime& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Time", szName, bGlobalName, true);
  {
    const double d = value.GetSeconds();

    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Double);
    ref_writer.WriteDouble(&d, 1);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec2(ezOpenDdlWriter& ref_writer, const ezVec2& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec3(ezOpenDdlWriter& ref_writer, const ezVec3& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec4(ezOpenDdlWriter& ref_writer, const ezVec4& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec2I(ezOpenDdlWriter& ref_writer, const ezVec2I32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2i", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec3I(ezOpenDdlWriter& ref_writer, const ezVec3I32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3i", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec4I(ezOpenDdlWriter& ref_writer, const ezVec4I32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4i", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec2U(ezOpenDdlWriter& ref_writer, const ezVec2U32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2u", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec3U(ezOpenDdlWriter& ref_writer, const ezVec3U32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3u", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVec4U(ezOpenDdlWriter& ref_writer, const ezVec4U32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4u", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}


void ezOpenDdlUtils::StoreMat3(ezOpenDdlWriter& ref_writer, const ezMat3& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Mat3", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);

    float f[9];
    value.GetAsArray(f, ezMatrixLayout::ColumnMajor);
    ref_writer.WriteFloat(f, 9);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreMat4(ezOpenDdlWriter& ref_writer, const ezMat4& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Mat4", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);

    float f[16];
    value.GetAsArray(f, ezMatrixLayout::ColumnMajor);
    ref_writer.WriteFloat(f, 16);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreTransform(ezOpenDdlWriter& ref_writer, const ezTransform& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Transform", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);

    float f[10];

    f[0] = value.m_vPosition.x;
    f[1] = value.m_vPosition.y;
    f[2] = value.m_vPosition.z;

    f[3] = value.m_qRotation.v.x;
    f[4] = value.m_qRotation.v.y;
    f[5] = value.m_qRotation.v.z;
    f[6] = value.m_qRotation.w;

    f[7] = value.m_vScale.x;
    f[8] = value.m_vScale.y;
    f[9] = value.m_vScale.z;

    ref_writer.WriteFloat(f, 10);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreQuat(ezOpenDdlWriter& ref_writer, const ezQuat& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Quat", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.v.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreUuid(ezOpenDdlWriter& ref_writer, const ezUuid& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Uuid", szName, bGlobalName, true);
  {
    ezUInt64 ui[2];
    value.GetValues(ui[0], ui[1]);

    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt64);
    ref_writer.WriteUInt64(ui, 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreAngle(ezOpenDdlWriter& ref_writer, const ezAngle& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Angle", szName, bGlobalName, true);
  {
    // have to use radians to prevent precision loss
    const float f = value.GetRadian();

    ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(&f, 1);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void ezOpenDdlUtils::StoreVariant(ezOpenDdlWriter& ref_writer, const ezVariant& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  switch (value.GetType())
  {
    case ezVariant::Type::Invalid:
      return; // store anything ?

    case ezVariant::Type::Bool:
    {
      StoreBool(ref_writer, value.Get<bool>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Int8:
    {
      StoreInt8(ref_writer, value.Get<ezInt8>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::UInt8:
    {
      StoreUInt8(ref_writer, value.Get<ezUInt8>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Int16:
    {
      StoreInt16(ref_writer, value.Get<ezInt16>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::UInt16:
    {
      StoreUInt16(ref_writer, value.Get<ezUInt16>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Int32:
    {
      StoreInt32(ref_writer, value.Get<ezInt32>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::UInt32:
    {
      StoreUInt32(ref_writer, value.Get<ezUInt32>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Int64:
    {
      StoreInt64(ref_writer, value.Get<ezInt64>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::UInt64:
    {
      StoreUInt64(ref_writer, value.Get<ezUInt64>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Float:
    {
      StoreFloat(ref_writer, value.Get<float>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Double:
    {
      StoreDouble(ref_writer, value.Get<double>(), szName, bGlobalName);
    }
      return;

    case ezVariant::Type::String:
    {
      const ezString& var = value.Get<ezString>();
      ezOpenDdlUtils::StoreString(ref_writer, var, szName, bGlobalName);
    }
      return;

    case ezVariant::Type::StringView:
    {
      const ezStringView& var = value.Get<ezStringView>();
      ezOpenDdlUtils::StoreString(ref_writer, var, szName, bGlobalName);
    }
      return;

    case ezVariant::Type::Color:
      StoreColor(ref_writer, value.Get<ezColor>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector2:
      StoreVec2(ref_writer, value.Get<ezVec2>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector3:
      StoreVec3(ref_writer, value.Get<ezVec3>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector4:
      StoreVec4(ref_writer, value.Get<ezVec4>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector2I:
      StoreVec2I(ref_writer, value.Get<ezVec2I32>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector3I:
      StoreVec3I(ref_writer, value.Get<ezVec3I32>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector4I:
      StoreVec4I(ref_writer, value.Get<ezVec4I32>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector2U:
      StoreVec2U(ref_writer, value.Get<ezVec2U32>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector3U:
      StoreVec3U(ref_writer, value.Get<ezVec3U32>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Vector4U:
      StoreVec4U(ref_writer, value.Get<ezVec4U32>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Quaternion:
      StoreQuat(ref_writer, value.Get<ezQuat>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Matrix3:
      StoreMat3(ref_writer, value.Get<ezMat3>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Matrix4:
      StoreMat4(ref_writer, value.Get<ezMat4>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Transform:
      StoreTransform(ref_writer, value.Get<ezTransform>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Time:
      StoreTime(ref_writer, value.Get<ezTime>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Uuid:
      StoreUuid(ref_writer, value.Get<ezUuid>(), szName, bGlobalName);
      return;

    case ezVariant::Type::Angle:
      StoreAngle(ref_writer, value.Get<ezAngle>(), szName, bGlobalName);
      return;

    case ezVariant::Type::ColorGamma:
      StoreColorGamma(ref_writer, value.Get<ezColorGammaUB>(), szName, bGlobalName);
      return;

    case ezVariant::Type::VariantArray:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarArray", szName, bGlobalName);

      const ezVariantArray& arr = value.Get<ezVariantArray>();
      for (ezUInt32 i = 0; i < arr.GetCount(); ++i)
      {
        ezOpenDdlUtils::StoreVariant(ref_writer, arr[i]);
      }

      ref_writer.EndObject();
    }
      return;

    case ezVariant::Type::VariantDictionary:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarDict", szName, bGlobalName);

      const ezVariantDictionary& dict = value.Get<ezVariantDictionary>();
      for (auto it = dict.GetIterator(); it.IsValid(); ++it)
      {
        ezOpenDdlUtils::StoreVariant(ref_writer, it.Value(), it.Key(), false);
      }

      ref_writer.EndObject();
    }
      return;

    case ezVariant::Type::DataBuffer:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarDataBuffer", szName, bGlobalName);
      ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::String);

      const ezDataBuffer& db = value.Get<ezDataBuffer>();
      ref_writer.WriteBinaryAsString(db.GetData(), db.GetCount());

      ref_writer.EndPrimitiveList();
      ref_writer.EndObject();
    }
      return;

    case ezVariant::Type::TypedObject:
    {
      ezTypedObject obj = value.Get<ezTypedObject>();
      if (ezVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(obj.m_pType))
      {
        ref_writer.BeginObject(obj.m_pType->GetTypeName(), szName, bGlobalName);
        {
          ezHybridArray<ezAbstractProperty*, 32> properties;
          obj.m_pType->GetAllProperties(properties);
          for (const ezAbstractProperty* pProp : properties)
          {
            // Custom types should be POD and only consist of member properties.
            switch (pProp->GetCategory())
            {
              case ezPropertyCategory::Member:
              {
                ezVariant subValue = ezReflectionUtils::GetMemberPropertyValue(static_cast<const ezAbstractMemberProperty*>(pProp), obj.m_pObject);
                StoreVariant(ref_writer, subValue, pProp->GetPropertyName(), false);
              }
              break;
              case ezPropertyCategory::Array:
              case ezPropertyCategory::Set:
              case ezPropertyCategory::Map:
                EZ_REPORT_FAILURE("Only member properties are supported in custom variant types!");
                break;
              case ezPropertyCategory::Constant:
              case ezPropertyCategory::Function:
                break;
            }
          }
        }
        ref_writer.EndObject();
      }
      else
      {
        ezLog::Error("The type '{0}' was declared but not defined, add EZ_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", obj.m_pType->GetTypeName());
      }
    }
      return;
    default:
      EZ_REPORT_FAILURE("Can't write this type of Variant");
  }
}

void ezOpenDdlUtils::StoreString(ezOpenDdlWriter& ref_writer, const ezStringView& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::String, szName, bGlobalName);
  ref_writer.WriteString(value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreBool(ezOpenDdlWriter& ref_writer, bool value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Bool, szName, bGlobalName);
  ref_writer.WriteBool(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreFloat(ezOpenDdlWriter& ref_writer, float value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Float, szName, bGlobalName);
  ref_writer.WriteFloat(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreDouble(ezOpenDdlWriter& ref_writer, double value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Double, szName, bGlobalName);
  ref_writer.WriteDouble(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreInt8(ezOpenDdlWriter& ref_writer, ezInt8 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int8, szName, bGlobalName);
  ref_writer.WriteInt8(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreInt16(ezOpenDdlWriter& ref_writer, ezInt16 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int16, szName, bGlobalName);
  ref_writer.WriteInt16(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreInt32(ezOpenDdlWriter& ref_writer, ezInt32 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int32, szName, bGlobalName);
  ref_writer.WriteInt32(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreInt64(ezOpenDdlWriter& ref_writer, ezInt64 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::Int64, szName, bGlobalName);
  ref_writer.WriteInt64(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreUInt8(ezOpenDdlWriter& ref_writer, ezUInt8 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt8, szName, bGlobalName);
  ref_writer.WriteUInt8(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreUInt16(ezOpenDdlWriter& ref_writer, ezUInt16 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt16, szName, bGlobalName);
  ref_writer.WriteUInt16(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreUInt32(ezOpenDdlWriter& ref_writer, ezUInt32 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt32, szName, bGlobalName);
  ref_writer.WriteUInt32(&value);
  ref_writer.EndPrimitiveList();
}

void ezOpenDdlUtils::StoreUInt64(ezOpenDdlWriter& ref_writer, ezUInt64 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(ezOpenDdlPrimitiveType::UInt64, szName, bGlobalName);
  ref_writer.WriteUInt64(&value);
  ref_writer.EndPrimitiveList();
}



EZ_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_OpenDdlUtils);
