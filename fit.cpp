/****************************************************************************
*
* Copyright (c) 2008 by Yao Wei, all rights reserved.
*
* Author:      	Yao Wei
* Contact:     	njustyw@gmail.com
*
* This software is partly based on the following open source:
*
*		- OpenCV
*
****************************************************************************/

#include <cstdio>
#include <string>
#include <opencv/highgui.h>
#include <string.h>
#include <stdlib.h>

#include "AAM_IC.h"
#include "AAM_Basic.h"
#include "AAM_VJFaceDetect.h"
#include "FacePredict.h"
#include "AAM_Util.h"

using namespace std;

std::string resultDir = "./trainingSets/";
std::string trainDir = "./trainingSets/";
//std::string resultDir = "../test2/";


AAM_Shape makeShape(double points[], int sizes) {
    AAM_Shape Shape(points, sizes);
    return Shape;
}

extern "C"
{
    int fit(char* originalImageFileName, char* curAge, char* predictAge, char* pointsStr, int sizes) {
        int processState = 0;
        char* pch;
        double* points = new double[sizes];

        int index = 0;
        pch = strtok(pointsStr, " ");
        while (pch != NULL) {
            points[index++] = atof(pch);
            pch = strtok(NULL, " ");
        }

        try {
            IplImage* originalImage = cvLoadImage(originalImageFileName, 1);
            IplImage *image = cvCreateImage(cvGetSize(originalImage), originalImage->depth, originalImage->nChannels);

            AAM_Shape Shape = makeShape(points, sizes);

            //Facial Prediction
            FacePredict face_predict;
            std::string mfile = resultDir + "facial.predict_model";
            std::ifstream model(mfile.c_str());
            face_predict.Read(model);
            model.close();

            IplImage* newImage = face_predict.predict(Shape, *originalImage, atoi(curAge), atoi(predictAge), false);
            std::string newfile = std::string(originalImageFileName);
            newfile = newfile.insert(newfile.find_last_of('/') + 1, "result_");
            //newfile = newfile.insert(newfile.find_last_of('.'), std::string("_G" + string(predictAge)));
            cvSaveImage(newfile.c_str(), newImage);

            /*
            cvNamedWindow("PredictedFacialImage");
            cvShowImage("PredictedFacialImage", newImage);
            cvWaitKey(0);*/

            cvReleaseImage(&image);
        }
        catch (AgingException ex) {
            processState = ex.getStateCode();
        }

        return processState;
    }

    double* getShape(char* originalImageFileName, char* curAge) {
        AAM_Shape Shape;
        double* points;
        int processState = 0;

        try {
            //load image
            IplImage* originalImage = cvLoadImage(originalImageFileName, 1);
            if (originalImage == 0) {
                AgingException agingException(1);
                throw agingException;
            }

            IplImage *image = cvCreateImage(cvGetSize(originalImage), originalImage->depth, originalImage->nChannels);
            cvCopy(originalImage, image);
            AAM_Shape Shape;

            AAM *aam = NULL;
            int type;
            std::string aamFileName = resultDir + "Group" + /*std::string(argv[3])*/curAge + ".aam_ic";
            std::ifstream fs(aamFileName.c_str());
            if(fs == 0) {
                AgingException agingException(3);
                throw agingException;
            }
            fs >> type;

            //aam-basic
            if(type == 0)		aam = new AAM_Basic;
            else if(type == 1)  aam = new AAM_IC;

            //read model from file
            aam->Read(fs);
            fs.close();

            //intial face detector
            AAM_VJFaceDetect fjdetect;
            fjdetect.LoadCascade("haarcascade_frontalface_alt2.xml");

            //detect face for intialization
            Shape = fjdetect.Detect(image, aam->GetMeanShape());

            //do image alignment
            aam->Fit(image, Shape, 30, false);  //if true, show process

            ofstream outfile;
            string wuxuefTmpResultDir = resultDir + "aam_result.txt";
            outfile.open(wuxuefTmpResultDir.c_str());
            Shape.Write(outfile);
            outfile.close();


            // show face detect result
            //cvNamedWindow("AAMFitting", CV_WINDOW_AUTOSIZE);
            //aam->Draw(image, 0);
            //cvShowImage("AAMFitting", image);
            //cvWaitKey(0);
            //}



            //resize the current image
            cvSetImageROI(originalImage, cvRect(Shape.MinX(), Shape.MinY(), Shape.GetWidth(), Shape.GetHeight()));
            IplImage *facialImage = cvCreateImage(cvGetSize(originalImage), originalImage->depth, originalImage->nChannels);
            cvCopy(originalImage, facialImage, NULL);
            cvResetImageROI(originalImage);

            CvSize stdsize;
            stdsize.width = stdwidth;
            stdsize.height = stdwidth / facialImage->width * facialImage->height;
            IplImage *stdImage = cvCreateImage(stdsize, originalImage->depth, originalImage->nChannels);
            cvResize(facialImage, stdImage, CV_INTER_LINEAR);

            //cvNamedWindow("CurrentFacialImage");
            //cvShowImage("CurrentFacialImage", stdImage);
            //cvWaitKey(0);

            //draw the shape
            CvSize ssize;
            ssize.width = 130;
            ssize.height = 130;
            IplImage *shapeImg = cvCreateImage(ssize, originalImage->depth, originalImage->nChannels);
            cvSet(shapeImg, CV_RGB(0,0,0));
            AAM_Shape temShape = Shape;
            double orgwid = temShape.MaxX() - temShape.MinX();
            double orghei = temShape.MaxY() - temShape.MinY();
            for (int i = 0; i < 68; i++) {
                temShape[i].x = (temShape[i].x - Shape.MinX()) * stdwidth / orgwid;
                temShape[i].y = (temShape[i].y - Shape.MinY()) * stdsize.height / orghei;
            }
            temShape.Sketch(shapeImg);
            cvShowImage("shape", shapeImg);
            cvReleaseImage(&shapeImg);



            int sizes = Shape.NPoints() * 2;
            points = new double[sizes];
            int index = 0;
            for (int i = 0; i < sizes; i += 2) {
                index = i / 2;
                points[i] = Shape[index].x;
                points[i + 1] = Shape[index].y;
            }
        }
        catch (AgingException ex) {
            processState = ex.getStateCode();
            points = new double[68 * 2];
            points[0] = 0 - processState;
        }

        return points;
    }
}

/*
int main() {
    char* originalImageFileName = "input.jpg";
	char* curAge = "1";
	char* predictAge = "3";
	char* ResultsSavePath = "ResultsSavePath";

    try {
        double* points = getShape(originalImageFileName, curAge);
        int sizes = 68 * 2;
        char buffer[68 * 2 * 11];

        char* ptr = buffer;

        for (int i = 0; i < sizes; i++) {
            memcpy(ptr, points + i, sizeof(points[i]));
            ptr += sizeof(points[i]) + 1;
            *ptr = ' ';
            ptr++;
        }


        printf("%s\n", buffer);

        int state = fit(originalImageFileName, curAge, predictAge, buffer, sizes);
        cout << state << endl;
    }
    catch (AgingException ex) {
        cout << ex.getStateCode() << endl;
    }

    return 0;
}
*/
