/*C
 * Original project: Lars Arge, Jeff Chase, Pat Halpin, Laura Toma, Dean
 *                   Urban, Jeff Vitter, Rajiv Wickremesinghe 1999
 * 
 * GRASS Implementation: Lars Arge, Helena Mitasova, Laura Toma 2002
 *
 * Copyright (c) 2002 Duke University -- Laura Toma 
 *
 * Copyright (c) 1999-2001 Duke University --
 * Laura Toma and Rajiv Wickremesinghe
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Duke University
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE TRUSTEES AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TRUSTEES OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *C*/


#ifndef _MINMAXHEAP_H
#define _MINMAXHEAP_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include <sstream>
using namespace std;

#include "mm_utils.h"
#include "ami_config.h"  //for SAVE_MEMORY flag
/* this flag is set if we are stingy on memory; in that case 'reset'
   deletes the pq memory and the subsequent insert reallocates it; if
   the operation following a reset is not an insert or an operation
   which does not touch the array A, behaviour is unpredictable (core
   dump probably) */





/***************************************************************** 
 ***************************************************************** 
 ***************************************************************** 

Priority queue templated on a single type (assumed to be a class with
getPriority() and getValue() implemented);

Supported operations: min, extract_min, insert, max, extract_max in
O(lg n)

***************************************************************** 
***************************************************************** 
*****************************************************************/

#undef XXX
#define XXX if(0)


#define MY_LOG_DEBUG_ID(x) //inhibit debug printing
//#define MY_LOG_DEBUG_ID(x) LOG_DEBUG_ID(x)

typedef unsigned int HeapIndex;


template <class T>
class BasicMinMaxHeap {

protected:  
  HeapIndex maxsize;
  HeapIndex lastindex;			// last used position (0 unused) (?)
  T *A;
  /* couple of memory mgt functions to keep things consistent */
  static T *allocateHeap(HeapIndex n);
  static void freeHeap(T *);
  virtual void grow()=0;
  
public:
  BasicMinMaxHeap(HeapIndex size) : maxsize(size) { 
    char str[100];
    sprintf(str, "BasicMinMaxHeap: allocate %ld\n", 
			(long)((size+1)*sizeof(T)));
    // MEMORY_LOG(str);
    
    lastindex = 0;
    MY_LOG_DEBUG_ID("minmaxheap: allocation");
	A = allocateHeap(maxsize);
  };
  
  virtual ~BasicMinMaxHeap(void) { 
    MY_LOG_DEBUG_ID("minmaxheap: deallocation");
	freeHeap(A);
  };

  bool empty(void) const { return size() == 0; };
  HeapIndex size() const;

  T get(HeapIndex i) const { assert(i <= size()); return A[i]; }
   
  //build a heap from an array of elements; 
  //if size > maxsize, insert first maxsize elements from array;
  //return nb of elements that did not fit;
  
  void insert(const T& elt);

  bool min(T& elt) const ;
  bool extract_min(T& elt);
  bool max(T& elt) const;
  bool extract_max(T& elt);
  //extract all elts with min key, add them and return their sum
  bool extract_all_min(T& elt);

  void reset();

  void destructiveVerify();
  void verify();
  
  void print() const;
  void print_range() const;
  friend ostream& operator<<(ostream& s, const BasicMinMaxHeap<T> &pq) {
    HeapIndex i;
    s <<  "[";
    for(i = 1; i <= pq.size(); i++) {
      s << " " << pq.get(i);
    }
    s << "]";
    return s;
  }


private:
  // Changed log2() to log2_() just in case log2() macro was already
  // defined in math.h: e.g., log2() is defined in Cygwin gcc by default.
  long log2_(long n) const;
  int isOnMaxLevel(HeapIndex i) const { return (log2_(i) % 2); };
  int isOnMinLevel(HeapIndex i) const { return !isOnMaxLevel(i); };

