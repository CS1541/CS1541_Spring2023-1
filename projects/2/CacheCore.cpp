#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "CacheCore.h"

CacheCore::CacheCore(uint32_t s, uint32_t a, uint32_t b, const char *pStr)
  : size(s)
  ,lineSize(b)
  ,assoc(a)
  ,numLines(s/b)
{
  if (strcasecmp(pStr, "RANDOM") == 0)
    policy = RANDOM;
  else if (strcasecmp(pStr, "LRU") == 0)
    policy = LRU;
  else {
    assert(0);
  }

  content = new CacheLine[numLines + 1];

  for(uint32_t i = 0; i < numLines; i++) {
    content[i].initialize();
  }
}

CacheCore::~CacheCore() {
  delete [] content;
}

CacheLine *CacheCore::accessLine(uint32_t addr)
{
  // TODO: Implement
  
  int cacheBlockOffset = log2i(getLineSize()); 
  int rowBits = log2i(getNumLines()/getAssoc());                     
  int tagSize = 32 - rowBits - cacheBlockOffset;     
  uint32_t tag = addr >> (32-tagSize);                  
  uint32_t shift = (addr >> cacheBlockOffset) & (( 1 << rowBits)-1);
  int64_t x = -1;


  for(uint32_t i = shift * getAssoc(); i < (shift * getAssoc())+ getAssoc(); i++) {          
    if(content[i].isValid()){
      content[i].incAge();                            
      if(content[i].getTag() == tag){ 
        x = i;                  
        content[i].resetAge();                                           
      }
    }
  } 
  if(x>=0) return &content[x];
  else return NULL;
}

CacheLine *CacheCore::allocateLine(uint32_t addr, uint32_t *rplcAddr) {

  int cacheBlockOffset = log2i(getLineSize()); 
                              
  int rowBits = log2i(getNumLines()/getAssoc());    
  int tagSize = 32 - rowBits - cacheBlockOffset;      
  uint32_t tag = addr >> (32-tagSize);
  uint32_t shift = (addr >> cacheBlockOffset ) & (( 1 << rowBits)-1);

  
  int indx = shift * getAssoc();                 
  uint32_t oldest = 0;              
  *rplcAddr = 0;  
  for(uint32_t i = shift * getAssoc(); i < (shift * getAssoc()) + getAssoc(); i++){
    if(content[i].getAge()>oldest){ 
      indx = i;                    
      oldest = content[i].getAge(); 
    }
    if(!content[i].isValid()){     
      content[i].initialize();
      content[i].validate();        
      content[i].setTag(tag);       
      return &content[i];           
    }
  }


  
  if(content[indx].isDirty() && content[indx].isValid()){       
      *rplcAddr = (content[indx].getTag() << (32 - tagSize)) | (index2Row(indx) << cacheBlockOffset) ;
  }
  content[indx].initialize();
  content[indx].validate();     
  content[indx].setTag(tag);      

  return &content[indx];             
}
