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


#ifndef __EMPQ_ADAPTIVE_IMPL_H
#define __EMPQ_ADAPTIVE_IMPL_H

#include <stdio.h>
#include <assert.h>

#include "ami_config.h"
#include "ami_stream.h"
#include "mm.h"
#include "mm_utils.h"
#include "empq_adaptive.h"



//defined in "empqAdaptive.H"
//#define EMPQAD_DEBUG if(0)




//------------------------------------------------------------
//allocate an internal pqueue of size precisely twice 
//the size of the pqueue within the em_pqueue;

template<class T, class Key> 
EMPQueueAdaptive<T,Key>::EMPQueueAdaptive() {
  regim = INMEM;
  EMPQAD_DEBUG cout << "EMPQUEUEADAPTIVE: starting in-memory pqueue" 
		    << endl;
  
  //------------------------------------------------------------
  //set the size precisely as in empq constructor since we cannot 
  //really call the em__pqueue constructor, because we don't want 
  //the space allocated; we just want the sizes;
  AMI_err ae;
  size_t mm_avail = getAvailableMemory();
  EMPQAD_DEBUG cout << "EMPQUEUEADAPTIVE: available memory: " 
		    << ( (float)mm_avail/ (1<< 20)) << "MB" << endl;
  
  /* same calculations as empq constructor in order to estimate
     overhead memory; this is because we want to allocate a pqueue of
     size exactly double the size of the pqueue inside the empq;
     switching from this pqueue to empq when the memory fills up will
     be simple */
  //------------------------------------------------------------
  //AMI_STREAM memory usage
  size_t sz_stream;
  AMI_STREAM<T> dummy;
  if ((ae = dummy.main_memory_usage(&sz_stream,
				    MM_STREAM_USAGE_MAXIMUM)) !=
      AMI_ERROR_NO_ERROR) {
    cerr << "EMPQueueAdaptive constr: failing to get stream_usage\n";
    exit(1);
  }  
  //account for temporary memory usage
  unsigned short max_nbuf = 2;
  unsigned int buf_arity = mm_avail/(2 * sz_stream);
  unsigned long mm_overhead = buf_arity*sizeof(merge_key<Key>) + 
    max_nbuf * sizeof(em_buffer<T,Key>) +
    2*sz_stream + max_nbuf*sz_stream;
  mm_overhead *= 8; //overestimate..this should be fixed with
  //a precise accounting of the extra memory required
  EMPQAD_DEBUG cout << "EMPQUEUEADAPTIVE: memory overhead set to " 
		    << ((float)mm_overhead / (1 << 20)) << "MB" << endl;
  if (mm_overhead > mm_avail) {
    cerr << "overhead bigger than available memory ("<< mm_avail << "); "
	 << "increase -m and try again\n";
    exit(1);
  }
  mm_avail -= mm_overhead;
  //------------------------------------------------------------


  long pqsize = mm_avail/sizeof(T);
  EMPQAD_DEBUG cout << "EMPQUEUEADAPTIVE: pqsize set to " << pqsize << endl;

  //initialize in memory pqueue and set em to NULL
  im = new MinMaxHeap<T>(pqsize);
  assert(im);
  em = NULL;
};




template<class T, class Key> 
EMPQueueAdaptive<T,Key>::~EMPQueueAdaptive() {
  switch(regim) {
  case INMEM:
	delete im;
	break;
  case EXTMEM:
	delete em; 
	break;
  case EXTMEM_DEBUG:
	delete dim;
	delete em; 
	break;
  }
};



//return the maximum nb of elts that can fit
template<class T, class Key> 
long 
EMPQueueAdaptive<T,Key>::maxlen() const {
  long m;
  switch(regim) {
  case INMEM:
	assert(im);
	m = im->get_maxsize();
	break;
  case EXTMEM:
	assert(em);
	m = em->maxlen();
	break;
  case EXTMEM_DEBUG:
	m = em->maxlen();
	break;
  }
  return m;
};




//return true if empty
template<class T, class Key> 
bool
EMPQueueAdaptive<T,Key>::is_empty() const {
  bool v = false;
  switch(regim) {
  case INMEM:
	assert(im);
	v = im->empty();
	break;
  case EXTMEM:
	assert(em);
	v = em->is_empty(); 
	break;
  case EXTMEM_DEBUG:
	assert(dim->empty() == em->is_empty());
	v = em->is_empty(); 
	break;
  }
  return v;
};


//return true if full
template<class T, class Key> 
bool
EMPQueueAdaptive<T,Key>::is_full() const { 
  cerr << "EMPQueueAdaptive::is_full(): sorry not implemented\n";
  assert(0);
  exit(1);
}


//return the element with minimum priority in the structure
template<class T, class Key> 
bool
EMPQueueAdaptive<T,Key>::min(T& elt) {
  bool v=false, v1;
  T tmp;
  switch(regim) {
  case INMEM:
	assert(im);
	v = im->min(elt);
	break;
  case EXTMEM:
	assert(em);
	v = em->min(elt);
	break;
  case EXTMEM_DEBUG:
	v1 = dim->min(tmp);
	v = em->min(elt);
	//dim->verify();
	if(!(tmp==elt)) {
	  cerr << "------------------------------" << endl;
	  cerr << dim << endl;
	  cerr << "------------------------------" << endl;
	  em->print();
	  cerr << "------------------------------" << endl;
	  cerr << "tmp=" << tmp << endl;
	  cerr << "elt=" << elt << endl;
	  cerr << "------------------------------" << endl;
	  dim->destructiveVerify();
	}
	assert(v == v1);
	assert(tmp == elt);
	break;
  }
  return v;
};

