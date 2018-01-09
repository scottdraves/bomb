/*
    bomb - automatic interactive visual stimulation
    Copyright (C) 1994  Scott Draves <spot@cs.cmu.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

static char *sound_c_id = "@(#) $Id: sound.c,v 1.1.1.1 2002/12/08 20:49:54 spotspot Exp $";

#include "defs.h"
#include "sound.h"

int sound_present;

#if 0
static double high_pass(double x) {
  static double last = 0.0;
  double r;
  r = x - last;
  last = x;
  return r;
}

static double rec_low_pass(double x) {
  static double r0 = 0.0;
  
  r0 = (x + r0) / 2.0;
  
  return r0;
}
#endif

static double rec_low_rec_low_pass(double x) {
  static double r0 = 0.0;
  static double r1 = 0.0;
  
  r0 = (x + 2*r0) / 3.0;
  r1 = (r0 + 2*r1) / 3.0;
  
  return r1;
}


int get_beat(int type) {
  double out, in = get_sound();
  switch (type) {
  case 0:
    out = in - rec_low_rec_low_pass(in);
    break;
  default:
  case 1:
    out = in;
    break;
  }
#if 0
  {
    int i;
    printf("\rgetbeat: [");
    for(i=0;i<60;i++) {
      printf("%c",i < (int) out ? 'X' : ' ');
    }
    printf("]");
    fflush( stdout );
  }
#endif
  return (int)out;
}

#if 0 && win_bomb


#include <windows.h>
#include <mmsystem.h>

#ifndef wbomb


#define snd_buf_size 1000
#define snd_nbufs 3

HWAVEIN hwi;
struct {
	WAVEHDR header;
	unsigned char buffer[snd_buf_size];
} snd_buffers[snd_nbufs];

int snd_current_buf;

double snd_power;

#define power_history_len 50
double power_history[power_history_len];
int power_index = 0;

void CALLBACK buffer_done(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

	int i, sum = 0, nsamps;
	unsigned char *samps;
	WAVEHDR *wh = &snd_buffers[snd_current_buf].header;

	if (uMsg != MM_WIM_DATA) return;

	nsamps = wh->dwBytesRecorded;
	samps = wh->lpData;
	wh->dwUser=1;

	for (i = 0; i < nsamps; i++) {
		/* actual zero value varies */
		int s = ((int)samps[i])-127;
		sum += s * s;
	}
	if (nsamps != 0) {
		double max_power = 0.0;
		int i;
		snd_power = sqrt(sum / nsamps);
		power_history[power_index++] = snd_power;
		if (power_history_len == power_index)
		  power_index = 0;
		for (i = 0; i < power_history_len; i++)
		  if (power_history[i] > max_power)
		    max_power = power_history[i];
		sprintf(status_string+10, "%05.1lf %05.1lf ", snd_power, max_power);
	}

	if (snd_nbufs == ++snd_current_buf)
		snd_current_buf = 0;
}

void init_sound()
{

	MMRESULT res;
    WAVEFORMATEX wfx;
    WAVEINCAPS wi;
	int i;

	sound_present = 0;
	return;

    res = waveInGetDevCaps(0, &wi, sizeof(wi));
	
    wfx.wFormatTag=WAVE_FORMAT_PCM; 
    wfx.nChannels=1; 
    wfx.nSamplesPerSec=44100; 
    wfx.nAvgBytesPerSec=44100; 
    wfx.nBlockAlign=1;
    wfx.wBitsPerSample=8;
    wfx.cbSize=0;

    res=waveInOpen(&hwi, WAVE_MAPPER, &wfx, 
		&buffer_done, 0, CALLBACK_FUNCTION); 
	
    if (res !=  MMSYSERR_NOERROR) {
		sound_present = 0;
		Beep(2000, 500);
		return;
	}

	for (i = 0; i < snd_nbufs; i++) {
		WAVEHDR *wh = &snd_buffers[i].header;
		wh->lpData=snd_buffers[i].buffer;
		wh->dwBufferLength=snd_buf_size;
		wh->dwBytesRecorded=0;
		wh->dwUser=0;
		wh->dwFlags=0;
		wh->dwLoops=0;
		wh->lpNext=0;
		wh->reserved=0;
		res=waveInPrepareHeader(hwi,wh,sizeof(WAVEHDR));
		res=waveInAddBuffer(hwi,wh,sizeof(WAVEHDR));
	}

	sound_present = 1;
	snd_power = 0.0;
	snd_current_buf = 0;

    res=waveInStart(hwi);

}

