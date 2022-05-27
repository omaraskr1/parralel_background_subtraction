# parralel_background_subtraction
: Parallel Background Subtraction 
 
Description: 
 
It’s a technique for removing static background by subtracting a set of images to obtain a final image with the objects only without a background. This is a basic technique which will work only on tiny/small motion changes in the images that are given. 
 
 
Input and Output:  
  
 
Steps: 
• Transform Images (NXN) to Vectors [(N2) X 1] and put them in large array [ N2 X M ] such that M = Number of Images 
  
1.	Estimated Background image B is the mean of the collection input streams of size M 
For example : 
B0 = ( ( pixel 0 in image 1 + pixel 0 in image 2 +........ ) / M) 
B1 = ( ( pixel 1 in image 1 + pixel 1 in image 2 +........ ) / M) 
2.	Foreground Mask image X is check of subtract mean from image based on a threshold  
 For example : 
X0_new = | B0 - X0_new_input_frame | > TH 
X1_new = | B1 - X1_new_input_frame | > TH 
*TH is a threshold which you can tune (dependent on your visual results) 
•	Render your results 
•	Test your code with these different conditions: 
1- different image sizes (e.g if the image size is N * N, so test your code if the image size is 5N * 5N and 10N * 10N) 
•	Record your Results 
 
   
	N* N 
 	5N* 5N 
 	10N* 10N 
 
sequential code 	 	 	 
MPI Sol1 	 	 	 
MPI Sol (bonus) 			
Deliverables 
•	The source code 
•	A report on the findings and enhancements made to performance. The project discussion will be individual   Check these videos: 
●	Basic Background Subtraction - Motivation 
●	Background Mean Averaging 
