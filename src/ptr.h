#pragma once

#include "defines.h"
#include "logging.h"
#include "memory_manager.h"

template <typename T, void(*destroyFunc)(T*) = nullptr>
class SharedPtr
{
public:
  T* ptr = nullptr;
  
public:
  SharedPtr(T* rawPtr = nullptr)
  {
    ptr = rawPtr;
    m_refCounter = engineAllocObject<uint32>(MEMORY_TYPE_GENERAL);
    retain();    
  }
  
  SharedPtr(const SharedPtr& sharedPtr)
  {
    m_refCounter = sharedPtr.m_refCounter;
    ptr = sharedPtr.ptr;

    retain();
  }
  
  ~SharedPtr()
  {
    release();
  }

  SharedPtr& operator=(const SharedPtr& sharedPtr)
  {
    if(this == &sharedPtr)
    {
      return *this;
    }

    release();
    m_refCounter = sharedPtr.m_refCounter;
    ptr = sharedPtr.ptr;
    retain();
    
    return *this;
  }
  
  T* operator->()
  {
    return ptr;
  }
  
  T& operator*()
  {
    return *ptr;
  }
  
  T* operator&()
  {
    return ptr;
  }
  
  void retain()
  {
    if(m_refCounter != nullptr)
    {
      (*m_refCounter)++;
    }
    else
    {
      LOG_WARNING("Attempt to retain not allocated SharedPtr!");
    }
  }

  void release()
  {
    if(m_refCounter == nullptr)
    {
      LOG_WARNING("Attempt to release already released SharedPtr?");
      return;
    }
    
    assert(m_refCounter > 0 && "Attempt to release an empty shared ptr!");
    (*m_refCounter)--;
    if((*m_refCounter) == 0)
    {
      if(ptr == nullptr)
      {
        LOG_WARNING("Empty SharedPtr was released!");
      }
      else
      {
        destroyFunc(ptr);
        ptr = nullptr;
      }

      engineFreeObject(m_refCounter, MEMORY_TYPE_GENERAL);
      m_refCounter = nullptr;
    }
  }

  uint32 getRefCount()
  {
    return m_refCounter == nullptr ? 0 : *m_refCounter;
  }

  operator T*() const { return ptr; }
  
private:
  uint32* m_refCounter = nullptr;
  
};
