/*
 * main.cpp
 *
 *  Created on: Nov 5, 2020
 *      Author: ahmed
 */


#include "readfile.h"


//#include <iostream>
//using namespace std;

void insert(int key,int data,bool isRe);
int deleteItem(int key,bool isRe);
struct DataItem * search(int key,bool isRe);


int filehandle;   //handler for the database file

/* DBMS (DataBase Management System) needs to store its data in something non-volatile
 * so it stores its data into files (manteqy :)).

 * Some DBMS or even file-systems constraints the size of that file.

 * for the efficiency of storing and retrieval, DBMS uses hashing
 * as hashing make it very easy to store and retrieve, it introduces
 * another type of problem which is handling conflicts when two items
 * have the same hash index

 * In this exercise we will store our database into a file and experience
 * how to retrieve, store, update and delete the data into this file

 * This file has a constant capacity and uses external-hashing to store records,
 * however, it suffers from hashing conflicts.
 *
 * You are required to help us resolve the hashing conflicts

 * For simplification, consider the database has only one table
 * which has two columns key and data (both are int)

 * Functions in this file are just wrapper functions, the actual functions are in openAddressing.cpp

*/

void runRehashing(){
   filehandle = createFile(FILESIZE, "RehashingMainTest");
   DisplayFile(filehandle);
   //main_testCase
   insert(1, 20,true);
   insert(2, 70,true);
   insert(42, 80,true);
   insert(4, 25,true);
   insert(12, 44,true);
   insert(14, 32,true);
   insert(17, 11,true);
   insert(13, 78,true);
   insert(37, 97,true);
   insert(11, 34,true);
   insert(22, 730,true);
   insert(46, 840,true);
   insert(9, 83,true);
   insert(21, 424,true);
   insert(41, 115,true);
   insert(71, 47,true);
   insert(31, 92,true);
   insert(73, 45,true);
   DisplayFile(filehandle);
   search(13,true);
   deleteItem(31,true);
   DisplayFile(filehandle);
   close(filehandle);
   printf("----------------------------------------------\n");
   filehandle = createFile(FILESIZE, "Rehashing2Tests");
   DisplayFile(filehandle);
   //testCase1
   insert(1, 20,true);
   insert(11, 34,true);
   insert(2, 70,true);
   insert(42, 80,true);
   insert(4, 25,true);
   insert(12, 44,true);
   DisplayFile(filehandle);
   search(11,true);
   deleteItem(12,true);
   DisplayFile(filehandle);
   printf("----------------------------------------------\n");
   //testCase2
   insert(43, 840,true);
   insert(9, 83,true);
   insert(23, 424,true);
   insert(39, 115,true);
   insert(49, 47,true);
   insert(50, 92,true);
   insert(60, 45,true);
   insert(20, 128,true);
   insert(25, 128,true);
   insert(35, 128,true);
   insert(45, 128,true);
   insert(55, 128,true);
   DisplayFile(filehandle);
   search(49,true);
   deleteItem(35,true);
   DisplayFile(filehandle);
   printf("----------------------------------------------\n");
   close(filehandle);
}

void runOpenAddressing(){
	//here we create a sample test to read and write to our database file
	  //1. Create Database file or Open it if it already exists, check readfile.cpp
	   filehandle = createFile(FILESIZE, "openaddressing");
	  //2. Display the database file, check openAddressing.cpp
	   DisplayFile(filehandle);

	  //3. Add some data in the table
	   insert(1, 20,false);
	   insert(2, 70,false);
	   insert(42, 80,false);
	   insert(4, 25,false);
	   insert(12, 44,false);
	   insert(14, 32,false);
	   insert(17, 11,false);
	   insert(13, 78,false);
	   insert(37, 97,false);
	   insert(11, 34,false);
	   insert(22, 730,false);
	   insert(46, 840,false);
	   insert(9, 83,false);
	   insert(21, 424,false);
	   insert(41, 115,false);
	   insert(71, 47,false);
	   insert(31, 92,false);
	   insert(73, 45,false);

	   //4. Display the database file again
	   DisplayFile(filehandle);
	   //5. Search the database
	   search(13,false);

	   //6. delete an item from the database
	   deleteItem(31,false);

	   //7. Display the final data base
	   DisplayFile(filehandle);
	   // And Finally don't forget to close the file.
	   close(filehandle);
}

int main(){
   runRehashing();
   //runOpenAddressing();
   return 0;
}

/* functionality: insert the (key,data) pair into the database table
                  and print the number of comparisons it needed to find
    Input: key, data
    Output: print statement with the no. of comparisons
*/
void insert(int key,int data, bool isRe=false){
     struct DataItem item ;
     item.data = data;
     item.key = key;
     item.valid = 1;
     int result=0;
     if(isRe)
    	 result= insertItem3(filehandle,item);  //TODO: implement this function
     else
    	 result= insertItem(filehandle,item);
     printf("Insert: No. of searched records:%d\n",abs(result));
}

/* Functionality: search for a data in the table using the key

   Input:  key
   Output: the return data Item
*/
struct DataItem * search(int key, bool isRe=false)
{
  struct DataItem* item = (struct DataItem *) malloc(sizeof(struct DataItem));
     item->key = key;
     int diff = 0;
     int Offset=0;
     if(isRe)
    	 Offset= searchItem3(filehandle,item,&diff);
     else
    	 Offset= searchItem(filehandle,item,&diff);
     printf("Search: No of records searched is %d\n",diff);
     if(Offset <0)  //If offset is negative then the key doesn't exists in the table
       printf("Item not found\n");
     else
        printf("Item found at Offset: %d,  Data: %d and key: %d\n",Offset,item->data,item->key);
  return item;
}

/* Functionality: delete a record with a certain key

   Input:  key
   Output: return 1 on success and -1 on failure
*/
int deleteItem(int key,bool isRe=false){
   struct DataItem* item = (struct DataItem *) malloc(sizeof(struct DataItem));
   item->key = key;
   int diff = 0;
   int Offset=0;
   if(isRe)
	 Offset= searchItem3(filehandle,item,&diff);
   else
	 Offset= searchItem(filehandle,item,&diff);
   printf("Delete: No of records searched is %d\n",diff);
   if(Offset >=0 )
   {
    return deleteOffset(filehandle,Offset);
   }
   return -1;
}
