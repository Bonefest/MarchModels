#pragma once

#include <memory>

#include "defines.h"
#include "logging.h"
#include "memory_manager.h"

template <typename T, void(*)(T*)>
class WeakPtr;

/**
 * It's a std::shared_ptr analog adapted to the API of the engine. It's applied only
 * by types that are created by API itself and types which are using memory
 * allocated from the engine.
 *
 * Reasons of existing of SharedPtr:
 *   1. We need to have a way to pass destroyFunc as part of a type (std::shared_ptr
 *      only provides a way to pass a Deleter object.
 *   2. We'd like to have implicit conversions between SharedPtr --> T*, because
 *      most of the API's functions are using raw pointers and it should look
 *      consistently.
 */
template <typename T, void(*destroyFunc)(T*) = nullptr>
class SharedPtr
{
  friend class WeakPtr<T, destroyFunc>;
private:

  // NOTE: Custom memory deallocator
  struct RefCounterDeleter
  {
    void operator()(uint32* counter)
    {
      engineFreeObject(counter, MEMORY_TYPE_GENERAL);
    }
  };
  
public:
  SharedPtr() = default;
  
  // NOTE: We make it explicit in order to avoid situations, where a function/method accepts a
  // shared pointer and a raw data is passed (as result, raw data will be destroyed as soon as
  // function is over)
  explicit SharedPtr(T* rawPtr)
  {
    if(rawPtr != nullptr)
    {
      m_ptr = rawPtr;
      m_refCounter = std::shared_ptr<uint32>(engineAllocObject<uint32>(MEMORY_TYPE_GENERAL),
                                             [](uint32* counter) { engineFreeObject(counter, MEMORY_TYPE_GENERAL); });
      retain();          
    }
  }
  
  SharedPtr(const SharedPtr& sharedPtr)
  {
    m_refCounter = sharedPtr.m_refCounter;
    m_ptr = sharedPtr.m_ptr;

    retain();
  }

  SharedPtr(SharedPtr&& sharedPtr)
  {
    m_refCounter = sharedPtr.m_refCounter;
    m_ptr = sharedPtr.m_ptr;

    retain();    
  }
  
  ~SharedPtr()
  {
    release();
  }

  SharedPtr& operator=(SharedPtr&& sharedPtr)
  {
    release();
    m_refCounter = sharedPtr.m_refCounter;
    m_ptr = sharedPtr.m_ptr;
    retain();

    return *this;
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

  // NOTE: For data structures that require the less operator (e.g set, map)
  bool operator<(const SharedPtr& ptr) const
  {
    return m_ptr < ptr.m_ptr;
  }
  
  bool operator==(T* data)
  {
    return m_ptr == data;
  }

  bool operator!=(T* data)
  {
    return m_ptr != data;
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
    if(m_ptr == nullptr && m_refCounter == nullptr)
    {
      return; // NOTE: It's nullptr - do nothing
    }
    
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
      if(m_ptr == nullptr)
      {
        return; // NOTE: It's a nullptr shared ptr - do nothing
      }
      
      LOG_WARNING("Attempt to release already released SharedPtr?");
      return;
    }
    
    assert(*m_refCounter > 0 && "Attempt to release an empty shared ptr!");
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

      m_refCounter = nullptr;
    }
  }

  uint32 getRefCount()
  {
    return m_refCounter == nullptr ? 0 : *m_refCounter;
  }

  T* raw() const
  {
    return m_ptr;
  }
  
  operator T*() const { return m_ptr; }
  
private:
  std::shared_ptr<uint32> m_refCounter = nullptr;

  // NOTE: Previously raw pointer was public. It's a bad idea because user has a chance
  // to write something like: "createNewWindow(&sharedPtr.ptr)", meaning that previous raw pointer
  // will be simply replaced (not deleted, reference counter won't decrease, nothing will happen
  // from the shared ptr side!).
  //
  // You still can access the raw pointer, but now you are forced to change it only through the
  // corresponding API (e.g create a new shared ptr and reassign it)
  T* m_ptr = nullptr;
};

template <typename T, void(*destroyFunc)(T*)>
class WeakPtr
{
public:
  WeakPtr() = delete;
  
  WeakPtr(const SharedPtr<T, destroyFunc>& sptr)
  {
    m_refCounter = sptr.m_refCounter;
    m_ptr = sptr.m_ptr;
  }

  T* raw() const
  {
    assert(*m_refCounter > 0);
    return m_ptr;
  }
  
  bool8 available() const
  {
    return *m_refCounter > 0 ? TRUE : FALSE;
  }

  // NOTE: For data structures that require the less operator (e.g set, map)
  bool operator<(const WeakPtr& ptr) const
  {
    return m_ptr < ptr.m_ptr;
  }
  
  bool operator==(const SharedPtr<T, destroyFunc>& sptr) const
  {
    return m_ptr == sptr.m_ptr;
  }
  
  bool operator==(const WeakPtr& ptr) const 
  {
    return m_ptr == ptr.m_ptr;
  }

  bool operator!=(const SharedPtr<T, destroyFunc>& sptr) const
  {
    return m_ptr != sptr.m_ptr;
  }
  
  bool operator!=(const WeakPtr& ptr) const
  {
    return m_ptr != ptr.m_ptr;
  }
  
  operator T*() const { return raw(); }
  
private:
  std::shared_ptr<uint32> m_refCounter;
  T* m_ptr;
};