  HeapIndex leftChild(HeapIndex i) const { return 2*i; };
  HeapIndex rightChild(HeapIndex i) const { return 2*i + 1; };
  int hasRightChild(HeapIndex i) const { return (rightChild(i) <= size()); };
  HeapIndex parent(HeapIndex i) const { return (i/2); };
  HeapIndex grandparent(HeapIndex i) const { return (i/4); };
  int hasChildren(HeapIndex i) const { return (2*i) <= size(); }; // 1 or more
  void swap(HeapIndex a, HeapIndex b);

  T leftChildValue(HeapIndex i) const;
  T rightChildValue(HeapIndex i) const;
  HeapIndex smallestChild(HeapIndex i) const;
  HeapIndex smallestChildGrandchild(HeapIndex i) const;
  HeapIndex largestChild(HeapIndex i) const;
  HeapIndex largestChildGrandchild(HeapIndex i) const;
  int isGrandchildOf(HeapIndex i, HeapIndex m) const;
  
  void trickleDownMin(HeapIndex i);
  void trickleDownMax(HeapIndex i);
  void trickleDown(HeapIndex i);
  
  void bubbleUp(HeapIndex i);
  void bubbleUpMin(HeapIndex i);
  void bubbleUpMax(HeapIndex i);
};


// index 0 is invalid
// index <= size


// ----------------------------------------------------------------------
template <class T>
HeapIndex BasicMinMaxHeap<T>::size() const { 
	assert(A || !lastindex);
  return lastindex; 
}

// ----------------------------------------------------------------------

template <class T> 
long BasicMinMaxHeap<T>::log2_(long n) const {
  long i=-1;
  // let log2_(0)==-1
  while(n) {
	n = n >> 1;
	i++;
  }
  return i;
}


// ----------------------------------------------------------------------

template <class T> 
void BasicMinMaxHeap<T>::swap(HeapIndex a, HeapIndex b) {
  T tmp;
  tmp = A[a];
  A[a] = A[b];
  A[b] = tmp;
}


// ----------------------------------------------------------------------

// child must exist
template <class T>
T BasicMinMaxHeap<T>::leftChildValue(HeapIndex i) const {
  HeapIndex p = leftChild(i);
  assert(p <= size());
  return A[p];
}

// ----------------------------------------------------------------------

// child must exist
template <class T>
T BasicMinMaxHeap<T>::rightChildValue(HeapIndex i) const {
  HeapIndex p = rightChild(i);
  assert(p <= size());
  return A[p];
}


// ----------------------------------------------------------------------

// returns index of the smallest of children of node
// it is an error to call this function if node has no children
template <class T>
HeapIndex BasicMinMaxHeap<T>::smallestChild(HeapIndex i) const {
  assert(hasChildren(i));
  if(hasRightChild(i) && (leftChildValue(i) > rightChildValue(i))) {
	return rightChild(i);
  } else {
	return leftChild(i);
  }
}

// ----------------------------------------------------------------------

template <class T>
HeapIndex BasicMinMaxHeap<T>::largestChild(HeapIndex i) const {
  assert(hasChildren(i));
  if(hasRightChild(i) && (leftChildValue(i) < rightChildValue(i))) {
	return rightChild(i);
  } else {
	return leftChild(i);
  }
}

// ----------------------------------------------------------------------

// error to call on node without children
template <class T>
HeapIndex BasicMinMaxHeap<T>::smallestChildGrandchild(HeapIndex i) const {
  HeapIndex p,q;
  HeapIndex minpos = 0;

  assert(hasChildren(i));

  p = leftChild(i);
  if(hasChildren(p)) {
	q = smallestChild(p);
	if(A[p] > A[q]) p = q;
  }
  // p is smallest of left child, its grandchildren
  minpos = p;

  if(hasRightChild(i)) {
	p = rightChild(i);
	if(hasChildren(p)) {
	  q = smallestChild(p);
	  if(A[p] > A[q]) p = q;
	}
	// p is smallest of right child, its grandchildren
	if(A[p] < A[minpos]) minpos = p;
  }
  return minpos;
}