void exit_sound()
{
	MMRESULT res;
	int i;

	if (sound_present) {
		res=waveInReset(hwi);
		for (i = 0; i < snd_nbufs; i++) {
			WAVEHDR *wh = &snd_buffers[i].header;
			res=waveInUnprepareHeader(hwi,wh,sizeof(WAVEHDR));
		}
		waveInClose(hwi);
	}
}

int get_sound()
{
	int i, j;
	MMRESULT res;
	j = (snd_current_buf + 1) % snd_nbufs;
	for (i = 0; i < snd_nbufs; i++) {
		WAVEHDR *wh = &snd_buffers[j].header;
		if (wh->dwUser) {
			res=waveInAddBuffer(hwi,wh,sizeof(WAVEHDR));
			wh->dwUser = 0;
		}
		j = (j + 1) % snd_nbufs;

	}
	return (int) snd_power;
}



#elif 0
/* code from zaph */
HWAVEIN hwi;


int init_audio(void)
{
	MMRESULT  res;
        WAVEINCAPS wi;
        WAVEFORMATEX wfx;
		int i;

        wfx.wFormatTag=WAVE_FORMAT_PCM; 
    wfx.nChannels=2; 
//    wfx.nSamplesPerSec=11025; 
//    wfx.nAvgBytesPerSec=22050; 
    wfx.nSamplesPerSec=22050; 
    wfx.nAvgBytesPerSec=44100; 
    wfx.nBlockAlign=2;
    wfx.wBitsPerSample=8;
    wfx.cbSize=0; 

        //WAVE_FORMAT_2S08;  WAVERR_BADFORMAT

        for (i=0; i<10; i++)
        {
                waveInGetDevCaps(i,&wi,sizeof(WAVEINCAPS));
                TRACE("%d %s\n",i,wi.szPname);
        }
        res=waveInOpen(&hwi,0,&wfx,0, 0,0); 
        //waveInGetDevCaps(hwi,&wi,sizeof(WAVEINCAPS));

        TRACE("WaveInOpen returned %d\n",res);
        switch (res)
        {
        case MMSYSERR_NOERROR:
                break;
        case MMSYSERR_ALLOCATED:
                break;
        case MMSYSERR_BADDEVICEID:
                break;
        case MMSYSERR_NODRIVER:
                break;
        case MMSYSERR_NOMEM:
                break;
        case WAVERR_BADFORMAT:
                break;
        default:
                break;
        }

        return res;
}

WAVEHDR wh1,wh2;
#define MAX_SOUND 22050
char space1[MAX_SOUND*2+100];
//char space2[44200];

int audio_firsttime(void)
{
	MMRESULT res;
        init_audio();

        {
    wh1.lpData=space1;                   // address of the waveform buffer 
    wh1.dwBufferLength=MAX_SOUND*2;           // length, in bytes, of the buffer 
    wh1.dwBytesRecorded=0;          // see below 
    wh1.dwUser=0;                   // 32 bits of user data 
    wh1.dwFlags=0;                  // see below 
    wh1.dwLoops=0;                  // see below 
    wh1.lpNext=0; // reserved; must be zero 
    wh1.reserved=0;                 // reserved; must be zero 

        res=waveInPrepareHeader(hwi,&wh1,sizeof(WAVEHDR));
        TRACE("InPrepare returned %d\n",res);
        res=waveInAddBuffer(hwi,&wh1,sizeof(WAVEHDR));
        TRACE("InAdd returned %d\n",res);


        res=waveInStart(hwi);
        TRACE("InStart returned %d\n",res);

        }
        return 0;
}

int close_audio(void)
{
        MMRESULT res=waveInUnprepareHeader(hwi,&wh1,sizeof(WAVEHDR));
        waveInClose(hwi);
        return 1;

}

