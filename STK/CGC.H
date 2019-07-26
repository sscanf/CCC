

#define MAX_TAREAS 10
#define T_TRUC_GRUP	198

#define T_PTT_ON	0x20
#define T_PTT_OFF 0x21
#define T_ASI		0x15
#define T_VEU		0x0a
#define T_FI_VEU	0x0b
#define T_PRG_IND 0x0c
#define T_MOD		0x16
#define T_ANU		0x09

#define RESET		0xff
#define mDES		0x01
#define mOCU		0x02
#define mPAR		0x03
#define mFIN		0x04
#define mERR		0x05
#define mEME		0x06
#define mVEU		0x07
#define mTAN		0x08
#define mMEC		0x09
#define mLLI		0x10
#define mSQL		0x22
#define mNOSQL		0x11
#define mSEND_PROG 0x12
#define mCLEARED	0x13

#define CONFIRMACIO_AVIS_SOCI 9

#define T_INI	 	100
#define T_DESCON 	101

#define LIBRE		200
#define OCUPADO	201
#define FINAL		202

#define RTS			0x01
#define PTT			0x02
#define DTR			0x04
#define CTS			0x08
#define RESET		0x10
#define WATCH_DOG 0x80


extern void send_CGC_message (char *mensaje, int len);
extern void InitSys(void);
extern void InitPorts(void);
extern void InitCom(void);
extern void timer_init(void);
void EncolaTarea (unsigned char flota, unsigned char IndiH, unsigned char IndiL, unsigned int mensaje);


typedef struct
{
	unsigned char	flota;
	unsigned char	IndiH;
	unsigned char	IndiL;
	unsigned int 	mensaje;
	unsigned	int   NumBytes;
	char	bloque[48];
	char  status;
}
tareas;

struct MensToCGC
{
	unsigned char header1;
	unsigned char header2;
	unsigned char flota;
	unsigned char IndiH;
	unsigned char IndiL;
	unsigned char mensaje;
	unsigned char nBytes;

};

struct MovilActual
{
	unsigned char flota;
	unsigned char IndiH;
	unsigned char IndiL;
};