// SPDX-License-Identifier: LGPL-2.1-or-later

#include <gtest/gtest.h>

#include <Base/Bitmask.h>
#include <type_traits>

enum class TestFlagEnum
{
    None  = 0,
    Flag1 = 1 << 0,
    Flag2 = 1 << 1,
    Flag3 = 1 << 2,
    All   = Flag1 | Flag2 | Flag3
};
ENABLE_BITMASK_OPERATORS(TestFlagEnum)

// ---- Compile-time constexpr validation --------------------------------------

// Free bitwise operators
static_assert((TestFlagEnum::Flag1 | TestFlagEnum::Flag2) == static_cast<TestFlagEnum>(3), "operator|");
static_assert(((TestFlagEnum::Flag1 | TestFlagEnum::Flag2) & TestFlagEnum::Flag1) == TestFlagEnum::Flag1, "operator&");
static_assert(((TestFlagEnum::Flag1 | TestFlagEnum::Flag2) ^ TestFlagEnum::Flag1) == TestFlagEnum::Flag2, "operator^");
static_assert(~TestFlagEnum::None != TestFlagEnum::None, "operator~");

// Flags class methods
static_assert(Base::Flags<TestFlagEnum>().toUnderlyingType() == 0, "Default constructor / toUnderlyingType");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1).getFlags() == TestFlagEnum::Flag1, "Value constructor / getFlags");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1).testFlag(TestFlagEnum::Flag1), "testFlag true");
static_assert(!Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1).testFlag(TestFlagEnum::Flag2), "testFlag false");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::None).testFlag(TestFlagEnum::None), "testFlag None (empty)");
static_assert(!Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1).testFlag(TestFlagEnum::None), "testFlag None (non-empty)");

static_assert((Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1) | TestFlagEnum::Flag2).getFlags() == static_cast<TestFlagEnum>(3), "Flags operator|");
static_assert((Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1 | TestFlagEnum::Flag2) & TestFlagEnum::Flag1).getFlags() == TestFlagEnum::Flag1, "Flags operator&");
static_assert((Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1) ^ TestFlagEnum::Flag2).getFlags() == static_cast<TestFlagEnum>(3), "Flags operator^");
static_assert((~Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1)).testFlag(TestFlagEnum::Flag2), "Flags operator~");
static_assert(!Base::Flags<TestFlagEnum>(TestFlagEnum::None), "Flags operator!");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1).isEqual(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1)), "isEqual");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1) == Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1), "operator==");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1) != Base::Flags<TestFlagEnum>(TestFlagEnum::Flag2), "operator!=");
static_assert(Base::Flags<TestFlagEnum>(TestFlagEnum::Flag1) == TestFlagEnum::Flag1, "operator== enum");

// ---- Alternative underlying types -------------------------------------------

enum class U64FlagEnum : uint64_t
{
    None  = 0,
    Flag1 = 1ULL << 0,
    Flag2 = 1ULL << 1,
    Flag63 = 1ULL << 63
};
ENABLE_BITMASK_OPERATORS(U64FlagEnum)

class BitmaskTest: public ::testing::Test
{
protected:
    // void SetUp() override {};
    // void TearDown() override {};
};

// ---- Alternative underlying types -------------------------------------------

TEST_F(BitmaskTest, alternativeUnderlyingType64Bit)
{
    Base::Flags<U64FlagEnum> flags(U64FlagEnum::Flag1 | U64FlagEnum::Flag63);
    EXPECT_TRUE(flags.testFlag(U64FlagEnum::Flag1));
    EXPECT_TRUE(flags.testFlag(U64FlagEnum::Flag63));
    EXPECT_EQ(flags.toUnderlyingType(), (1ULL << 0) | (1ULL << 63));
    EXPECT_EQ(sizeof(flags.toUnderlyingType()), 8);
}

// ---- Free bitwise operators (ENABLE_BITMASK_OPERATORS) ----------------------

TEST_F(BitmaskTest, freeOperatorOr)
{
    auto result = TestFlagEnum::Flag1 | TestFlagEnum::Flag2;
    EXPECT_EQ(static_cast<int>(result), 3);
}

TEST_F(BitmaskTest, freeOperatorAnd)
{
    TestFlagEnum combined = TestFlagEnum::Flag1 | TestFlagEnum::Flag2;
    auto result = combined & TestFlagEnum::Flag1;
    EXPECT_EQ(result, TestFlagEnum::Flag1);
}

