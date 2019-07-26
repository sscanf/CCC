/*      moxa-c.h        ver 2.00        03/03/1993
 *      Definitions for MOXA Serial I/O Controller ioctrl
 */

/*      BAUD rate setting       */
#define B50             0x00
#define B75             0x01
#define B110            0x02
#define B134            0x03
#define B150            0x04
#define B300            0x05
#define B600            0x06
#define B1200           0x07
#define B1800           0x08
#define B2400           0x09
#define B4800           0x0A
#define B7200           0x0B
#define B9600           0x0C
#define B19200          0x0D
#define B38400          0x0E
#define B57600          0x0F
#define B115200         0x10

/*      MODE setting            */
#define BIT_5           0x00                    /* Word length define   */
#define BIT_6           0x01
#define BIT_7           0x02
#define BIT_8           0x03

#define STOP_1          0x00                    /* Stop bits define     */
#define STOP_2          0x04

#define P_EVEN          0x18                    /* Parity define        */
#define P_ODD           0x08
#define P_SPC           0x38
#define P_MRK           0x28
#define P_NONE          0x00

/*      MODEM CONTROL setting   */
#define C_DTR           0x01
#define C_RTS           0x02

/*      MODEM LINE STATUS       */
#define S_CTS           0x01
#define S_DSR           0x02
#define S_RI            0x04
#define S_CD            0x08
/*
long    sio_iqueue(int port);
long    sio_ifree(int port);
long    sio_oqueue(int port);
long    sio_ofree(int port);
*/
/**********************************************************/
/*           For Borland C++ version 2.00                 */
/**********************************************************/
/* If you are using Borland C++ v2.00, remove the comment */
/* from the following paragraph.                          */
/**********************************************************/


#ifdef __cplusplus
extern "C" {
#endif
int     cdecl   sio_reset();
int     cdecl   sio_getports(int *port_no_array, int array_size);
int     cdecl   sio_loopback(int port, char *buf, int len);
int     cdecl   sio_ioctl(int port, int baud, int mode);
int     cdecl   sio_getch(int port);
int     cdecl   sio_linput(int port, char *buf, int len, int term);
int     cdecl   sio_read(int port, char *buf, int len);
int     cdecl   sio_putch(int port, int term);
int     cdecl   sio_putb(int port, char *buf, int len);
int     cdecl   sio_write(int port, char *buf, int len);
int     cdecl   sio_flush(int port, int func);
long    cdecl   sio_iqueue(int port);
long    cdecl   sio_ifree(int port);
long    cdecl   sio_oqueue(int port);
long    cdecl   sio_ofree(int port);
int     cdecl   sio_lstatus(int port);
int     cdecl   sio_lctrl(int port, int mode);
int     cdecl   sio_term_irq(int port, void interrupt (*func)(), char code);
int     cdecl   sio_cnt_irq(int port, void interrupt (*func)(), int count);
int     cdecl   sio_modem_irq(int port, void interrupt (*func)());
int     cdecl   sio_break_irq(int port, void interrupt (*func)());
int     cdecl   sio_break(int port, int time);
int     cdecl   sio_brk_cnt(int port);
int     cdecl   sio_flowctrl(int port, int mode);
int     cdecl   sio_Tx_hold(int port);
int     cdecl   sio_disableTx(int port);
int     cdecl   sio_enableTx(int port);
int     cdecl   sio_close(int port);
int     cdecl   sio_open(int port);
int     cdecl   sio_getbaud(int port);
int     cdecl   sio_getmode(int port);
int     cdecl   sio_getflow(int port);
int     cdecl   sio_timeout(int time_tic);
int     cdecl   sio_linput_t(int port, char *buf, int len, int term);
int     cdecl   sio_putb_t(int port, char *buf, int len);
int     cdecl   sio_overflow(int port);
int     cdecl   sio_overlap(int port, int mode);
int     cdecl   sio_cardno(int drvname);
int     cdecl   sio_id(int drvname, int card);
int     cdecl   sio_bank(int drvname, int card);
#ifdef __cplusplus
}
#endif

