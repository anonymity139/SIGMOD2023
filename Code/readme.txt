Readme (Interactive Search with Mixed Attributes)
=========================
This package contains all source codes as follows.
a. Algorithm Tree 
	1. It only works for the special case of ISM.
	2. The code is in folder Tree.
b. Algorithm Separation 
	1. It works for the general case of ISM.
	2. The code is in folder Separation.
c. Algorithm SeparationCf 
	1. It works for the general case of ISM (for confidence study).
	2. It considers the case that users may not answer the questions.
	3. The code is in folder Separation.
d. Algorithm Combination 
	1. It works for the general case of ISM. 
	2. The code is in folder Combination.
e. Algorithm CombinationCf 
	1. It works for the general case of ISM (for confidence study). 
	2. It considers the case that users may not answer the questions.
	3. The code is in folder Combination.
f. Algorithm UH-Simplex 
	1. It is an adapted existing algorithm.
	2. The code is in folder UH.
g. Algorithm UH-Random 
	1. It is an adapted existing algorithm.
	2. The code is in folder UH.
h. Algorithm ActiveRanking 
	1. It is an adapted existing algorithm.
	2. The code is in folder Ranking.
i. Algorithm Adaptive 
	1. It is an adapted existing algorithm without pruning strategies.
	2. The code is in folder Adaptive.
j. Algorithm Adaptive-Prune 
	1. It is an adapted existing algorithm with pruning strategies.
	2. The code is in folder Adaptive.

Make sure there is a folder called "input/" and a folder called "output/" under the working directory.
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
		set(INC_DIR /usr/local/Cellar/glpk/5.0/include)
		set(LINK_DIR /usr/local/Cellar/glpk/5.0/lib)
	Update the path "/usr/local/Cellar/glpk/5.0" to the path you install the GLPK package
	
b. Execution
	./run

c. Input
	The input file contains the dataset (whose format will be described in Appendix A.)
	
d. Output
	The output will be shown on the console (whose format will be described in Appendix B.)

Example
=======
Sample input (input/special(c=4).txt) are provided. 
Try: ./run

Appendix A. Format of Input File
------------------------------------
The format of the first line is: n d_cat d_num
n - the number of tuples in the dataset, integer
d_cat - the number of categorical attributes in the dataset, integer
d_num - the number of numerical attributes in the dataset, integer
The format of the following n lines is
-----------------------------------------------------------------
<categorical attribute 1> <categorical attribute 2> ... <<categorical attribute d_cat> <numerical attribute 1> <numerical attribute 2> ... <numerical attribute d_num> 
-----------------------------------------------------------------
Each line corresponds to a tuple.
	
Appendix B. Format of Console Output
-----------------------------------------------------------------
The format of the output is
-----------------------------------------------------------------
|         Method | # of Questions |      Time Cost | Point #ID |
-----------------------------------------------------------------
-----------------------------------------------------------------
|   Ground Truth |              - |              - |    PtID-0 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|           Tree |      Q-count-1 |         Time-1 |    PtID-1 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|     Separation |      Q-count-2 |         Time-2 |    PtID-2 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|   SeparationCf |      Q-count-3 |         Time-3 |    PtID-3 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|    Combination |      Q-count-4 |         Time-4 |    PtID-4 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|  CombinationCf |      Q-count-5 |         Time-5 |    PtID-5 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|     UH-Simplex |      Q-count-6 |         Time-6 |    PtID-6 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|      UH-Random |      Q-count-7 |         Time-7 |    PtID-7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|  ActiveRanking |      Q-count-8 |         Time-8 |    PtID-8 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|       Adaptive |      Q-count-9 |         Time-9 |    PtID-9 |
-----------------------------------------------------------------
-----------------------------------------------------------------
| Adaptive-Prune |     Q-count-10 |        Time-10 |   PtID-10 |
-----------------------------------------------------------------
PtID-0 is the tuple ID of the user's favorite tuple (ground truth).
PtID-1 is the tuple ID of the user's favorite tuple returned by algorithm Tree.
PtID-2 is the tuple ID of the user's favorite tuple returned by algorithm Separation.
PtID-3 is the tuple ID of the user's favorite tuple returned by algorithm SeparationCf.
PtID-4 is the tuple ID of the user's favorite tuple returned by algorithm Combination.
PtID-5 is the tuple ID of the user's favorite tuple returned by algorithm CombinationCf.
PtID-6 is the tuple ID of the user's favorite tuple returned by algorithm UH-Simplex.
PtID-7 is the tuple ID of the user's favorite tuple returned by algorithm UH-Random.
PtID-8 is the tuple ID of the user's favorite tuple returned by algorithm ActiveRanking.
PtID-9 is the tuple ID of the user's favorite tuple returned by algorithm Adaptive.
PtID-10 is the tuple ID of the user's favorite tuple returned by algorithm Adaptive-Prune.
Q-count-1 is the number of questions asked by algorithm Tree.
Q-count-2 is the number of questions asked by algorithm Separation.
Q-count-3 is the number of questions asked by algorithm SeparationCf.
Q-count-4 is the number of questions asked by algorithm Combination.
Q-count-5 is the number of questions asked by algorithm CombinationCf.
Q-count-6 is the number of questions asked by algorithm UH-Simplex.
Q-count-7 is the number of questions asked by algorithm UH-Random.
Q-count-8 is the number of questions asked by algorithm ActiveRanking.
Q-count-9 is the number of questions asked by algorithm Adaptive.
Q-count-10 is the number of questions asked by algorithm Adaptive-Prune.
Time-1 is the execution time of algorithm Tree.
Time-2 is the execution time of algorithm Separation.
Time-3 is the execution time of algorithm SeparationCf.
Time-4 is the execution time of algorithm Combination.
Time-5 is the execution time of algorithm CombinationCf.
Time-6 is the execution time of algorithm UH-Simplex.
Time-7 is the execution time of algorithm UH-Random.
Time-8 is the execution time of algorithm ActiveRanking.
Time-9 is the execution time of algorithm Adaptive.
Time-10 is the execution time of algorithm Adaptive-Prune.

For example, you might see:
-----------------------------------------------------------------
|         Method | # of Questions |      Time Cost | Point #ID |
-----------------------------------------------------------------
-----------------------------------------------------------------
|   Ground Truth |              - |              - |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|        Special |              4 |       0.000145 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|     Separation |              4 |       0.001256 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|   SeparationCf |              4 |       0.000690 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|    Combination |              4 |       0.000717 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|  CombinationCf |              4 |       0.000700 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|     UH-Simplex |              4 |       0.017552 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|      UH-Random |              6 |       0.014837 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|  ActiveRanking |             11 |       0.009737 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
|       Adpative |             12 |       0.002053 |         7 |
-----------------------------------------------------------------
-----------------------------------------------------------------
| Adpative_Prune |              6 |       0.007971 |         7 |
-----------------------------------------------------------------