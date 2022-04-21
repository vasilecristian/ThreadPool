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

#define CHECK(condition) if(!(condition)) { std::cout << testName << " failed! (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; testReturnValue++; }	\
						 else { std::cout << testName << " passed!" << std::endl; }

int Test1(std::string name)
{
	TEST_INIT(name);
	
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
			std::cout << "Job " << i << std::endl;
			jobsExecuted++;
		});
	}
	pool.JoinAll();
	
	// Assert
	CHECK(jobsQuewed == jobsExecuted);
	
	
	return TEST_RETURN_VALUE;
}

int main(int argc, const char * argv[])
{
	int returnValue = 0;
	std::cout << "Tests Started...\n";
	
	returnValue += Test1("Test 1");
	
	return returnValue;
}


