//
//  main.cpp
//  test
//
//  Created by vasile.cristian on 20.04.2022.
//

#include <iostream>
#include "ThreadPool/ThreadPool.h"

int Test1()
{
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
	return (jobsQuewed == jobsExecuted) ? 0 : 1;
}

int main(int argc, const char * argv[])
{
	std::cout << "Tests Started...\n";
	
	int a = Test1();
	
	return a;
}


