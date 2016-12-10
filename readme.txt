

1) How to build/run your project

   Our project uses the GMP public library for for handing large numbers.  The library can be found here: https://gmplib.org/ .

   A guide to installing GMP can be found here:  https://gmplib.org/manual/Installing-GMP.html#Installing-GMP



   Once GMP is installed, the program can be built and executed just like any normal c++ program.

   The program must be compiled with the following tags: -lgmp -lgmpxx

   The -lgmp tag is required for the the program to compile properly with the GMP library.



   When executing, the program takes in 2 text files:

   	1) a file containing the last name, first name, and voter id number of a voter, delimited by spaces. the voters are delimited by new lines.

	2) a file containing the last name, first name of a candidate, delimited by spaces. the candidates are delimited by new lines.



2) We assume that our text files are formatted properly so they can be read by our program. If there is a tie, we assume that the first candidate in the list wins.



3) We used the GMP library for handling large numbers.

   It can be found here: https://gmplib.org/ 

   The GMP manual can be found here: https://gmplib.org/manual/