TEST_F(BitmaskTest, freeOperatorXor)
{
    TestFlagEnum combined = TestFlagEnum::Flag1 | TestFlagEnum::Flag2;
    auto result = combined ^ TestFlagEnum::Flag1;
    EXPECT_EQ(result, TestFlagEnum::Flag2);
}

TEST_F(BitmaskTest, freeOperatorNot)
{
    // ~None should flip all bits; result must be non-zero
    auto result = ~TestFlagEnum::None;
    EXPECT_NE(static_cast<int>(result), 0);
}

TEST_F(BitmaskTest, freeOperatorOrAssign)
{
    TestFlagEnum flags = TestFlagEnum::Flag1;
    flags |= TestFlagEnum::Flag2;
    EXPECT_EQ(static_cast<int>(flags), 3);
}

TEST_F(BitmaskTest, freeOperatorAndAssign)
{
    TestFlagEnum flags = TestFlagEnum::Flag1 | TestFlagEnum::Flag2;
    flags &= TestFlagEnum::Flag1;
    EXPECT_EQ(flags, TestFlagEnum::Flag1);
}

TEST_F(BitmaskTest, freeOperatorXorAssign)
{
    TestFlagEnum flags = TestFlagEnum::Flag1 | TestFlagEnum::Flag2;
    flags ^= TestFlagEnum::Flag1;
    EXPECT_EQ(flags, TestFlagEnum::Flag2);
}

// ---- Flags<Enum> construction -----------------------------------------------

TEST_F(BitmaskTest, defaultConstructionIsZero)
{
    Base::Flags<TestFlagEnum> flags;
    EXPECT_EQ(flags.toUnderlyingType(), 0);
    EXPECT_FALSE(static_cast<bool>(flags));
}

TEST_F(BitmaskTest, constructionWithEnumValue)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1);
    EXPECT_EQ(flags.getFlags(), TestFlagEnum::Flag1);
    EXPECT_TRUE(static_cast<bool>(flags));
}

TEST_F(BitmaskTest, copyConstruction)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag1);
    Base::Flags<TestFlagEnum> b(a);
    EXPECT_TRUE(b.testFlag(TestFlagEnum::Flag1));
    EXPECT_EQ(a, b);
}

TEST_F(BitmaskTest, copyAssignment)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag1);
    Base::Flags<TestFlagEnum> b;
    b = a;
    EXPECT_TRUE(b.testFlag(TestFlagEnum::Flag1));
    EXPECT_EQ(a, b);
}

TEST_F(BitmaskTest, moveConstruction)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag2);
    Base::Flags<TestFlagEnum> b(std::move(a));
    EXPECT_TRUE(b.testFlag(TestFlagEnum::Flag2));
    EXPECT_EQ(b.getFlags(), TestFlagEnum::Flag2);
}

TEST_F(BitmaskTest, moveAssignment)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag3);
    Base::Flags<TestFlagEnum> b;
    b = std::move(a);
    EXPECT_TRUE(b.testFlag(TestFlagEnum::Flag3));
    EXPECT_EQ(b.getFlags(), TestFlagEnum::Flag3);
}

// ---- toUnderlyingType -------------------------------------------------------

TEST_F(BitmaskTest, toUnderlyingType)
{
    Base::Flags<TestFlagEnum> flag1(TestFlagEnum::Flag1);
    auto result = flag1.toUnderlyingType();
    EXPECT_EQ(typeid(result), typeid(std::underlying_type<TestFlagEnum>::type));
    EXPECT_EQ(result, 1);
}

// ---- testFlag ---------------------------------------------------------------

TEST_F(BitmaskTest, testFlagReturnsTrueWhenSet)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag2));
}

TEST_F(BitmaskTest, testFlagReturnsFalseWhenNotSet)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1);
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag2));
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag3));
}

TEST_F(BitmaskTest, testFlagZeroReturnsTrueOnlyWhenEmpty)
{
    // The implementation has a special case: testFlag(0) returns true only when
    // the Flags value itself is also zero.
    Base::Flags<TestFlagEnum> emptyFlags(TestFlagEnum::None);
    Base::Flags<TestFlagEnum> nonEmptyFlags(TestFlagEnum::Flag1);
    EXPECT_TRUE(emptyFlags.testFlag(TestFlagEnum::None));
    EXPECT_FALSE(nonEmptyFlags.testFlag(TestFlagEnum::None));
}