int audio_everytime(int wait)
{
        static int lasttime=0;
        MMTIME mm;
        MMRESULT res;//=waveInStop(hwi);
		int timeout;
        waveInGetPosition(hwi,&mm,sizeof(MMTIME));
        timeout=40000;

        lasttime=0;

        while (((mm.u.ms-lasttime) < BUFF_WIDTH*2) && timeout)
        {
                waveInGetPosition(hwi,&mm,sizeof(MMTIME));
                timeout--;
        }

        if (timeout<=0)
                TRACE("Timed Out\n");


#if 0
        if (mm.u.ms >= BUFF_WIDTH*2)
                memcpy(stereo,&(space1[mm.u.ms-BUFF_WIDTH*2]),BUFF_WIDTH*2);
        else
                memcpy(stereo,&(space1[0]),mm.u.ms);
#endif
        lasttime=mm.u.ms;  // (where were we up to again ???)

        if (lasttime+BUFF_WIDTH > MAX_SOUND)
        {
        lasttime=0;
        res=waveInReset(hwi);
        res=waveInAddBuffer(hwi,&wh1,sizeof(WAVEHDR));
//      TRACE("InAdd returned %d\n",res);
        res=waveInStart(hwi);
        }
        //TRACE(": %d\n",res);
        return 1;
}



#else
void init_sound() {sound_present=0;}
int get_sound() {return 0;}
void exit_sound() {}
#endif
#endif

#if ogl_bomb

#include <dmedia/audio.h>

#define max_buf (1<<16)

ALconfig al_config;
ALport al_port;

void init_sound() {
  al_config = ALnewconfig();
  ALsetchannels(al_config, 1);
  ALsetwidth(al_config, AL_SAMPLE_8);
  ALsetqueuesize(al_config, max_buf);
  al_port = ALopenport("bomb_in", "r", al_config);
}

void exit_sound() {
  ALcloseport(al_port);
}

static signed char buf[max_buf];

int get_sound() {
  int sum, i, nsamps = ALgetfilled(al_port);
  static double hold = 0.0;
  if (nsamps > max_buf) nsamps = max_buf;
  ALreadsamps(al_port, buf, nsamps);
  sum = 0;
  for (i = 0; i < nsamps; i++) {
    signed char s = buf[i];
    sum += s * s;
  }
  hold = (hold + sqrt((double)sum / nsamps))/2.0;
  return hold;
}

#endif



#if mac_bomb

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <Sound.h>
#include <SoundInput.h>
#include <FixMath.h>
#include "sound_in.h"

#define max(x,y) (((x)>(y))?(x):(y))


/*
 * This file is used for the "normal" cthugha-L and for the cthugha-server.
 * cthugha-server displays no graphic and accepts only a subset of the
 * options cthugha-L supports.
 *
 * If CTH_SERVER is defined, then the cthugha-server version should be 
 * generated.
 */

pascal void sound_done(SPBPtr inParamPtr);

//  public Variables 

void (* display_wave)(void);			//  Pointer to active wave 
char sound_data[MAX_BUFF_WIDTH][2];		//  Sound-Data in stereo 
int sound_stereo = 1;				//  stereo/mono 
int sound_sample_rate = 44100;			//  sampel rate 
int sound_source = SNDSRC_DEFAULT;		//  default, Line-In, Mic, CD 
//int sound_source = SNDSRC_DEBUG;
int sound_volume_line = -1;		
int sound_volume_mic = -1;		
int sound_volume_cd = -1;		
int sound_volume_rec = -1;
int sound_minnoise = 30;			//  quiet is below this 
int sound_wait_quiet = 255;			//  max. quiet interval  
int sound_quiet_change = 20;			//  change after quiete-pause 
int sound_peaklevel = 230;			//  above that is loud 
int sound_wait = 200;				//  time till change 
int sound_wait_beat = 4;			//  peaks till change 
int sound_wait_random = 750;			//  extra random wait-time 
int sound_wait_min = 200;			//  min time between change 
int sound_FFT = 0;				//  use FFT or don't 
int sound_use_fft = 0;				//  allow fft-usage 
char wave_first[256] = "";			//  Start with this wave 
int sound_massage_style = 1;			//  massage style 
double sam_usage;

