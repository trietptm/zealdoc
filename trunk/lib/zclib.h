/* zclib.h -- interface of the 'zclib' 
 *
 */

#ifndef __ZCLIB_H__
#define __ZCLIB_H__

int zclib_open(const char *name, int flag);
void zclib_close(int fd);

#endif /* __ZCLIB_H__ */
