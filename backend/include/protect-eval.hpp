// This file provides a function protect_eval for calling another
// function (that might crash or call exit) within a forked process,
// allowing the calling process to continue and recover.
//
// It requires POSIX fork/wait, nanosleep and anonymous shared memory.
//
// The function protect_eval will cause the parent process to sleep
// until the forked child process returns, at which time the process
// receives a signal SIGCHLD and wakes up.  To make sure that this
// signal is received, and not ignored, it may be necessary on your
// platform to specify a signal handler for it, even if this does
// nothing, e.g. by
//
// static void do_nothing(int) { }
// int main(void) {
//   ...
//   signal(SIGCHLD, do_nothing);
//   ...
// }
//
// SharedBuffer is a helper class that wraps mmap/munmap to provide a
// buffer through which to return data back to the calling process.
// An important restriction is that the length of this buffer must be
// known by the calling process and specified in advance.

#ifndef PROTECT_EVAL_HPP
#define PROTECT_EVAL_HPP

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

// An anonymous shared-memory-buffer object, intended for use by
// protect_eval.  Implemented as a simple wrapper around
// mmap/munmap.  Objects of this type are non-copyable.
//
template <typename DataT>
class SharedBuffer {
  size_t n;
  DataT *m_data;

public:
  DataT *data(void)
  {
    return m_data;
  }
  
  DataT& operator*()
  {
    return *m_data;
  }

  DataT& operator[](size_t idx)
  {
    return m_data[idx];
  }
	
  SharedBuffer(size_t n_ = 1)
    : n(n_)
  {
    m_data = (DataT *)mmap(
      NULL, n * sizeof(DataT), PROT_READ | PROT_WRITE,
      MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    
    if (m_data == MAP_FAILED) throw std::bad_alloc();
  }
    
  ~SharedBuffer()
  {
    munmap(m_data, n * sizeof(DataT));
  }

  // not copy constructable or assignable
  SharedBuffer(const SharedBuffer&) =delete;
  SharedBuffer& operator=(const SharedBuffer&) =delete;
};
  
enum PE_ErrorCodes {PE_TIMEOUT = -2, PE_BADFORK, PE_SUCCESS};

// Wrap the functor fn (of templated type) in a forked child process.
// A timeout in seconds should be provided as the first argument.  The
// functor fn must be callable with no arguments; its return value is
// not used or stored.
// 
// Return values
//   PE_TIMEOUT (-2): the fork succeeded but the child process exceeded
//                    its permitted runtime of timeout_secs seconds
//
//   PE_BADFORK (-1): forking did not succeed
//
//   PE_SUCCESS (0) : successful evaluation of function fn and termination
//                    of the child process
//
//   other value >0 : abnormal termination of the child process.  The
//                    status code is returned.
//
template <typename FnT>  
int protect_eval(long timeout_secs, FnT&& fn)
{
  pid_t child_pid = fork();
  if (child_pid == -1) {
    // fork did not succeed
    return PE_BADFORK;
  } else if (!child_pid) {
    // fork succeeded; in child
    // call the dangerous function    
    fn();
    // exit, sending SIGCHLD to parent
    _exit(0);
  } else {
    // fork succeeded; in parent
    struct timespec req, rem;
    req.tv_nsec = 0L;
    req.tv_sec = timeout_secs;
    nanosleep(&req, &rem);
    // woke up: either sleep finished, or interrupted by signal here
    // on waking up, is the child still alive?
    int status;
    if (!waitpid(child_pid, &status, WNOHANG)) {
      // child is still alive after the time-out
      kill(child_pid, SIGKILL);
      return PE_TIMEOUT;
    } else {
      // Either success (zero status), or abnormal termination of
      // child (non-zero status), indicated to caller via return
      // status
      return status;
    } 
  }
};
#endif