TEST_F(BitmaskTest, testFlagMultipleFlagsAtOnce)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag2));
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag3));
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag2 | TestFlagEnum::Flag3));
}

TEST_F(BitmaskTest, testFlagAll)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::All);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag2));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag3));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::All));
}

// ---- setFlag ----------------------------------------------------------------

TEST_F(BitmaskTest, setFlagOn)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::None);
    flags.setFlag(TestFlagEnum::Flag1);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
}

TEST_F(BitmaskTest, setFlagOnDefaultParamIsTrue)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::None);
    flags.setFlag(TestFlagEnum::Flag2);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag2));
}

TEST_F(BitmaskTest, setFlagOff)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    flags.setFlag(TestFlagEnum::Flag1, false);
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag2));
}

TEST_F(BitmaskTest, setFlagDoesNotAffectOtherFlags)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag2 | TestFlagEnum::Flag3);
    flags.setFlag(TestFlagEnum::Flag1);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag2));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag3));
}

TEST_F(BitmaskTest, setMultipleFlagsAtOnce)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::None);
    flags.setFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag2));
    
    flags.setFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag2, false);
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag2));
}

TEST_F(BitmaskTest, setFlagAlreadySet)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1);
    flags.setFlag(TestFlagEnum::Flag1, true);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
}

TEST_F(BitmaskTest, setFlagAlreadyUnset)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::None);
    flags.setFlag(TestFlagEnum::Flag1, false);
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_EQ(flags.toUnderlyingType(), 0);
}

// ---- isEqual, operator==, operator!= ----------------------------------------

TEST_F(BitmaskTest, equalityOperators)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    Base::Flags<TestFlagEnum> b(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    Base::Flags<TestFlagEnum> c(TestFlagEnum::Flag1);
    
    EXPECT_TRUE(a.isEqual(b));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    
    EXPECT_FALSE(a.isEqual(c));
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
    
    EXPECT_TRUE(a == (TestFlagEnum::Flag1 | TestFlagEnum::Flag2));
    EXPECT_FALSE(a == TestFlagEnum::Flag1);
    EXPECT_TRUE(a != TestFlagEnum::Flag1);
}

TEST_F(BitmaskTest, equalityWithEmpty)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::None);
    Base::Flags<TestFlagEnum> b(TestFlagEnum::None);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a == TestFlagEnum::None);
}

// ---- getFlags ---------------------------------------------------------------

TEST_F(BitmaskTest, getFlagsReturnsCorrectValue)
{
    TestFlagEnum combined = TestFlagEnum::Flag1 | TestFlagEnum::Flag2;
    Base::Flags<TestFlagEnum> flags(combined);
    EXPECT_EQ(flags.getFlags(), combined);
}

// ---- operator|= and operator| -----------------------------------------------

TEST_F(BitmaskTest, orOperators)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag1);
    Base::Flags<TestFlagEnum> b(TestFlagEnum::Flag2);
    
    Base::Flags<TestFlagEnum> result = a | b;
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag2));
    
    a |= b;
    EXPECT_TRUE(a.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(a.testFlag(TestFlagEnum::Flag2));
    
    Base::Flags<TestFlagEnum> c(TestFlagEnum::Flag1);
    result = c | TestFlagEnum::Flag3;
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag3));
    
    c |= TestFlagEnum::Flag2;
    EXPECT_TRUE(c.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(c.testFlag(TestFlagEnum::Flag2));
}

// ---- operator&= and operator& -----------------------------------------------