template<class T, class Key> 
void
EMPQueueAdaptive<T,Key>::verify() {
  switch(regim) {
  case INMEM:
	im->verify();
	break;
  case EXTMEM:
	break;
  case EXTMEM_DEBUG:
	dim->verify();
	break;
  }
}

//extract all elts with min key, add them and return their sum
template<class T, class Key> 
bool 
EMPQueueAdaptive<T,Key>::extract_all_min(T& elt) {
  bool v=false, v1;
  T tmp;
  switch(regim) {
  case INMEM:
	assert(im);
	v = im->extract_all_min(elt);
	break;
  case EXTMEM:
	assert(em);
	v = em->extract_all_min(elt);
	break;
  case EXTMEM_DEBUG:
	v1 =  dim->extract_all_min(tmp);
	v = em->extract_all_min(elt);
	assert(dim->BasicMinMaxHeap<T>::size() == em->size());
	assert(v == v1);
	assert(tmp == elt);
	break;
  }
  return v;
};

//return the nb of elements in the structure 
template<class T, class Key> 
long
EMPQueueAdaptive<T,Key>::size() const {
  long v=0, v1;
  switch(regim) {
  case INMEM:
	assert(im);
	v = im->size();
	break;
  case EXTMEM:
	assert(em);
	v = em->size();
	break;
  case EXTMEM_DEBUG:
	v1 = dim->BasicMinMaxHeap<T>::size();
	v = em->size();
	assert(v == v1);
	break;
  }
  return v;
}




// ----------------------------------------------------------------------
template<class T, class Key> 
bool 
EMPQueueAdaptive<T,Key>::extract_min(T& elt) {
    bool v=false, v1;
	T tmp;
	switch(regim) {
	case INMEM:
	  assert(im);
	  v = im->extract_min(elt);
	  break;
	case EXTMEM:
	  assert(em);
	  v = em->extract_min(elt);
	  break;
	case EXTMEM_DEBUG:
	  v1 =  dim->extract_min(tmp);
	  v = em->extract_min(elt);
	  assert(v == v1);
	  assert(tmp == elt);
	  assert(dim->BasicMinMaxHeap<T>::size() == em->size());
	  break;
    }
	return v;
};
 



//------------------------------------------------------------
 /* insert an element; if regim == INMEM, try insert it in im, and if
    it is full, extract_max pqsize/2 elements of im into a stream,
    switch to EXTMEM and insert the stream into em; if regim is
    EXTMEM, insert in em; */
template<class T, class Key> 
bool 
EMPQueueAdaptive<T,Key>::insert(const T& elt) {
  bool v=false;
  switch(regim) {
  case INMEM:
	if (!im->full()) {
      im->insert(elt);
	  v = true;
    } else {
	  makeExternal();
	  v = em->insert(elt);      //insert the element
	} 
	break;
  case EXTMEM:
	v = em->insert(elt);
	break;
  case EXTMEM_DEBUG:
	dim->insert(elt);
	v = em->insert(elt);
	assert(dim->BasicMinMaxHeap<T>::size() == em->size());
	break;
  }
  return v;
};

template<class T, class Key> 
void
EMPQueueAdaptive<T,Key>::makeExternalDebug() {
  assert(size() == 0);
  switch(regim) {
  case INMEM:
	makeExternal();
	break;
  case EXTMEM:
	break;
  case EXTMEM_DEBUG:
	assert(0);
	break;
  }
  dim = new UnboundedMinMaxHeap<T>();
  regim = EXTMEM_DEBUG;
}



template<class T>
class baseCmpType {
public:
  static int compare(const T& x, const T& y) {
    return  (x < y ? -1 : (x > y ? 1 : 0));
  }

};

/* switch over to using an external priority queue */
template<class T, class Key> 
void
EMPQueueAdaptive<T,Key>::makeExternal() {
  AMI_err ae;
#ifndef NDEBUG
  long sizeCheck;
  sizeCheck = size();
#endif

  assert(regim == INMEM);  
  regim = EXTMEM;

  EMPQAD_DEBUG cout << endl 
			 << "EMPQUEUEADAPTIVE: memory full: "
			 << "switching to external-memory pqueue " << endl;
  
  //create an AMI_stream and write in it biggest half elts of im;
  AMI_STREAM<T> *amis0 = new AMI_STREAM<T>();
  AMI_STREAM<T> *amis1;
  assert(amis0 && amis1);
  unsigned long pqsize = im->size();
  //assert(im->size() == im->get_maxsize());
  T x;
  for (unsigned long i=0; i< pqsize/2; i++) {
	int z = im->extract_max(x);
	assert(z);
	ae = amis0->write_item(x);
	assert(ae == AMI_ERROR_NO_ERROR);
  }
  assert(amis0->stream_len() == pqsize/2);
  EMPQAD_DEBUG { cout << "written " << pqsize/2 
			   << " elts to stream\n"; cout.flush(); }
  
  assert(im->size() == pqsize/2 + (pqsize % 2));
  
  EMPQAD_DEBUG LOG_avail_memo();
  
  //sort the stream
  baseCmpType<T> fun;
  AMI_sort(amis0, &amis1, &fun); //XXX laura: replaced this to use a cmp obj
  delete amis0;
  EMPQAD_DEBUG { cout << "sorted the stream\n"; cout.flush(); }
  
  EMPQAD_DEBUG LOG_avail_memo();
  
  //set im to NULL and initialize em from im and amis1
  em = new em_pqueue<T,Key>(im, amis1);
  im = NULL;
  assert(em);
  EMPQAD_DEBUG { cout << "empq initialized from im\n"; cout.flush(); }
  EMPQAD_DEBUG {em->print_size();}
  
  EMPQAD_DEBUG LOG_avail_memo();
#ifndef NDEBUG
  assert(sizeCheck == size());
#endif
};

#endif
