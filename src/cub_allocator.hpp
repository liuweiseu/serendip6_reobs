// This file provides a convenient interface to the CUB caching allocator class.

// Allocation requests below (BIN_GROWTH ^ MIN_BIN) are rounded up to (BIN_GROWTH ^ MIN_BIN).
#define BIN_GROWTH (2)
#define MIN_BIN    (2)

class cub_device_allocator {
  cub::CachingDeviceAllocator _allocator;
public:
  typedef char value_type;
  typedef char& reference;
  typedef char const& const_reference;
  typedef char* pointer;
  typedef char const* const_pointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  static const unsigned int INVALID_BIN =
      cub::CachingDeviceAllocator::INVALID_BIN;
  static const size_t INVALID_SIZE = cub::CachingDeviceAllocator::INVALID_SIZE;

  cub_device_allocator(unsigned int bin_growth, unsigned int min_bin=1,
                       size_t max_cached_bytes=INVALID_SIZE)
    : _allocator(bin_growth, min_bin, INVALID_BIN, max_cached_bytes,
                 // Set debug=true to see log output of all allocations
                 /*skip_cleanup=*/true, /*debug=*/false) {}

  pointer allocate(size_type num_bytes) {
    pointer ptr;
    cudaError_t ret = _allocator.DeviceAllocate((void**)&ptr, num_bytes);
    if (ret != cudaSuccess) {
      std::stringstream msg;
      msg << "CUDA failed to allocate " << num_bytes << " bytes: "
          << cudaGetErrorString(ret);
      throw std::runtime_error(msg.str());
    }
    return ptr;
  }

  void deallocate(pointer ptr, size_t num_bytes) {
    _allocator.DeviceFree(ptr);
  }

    void free_all_cached() {
	_allocator.FreeAllCached();
  }
};

static cub_device_allocator* get_singleton_device_allocator() {
  // These parameters can be tuned if necessary to optimize the memory usage
  //static cub_device_allocator instance(/*bin_growth=*/4, /*min_bin=*/6);
  static cub_device_allocator instance(/*bin_growth=*/BIN_GROWTH, /*min_bin=*/MIN_BIN);
  return &instance;
}

template<typename T>
class thrust_device_allocator {
public:
  typedef T value_type;
  typedef thrust::device_reference<T> reference;
  typedef thrust::device_reference<T const> const_reference;
  typedef thrust::device_ptr<T> pointer;
  typedef thrust::device_ptr<T const> const_pointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  pointer allocate(size_type num_elements) {
    char* raw_ptr = get_singleton_device_allocator()->allocate(
        num_elements * sizeof(T));
    return pointer(reinterpret_cast<T*>(raw_ptr));
  }

  void deallocate(pointer ptr, size_t num_elements) {
    char* raw_ptr = reinterpret_cast<char*>(ptr.get());
    get_singleton_device_allocator()->deallocate(
        raw_ptr, num_elements * sizeof(T));
  }
};

// This convenience requires compiling with C++11 support (nvcc -std=c++11)
template<typename T>
using cub_device_vector = thrust::device_vector<T, thrust_device_allocator<T> >;
