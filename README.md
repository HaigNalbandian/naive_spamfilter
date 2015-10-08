#Haig Nalbandian Sample AI Code

*Name: Haig Nalbandian
*email: hnalband@usc.edu or haignalbandian@gmail.com

###Descrption
This is a naive bayesian learner that classifies emails (treated as "bags of words") as either spam or ham. This is done by evaluating the number of occurences of any given word in an email and assessian a probability of its spam-hood based on a given training set.

The learner is first trained by the training set and provides output on the testing set.

In its current form, it does not output the status of individual emails as that was not a requirement for the project, but that extension would take a line or two of code.

###Compilation

Compile and run haignalbandian_proj3.cpp using g++4.8 and C++11. (I use auto iterator functionality, making this a requirement). Ensure you maintain current directory structure as finding the training and testing sets depend upon the directory structure.

