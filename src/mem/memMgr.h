/****************************************************************************
  FileName     [ memMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define Memory Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/

#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdlib.h>

using namespace std;

// Turn this on for debugging
// #define MEM_DEBUG

//--------------------------------------------------------------------------
// Define MACROs
//--------------------------------------------------------------------------
#define MEM_MGR_INIT( T )  MemMgr<T>* const T::_memMgr = new MemMgr<T>

#define USE_MEM_MGR(T)                                                   \
  public:                                                                \
void* operator new(size_t t) { return (void*)(_memMgr->alloc(t)); }      \
void* operator new[](size_t t) { return (void*)(_memMgr->allocArr(t)); } \
void  operator delete(void* p) { _memMgr->free((T*)p); }                 \
void  operator delete[](void* p) { _memMgr->freeArr((T*)p); }            \
static void memReset(size_t b = 0) { _memMgr->reset(b); }                \
static void memPrint() { _memMgr->print(); }                             \
private:                                                                 \
static MemMgr<T>* const _memMgr

// You should use the following two MACROs whenever possible to 
// make your code 64/32-bit platform independent.
// DO NOT use 4 or 8 for sizeof(size_t) in your code
//
#define SIZE_T      (sizeof(size_t))
#define SIZE_T_1    (sizeof(size_t) - 1)

// TODO: Define them by SIZE_T and/or SIZE_T_1 MACROs.
//
// To promote 't' to the nearest multiple of SIZE_T; 
// e.g. Let SIZE_T = 8;  toSizeT(7) = 8, toSizeT(12) = 16
#define toSizeT(t)      ( (t%( sizeof(size_t) )) ? (t-(t%(sizeof(size_t)))+sizeof(size_t)):t) 
// TODO ...done 10/30 12:50
//
// To demote 't' to the nearest multiple of SIZE_T
// e.g. Let SIZE_T = 8;  downtoSizeT(9) = 8, downtoSizeT(100) = 96
#define downtoSizeT(t)  ( t - ( t% ( SIZE_T ) ) )  // TODO ...done 10/30 12:51

// R_SIZE is the size of the recycle list
#define R_SIZE 256

//--------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------
template <class T> class MemMgr;


//--------------------------------------------------------------------------
// Class Definitions
//--------------------------------------------------------------------------
// T is the class that use this memory manager
//
// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemBlock
{
  friend class MemMgr<T>;

  // Constructor/Destructor
  MemBlock(MemBlock<T>* n, size_t b) : _nextBlock(n) {
    _begin = _ptr = new char[b]; _end = _begin + b; }
  ~MemBlock() { delete [] _begin; }

  // Member functions
  void reset() { _ptr = _begin; }
  // 1. Get (at least) 't' bytes memory from current block
  //    Promote 't' to a multiple of SIZE_T
  // 2. Update "_ptr" accordingly
  // 3. The return memory address is stored in "ret"
  // 4. Return false if not enough memory
  bool getMem(size_t t, T*& ret) {
    // TODO ... done 1104 0206
    t = toSizeT ( t );
    if( t > _end - _ptr ){
      ret = nullptr;
      return false;
    }
    ret = reinterpret_cast<T*>(_ptr);
    _ptr = _ptr + t;
    return true;
  }
  size_t getRemainSize() const { return size_t(_end - _ptr); }

  MemBlock<T>* getNextBlock() const { return _nextBlock; }

  // Data members
  char*             _begin;
  char*             _ptr;
  char*             _end;
  MemBlock<T>*      _nextBlock;
};

// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemRecycleList
{
  friend class MemMgr<T>;

  // Constructor/Destructor
  MemRecycleList(size_t a = 0) : _arrSize(a), _first(0), _nextList(0) {}
  ~MemRecycleList() { reset(); }

  // Member functions
  // ----------------
  size_t getArrSize() const { return _arrSize; }
  MemRecycleList<T>* getNextList() const { return _nextList; }
  void setNextList(MemRecycleList<T>* l) { _nextList = l; }
  // pop out the first element in the recycle list
  T* popFront() {
    // TODO done 1104 0230
#ifdef MEM_DEBUG
    assert( _first != nullptr && "MemRecycleList::popFront() error");
#endif // MEM_DEBUG
    size_t* ptr = reinterpret_cast<size_t*>(_first);
    ++ptr;
    _first = reinterpret_cast<T*>( *ptr );
    --ptr;
#ifdef MEM_DEBUG
    if( _arrSize != 0 )
      assert( _arrSize == *ptr && "sth wrong in MemRecycleList::popFront()");
#endif // MEM_DEBUG
    return reinterpret_cast<T*>(ptr);
  }
  // push the element 'p' to the beginning of the recycle list
  void  pushFront(T* p) {
    // TODO done 1104 0304
#ifdef MEM_DEBUG
    assert( p != nullptr && "MemRecycleList::pushFront() error" );
#endif // MEM_DEBUG
    // *((size_t*)p) is the size information for "delete []";
    // we assume managed class have S > size_t;
    // thus use next 8 byte to store our pointer.
    size_t* ptr = reinterpret_cast<size_t*>(p);
    ptr ++;
    *ptr = (size_t) _first;
    _first = p;
  }
  // Release the memory occupied by the recycle list(s)
  // DO NOT release the memory occupied by MemMgr/MemBlock
  void reset() {
    // TODO done 1104 0315
    if( _nextList != nullptr ){
      _nextList->reset();
      delete _nextList;
      _nextList = nullptr;
    }
    _first = nullptr;
    _nextList = nullptr;
  }

  // Helper functions
  // ----------------
  // count the number of elements in the recycle list
  size_t numElm() const {
    // TODO done 1104 0346
    size_t ret = 0;
    size_t* ptr = reinterpret_cast<size_t*> (_first);
    while( ptr != nullptr ){
      ++ptr;
      ptr = reinterpret_cast<size_t*>(*ptr);
      ++ret;
    }
    return ret;
  }

  // Data members
  size_t              _arrSize;   // the array size of the recycled data
  T*                  _first;     // the first recycled data
  MemRecycleList<T>*  _nextList;  // next MemRecycleList
  //      with _arrSize + x*R_SIZE
};

template <class T>
class MemMgr
{
#define S sizeof(T)

  public:
    MemMgr(size_t b = 65536) : _blockSize(b) {
      assert(b % SIZE_T == 0);
      _activeBlock = new MemBlock<T>(0, _blockSize);
      for (int i = 0; i < R_SIZE; ++i)
        _recycleList[i]._arrSize = i;
    }
    ~MemMgr() { reset(); delete _activeBlock; }

    // 1. Remove the memory of all but the firstly allocated MemBlocks
    //    That is, the last MemBlock searchd from _activeBlock.
    //    reset its _ptr = _begin (by calling MemBlock::reset())
    // 2. reset _recycleList[]
    // 3. 'b' is the new _blockSize; "b = 0" means _blockSize does not change
    //    if (b != _blockSize) reallocate the memory for the first MemBlock
    // 4. Update the _activeBlock pointer
    void reset(size_t b = 0) {
      assert(b % SIZE_T == 0);
#ifdef MEM_DEBUG
      cout << "Resetting memMgr...(" << b << ")" << endl;
      assert( _activeBlock != nullptr && "sth wrong in MemBlock::reset()" );
#endif // MEM_DEBUG
      MemBlock<T>* tmp_ptr = nullptr;
      while( _activeBlock -> getNextBlock() != nullptr ){
        tmp_ptr = _activeBlock;
        _activeBlock = _activeBlock->getNextBlock();
        delete tmp_ptr;
      }
      if( b != 0 ){
        delete _activeBlock;
        _blockSize = b;
        _activeBlock = new MemBlock<T> ( nullptr, _blockSize );
      }else{
        _activeBlock->reset();
        _activeBlock->_nextBlock = nullptr;
      }
      for( size_t i = 0; i < R_SIZE; ++i ){
        _recycleList[i].reset();
      }
      // TODO ... done 1106 2248
    }
    // Called by new
    T* alloc(size_t t) {
      assert(t == S);
#ifdef MEM_DEBUG
      cout << "Calling alloc...(" << t << ")" << endl;
#endif // MEM_DEBUG
      return getMem(t);
    }
    // Called by new[]
    T* allocArr(size_t t) {
#ifdef MEM_DEBUG
      cout << "Calling allocArr...(" << t << ")" << endl;
#endif // MEM_DEBUG
      // Note: no need to record the size of the array == > system will do
      return getMem(t);
    }
    // Called by delete
    void  free(T* p) {
#ifdef MEM_DEBUG
      cout << "Calling free...(" << p << ")" << endl;
#endif // MEM_DEBUG
      getMemRecycleList(0)->pushFront(p);
    }
    // Called by delete[]
    void  freeArr(T* p) {
#ifdef MEM_DEBUG
      cout << "Calling freeArr...(" << p << ")" << endl;
#endif // MEM_DEBUG
      // TODO
      // Get the array size 'n' stored by system,
      // which is also the _recycleList index
      size_t n = 0;
      // it's the offset-ed version by system.
      size_t* ptr = reinterpret_cast<size_t*>( p );
      n = *ptr;
#ifdef MEM_DEBUG
      cout << ">> Array size = " << n << endl;
      cout << "Recycling " << p << " to _recycleList[" << n << "]" << endl;
#endif // MEM_DEBUG
      // add to recycle list...
      getMemRecycleList(n)->pushFront(p);
    }
    void print() const {
      cout << "=========================================" << endl
        << "=              Memory Manager           =" << endl
        << "=========================================" << endl
        << "* Block size            : " << _blockSize << " Bytes" << endl
        << "* Number of blocks      : " << getNumBlocks() << endl
        << "* Free mem in last block: " << _activeBlock->getRemainSize()
        << endl
        << "* Recycle list          : " << endl;
      int i = 0, count = 0;
      while (i < R_SIZE) {
        const MemRecycleList<T>* ll = &(_recycleList[i]);
        while (ll != 0) {
          size_t s = ll->numElm();
          if (s) {
            cout << "[" << setw(3) << right << ll->_arrSize << "] = "
              << setw(10) << left << s;
            if (++count % 4 == 0) cout << endl;
          }
          ll = ll->_nextList;
        }
        ++i;
      }
      cout << endl;
    }

  private:
    size_t                     _blockSize;
    MemBlock<T>*               _activeBlock;
    MemRecycleList<T>          _recycleList[R_SIZE];

    // Private member functions
    //
    // t: #Bytes; MUST be a multiple of SIZE_T
    // return the size of the array with respect to memory size t
    // [Note] t must >= S
    // [NOTE] Use this function in (at least) getMem() to get the size of array
    //        and call getMemRecycleList() later to get the index for
    //        the _recycleList[]
    size_t getArraySize(size_t t) const {
      assert(t % SIZE_T == 0);
      assert(t >= S);
      // TODO done 1107 0312
      if( t == toSizeT( S ) ){
        return 0;
      }else{
        return (t-SIZE_T)/S ;
      }
      // we assume S > SIZE_T here !!
    }
    // Go through _recycleList[m], its _nextList, and _nexList->_nextList, etc,
    //    to find a recycle list whose "_arrSize" == "n"
    // If not found, create a new MemRecycleList with _arrSize = n
    //    and add to the last MemRecycleList
    // So, should never return NULL
    // [Note]: This function will be called by MemMgr->getMem() to get the
    //         recycle list. Therefore, the recycle list is first created
    //         by the MTNew command, not MTDelete.
    MemRecycleList<T>* getMemRecycleList(size_t n) {
      size_t m = n % R_SIZE;
      // TODO done 1104 0333
      MemRecycleList<T>* ptr = &(_recycleList[m]);
      while( ptr -> _arrSize != n ){
        if( ptr -> _nextList == nullptr ){
          ptr -> _nextList = new MemRecycleList<T> (n );
          return ptr -> _nextList;
        }else{
          ptr = ptr->_nextList;
        }
      }

      return ptr;
    }
    // t is the #Bytes requested from new or new[]
    // Note: Make sure the returned memory is a multiple of SIZE_T
    T* getMem(size_t t) {
      T* ret = 0;
#ifdef MEM_DEBUG
      cout << "Calling MemMgr::getMem...(" << t << ")" << endl;
#endif // MEM_DEBUG
      // 1. Make sure to promote t to a multiple of SIZE_T
      size_t n = getArraySize(t);
      t = toSizeT(t);
      // 2. Check if the requested memory is greater than the block size.
      //    If so, throw a "bad_alloc()" exception.
      //    Print this message for exception
      //    cerr << "Requested memory (" << t << ") is greater than block size"
      //         << "(" << _blockSize << "). " << "Exception raised...\n";
      // TODO done 1104 1520
      try{
        if( t > _blockSize ){
          throw bad_alloc();
        }
      }catch ( bad_alloc& bd ){
        cerr << "Requested memory (" << t << ") is greater than block size"
          << "(" << _blockSize << "). Exception raised...\n";
        throw;
      }

      // 3. Check the _recycleList first...
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
      //    #endif // MEM_DEBUG
      //    => 'n' is the size of array
      //    => "ret" is the return address
      // TODO done 1104 1543
      MemRecycleList<T>* mem_re_list_ptr = getMemRecycleList(n);
      if( mem_re_list_ptr-> _first != nullptr ){
        ret = mem_re_list_ptr->popFront();
#ifdef MEM_DEBUG
        cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
#endif // MEM_DEBUG
        return ret;
      }

      // If no match from recycle list...
      // 4. Get the memory from _activeBlock
      // 5. If not enough, recycle the remained memory and print out ---
      //    Note: recycle to the as biggest array index as possible
      //    Note: rn is the array size
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycling " << ret << " to _recycleList[" << rn << "]\n";
      //    #endif // MEM_DEBUG
      //    ==> allocate a new memory block, and print out ---
      //    #ifdef MEM_DEBUG
      //    cout << "New MemBlock... " << _activeBlock << endl;
      //    #endif // MEM_DEBUG
      // TODO done 1104 1544

      if( !_activeBlock->getMem( t, ret )){
        // no enough space in (*_activeBlock),
        // try put remained space in (*_activeBlock) to _recycleList;
        if( _activeBlock->getRemainSize() >= toSizeT( S ) ){
          // push current remaining space to corresponding _recycleList[x];
#ifdef MEM_DEBUG
          ret = reinterpret_cast<T*>(_activeBlock -> _ptr);
          cout << "Recycling " << ret << " to _recycleList[" << 
            getArraySize( _activeBlock->getRemainSize() ) << "]\n";
#endif // MEM_DEBUG
          size_t remaining_size = _activeBlock->getRemainSize();
          size_t* dirty_ptr = reinterpret_cast<size_t*>(_activeBlock->_ptr );
          remaining_size = downtoSizeT( remaining_size ) ;
          if( remaining_size >= S ){
            *dirty_ptr = remaining_size / S ;
            getMemRecycleList( getArraySize( remaining_size ) ) ->
              pushFront( reinterpret_cast<T*> ( dirty_ptr ) );
            _activeBlock -> _ptr = _activeBlock -> _end;
          }else{
            assert( 0 && "something wrong in MemMgr::getMem" );
          }
          ret = nullptr;
        }
        _activeBlock = new MemBlock<T>( _activeBlock, _blockSize );
#ifdef MEM_DEBUG
        cout << "New MemBlock... " << _activeBlock << endl;
#endif // MEM_DEBUG
        _activeBlock->getMem( t, ret );
      }

      // 6. At the end, print out the acquired memory address
#ifdef MEM_DEBUG
      cout << "Memory acquired... " << ret << endl;
#endif // MEM_DEBUG
      return ret;
    }
    // Get the currently allocated number of MemBlock's
    size_t getNumBlocks() const {
      // TODO done 1104 1547
      size_t ret = 0;
      MemBlock<T>* ptr = _activeBlock;
      while( ptr != nullptr ){
        ptr = (ptr -> getNextBlock());
        ++ret;
      }
      return ret;
    }

};

#endif // MEM_MGR_H
