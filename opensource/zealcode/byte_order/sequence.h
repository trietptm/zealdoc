#ifndef		__CJ_SEQ_H__
#define		__CJ_SEQ_H__

#if defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)

  #define htons(A) (A)
  #define htonl(A) (A)
  #define ntohs(A) (A)
  #define ntohl(A) (A)

#elif defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

  #define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                    (((uint16_t)(A) & 0x00ff) << 8))
  #define htonl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
                    (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                    (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                    (((uint32_t)(A) & 0x000000ff) << 24))

  #define ntohs  htons
  #define ntohl  htohl

#else

 // #error "Must define one of BIG_ENDIAN or LITTLE_ENDIAN"

#endif

#endif /* __CJ_SEQ_H__*/