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

#include "AAM_IC.h"
#include "AAM_Basic.h"
#include "AAM_VJFaceDetect.h"
#include "FacePredict.h"
#include "AAM_Util.h"

using namespace std;

std::string resultDir = "./trainingSets/";
std::string trainDir = "./trainingSets/";
//std::string resultDir = "../test2/";

static void usage()
{
	printf("AAMFitting int string int int\n"
		"---- int:		0:search shape by aam, 1:read shape from file"
		"---- string:	image file\n"
		"---- int:		current age group\n"
		"---- int:		predicted age group\n");
		//"Group0:[0,1), Group1:[1,2), Group2:[2,3), Group3:[3,6), Group4:[6,10), Group5:[10,18]";
		//"Group0:[0,2), Group1:[2,4), Group2:[4,6), Group3:[6,10), Group4:[10,15), Group5:[15,20), Group6:[20, 30), Group7: [30, 40), Group8: [40, 68) ";
	exit(0);
}

extern "C"
{
    int fit(char* originalImageFileName, char* curAge, char* predictAge, char* ResultsSavePath) {
        int processState = 0;
        try {


            IplImage *image = cvCreateImage(cvGetSize(originalImage), originalImage->depth, originalImage->nChannels);


            //do image alignment
            aam->Fit(image, Shape, 30, false);  //if true, show process

            ofstream outfile;
            string wuxuefTmpResultDir = resultDir + "aam_result.txt";
            outfile.open(wuxuefTmpResultDir.c_str());
            Shape.Write(outfile);
            outfile.close();

            //
            // show face detect result
            //cvNamedWindow("AAMFitting", CV_WINDOW_AUTOSIZE);
            //aam->Draw(image, 0);
            //cvShowImage("AAMFitting", image);
            //cvWaitKey(0);
            //}

            /*
            else {
                //read shape from file
                std::string filename = std::string(argv[2]);
                filename = filename.substr(0, filename.length()-3) + "pts";
                Shape.ReadPTS(filename);

                if (argc == 9) {
                    //read father's shape
                    std::string filenameF = std::string(argv[5]);
                    filenameF = filenameF.substr(0, filenameF.length()-3) + "pts";
                    ShapeF.ReadPTS(filenameF);

                    //read mother's shape
                    std::string filenameM = std::string(argv[7]);
                    filenameM = filenameM.substr(0, filenameM.length()-3) + "pts";
                    ShapeM.ReadPTS(filenameM);
                }
            }*/

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

            cvNamedWindow("CurrentFacialImage");
            cvShowImage("CurrentFacialImage", stdImage);


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


            //Facial Prediction
            FacePredict face_predict;
            std::string mfile = resultDir + "facial.predict_model";
            std::ifstream model(mfile.c_str());
            face_predict.Read(model);
            model.close();
            //IplImage* newImage = face_predict.predict(Shape, *originalImage, ShapeF, *ImageF, ratioF, ShapeM, *ImageM, ratioM, atoi(argv[3]), atoi(argv[4]), false);


            IplImage* newImage = face_predict.predict(Shape, *originalImage, atoi(curAge), atoi(predictAge), true);
            std::string newfile = std::string(originalImageFileName);
            newfile = newfile.insert(newfile.find_last_of('/')+1, "result_" );
            newfile = newfile.insert(newfile.find_last_of('.'), std::string("_G" + string(predictAge)));
            cvSaveImage(newfile.c_str(), newImage);

            cvNamedWindow("PredictedFacialImage");
            cvShowImage("PredictedFacialImage", newImage);
            cvWaitKey(0);

            cvReleaseImage(&image);
        }
        catch (AgingException ex) {
            processState = ex.getStateCode();
        }

        return processState;
    }

    int* getShape(char* originalImageFileName) {
        AAM_Shape Shape;
        int* points;

        try {
            //load image
            IplImage* originalImage = cvLoadImage(originalImageFileName, 1);
            if (originalImage == 0) {
                AgingException ex(1);
                throw ex;
            }

            IplImage *image = cvCreateImage(cvGetSize(originalImage), originalImage->depth, originalImage->nChannels);
            cvCopy(originalImage, image);

            AAM *aam = NULL;
            int type;
            std::string aamFileName = resultDir + "Group" + curAge + ".aam_ic";
            std::ifstream fs(aamFileName.c_str());
            if(fs == 0) {
                AgingException ex(3);
                throw ex;
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
            int sizes = Shape.NPoints();
            int* points = new int[sizes * 2];
            for (int i = 0; i < sizes; i += 2) {
                points[i] = Shape[i].x;
                points[i + 1] = Shape[i].y;
            }
        }
        catch (AgingException ex) {
            processState = ex.getStateCode();
            points = new int[sizes * 2];
            points[0] = 0 - processState;
        }

        return points;
    }
}

Shape makeShape() {

}


int main() {
    char* originalImageFileName = "input.jpg";
	char* curAge = "9";
	char* predictAge = "3";
	char* ResultsSavePath = "ResultsSavePath";

    cout << fit(originalImageFileName, curAge, predictAge, ResultsSavePath) << endl;
    system("pause");
    return 0;
}