//  private Variables 

int sound_blk_size = 0;				//  Size of DMA-Buffer 
int sound_bits = 8;				//  bits per sample 
int sound_div = 1;				//  reduction of DMA-Buffer 
int sound_bsize = 0;				//  size of sound-buffer # of samples  (external)
int sam_bsize = 0;					//  size of sample-buffer # of bytes 2x for stereo (internal)

static long myInRefNum = 0;			// device reference number
static long myBuffSize; 			// size of buffer to record into
static short myHeadrLen;			// length of sound header
static short myNumChans;			// number of channels
static short mySampSize;			// size of a sample
static Fixed mySampRate; 		// sample rate
static OSType myCompType;		// compression type
static SPBPtr mySPB;	// a sound input parameter block
static SICompletionUPP SICompUPP = NULL;


char sound_buffer1[65536],sound_buffer2[65536];		//  temporary buffers
char * recBuf;


int sound_quiet = 0;				//  quiete since 
int sound_beats = 0;				//  loudness-count 
int sound_count = 0;				//  counter for change 


#ifndef CTH_SERVER

//  for FFT 
int sine[320];
#define N 64
#define SCALE 7
static int s[N+N], c[N+N];
#define CTEPI (3.14159265358979323846/(float)N) 
#define TRIG  16 /* trig table lookup precision compatible with S? way above */ 
#endif

int init_sound_mixer();				//  initialize mixer 
int init_sound_dsp();				//  initialize dsp 
int init_sound_FFT();				//  initialize FFT-tables 

/* 
 *  Initialize the Sound-Interface
 */
void init_sound() {
	OSErr myErr;
	Fixed samRate;
	float myMess;
	short chans,chav,ssiz;
	OSType qual;
	long chanmask;

	mySPB = (SPBPtr)NewPtr(sizeof(SPB));
	
	if (!mySPB) {
		sound_present = 0;
		printf("Unable to allocate sound parameter block");
		return;

	}
	sound_present = 1;
	
	mySPB->count = 0;
	mySPB->userLong = 1;

	SICompUPP = NewSICompletionProc(sound_done);

	myErr = SPBOpenDevice("\p", siWritePermission, &myInRefNum);

	if (myErr) {
		printf("Error initializing sound device");
		sound_present = 0;
		myInRefNum = 0;
		return;
	}

	samRate = 0xAC440000;
//	samRate = 0x56EE8BA3;
//	myErr = SPBSetDeviceInfo(myInRefNum,siSampleRate,&samRate);

	if (myErr) {
		printf("Error %d setting sample rate",myErr);
	}

	if (sound_stereo == 2)
		chans = 2;
	else
		chans = 1;

	ssiz = 8;
	myErr = SPBSetDeviceInfo(myInRefNum,siSampleSize,&ssiz);

	if (myErr) {
		printf("Error %d setting sample size",myErr);
	}

	myErr = SPBSetDeviceInfo(myInRefNum,siNumberChannels,&chans);

	if (myErr) {
		printf("Error %d setting # of channels",myErr);
	}

	myErr = SPBGetDeviceInfo(myInRefNum,siSampleRate,&samRate);
	myErr = SPBGetDeviceInfo(myInRefNum,siNumberChannels,&chans);
	myErr = SPBGetDeviceInfo(myInRefNum,siSampleSize,&ssiz);
//	myErr = SPBGetDeviceInfo(myInRefNum,'chac',&chanmask);
//	myErr = SPBGetDeviceInfo(myInRefNum,'chav',&chav);

//	alert_error("Sample size %d",ssiz);

	myMess = Fix2X(samRate);

	if (myMess < 0)
		myMess += 65536;

//	alert_error("Recording at %1.2f on %d channel(s)\n",myMess,chans);		

    init_sound_FFT();
	
    if ( sound_wait_random <= 0)
		sound_wait_random = 1;

    sam_bsize = sound_bsize = max(BUFF_WIDTH, BUFF_HEIGHT);
    
    if (sound_stereo == 2)
    	sam_bsize *= 2;
    
    recBuf = sound_buffer1;

}


