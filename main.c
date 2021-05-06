//
//  main.c
//  webRTC_vad
//
//  Created by 陈嘉捷 on 2019/11/19.
//  Copyright © 2019 陈嘉捷. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <windows.h>
#include "vad_core.h"
#include "include/webrtc_vad.h"
#include <sys/stat.h> // statbuf using
#include <time.h>

long long file_size(char* filename)
{
    struct stat statbuf;
    stat(filename,&statbuf);
    long long size=statbuf.st_size;
    
    return size;
}

const char *newNameIdent(int vad_mode, int frame_length)
{
    char *result = NULL;
    switch (vad_mode)
    {
        case 0:
            switch (frame_length)
            {
                case 160:
                    result = "_webRTC_VAD_0_160";
                    break;
                case 320:
                    result = "_webRTC_VAD_0_320";
                    break;
                case 480:
                    result = "_webRTC_VAD_0_480";
                    break;
            }
            break;
        case 1:
            switch (frame_length)
            {
                case 160:
                    result = "_webRTC_VAD_1_160";
                    break;
                case 320:
                    result = "_webRTC_VAD_1_320";
                    break;
            case 480:
                    result = "_webRTC_VAD_1_480";
                    break;
            }
            break;
        case 2:
            switch (frame_length)
            {
                case 160:
                    result = "_webRTC_VAD_2_160";
                    break;
            case 320:
                    result = "_webRTC_VAD_2_320";
                    break;
            case 480:
                    result = "_webRTC_VAD_2_480";
                    break;
            }
            break;
        case 3:
            switch (frame_length)
            {
                case 160:
                    result = "_webRTC_VAD_3_160";
                    break;
                case 320:
                    result = "_webRTC_VAD_3_320";
                    break;
                case 480:
                    result = "_webRTC_VAD_3_480";
                    break;
            }
            break;
    }
    return result;
}

int util_webrtcVAD_inOneFunc(const char *filename, int vad_mode, int fs, int frame_length)
{
    VadInst *NS_inst;
    int suc = WebRtcVad_Create (&NS_inst);
    printf("suc=%d.\n",suc);
    suc=WebRtcVad_Init(NS_inst);
    printf("suc=%d.\n",suc);
    suc=WebRtcVad_set_mode(NS_inst,vad_mode);
    printf("suc=%d.\n",suc);
    short spframe[frame_length];
    /*
     short spframe[var_len]={0}; 报错：
     因为{0}是编译时决定的，而var_len是运行时决定的。
     */

    FILE *fRead=NULL;
    fRead=fopen(filename, "r" );
    if(!fRead)
        printf("cant open fRead\n");
    
    // TIMIT原生SPHERE文件，offset为1024。.wav文件文件头为44Byte
    int audio_offset = 44;
    
    fseek(fRead, audio_offset, SEEK_SET);

    char filename_no_wav[1024] = {0};// 去除wav后缀的文件路径名
    memcpy(filename_no_wav,filename,(strlen(filename)-4));
    char new_name[1024] = {0};// vad flag 输出的文件名
    strcpy(new_name, filename_no_wav);
    strcat(new_name, newNameIdent(vad_mode, frame_length));
    strcat(new_name, "_flag.webrtc");

    char new_name2[1024] = {0};// vad running time 输出的文件名
    strcpy(new_name2, filename_no_wav);
    strcat(new_name2, newNameIdent(vad_mode, frame_length));
    strcat(new_name2, "_runtime.webrtc");
    
    FILE *fWrite;
    fWrite = fopen(new_name,"w");
    if(!fWrite)
        printf("cant open fWrite\n");
    
    FILE *fWrite2;
    fWrite2 = fopen(new_name2, "w");
    if (!fWrite2)
        printf("cant open fWrite\n");
    
    clock_t start, finish; // 用于计算WebRTC VAD运算时间
    int flag_cnt = 0;
    while(!feof(fRead))
    {
        double duration = 0;
        memset(spframe,0,frame_length);
        fread(spframe,2,frame_length,fRead); // 2bytes and frame_length个 sound points
        start = clock();
        int flag = WebRtcVad_Process(NS_inst,fs,spframe,frame_length);
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC; // 秒为单位
        
        fprintf(fWrite2, "%f\n", duration);
        for (int repeat = 0; repeat < frame_length; repeat++) // frame group
        {
            fprintf(fWrite,"%d ",flag);
            flag_cnt += 1;
        }
    }

    printf("flag count : %d\n", flag_cnt);
    printf("ok!\n\n");
    fclose(fRead);
    fclose(fWrite);
    fclose(fWrite2);
    fRead = NULL;
    fWrite = NULL;
    fWrite2 = NULL;
    
    WebRtcVad_Free(NS_inst);

    return 0;

}


//int main()
//{
//    char file_name[1024] = {0};
//    FILE *f_all_wav_name;
//    f_all_wav_name = fopen("/Users/chenjiajie/Desktop/本科毕业设计/数据/WebRTC_test_nonspeech/TIMIT_10_dB_file_index.txt", "r");
//    // 改SNR的地方,运行完后，要手工将.webetc文件 放到TESTING_OUTPUT里去
//    if(!f_all_wav_name)
//        printf("cant open f_all_wav_name\n");
//
//    int file_cnt = 1;
//    while(!feof(f_all_wav_name)) // 对文件遍历
//    {
//        memset(file_name, 0, sizeof(file_name));
//        fgets(file_name, sizeof(file_name) - 1, f_all_wav_name);
//        if (strlen(file_name) == 0) break; // 所有文件读取完毕
//        strtok(file_name, "\n");
//        printf("------------------------------------\n");
//        printf("当前文件：%s\n", file_name);
//
//        for(int cur_mode = 0; cur_mode <= 3; cur_mode++) // 对WebRTC VAD模式遍历
//        {
////            for (int cur_winlen = 160; cur_winlen == 320; cur_winlen += 160) // 对三种窗长遍历,esp.对20ms遍历
////            {
//            int cur_winlen = 320;
//                printf("当前模式：%d\n", cur_mode);
//                printf("当前窗长：%d\n", cur_winlen);
//                util_webrtcVAD_inOneFunc(file_name, cur_mode, 16000, cur_winlen);
//            printf("当前进度：%d/5376\n", file_cnt);
//            file_cnt += 1;
////            }
//        }
//    }
//    printf("All Finished!\n");
//    fclose(f_all_wav_name);
//}

//TEST main()
int main()
{
    util_webrtcVAD_inOneFunc("/Users/chenjiajie/Desktop/本科毕业设计/数据/TIMIT_with_Nonspeech/TEST_SILENCE_-10dB_WAV/silence_DR1_FAKS0_SI2203.WAV", 3, 16000, 320);
}