// ----------------------------------------------------------------------

template <class T>
HeapIndex BasicMinMaxHeap<T>::largestChildGrandchild(HeapIndex i) const {
  HeapIndex p,q;
  HeapIndex maxpos = 0;

  assert(hasChildren(i));

  p = leftChild(i);
  if(hasChildren(p)) {
	q = largestChild(p);
	if(A[p] < A[q]) p = q;
  }
  // p is smallest of left child, its grandchildren
  maxpos = p;

  if(hasRightChild(i)) {
	p = rightChild(i);
	if(hasChildren(p)) {
	  q = largestChild(p);
	  if(A[p] < A[q]) p = q;
	}
	// p is smallest of right child, its grandchildren
	if(A[p] > A[maxpos]) maxpos = p;
  }
  return maxpos;
}

// ----------------------------------------------------------------------

// this is pretty loose - only to differentiate between child and grandchild
template <class T>
int BasicMinMaxHeap<T>::isGrandchildOf(HeapIndex i, HeapIndex m) const {
  return (m >= i*4);
}

// ----------------------------------------------------------------------

template <class T>
void BasicMinMaxHeap<T>::trickleDownMin(HeapIndex i) {
  HeapIndex m;
  bool done = false;
  
  while (!done) {
    
    if (!hasChildren(i)) {
      done = true;
      return;
    }
    m = smallestChildGrandchild(i);
    if(isGrandchildOf(i, m)) {
      if(A[m] < A[i]) {
	swap(i, m);
	if(A[m] > A[parent(m)]) {
	  swap(m, parent(m));
	}
	//trickleDownMin(m);
	i = m;
      } else {
	done = true;
      }
    } else {
      if(A[m] < A[i]) {
	swap(i, m);
      }
      done = true;
    }
  }//while
}

// ----------------------------------------------------------------------

// unverified
template <class T>
void BasicMinMaxHeap<T>::trickleDownMax(HeapIndex i) {
  HeapIndex m;
  bool done = false;

  while (!done) {
    if(!hasChildren(i)) {
     done = true;
     return;
    }
    
    m = largestChildGrandchild(i);
    if(isGrandchildOf(i, m)) {
      if(A[m] > A[i]) {
	swap(i, m);
	if(A[m] < A[parent(m)]) {
	  swap(m, parent(m));
	}
	//trickleDownMax(m);
	i = m;
      } else {
	done = true;
      }
    } else {
      if(A[m] > A[i]) {
	swap(i, m);
      }
      done = true;
    }
  } //while
}


// ----------------------------------------------------------------------


template <class T>
void BasicMinMaxHeap<T>::trickleDown(HeapIndex i) {
  if(isOnMinLevel(i)) {
	trickleDownMin(i);
  } else {
	trickleDownMax(i);
  }
}

// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::bubbleUp(HeapIndex i) {
  HeapIndex m;
  m = parent(i);
  
  if(isOnMinLevel(i)) {
	if (m && (A[i] > A[m])) {
	  swap(i, m);
	  bubbleUpMax(m);
	} else {
	  bubbleUpMin(i);
	} 
  } else {
	if (m && (A[i] < A[m])) {
	  swap(i, m);
	  bubbleUpMin(m);
	} else {
	  bubbleUpMax(i);
	}
  }
}


// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::bubbleUpMin(HeapIndex i) {
  HeapIndex m;
  m = grandparent(i);

  while (m && (A[i] < A[m])) {
	 swap(i,m);
	 //bubbleUpMin(m);
	 i = m;
	 m = grandparent(i);
	 
  }
}



// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::bubbleUpMax(HeapIndex i) {
  HeapIndex m;
  m = grandparent(i);
  
  while(m && (A[i] > A[m])) {
	swap(i,m);
	//bubbleUpMax(m);
	i=m;
	m = grandparent(i);
  }
}


