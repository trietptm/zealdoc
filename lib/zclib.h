/* zclib.h -- interface of the 'zclib' 
 *
 */

#ifndef __ZCLIB_H__
#define __ZCLIB_H__

int zc_open(const char *name, int flag);
void zc_close(int fd);

#endif /* __ZCLIB_H__ */
