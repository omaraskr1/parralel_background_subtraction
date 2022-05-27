#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#pragma once
#include <mpi.h>
#include<stdio.h>
#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height*BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}
static string ext = ".jpg";
int** inputVideoframe(int numberOfImages, int* width, int* hight)
{
	int** imagArray = new int* [numberOfImages];
	System::String^ imagePath;
	string img, imageNum, image;
	string in = "in";
	string zeros = "00000";
	int i = 1;
	while (i <= numberOfImages)
	{
		if (i > 9) { zeros = "0000"; }
		if (i > 99) { zeros = "000"; }
		imageNum = in + zeros + to_string(i);
		image = imageNum + ext;
		img = "..//Data//Input//" + image;
		imagePath = marshal_as<System::String^>(img);
		int* imageData = inputImage(&*width, &*hight, imagePath);
		imagArray[i - 1] = new int[*width * *hight];
		imagArray[i - 1] = imageData;
		i++;
	}

	return imagArray;
}
int main()
{
	int start_s, stop_s, TotalTime = 0;
	start_s = clock();
	MPI_Init(NULL, NULL);
	int ImageWidth = 10*4, ImageHeight = 10*4;

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	//cout << "size= " << size;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int image_fullsize = 0;


	int** images_arr = 0;
	int* threahold_image = 0;
	int number_of_images;
	int threahold_power;
	
	
	if (rank == 0)
	{
		
		cout << "how many image you want to add: ";
		cin >> number_of_images;

		cout << "Enter threahold power (25-100): ";
		cin >> threahold_power;
		images_arr = inputVideoframe(number_of_images, &ImageWidth, &ImageHeight);
		
		System::String^ imagePath;
		std::string img;
		img = "..//Data//Input//in000376" + ext;;

		imagePath = marshal_as<System::String^>(img);
		 threahold_image = inputImage(&ImageWidth, &ImageHeight, imagePath);

		image_fullsize = ImageWidth * ImageHeight;
	}
	
	//createImage(imageData, ImageWidth, ImageHeight, 0);
	//data send,count,data type,root
	MPI_Bcast(&number_of_images, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&image_fullsize, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&threahold_power, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	int** array_of_images_array = new int* [number_of_images];
	int* background_image = new int[image_fullsize];
	int* foreground_image = new int[image_fullsize];
	int size_of_each_rank_take = (image_fullsize) / size;

	for (int i = 0; i < number_of_images; i++)
	{
		if (rank == 0)
		{
			int avg = 0;
			
				int j = 1;
				while (j < size) {
				
					MPI_Send(&images_arr[i][j * size_of_each_rank_take], size_of_each_rank_take, MPI_INT, j, 0, MPI_COMM_WORLD);
					j++;
				}
			
			for (int x = 0; x < size_of_each_rank_take; x++) {
				int k = 0;
				while (k < number_of_images) {

					avg += images_arr[k][x];
					k++;
				}

				avg =avg / number_of_images;
				background_image[x] = avg;

				int diff_between_two_imgs = abs(background_image[x] - threahold_image[x]);
				
				if (diff_between_two_imgs < threahold_power) 
				{
					foreground_image[x] = 0;
				}
				else 
				{
					foreground_image[x] = diff_between_two_imgs;
				}
					
			}
			if (i == number_of_images - 1) 
			{
				int k = 1;
				while ( k < size)
				{
					MPI_Status status;	
					MPI_Send(&threahold_image[k * size_of_each_rank_take], size_of_each_rank_take, MPI_INT, k, 0, MPI_COMM_WORLD);
					MPI_Recv(&background_image[k * size_of_each_rank_take], size_of_each_rank_take, MPI_INT, k, 0, MPI_COMM_WORLD, &status);
					MPI_Recv(&foreground_image[k * size_of_each_rank_take], size_of_each_rank_take, MPI_INT, k, 0, MPI_COMM_WORLD, &status);
					k++;
				}


				if (rank == 0) 
				{
					createImage(background_image, ImageWidth, ImageHeight, 0);
					createImage(foreground_image, ImageWidth, ImageHeight, 1);
			    }
			}

		}
		else if (rank <= size)
		{
			MPI_Status status;
			array_of_images_array[i] = new int[size_of_each_rank_take];
			MPI_Recv(array_of_images_array[i], size_of_each_rank_take, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

			if (i == number_of_images - 1)
			{
				int* threshold_image_loca = new int[size_of_each_rank_take];
				MPI_Recv(threshold_image_loca, size_of_each_rank_take, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);



				int* forground_image_local = new int[size_of_each_rank_take];
				int* background_image_local = new int[size_of_each_rank_take];
				int avg = 0;
				
				for (int x = 0; x < size_of_each_rank_take; x++) {
					int sum = 0;
					for (int k = 0; k < number_of_images; k++) {
					

						sum =sum+ array_of_images_array[k][x];

					}
					int difference;
					avg =sum/ number_of_images;
					background_image_local[x] = avg;
					difference = abs(background_image_local[x] - threshold_image_loca[x]);
					if (threahold_power > difference)
					{
						forground_image_local[x] = 0;
					}
					else
					{
						forground_image_local[x] = difference;
					}


				}
				MPI_Send(background_image_local, size_of_each_rank_take, MPI_INT, 0, 0, MPI_COMM_WORLD);
				MPI_Send(forground_image_local, size_of_each_rank_take, MPI_INT, 0, 0, MPI_COMM_WORLD);
			}
		}

	}


	MPI_Finalize();


	// seq = 128381 
	// mpi = 45148
	stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "time: " << TotalTime << endl;
	system("pause");
	//free(imageData);
	return 0;

}



