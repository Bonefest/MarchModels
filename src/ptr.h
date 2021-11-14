#pragma once

#include "defines.h"
#include "logging.h"
#include "memory_manager.h"

template <typename T, void(*destroyFunc)(T*) = nullptr>
class SharedPtr
{
public:
  SharedPtr(): SharedPtr(nullptr) { }

  // NOTE: We make it explicit in order to avoid situations, where a function/method accepts a
  // shared pointer and a raw data is passed (as result, raw data will be destroyed as soon as
  // function is over)
  explicit SharedPtr(T* rawPtr)
  {
    m_ptr = rawPtr;
    m_refCounter = engineAllocObject<uint32>(MEMORY_TYPE_GENERAL);
    retain();    
  }
  
  SharedPtr(const SharedPtr& sharedPtr)
  {
    m_refCounter = sharedPtr.m_refCounter;
    m_ptr = sharedPtr.m_ptr;

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
    m_ptr = sharedPtr.m_ptr;
    retain();
    
    return *this;
  }

  bool operator==(T* data)
  {
    return m_ptr == data;
  }
  
  T* operator->()
  {
    return m_ptr;
  }
  
  T& operator*()
  {
    return *m_ptr;
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
      if(m_ptr == nullptr)
      {
        // LOG_WARNING("Empty SharedPtr was released!");
      }
      else
      {
        destroyFunc(m_ptr);
        m_ptr = nullptr;
      }

      engineFreeObject(m_refCounter, MEMORY_TYPE_GENERAL);
      m_refCounter = nullptr;
    }
  }

  uint32 getRefCount()
  {
    return m_refCounter == nullptr ? 0 : *m_refCounter;
  }

  T* raw()
  {
    return m_ptr;
  }
  
  operator T*() const { return m_ptr; }
  
private:
  uint32* m_refCounter = nullptr;

  // NOTE: Previously raw pointer was public. It's a bad idea because user has a chance
  // to write something like: "createNewWindow(&sharedPtr.ptr)", meaning that previous raw pointer
  // will be simply replaced (not deleted, reference counter won't decrease, nothing will happen
  // from the shared ptr side!).
  //
  // You still can access the raw pointer, but now you are forced to change it only through the
  // corresponding API (e.g create a new shared ptr and reassign it)
  T* m_ptr = nullptr;
  
};