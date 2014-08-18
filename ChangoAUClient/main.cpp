//
//  main.cpp
//  ChangoAUClient
//
//  Created by Brandon Lucia on 8/14/14.
//  Copyright (c) 2014 Brandon Lucia. All rights reserved.
//

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "pixelator.h"

#define WINDOW_NAME_STR "Chango Audio Unit Capture Window - Brandon Lucia 2014"

using namespace cv;

std::string messageBuffer; //Message buffer -- shared from video thread to transmission thread
pthread_mutex_t schlock; //Lock protecting the shared string message buffer
int datafd; //File Descriptor to ChangoAU
float shamps[25];

void *writerThread(void*data){
    
    float myamps[25];
    while(true){
        
        pthread_mutex_lock(&schlock);
        for(int i = 0; i < 25; i++){ myamps[i] = shamps[i]; }
        pthread_mutex_unlock(&schlock);
        write(datafd, myamps, 25 * sizeof(float));
        std::cerr << "Writer got: ";
        for(int i = 0; i < 25; i++){ std::cerr << myamps[i] << ":"; }
        std::cerr << "\n";
        
        
    }
    return NULL;
}

int main(int argc, const char * argv[])
{
    
    pthread_mutex_init(&schlock,NULL);
    messageBuffer = std::string("1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:1.0000:");
    
    VideoCapture camera(0);
    namedWindow(WINDOW_NAME_STR,WINDOW_NORMAL);
    
    float amps[25];
    
    /*The AU Plugin needs to know about these three strings.  
      Could be through the environment, too...*/
    const char *dataFIFO = "/Users/blucia/CHANGO_DATA_FIFO";
    mkfifo(dataFIFO, 0644);
    
    fprintf(stderr,"Opening data\n");
    datafd = open(dataFIFO,O_WRONLY);

    fprintf(stderr,"All Open!\n");
    
    
    std::ostringstream databuf;
    databuf << std::setiosflags(std::ios::fixed) << std::setprecision(4);
    
    pthread_t thd;
    pthread_create(&thd,NULL,writerThread,NULL);
    while(true){

        Mat frame;
        camera.read(frame);
        IplImage f2 = frame;
        pixelate(&f2,amps);
       
        std::cerr << "Camera got: ";
        for(int i = 0; i < 25; i++){ std::cerr << amps[i] << ":"; }
        std::cerr << "\n";
        
        pthread_mutex_lock(&schlock);
        for(int i = 0; i < 25; i++){ shamps[i] = amps[i]; };
        pthread_mutex_unlock(&schlock);
        
        imshow(WINDOW_NAME_STR,frame);
        usleep(2000);//50 frames / s if frames are free (they're not, so ~30 FPS)
        
    }
    
    return 0;
}

