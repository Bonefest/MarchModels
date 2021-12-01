#pragma once

#include <gtest/gtest.h>
#include <memory_manager.h>

TEST(MemoryManagerTests, AllocatedMemoryIsZeroed)
{
  uint32* value = (uint32*)engineAllocMem(sizeof(uint32), MEMORY_TYPE_GENERAL);
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, 0);

  engineFreeMem(value, sizeof(value), MEMORY_TYPE_GENERAL);
}

TEST(MemoryManagerTests, GivenMemoryIsZeroed)
{
  uint32 value = 873;
  engineSetZeroMem(&value, sizeof(value));
  EXPECT_EQ(value, 0);
}

TEST(MemoryManagerTests, GivenMemoryIsCopied)
{
  uint32* value1 = (uint32*)engineAllocMem(sizeof(uint32), MEMORY_TYPE_GENERAL);
  ASSERT_NE(value1, nullptr);
  
  uint32* value2 = (uint32*)engineAllocMem(sizeof(uint32), MEMORY_TYPE_GENERAL);
  ASSERT_NE(value2, nullptr);

  EXPECT_NE(value1, value2);

  *value1 = 393;
  *value2 = 117;

  engineCopyMem(value1, value2, sizeof(value1));
  EXPECT_EQ(*value1, *value2);
  EXPECT_NE(value1, value2);
  
  engineFreeMem(value1, sizeof(uint32), MEMORY_TYPE_GENERAL);
  engineFreeMem(value2, sizeof(uint32), MEMORY_TYPE_GENERAL);  
}

TEST(MemoryManagerTests, ObjectIsProcessed)
{
  static bool8 constructorCalled = FALSE;
  static bool8 destructorCalled = FALSE;
  
  class SomeClassWithConstructorAndDestructor
  {
  public:
    SomeClassWithConstructorAndDestructor()
    {
      constructorCalled = TRUE;
    }

    ~SomeClassWithConstructorAndDestructor()
    {
      destructorCalled = TRUE;
    }
  };

  SomeClassWithConstructorAndDestructor* object = engineAllocObject<SomeClassWithConstructorAndDestructor>(MEMORY_TYPE_GENERAL);
  ASSERT_NE(object, nullptr);
  EXPECT_EQ(constructorCalled, TRUE);

  engineFreeObject(object, MEMORY_TYPE_GENERAL);
  EXPECT_EQ(destructorCalled, TRUE);
}

TEST(MemoryManagerTests, AccessInternalAllocatorExists)
{
  EXPECT_NE(engineGetAllocatorByType(MEMORY_TYPE_GENERAL), nullptr);
}  

TEST(MemoryManagerTests, RegisterUnexistingAllocatorIsImpossible)
{
  EXPECT_EQ(engineRegisterAllocator(nullptr), FALSE);
}

TEST(MemoryManagerTests, EngineMemoryTypesHaveSameAllocator)
{
  MemoryAllocator* generalAllocator = engineGetAllocatorByType(MEMORY_TYPE_GENERAL);
  EXPECT_EQ(generalAllocator, engineGetAllocatorByType(MEMORY_TYPE_UNDEFINED));
  EXPECT_EQ(generalAllocator, engineGetAllocatorByType(MEMORY_TYPE_APPLICATION));
  EXPECT_EQ(generalAllocator, engineGetAllocatorByType(MEMORY_TYPE_PER_FRAME));
  EXPECT_EQ(generalAllocator, engineGetAllocatorByType(MEMORY_TYPE_FILM));
}

// shared_ptr tests (e.g correct allocation function is called)