/*
 * Initalization for FFT 
 * the sine-table is also used by some other functions
 */
int init_sound_FFT() {
    int i; 
    float xx,ss,cc; 
    int k; 
 
    for (i=0; i<320; i++) {
		sine[i]=(int)((float)128*sin((float)i*0.03927)); 
 	}
 
    for (k=0 ; k<(N+N) ; k++) { 
		xx=CTEPI*k; 
		ss=TRIG*sin(xx); 
		cc=TRIG*cos(xx); 
		if (ss>0.0) ss+=0.5; else ss-=0.5; 
		if (cc>0.0) cc+=0.5; else cc-=0.5; 
		s[k]=(int)ss; /* truncate */ 
		c[k]=(int)cc; /* truncate */ 
    } 
    return 0;
}

/*
 * Clean up the sound-interface
 */
void exit_sound() {

	if (sound_source != SNDSRC_DEBUG && myInRefNum) {
		SPBStopRecording(myInRefNum);
		SPBCloseDevice(myInRefNum);
	}

}


pascal void sound_done(SPBPtr inParamPtr)
{
	inParamPtr->userLong = 1;
}


/*
 *  Get sound from sound-input device
 */
void sound_read() {
    register int nr_read, i;
    char * sbuff;
    OSErr myErr;

	
	//if (!mySPB)
	//	return;

	SPBStopRecording(myInRefNum);

	// wait for the sample to finish
//	while (!mySPB->userLong)
//		{}

	nr_read = mySPB->count;			// get how much was sampled
	
	if (nr_read > sam_bsize) {
		sbuff = recBuf + nr_read - sam_bsize;
//		sam_usage = sam_bsize*100.0/nr_read; 
	} else {
		sbuff = recBuf;
//		sam_usage = 100;
	}

	// swap asynch record buffer with working buffer
	if (recBuf == sound_buffer1) 
		recBuf = sound_buffer2;
	else
		recBuf = sound_buffer1;

	// configure the sound record
	mySPB->inRefNum = myInRefNum;	// input device reference number
	mySPB->count = 65536; 	// number of bytes to record
	mySPB->milliseconds = 0;	// no milliseconds
	mySPB->bufferLength = 65536;	// length of buffer
	mySPB->bufferPtr = recBuf;
		
	mySPB->completionRoutine = SICompUPP;	// no completion routine
	mySPB->interruptRoutine = nil;	// no interrupt routine
	mySPB->userLong = 0; 		// no user data
	mySPB->error = noErr;	// clear error field
	mySPB->unused1 = 0; 		// clear reserved field
	
	// record asynchronously through the open sound input device
	myErr = SPBRecord(mySPB, 1);

    // Bring the read data to the right place
    if ( sound_stereo == 2) {
		for(i=0; i < sound_bsize; i++) {
		    sound_data[i][1] = (*sbuff ++) - 128;
		    sound_data[i][0] = (*sbuff ++) - 128;
		}
    } else {
		for(i=0; i < sound_bsize; i++) {
		    sound_data[i][0] = (*sbuff) - 128;
		    sound_data[i][1] = (*sbuff ++) - 128;
		}	
    }

}



int get_sound() {
    int x;
    char * sdata;
    int v;
    
    double power;

    sound_read();
	
    sdata = (char *)sound_data;

    power = 0;
    for( x = sound_bsize; x != 0; x--) {
        double s = (double)*sdata;
	power += s*s;
	sdata ++;
    }
    v = sqrt( power / sound_bsize);
    return (int) v;
	
}

#endif

/* ugh */
#if vga_bomb | xws_bomb


#include <sys/ioctl.h>

#if use_esd

#include <esd.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#define ESD_SOUND_READSIZE 0x400 <<2 //Must be a mutiple of 4!
#define BOMB_BUF_SIZE 0x10000
#define min(x,y) ((x<y) ? x : y)

