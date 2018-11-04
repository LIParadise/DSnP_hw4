template <class T>
class MemMgr
{
#define S sizeof(T)

  public:
    MemMgr(size_t b = 65536) : _blockSize(b) ;
    ~MemMgr() ;

    void reset(size_t b = 0) ;
    T* alloc(size_t t) ;
    // Called by new[]
    T* allocArr(size_t t) ;
    // Called by delete
    void  free(T* p) ;
    // Called by delete[]
    void  freeArr(T* p) ;
    void print() const ;

  private:
    size_t                     _blockSize;
    MemBlock<T>*               _activeBlock;
    MemRecycleList<T>          _recycleList[R_SIZE];

    size_t getArraySize(size_t t) const ;
    MemRecycleList<T>* getMemRecycleList(size_t n) ;
    // t is the #Bytes requested from new or new[]
    // Note: Make sure the returned memory is a multiple of SIZE_T
    T* getMem(size_t t) ;
    // Get the currently allocated number of MemBlock's
    size_t getNumBlocks() const ;

};






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
  T* popFront() ;
  // push the element 'p' to the beginning of the recycle list
  void  pushFront(T* p) ;
  // Release the memory occupied by the recycle list(s)
  // DO NOT release the memory occupied by MemMgr/MemBlock
  void reset() ;

  // Helper functions
  // ----------------
  // count the number of elements in the recycle list
  size_t numElm() const ;

  // Data members
  size_t              _arrSize;   // the array size of the recycled data
  T*                  _first;     // the first recycled data
  MemRecycleList<T>*  _nextList;  // next MemRecycleList
  //      with _arrSize + x*R_SIZE
};
