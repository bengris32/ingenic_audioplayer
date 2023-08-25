#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <imp/imp_audio.h>

#include <lame/lame.h>

#define AUDIO_PTNUMPERFRM 441
#define MP3_SAMPLE (AUDIO_PTNUMPERFRM * sizeof(short))

static int chnVol = 50;
static int chnGain = 28;
static int chnID = 0;
static int devID = 0;

int init_audio_hw(void)
{
    int ret;
    IMPAudioIOAttr attr;

	attr.samplerate = AUDIO_SAMPLE_RATE_44100;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 30;
    // Must be a multiple of sample rate * 2 / 100.
	attr.numPerFrm = AUDIO_PTNUMPERFRM;
	attr.chnCnt = 1;

    ret = IMP_AO_SetPubAttr(devID, &attr);
    if (ret < 0) {
        printf("IMP_AO_SetPubAttr failed, devID=%d, ret=%d\n", devID, ret);
        return ret;
    }

    ret = IMP_AO_Enable(devID);
    if (ret < 0) {
        printf("IMP_AO_Enable failed, devID=%d, ret=%d\n", devID, ret);
        return ret;
    }

    ret = IMP_AO_EnableChn(devID, chnID);
    if (ret < 0) {
        printf("IMP_AO_EnableChn failed, chnID=%d, ret=%d\n", chnID, ret);
        return ret;
    }

    ret = IMP_AO_SetVol(devID, chnID, chnVol);
    if (ret < 0) {
        printf("IMP_AO_SetVol failed, chnID=%d, ret=%d\n", chnID, ret);
        return ret;
    }

    ret = IMP_AO_SetGain(devID, chnID, chnGain);
    if (ret < 0) {
        printf("IMP_AO_SetGain failed, chnID=%d, ret=%d\n", chnID, ret);
        return ret;
    }

    return 0;
}

int deinit_audio_hw(void)
{
    int ret;

    ret = IMP_AO_DisableChn(devID, chnID);
    if (ret < 0) {
        printf("IMP_AO_DisableChn failed, chnID=%d, ret=%d\n", chnID, ret);
        return ret;
    }

    ret = IMP_AO_Disable(devID);
    if (ret < 0) {
        printf("IMP_AO_Disable failed, devID=%d, ret=%d\n", devID, ret);
        return ret;
    }

    return 0;
}

int send_frame(unsigned char *data, int size)
{
    int ret = 0;
    IMPAudioFrame frame;

    frame.virAddr = (uint32_t *)data;
    frame.len = size;

    ret = IMP_AO_SendFrame(devID, chnID, &frame, BLOCK);
    if (ret < 0)
        printf("IMP_AO_SendFrame failed, chnID=%d, ret=%d\n", chnID, ret);
    
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    int nread;
    hip_t dec;
    FILE *f = stdin;
    unsigned char mp3buf[4096];
    short pcm_l[0x10000], pcm_r[0x10000];

    ret = init_audio_hw();
    if (ret != 0) {
        printf("init_audio_hw failed, ret=%d\n", ret);
        return -1;
    }

    dec = hip_decode_init();
    if (dec == NULL) {
        printf("hip_decode_init failed\n");
        return -1;
    }

    while ((nread = fread(mp3buf, 1, sizeof(mp3buf), f)) > 0) {
        int samples = hip_decode(dec, mp3buf, nread, pcm_l, pcm_r);
        unsigned char *sptr = (unsigned char *)pcm_l;
        unsigned char *end = sptr + samples * sizeof(short);
        while (sptr < end) {
            int n = end - sptr;
            if (n > MP3_SAMPLE)
                n = MP3_SAMPLE;
            ret = send_frame(sptr, n);
            if (ret != 0)
                printf("send_frame failed, ret=%d\n", ret);

            sptr += n;
        }
    }

    ret = hip_decode_exit(dec);
    if (ret != 0) {
        printf("hip_decode_exit failed, ret=%d\n", ret);
        ret = -1;
    }

    ret = deinit_audio_hw();
    if (ret != 0) {
        printf("deinit_audio_hw failed, ret=%d\n", ret);
        ret = -1;
    }

    return ret;
}