#if(0)
// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::print_rajiv() const {
  HeapIndex i;
  ostrstream *ostr = new ostrstream();
  
  *ostr << "[1]";
  for(i=1; i<=size(); i++) {
	*ostr << " " << A[i];
	if(ostr->pcount() > 70) {
	  cout << ostr->str() << endl;
	  delete ostr;
	  ostr = new ostrstream();
	  *ostr << "[" << i << "]";
	}
  }
  cout << ostr->str() << endl;
}
#endif



// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::print() const {
  cout << "[";
  for (unsigned int i=1; i<=size(); i++) {
    cout << A[i].getPriority() <<",";
  }
  cout << "]" << endl;
}

// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::print_range() const {
  cout << "[";
  T a, b;
  min(a);
  max(b);
  if (size) {
    cout << a.getPriority() << ".."
	 << b.getPriority();
  }
  cout << " (" << size() << ")]";
}


// ----------------------------------------------------------------------
template <class T>
void BasicMinMaxHeap<T>::insert(const T& elt) {
#ifdef SAVE_MEMORY 
  if (!A) {
    MY_LOG_DEBUG_ID("minmaxheap: re-allocation");
    A = allocateHeap(maxsize);
  }
#endif

  if(lastindex == maxsize) grow();

  XXX cerr << "insert: " << elt << endl;

  lastindex++;
  A[lastindex] = elt;
  bubbleUp(lastindex);
}

// ----------------------------------------------------------------------
template <class T>
bool BasicMinMaxHeap<T>::extract_min(T& elt) {

  assert(A);

  if(lastindex == 0) return false;

  elt = A[1];
  A[1] = A[lastindex];
  lastindex--;
  trickleDown(1);
  
  return true;
}

// ----------------------------------------------------------------------
//extract all elts with min key, add them and return their sum
template <class T>
bool BasicMinMaxHeap<T>::extract_all_min(T& elt) {
  T next_elt;
  bool done = false;
  
  //extract first elt
  if (!extract_min(elt)) {
    return false; 
  } else {
    while (!done) {
      //peek at the next min elt to see if matches
      if ((!min(next_elt)) || 
	  !(next_elt.getPriority() == elt.getPriority())) {
	done = true; 
      } else {
	extract_min(next_elt);
	elt = elt + next_elt;
      }
    }
  }
  return true;
}

// ----------------------------------------------------------------------
template <class T>
bool BasicMinMaxHeap<T>::extract_max(T& elt) {

  assert(A);
  
  HeapIndex p;					// max
  if(lastindex == 0) return false;
  
  if(hasChildren(1)) {
	p = largestChild(1);
  } else {
	p = 1;
  }
  elt = A[p];
  A[p] = A[lastindex];
  lastindex--;
  trickleDown(p);
  
  return true;
}

// ----------------------------------------------------------------------
template <class T>
bool BasicMinMaxHeap<T>::min(T& elt) const {
  
  assert(A);
  
  if(lastindex == 0) return false;

  elt = A[1];
  return true;
}

// ----------------------------------------------------------------------
template <class T>
bool BasicMinMaxHeap<T>::max(T& elt) const {
  
  assert(A);
  
  HeapIndex p;					// max
  if(lastindex == 0) return false;
  
  if(hasChildren(1)) {
	p = largestChild(1);
  } else {
	p = 1;
  }
  elt = A[p];
  return true;
}



// ----------------------------------------------------------------------
//free memory if SAVE_MEMORY is set
template <class T>
void BasicMinMaxHeap<T>::reset() {
#ifdef SAVE_MEMORY
  assert(empty());
  MY_LOG_DEBUG_ID("minmaxheap: deallocation");
  freeHeap(A);
  A = NULL;
#endif
}

