//
//  main.cpp
//  test
//
//  Created by vasile.cristian on 20.04.2022.
//

#include <iostream>
#include "ThreadPool/ThreadPool.h"

#define TEST_INIT(nameOfTest)	\
	std::string testName = nameOfTest;	\
	int testReturnValue = 0;

#define TEST_RETURN_VALUE testReturnValue

#define CHECK(condition) if(!(condition)) { std::cout << "Failed: " << testName << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; testReturnValue++; }	\
						 else { std::cout << "Passed: " << testName << std::endl; }

int Test1()
{
	TEST_INIT("Test1: All jobs need to be executed before Join!");
	
	// Arrange
	tp::ThreadPool pool(10);
	std::mutex protectedCode;
	int jobsQuewed = 100;
	int jobsExecuted = 0;
	
	// Act
	for(int i = 0; i < jobsQuewed; i++)
	{
		pool.AddJob([i, &protectedCode, &jobsExecuted]
		{
			std::unique_lock<std::mutex> lk(protectedCode);
			//std::cout << "Job " << i << std::endl;
			jobsExecuted++;
		});
	}
	pool.JoinAll();
	
	// Assert
	CHECK(jobsQuewed == jobsExecuted);
	
	
	return TEST_RETURN_VALUE;
}

int Test2()
{
	TEST_INIT("Test2: Afetr JoinAll no other job can be added!");

	// Arrange
	tp::ThreadPool pool(10);
	std::mutex protectedCode;
	int jobsExecuted = 0;

	// Act
	pool.AddJob([&protectedCode, &jobsExecuted]
	{
		std::unique_lock<std::mutex> lk(protectedCode);
		jobsExecuted++;
	});
	pool.JoinAll();
	pool.AddJob([&protectedCode, &jobsExecuted]
	{
		std::unique_lock<std::mutex> lk(protectedCode);
		jobsExecuted++;
	});

	// Assert
	CHECK(jobsExecuted == 1);

	return TEST_RETURN_VALUE;
}

int Test3()
{
	TEST_INIT("Test3: All jobs need to be executed before WaitAll!");

	// Arrange
	tp::ThreadPool pool(10);
	std::mutex protectedCode;
	int jobsQuewed = 100;
	int jobsExecuted = 0;

	// Act
	for (int i = 0; i < jobsQuewed; i++)
	{
		pool.AddJob([i, &protectedCode, &jobsExecuted]
		{
			std::unique_lock<std::mutex> lk(protectedCode);
			//std::cout << "Job " << i << std::endl;
			jobsExecuted++;
		});
	}
	pool.WaitAll();

	// Assert
	CHECK(jobsQuewed == jobsExecuted);

	return TEST_RETURN_VALUE;
}

int Test4()
{
	TEST_INIT("Test4: Afetr WaitAll other job can be added!");

	// Arrange
	tp::ThreadPool pool(10);
	std::mutex protectedCode;
	int jobsExecuted = 0;

	// Act
	pool.AddJob([&protectedCode, &jobsExecuted]
	{
		std::unique_lock<std::mutex> lk(protectedCode);
		jobsExecuted++;
	});
	pool.WaitAll();
	pool.AddJob([&protectedCode, &jobsExecuted]
	{
		std::unique_lock<std::mutex> lk(protectedCode);
		jobsExecuted++;
	});
	pool.JoinAll();

	// Assert
	CHECK(jobsExecuted == 2);

	return TEST_RETURN_VALUE;
}

int main(int argc, const char * argv[])
{
	int returnValue = 0;
	std::cout << "Tests Started...\n";
	
	returnValue += Test1();
	returnValue += Test2();
	returnValue += Test3();
	returnValue += Test4();
	
	return returnValue;
}


