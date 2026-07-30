//
// Created by castle on 7/31/26.
//

#ifndef CPP_REFLECT_TEST_TYPES_H
#define CPP_REFLECT_TEST_TYPES_H

struct Empty {};

struct Point {
  int    x;
  double y;
};

//*** Types for testing INHERITANCE

struct NoBases {};

struct PublicBase {};
struct ProtectedBase {};
struct PrivateBase {};
struct AccessTest : public PublicBase, protected ProtectedBase, private PrivateBase {};

struct VirtualBase {};
struct VirtualPublic : public virtual VirtualBase {};
struct VirtualProtected : protected virtual VirtualBase {};
struct VirtualPrivate : private virtual VirtualBase {};

struct NonVirtualBase {};
struct MixedInheritance : public NonVirtualBase, protected virtual VirtualBase {};

struct MultipleInheritance : public PublicBase,
                             protected ProtectedBase,
                             private PrivateBase,
                             public virtual VirtualBase {};

struct GrandParent {};
struct Parent : public GrandParent {};
struct Child : public Parent {};


//*** Types for testing ENUMS
enum PlainEnum {
  PlainZero,
  PlainOne,
  PlainFive = 5,
  PlainSix,
};

enum class ScopedEnum {
  First,
  Second = 42,
  Third = -7,
};

enum class ExplicitUnderlying : std::int8_t {
  Min = -128,
  Negative = -42,
  Zero = 0,
  Positive = 42,
  Max = 127,
};

enum class UnsignedUnderlying : std::uint64_t {
  Zero = 0,
  One = 1,
  Large = UINT64_C(0xFEDCBA9876543210),
};

enum class DuplicateValues {
  First = 1,
  AlsoFirst = 1,
  Second = 2,
};

enum class ExtremeSigned : std::int64_t {
  Min = std::numeric_limits<std::int64_t>::min(),
  Max = std::numeric_limits<std::int64_t>::max(),
};

#endif // CPP_REFLECT_TEST_TYPES_H