// ----------------------------------------------------------------------
template <class T> 
T *
BasicMinMaxHeap<T>::allocateHeap(HeapIndex n) {
  T *p;
#ifdef USE_LARGEMEM
  p = (T*)LargeMemory::alloc(sizeof(T) * (n+1));
#else
  p = new T[n+1]; 
#endif
  return p;
}

// ----------------------------------------------------------------------
template <class T> 
void
BasicMinMaxHeap<T>::freeHeap(T *p) {
  if (p) {
#ifdef USE_LARGEMEM
	LargeMemory::free(p);
#else
	delete [] p;
#endif
  }
}
  

// ----------------------------------------------------------------------

template <class T> 
void
BasicMinMaxHeap<T>::destructiveVerify() {
  HeapIndex n = size();
  T val, prev;
  bool ok;

  if(!n) return;

  XXX print();

  /* make sure that min works */
  extract_min(prev);
  for(HeapIndex i=1; i<n; i++) {
	ok = min(val);
	assert(ok);
	XXX cerr << i << ": " << val << endl;
	if(val.getPriority() < prev.getPriority()) { // oops!
	  print();
	  cerr << "n=" << n << endl;
	  cerr << "val=" << val << endl;
	  cerr << "prev=" << prev << endl;
	  cerr << "looks like minmaxheap.min is broken!!" << endl;
	  assert(0);
	  return;
	}
	prev = val;
	ok = extract_min(val);
	assert(ok);
	assert(prev == val);
  }
}


// ----------------------------------------------------------------------

template <class T> 
void
BasicMinMaxHeap<T>::verify() {
  long n = size();
  T *dup;
  
  if(!n) return;

  dup = allocateHeap(maxsize);
  for(HeapIndex i=0; i<n+1; i++) {
	dup[i] = A[i];
  }
  destructiveVerify();
  freeHeap(A);
  /* restore the heap */
  A = dup;
  lastindex = n;
}



// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

template <class T>
class MinMaxHeap : public BasicMinMaxHeap<T> {

//using BasicMinMaxHeap<T>::maxsize;
//using BasicMinMaxHeap<T>::lastindex;
//using BasicMinMaxHeap<T>::size;

public:
  MinMaxHeap(HeapIndex size) : BasicMinMaxHeap<T>(size) {};
  virtual ~MinMaxHeap() {};
  bool full(void) const { return this->size() >= this->maxsize; };
  HeapIndex get_maxsize() const { return this->maxsize; };
  HeapIndex fill(T* arr, HeapIndex n);
  
protected:
  virtual void grow() { assert(0); exit(1); };
};

// ----------------------------------------------------------------------
//build a heap from an array of elements; 
//if size > maxsize, insert first maxsize elements from array;
//return nb of elements that did not fit;
template <class T>
HeapIndex MinMaxHeap<T>::fill(T* arr, HeapIndex n) {
  HeapIndex i;
  //heap must be empty
  assert(get_maxsize()==0);
  for (i = 0; !full() && i<n; i++) {
    insert(arr[i]);
  }
  if (i < n) {
    assert(i == get_maxsize());
    return n - i;
  } else {
    return 0;
  }
}



#define MMHEAP_INITIAL_SIZE 1024

template <class T>
class UnboundedMinMaxHeap : public BasicMinMaxHeap<T> {

using BasicMinMaxHeap<T>::A;
using BasicMinMaxHeap<T>::maxsize;
using BasicMinMaxHeap<T>::size;

public:
  UnboundedMinMaxHeap() : BasicMinMaxHeap<T>(MMHEAP_INITIAL_SIZE) {};
  virtual ~UnboundedMinMaxHeap() {};
protected:
  virtual void grow();
};

template <class T>
void UnboundedMinMaxHeap<T>::grow() {
  T *old = A;
  maxsize *= 2;

  if(old) {
	A = allocateHeap(maxsize);	/* allocate a new array */
	/* copy over the old values */
	for(int i=0; i<size()+1; i++) {
	  A[i] = old[i];
	}	
	freeHeap(old);				/* free up old storage */
  }

}


#endif
