// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "test_types.h"

import std;
import refl.generation.Reflector;

using namespace refl;

TEST(TypeInfo, SameInstanceReturned) { //
  EXPECT_EQ(&reflect<int>(), &reflect<int>());
}

TEST(TypeInfo, ClassMetadata) {
  const auto &info = reflect<Point>();

  EXPECT_EQ(info.kind, TypeInfo::CLASS);

  EXPECT_EQ(info.name, "Point");
  EXPECT_EQ(info.size, sizeof(Point));
  EXPECT_EQ(info.alignment, alignof(Point));

  const auto &class_info = info.as<TypeInfo::CLASS>();

  ASSERT_EQ(class_info.fields.size(), 2);

  EXPECT_EQ(class_info.fields[0].name, "x");
  EXPECT_EQ(class_info.fields[1].name, "y");

  EXPECT_EQ(&class_info.fields[0].type(), &reflect<int>());
  EXPECT_EQ(&class_info.fields[1].type(), &reflect<double>());
}

TEST(TypeInfo, EmptyClass) {
  const auto &info = reflect<Empty>().as<TypeInfo::CLASS>();

  EXPECT_EQ(info.fields.size(), 0);
}

TEST(TypeInfo, PointerMetadata) {
  const auto &info = reflect<Point *>();

  EXPECT_EQ(info.kind, TypeInfo::POINTER);

  EXPECT_EQ(&info.as<TypeInfo::POINTER>().pointee, &reflect<Point>());
}

TEST(TypeInfo, PointerChain) {
  const auto &info = reflect<int ***>();

  EXPECT_EQ(&info //
                     .as<TypeInfo::POINTER>()
                     .pointee //
                     .as<TypeInfo::POINTER>()
                     .pointee //
                     .as<TypeInfo::POINTER>()
                     .pointee,
            &reflect<int>());
}


TEST(BaseClasses, NoBases) {
  const auto &type = refl::reflect<NoBases>();
  EXPECT_TRUE(type.as<TypeInfo::CLASS>().base_classes.empty());
}

TEST(BaseClasses, BaseTypes) {
  const auto &type  = refl::reflect<AccessTest>();
  const auto &bases = type.as<TypeInfo::CLASS>().base_classes;

  ASSERT_EQ(bases.size(), 3);

  EXPECT_EQ(&bases[0].type(), &reflect<PublicBase>());
  EXPECT_EQ(&bases[1].type(), &reflect<ProtectedBase>());
  EXPECT_EQ(&bases[2].type(), &reflect<PrivateBase>());
}

TEST(BaseClasses, AccessSpecifiers) {
  const auto &type  = refl::reflect<AccessTest>();
  const auto &bases = type.as<TypeInfo::CLASS>().base_classes;

  ASSERT_EQ(bases.size(), 3);

  EXPECT_EQ(bases[0].access, AccessSpec::PUBLIC);
  EXPECT_EQ(bases[1].access, AccessSpec::PROTECTED);
  EXPECT_EQ(bases[2].access, AccessSpec::PRIVATE);
}

TEST(BaseClasses, VirtualInheritance) {
  {
    const auto &bases = refl::reflect<VirtualPublic>().as<TypeInfo::CLASS>().base_classes;

    ASSERT_EQ(bases.size(), 1);
    EXPECT_TRUE(bases[0].is_virtual);
    EXPECT_EQ(bases[0].access, AccessSpec::PUBLIC);
  }
  {
    const auto &bases = refl::reflect<VirtualProtected>().as<TypeInfo::CLASS>().base_classes;

    ASSERT_EQ(bases.size(), 1);
    EXPECT_TRUE(bases[0].is_virtual);
    EXPECT_EQ(bases[0].access, AccessSpec::PROTECTED);
  }
  {
    const auto &bases = refl::reflect<VirtualPrivate>().as<TypeInfo::CLASS>().base_classes;

    ASSERT_EQ(bases.size(), 1);
    EXPECT_TRUE(bases[0].is_virtual);
    EXPECT_EQ(bases[0].access, AccessSpec::PRIVATE);
  }
}

TEST(BaseClasses, MixedVirtualInheritance) {
  const auto &bases = refl::reflect<MixedInheritance>().as<TypeInfo::CLASS>().base_classes;

  ASSERT_EQ(bases.size(), 2);

  EXPECT_FALSE(bases[0].is_virtual);
  EXPECT_EQ(bases[0].access, AccessSpec::PUBLIC);
  EXPECT_TRUE(bases[1].is_virtual);
  EXPECT_EQ(bases[1].access, AccessSpec::PROTECTED);
}

TEST(BaseClasses, MultipleInheritance) {
  const auto &bases = refl::reflect<MultipleInheritance>().as<TypeInfo::CLASS>().base_classes;

  ASSERT_EQ(bases.size(), 4);

  EXPECT_EQ(bases[0].access, AccessSpec::PUBLIC);
  EXPECT_FALSE(bases[0].is_virtual);

  EXPECT_EQ(bases[1].access, AccessSpec::PROTECTED);
  EXPECT_FALSE(bases[1].is_virtual);

  EXPECT_EQ(bases[2].access, AccessSpec::PRIVATE);
  EXPECT_FALSE(bases[2].is_virtual);

  EXPECT_EQ(bases[3].access, AccessSpec::PUBLIC);
  EXPECT_TRUE(bases[3].is_virtual);
}

TEST(BaseClasses, ReportsOnlyDirectBases) {
  const auto &bases = refl::reflect<Child>().as<TypeInfo::CLASS>().base_classes;

  ASSERT_EQ(bases.size(), 1);
  EXPECT_EQ(&bases[0].type(), &refl::reflect<Parent>());
}