const char *esd_hostname = NULL;
const char *esd_streamname = "Temp";
const char *esd_server = NULL;
unsigned int esd_port = ESD_DEFAULT_PORT;
esd_format_t esd_format = ESD_BITS8 | ESD_MONO | ESD_STREAM | ESD_PLAY;
int esd_rate = 44100;

int esd_fd = -1;
size_t esd_buf_sz = BOMB_BUF_SIZE;
unsigned char  esd_sound_buf[BOMB_BUF_SIZE];
unsigned char *esd_buf_last = esd_sound_buf;
unsigned char *esd_buf_end;
unsigned char *esd_buf_curptr;
pthread_mutex_t esd_buf_lock;
pthread_t esd_update_thread;
char esd_shutdown = 0;

void * esd_update(void *arg) {
    int length;
    while(1){
	pthread_mutex_lock(&esd_buf_lock);
	if (esd_shutdown == 1) {
	    pthread_mutex_unlock(&esd_buf_lock);
	    pthread_exit(0);
	}
	if (esd_buf_curptr == esd_sound_buf + esd_buf_sz) {
	    length = read(esd_fd,esd_sound_buf,ESD_SOUND_READSIZE);
	    if (length > -1 && length%4 == 0) {
		esd_buf_curptr = esd_sound_buf + length;
	    }
	    else
		printf("read error on esd_mon stream\n");
	} else {
	    length = read(esd_fd,
			esd_buf_curptr,
			min(ESD_SOUND_READSIZE,
			    (esd_sound_buf+esd_buf_sz-esd_buf_curptr)));
	    if (length > -1 && length%4 == 0) {
		esd_buf_curptr = esd_buf_curptr + length;
	    }
	    else
		printf("read error on esd_mon stream\n");
	}
	pthread_mutex_unlock(&esd_buf_lock);
    }
}


void init_sound() {

  if (nosound) {
    esd_fd = -1;
    sound_present = 0;
    return;
  }


    esd_fd = esd_monitor_stream(esd_format,
				esd_rate,
				esd_server,
				esd_streamname);

    if (esd_fd < 0) {
	fprintf(stderr,"Can't establish an ESD connection!\n");
	esd_fd = -1;
	sound_present = 0;
	return;
    }

    esd_buf_curptr = esd_sound_buf;
    pthread_mutex_init ( &esd_buf_lock, NULL);
    pthread_create(&esd_update_thread,
		   NULL,
		   esd_update,
		   NULL);
}


int get_sound() {
   int i, n;
   long s2 = 0;
   int result;
/*   signed short *data;*/

   if (esd_fd == -1) return 0;

   pthread_mutex_lock(&esd_buf_lock);
   n = (unsigned long)esd_buf_curptr - (unsigned long)esd_buf_last;
   if (0==n) {
       printf("non\n");
       pthread_mutex_unlock(&esd_buf_lock);
       return 0;
   }
   if (n < 0) {
/*       data = (signed short *)esd_buf_last;
 *       while ( (octet *) data < (octet *)((unsigned long)esd_sound_buf + esd_buf_sz) ) {
 *	   int s = (*(data) + *(data +1)) /2;
 *	   data += 2;
 *	   s2 += s*s;
 *      }*/
       for (i = (unsigned long)esd_buf_last - (unsigned long)esd_sound_buf; i < esd_buf_sz; i++) {
	   int s = ((int)esd_sound_buf[i])-128;
	   s2 += s*s;
       }
/*       data = (signed short *)esd_sound_buf;
 *      while ( (octet *) data < esd_buf_curptr ) {
 *	   int s = (*(data) + *(data +1)) /2;
 *	   data += 2;
 *	   s2 += s*s;
 *      }*/
       for (i = 0; i < esd_buf_curptr - esd_sound_buf; i++) {
	   int s = ((int)esd_sound_buf[i])-128;
	   s2 += s*s;
       }
       n += esd_buf_sz;
   } else /*{
 *       data = (signed short *)esd_buf_last;
 *      while ( (octet *) data < esd_buf_curptr ) {
 *	   int s = (*(data) + *(data +1)) /2;
 *	   data += 2;
 *	   s2 += s*s;
 *      }
 *  }*/
       for (i = (unsigned long)esd_buf_last - (unsigned long)esd_sound_buf;
	    i < (unsigned long)esd_buf_curptr - (unsigned long)esd_sound_buf;
	    i++) {
	   int s = ((int)esd_sound_buf[i])-128;
	   s2 += s*s;
       }
   esd_buf_last = esd_buf_curptr;
   pthread_mutex_unlock(&esd_buf_lock);
   result = (int) sqrt(s2/n);
   return result;
}

