Readme (Recommending Car System)
=========================
This package contains all source codes for user study on
a. Algorithm Combination. 
	1. It works for the general case of ISM. 
	2. The code is in files Combination.h and Combination.cpp
b. Algorithm Separation 
	1. It works for the general case of ISM.
	2. The code is in files Separation.h and Separation.cpp
c. Algorithm UH-Simplex 
	1. It is an adapted existing algorithm.
	2. The code is in folder UH.
d. Algorithm UH-Random 
	1. It is an adapted existing algorithm.
	2. The code is in folder UH.
e. Algorithm ActiveRanking 
	1. It is an adapted existing algorithm.
	2. The code is in folder Ranking.
f. Algorithm Adaptive 
	1. It is an adapted existing algorithm.
	2. The code is in folder Adaptive.

This package also provides the used car dataset from
https://www.kaggle.com/orgesleka/used-cars-database.

Make sure there are folders called "input/", "output/" and "Result/" under the working directory.
They will be used for storing the input/output files and some intermediate results.

Usage Step
==========
a. Compilation
	mkdir build
	cd build
	cmake ..
	make

	You will need to install the GLPK package (for solving LPs) at first.
	See GLPK webpage <http://www.gnu.org/software/glpk/glpk.html>.
	Then update the path in CMakeLists.txt
		set(INC_DIR /usr/local/Cellar/glpk/4.65/include)
		set(LINK_DIR /usr/local/Cellar/glpk/4.65/lib)
	Update "/usr/local/Cellar/glpk/4.65" to the path where you install the GLPK package
	
b. Execution
	./run

c. Input
	The used car dataset is shown in input/car1.txt (whose format will be described in Appendix A.)
	
d. Output
	1. The console output will be shown on the console (whose format will be described in Appendix B.)
	2. The user log could be found in Result/result.txt (whose format will be described in Appendix C.)

Example
=======
Try: ./run
Sample user log is provided: Result/result.txt

Appendix A. Format of Input File
------------------------------------
For input/car1.txt (the used car dataset with four attributes: price, year, power and used KM):
	The format of the first line is: n d_cat d_num
	n - the number of cars in the dataset, integer
	d_cat - the number of categorical attributes in the dataset, integer
	d_num - the number of numerical attributes in the dataset, integer
	The format of the following n lines is
	<fuel type> <price> <year of manufacture> <horse power> <used kilometers>.
	Each line corresponds to a car
	
Appendix B. Format of Console Output
------------------------------------
The console output consists of three components: (a) interaction, (b) evaluation and (c) order
The recommending car system interacts with you for different rounds.

a. Interaction
	Each round corresponds to an algorithm in our system. In each round, you will be 
	presented consecutive questions. Each question consists of two cars and asks you to 
	choose the one you favor more.
	For example, you might see:
	Please choose the car you favor more:
	-------------------------------------------------------------------------
                  Fuel    Price($)        Year     PowerPS    Used KM
	-------------------------------------------------------------------------
  	Option 1    benzin       28800        2013         432       20000
	-------------------------------------------------------------------------
  	Option 2    diesel       28600        2015         181       60000
	-------------------------------------------------------------------------
	Your choice: 
	
b. Evaluation
	At the end of each round, it will ask you to give a score to indicate how bored you
	feel when you are asked with XX questions in this round in order to obtain your 
	favorite car. 
	For example, you might see:
	-----------------------------------------------------------------------
   	Number of questions asked:    41 
	-----------------------------------------------------------------------
	-----------------------------------------------------------------------
     Result      Fuel    Price($)        Year     PowerPS    Used KM
	-----------------------------------------------------------------------
       Car    benzin       28800        2013         432       20000
	-----------------------------------------------------------------------
	Please give a number from 1 to 10 (i.e., 1, 2, .., 10) to indicate how 
	bored you feel when you are asked with 41 questions in this round in 
	order to obtain your favorite car (Note: 10 denotes that you feel the 
	most bored and 1 denotes that you feel the least bored.): 

c. Order
	After all the rounds. it will ask you to compare the recommended cars returned by 
	different rounds (i.e., different algorithms). You will be presented all the 
	recommended cars and you need to give an order of the recommended cars based on 
	your preference. 
	For example, you might see:
	The recommended tuples: 
	----------------------------------------------------------------------------
         1    benzin       16800        2005         500      125000
	----------------------------------------------------------------------------
         2    benzin        3600        2004         207       70000
	----------------------------------------------------------------------------
         3    benzin        3000        2008         192       70000
	----------------------------------------------------------------------------
         4    diesel        2800        2003         131       30000
	----------------------------------------------------------------------------
         5       cng        5555        2008         145      125000
	----------------------------------------------------------------------------
	Please give an order for the shown used car(s) (e.g., 1 2 3 4 5), where the first one 
	is the most preferred tuple and the last one is the least preferred tuple:
	
Appendix C. Format of User Log
------------------------------------
	The user log file is shown in Result/result.txt.
	It contains two parts: (a) The result of rounds and (b) The result of order

a. The result of rounds
	Lines 1-2 show the algorithm name and the number of questions asked.
	Lines 3-5 show the recommended car.
	Lines 6-7 show the evaluation result.
	For example, you might see:

	Algorithm: UH-Simplex   Question: 23 
	----------------------------------------------------------------------------
     Result      Fuel    Price($)        Year     PowerPS    Used KM
	----------------------------------------------------------------------------
        Car    benzin        3600        2004         207       70000
	----------------------------------------------------------------------------
	Boredness: 3
	
b. The result of order
	Line 1 is the title "order".
	The rest lines show the order of the recommended cars (which are returned by 
	different rounds) based on the user preference, where the first one is the most 
	preferred tuple and the last one is the least preferred tuple:
	For example, you might see:

	order: 
	---------------------------------------------------------------------------------
         1    benzin       16800        2005         500      125000
	---------------------------------------------------------------------------------
         2    benzin        3600        2004         207       70000
	---------------------------------------------------------------------------------
         3    benzin        3000        2008         192       70000
	---------------------------------------------------------------------------------
         4    diesel        2800        2003         131       30000
	---------------------------------------------------------------------------------
         5       cng        5555        2008         145      125000
	---------------------------------------------------------------------------------