TEST_F(BitmaskTest, andOperators)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    Base::Flags<TestFlagEnum> b(TestFlagEnum::Flag2 | TestFlagEnum::Flag3);
    
    Base::Flags<TestFlagEnum> result = a & b;
    EXPECT_FALSE(result.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag2));
    EXPECT_FALSE(result.testFlag(TestFlagEnum::Flag3));
    
    a &= b;
    EXPECT_FALSE(a.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(a.testFlag(TestFlagEnum::Flag2));
    
    Base::Flags<TestFlagEnum> c(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    result = c & TestFlagEnum::Flag2;
    EXPECT_TRUE(result == TestFlagEnum::Flag2);
    
    c &= TestFlagEnum::Flag1;
    EXPECT_TRUE(c == TestFlagEnum::Flag1);
}

// ---- operator^= and operator^ -----------------------------------------------

TEST_F(BitmaskTest, xorOperators)
{
    Base::Flags<TestFlagEnum> a(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    Base::Flags<TestFlagEnum> b(TestFlagEnum::Flag2 | TestFlagEnum::Flag3);
    
    // (1|2) ^ (2|3) = (1|3)
    Base::Flags<TestFlagEnum> result = a ^ b;
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag1));
    EXPECT_FALSE(result.testFlag(TestFlagEnum::Flag2));
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag3));
    
    a ^= b;
    EXPECT_TRUE(a.testFlag(TestFlagEnum::Flag1));
    EXPECT_FALSE(a.testFlag(TestFlagEnum::Flag2));
    EXPECT_TRUE(a.testFlag(TestFlagEnum::Flag3));
    
    Base::Flags<TestFlagEnum> c(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    result = c ^ TestFlagEnum::Flag1;
    EXPECT_TRUE(result == TestFlagEnum::Flag2);
    
    c ^= TestFlagEnum::Flag1;
    EXPECT_TRUE(c == TestFlagEnum::Flag2);
    c ^= TestFlagEnum::Flag2;
    EXPECT_TRUE(c == TestFlagEnum::None);
}

// ---- operator~ (Flags complement) ------------------------------------------

TEST_F(BitmaskTest, complement)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1);
    Base::Flags<TestFlagEnum> result = ~flags;
    
    EXPECT_FALSE(result.testFlag(TestFlagEnum::Flag1));
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag2));
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag3));
    
    // Double complement should return original (within bits of underlying type)
    Base::Flags<TestFlagEnum> doubleComplement = ~result;
    EXPECT_EQ(flags, doubleComplement);
}

// ---- operator! and operator bool --------------------------------------------

TEST_F(BitmaskTest, logicalOperators)
{
    Base::Flags<TestFlagEnum> empty(TestFlagEnum::None);
    Base::Flags<TestFlagEnum> nonEmpty(TestFlagEnum::Flag1);
    
    EXPECT_TRUE(!empty);
    EXPECT_FALSE(!nonEmpty);
    
    EXPECT_FALSE(static_cast<bool>(empty));
    EXPECT_TRUE(static_cast<bool>(nonEmpty));
    
    if (nonEmpty) {
        SUCCEED();
    } else {
        FAIL() << "nonEmpty should evaluate to true";
    }
    
    if (!empty) {
        SUCCEED();
    } else {
        FAIL() << "empty should evaluate to false";
    }
}

// ---- Const correctness ------------------------------------------------------

TEST_F(BitmaskTest, constCorrectness)
{
    const Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::Flag1));
    EXPECT_EQ(flags.getFlags(), TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    EXPECT_EQ(flags.toUnderlyingType(), 3);
    EXPECT_TRUE(flags == (TestFlagEnum::Flag1 | TestFlagEnum::Flag2));
    
    Base::Flags<TestFlagEnum> result = flags | TestFlagEnum::Flag3;
    EXPECT_TRUE(result.testFlag(TestFlagEnum::Flag3));
}

// ---- Interaction with raw enum bitwise ops ----------------------------------

TEST_F(BitmaskTest, interactionWithRawEnum)
{
    Base::Flags<TestFlagEnum> flags = TestFlagEnum::Flag1;
    flags = flags | (TestFlagEnum::Flag2 | TestFlagEnum::Flag3);
    EXPECT_TRUE(flags.testFlag(TestFlagEnum::All));
    
    flags = flags & (TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    EXPECT_EQ(flags, TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
}

// ---- Edge cases -------------------------------------------------------------

TEST_F(BitmaskTest, setFlagWithMultipleBits)
{
    Base::Flags<TestFlagEnum> flags;
    flags.setFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag2, true);
    EXPECT_EQ(flags, TestFlagEnum::Flag1 | TestFlagEnum::Flag2);
    
    flags.setFlag(TestFlagEnum::Flag1 | TestFlagEnum::Flag3, false);
    // Flag1 removed, Flag2 remains, Flag3 wasn't there
    EXPECT_EQ(flags, TestFlagEnum::Flag2);
}

TEST_F(BitmaskTest, testFlagWithZero)
{
    Base::Flags<TestFlagEnum> flags(TestFlagEnum::Flag1);
    // testFlag(0) returns true ONLY if flags is 0
    EXPECT_FALSE(flags.testFlag(TestFlagEnum::None));
    
    Base::Flags<TestFlagEnum> empty;
    EXPECT_TRUE(empty.testFlag(TestFlagEnum::None));
}