void exit_sound() {
    pthread_mutex_lock(&esd_buf_lock);
    esd_shutdown = 1;
    pthread_mutex_unlock(&esd_buf_lock);
    pthread_join(esd_update_thread,NULL);
    if (esd_fd != -1) {
	close(esd_fd);
    }
    esd_fd = -1;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/soundcard.h>



int snd_fd;
unsigned char *bomb_sound_buf;
int sound_buf_last;
int sound_buf_end;
int buf_sz;

void init_sound() {
  int stereo = 0;
  int format = AFMT_U8;
  int speed = 44000;
  int tmp;

  struct audio_buf_info info;

  if (nosound) {
    sound_present = 0;
    return;
  }

  snd_fd = open("/dev/dsp", O_RDONLY);
  if (-1 == snd_fd) {
    perror("open(/dev/dsp)"); 
    goto bail;
  }
  if (-1 == ioctl(snd_fd, SNDCTL_DSP_SETFMT, &format)) {
    perror("sound ioctl format");
    goto bail;
  }
  if (-1 == ioctl(snd_fd, SNDCTL_DSP_STEREO, &stereo)) {
    perror("sound ioctl stereo");
    goto bail;
  }
  if (-1 == ioctl(snd_fd, SNDCTL_DSP_SPEED, &speed)) {
    perror("sound ioctl speed");
    goto bail;
  }

  /* printf("format=%x stereo=%x speed=%d\n", format, stereo, speed); */

  if (ioctl(snd_fd, SNDCTL_DSP_GETISPACE, &info)==-1) {
    perror("GETISPACE");
    goto bail;
  }
  buf_sz = info.fragstotal * info.fragsize;
  sound_buf_end = buf_sz;
  bomb_sound_buf=mmap(NULL, buf_sz, PROT_READ, MAP_FILE|MAP_SHARED, snd_fd, 0);
  if (bomb_sound_buf ==(unsigned char *)-1) {
    perror("mmap (read)");
    goto bail;
  }
  tmp = 0;
  ioctl(snd_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
  tmp = PCM_ENABLE_INPUT;
  ioctl(snd_fd, SNDCTL_DSP_SETTRIGGER, &tmp);

  sound_present = 1;

  return;

  bail:
  sound_present = 0;
  fprintf(stderr, "disabling sound input.\n");
}

int get_sound() {
   int i, n;
   int s2 = 0;
   static int last = 0;
   int result;
   count_info info;
   if (!sound_present) return 0;
   if (ioctl(snd_fd, SNDCTL_DSP_GETIPTR, &info)==-1)
      {perror("ioctl SNDCTL_DSP_GETIPTR"); exit(-1);}

   n = info.ptr - last;
   sound_buf_last = info.ptr;
   if (0==n) {printf("non\n");return 0;}
   if (n < 0) {
      for (i = last; i < buf_sz; i++) {
        int s = ((int)bomb_sound_buf[i])-128;
        s2 += s*s;
      }
      for (i = 0; i < info.ptr; i++) {
        int s = ((int)bomb_sound_buf[i])-128;
        s2 += s*s;
      }
      n += buf_sz;
   } else
     for (i = last; i < info.ptr; i++) {
       int s = ((int)bomb_sound_buf[i])-128;
       s2 += s*s;
       // printf("%d ", s);
     }
   last = info.ptr;
   result = (int) sqrt(s2/n);
#if 0
   printf("%5d %8d %4d ", n, info.ptr, result);
   for (i = 0; i < result/5; i++)
     printf("O");
   printf("\n");
#endif
   return result;
}
void exit_sound() {
   if (sound_present)
       close(snd_fd);
}

#endif  /* use_esd */
#endif  /* vga_bomb | xws_bomb */
