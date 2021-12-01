#pragma once

#include <gtest/gtest.h>
#include <ptr.h>

TEST(SharedPtrTests, DefaultSharedPtrIsNull)
{
  SharedPtr<char> somePtr;
  EXPECT_EQ(somePtr, nullptr);
}

TEST(SharedPtrTests, NullptrHasZeroRefCount)
{
  SharedPtr<char> somePtr{nullptr};
  EXPECT_EQ(somePtr.getRefCount(), 0);
}

TEST(SharedPtrTests, NullptrSharedPtrAreEqual)
{
  SharedPtr<char> somePtr1{nullptr};
  SharedPtr<char> somePtr2{nullptr};
  EXPECT_EQ(somePtr1, somePtr2);
}

TEST(SharedPtrTests, CopyingSharedPtrNotRetain)
{
  SharedPtr<char> somePtr1{nullptr};
  SharedPtr<char> somePtr2{nullptr};

  somePtr2 = somePtr1;
  EXPECT_EQ(somePtr1.getRefCount(), 0);
  EXPECT_EQ(somePtr2.getRefCount(), 0);
}

TEST(SharedPtrTests, ReleasingNullptrNotThrowing)
{
  SharedPtr<char> somePtr{nullptr};
  EXPECT_NO_THROW(somePtr.release());
}

void destroyFunction(uint32* value)
{
  (*value)++;
}

TEST(SharedPtrTests, DestroyFunctionIsCalledWithZeroRef)
{
  uint32 someValue = 0;
  {
    SharedPtr<uint32, destroyFunction> someSharedPtr(&someValue);
    EXPECT_EQ(someValue, 0);
  }
  EXPECT_EQ(someValue, 1);
}


TEST(SharedPtrTests, DestroyFunctionIsNotCalledWithNotZeroRef)
{
  uint32 someValue = 0;
  {
    SharedPtr<uint32, destroyFunction> someSharedPtr(&someValue);
    EXPECT_EQ(someValue, 0);

    someSharedPtr.retain();
  }
  EXPECT_EQ(someValue, 0);
}

TEST(SharedPtrTests, DestroyFunctionIsCalledOnce)
{
  uint32 someValue = 0;
  {
    SharedPtr<uint32, destroyFunction> someSharedPtr(&someValue);
    SharedPtr<uint32, destroyFunction> sameSharedPtr = someSharedPtr;

    EXPECT_EQ(someSharedPtr.getRefCount(), 2);
    EXPECT_EQ(sameSharedPtr.getRefCount(), 2);
    EXPECT_EQ(someValue, 0);
  }

  EXPECT_EQ(someValue, 1);
}
