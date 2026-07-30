// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "test_types.h"

import std;
import refl.generation.Reflector;

using namespace refl;

TEST(EnumInfo, IdentifiesEnumType) {
  const auto &type = reflect<PlainEnum>();

  EXPECT_EQ(type.kind, TypeInfo::ENUM);
  EXPECT_EQ(type.name, "PlainEnum");
  EXPECT_EQ(type.size, sizeof(PlainEnum));
  EXPECT_EQ(type.alignment, alignof(PlainEnum));
}

TEST(EnumInfo, IdentifiesScopedness) {
  const auto &plain  = reflect<PlainEnum>().as<TypeInfo::ENUM>();
  const auto &scoped = reflect<ScopedEnum>().as<TypeInfo::ENUM>();

  EXPECT_FALSE(plain.is_scoped);
  EXPECT_TRUE(scoped.is_scoped);
}

TEST(EnumInfo, ReportsUnderlyingType) {
  const auto &info  = reflect<ExplicitUnderlying>().as<TypeInfo::ENUM>();
  const auto &uinfo = reflect<UnsignedUnderlying>().as<TypeInfo::ENUM>();

  EXPECT_EQ(&info.underlying_type, &reflect<std::int8_t>());
  EXPECT_EQ(&uinfo.underlying_type, &reflect<std::uint64_t>());
}

TEST(EnumInfo, EnumeratesAllEnumeratorsInDeclarationOrder) {
  const auto &info = reflect<ScopedEnum>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 3);

  EXPECT_EQ(info.enumerators[0].name, "First");
  EXPECT_EQ(info.enumerators[1].name, "Second");
  EXPECT_EQ(info.enumerators[2].name, "Third");
}

TEST(EnumInfo, ReflectsEnumeratorValues) {
  const auto &info = reflect<PlainEnum>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 4);

  EXPECT_EQ(info.enumerators[0].value.magnitude, 0);
  EXPECT_FALSE(info.enumerators[0].value.is_negative);

  EXPECT_EQ(info.enumerators[1].value.magnitude, 1);
  EXPECT_FALSE(info.enumerators[1].value.is_negative);

  EXPECT_EQ(info.enumerators[2].value.magnitude, 5);
  EXPECT_FALSE(info.enumerators[2].value.is_negative);

  EXPECT_EQ(info.enumerators[3].value.magnitude, 6);
  EXPECT_FALSE(info.enumerators[3].value.is_negative);
}

TEST(EnumInfo, ReflectsNegativeEnumeratorValues) {
  const auto &info = reflect<ScopedEnum>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 3);

  EXPECT_EQ(info.enumerators[0].value.magnitude, 0);
  EXPECT_FALSE(info.enumerators[0].value.is_negative);

  EXPECT_EQ(info.enumerators[1].value.magnitude, 42);
  EXPECT_FALSE(info.enumerators[1].value.is_negative);

  EXPECT_EQ(info.enumerators[2].value.magnitude, 7);
  EXPECT_TRUE(info.enumerators[2].value.is_negative);
}

TEST(EnumInfo, HandlesSignedUnderlyingTypeBoundaries) {
  const auto &info = reflect<ExplicitUnderlying>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 5);

  EXPECT_EQ(info.enumerators[0].value.magnitude, 128);
  EXPECT_TRUE(info.enumerators[0].value.is_negative);

  EXPECT_EQ(info.enumerators[1].value.magnitude, 42);
  EXPECT_TRUE(info.enumerators[1].value.is_negative);

  EXPECT_EQ(info.enumerators[2].value.magnitude, 0);
  EXPECT_FALSE(info.enumerators[2].value.is_negative);

  EXPECT_EQ(info.enumerators[3].value.magnitude, 42);
  EXPECT_FALSE(info.enumerators[3].value.is_negative);

  EXPECT_EQ(info.enumerators[4].value.magnitude, 127);
  EXPECT_FALSE(info.enumerators[4].value.is_negative);
}

TEST(EnumInfo, HandlesMinimumSignedInteger) {
  const auto &info = reflect<ExtremeSigned>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 2);

  EXPECT_EQ(info.enumerators[0].value.magnitude, std::uintmax_t{1} << 63);
  EXPECT_TRUE(info.enumerators[0].value.is_negative);

  EXPECT_EQ(info.enumerators[1].value.magnitude, std::uintmax_t{std::numeric_limits<std::int64_t>::max()});
  EXPECT_FALSE(info.enumerators[1].value.is_negative);
}

TEST(EnumInfo, HandlesLargeUnsignedValues) {
  const auto &info = reflect<UnsignedUnderlying>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 3);

  EXPECT_EQ(info.enumerators[0].value.magnitude, 0);
  EXPECT_FALSE(info.enumerators[0].value.is_negative);

  EXPECT_EQ(info.enumerators[1].value.magnitude, 1);
  EXPECT_FALSE(info.enumerators[1].value.is_negative);

  EXPECT_EQ(info.enumerators[2].value.magnitude, UINT64_C(0xFEDCBA9876543210));
  EXPECT_FALSE(info.enumerators[2].value.is_negative);
}

TEST(EnumInfo, PreservesEnumeratorsWithDuplicateValues) {
  const auto& info = reflect<DuplicateValues>().as<TypeInfo::ENUM>();

  ASSERT_EQ(info.enumerators.size(), 3);

  EXPECT_EQ(info.enumerators[0].name, "First");
  EXPECT_EQ(info.enumerators[1].name, "AlsoFirst");
  EXPECT_EQ(info.enumerators[2].name, "Second");

  EXPECT_EQ(info.enumerators[0].value.magnitude, 1);
  EXPECT_EQ(info.enumerators[1].value.magnitude, 1);
  EXPECT_EQ(info.enumerators[2].value.magnitude, 2);
}