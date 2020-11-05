///*
// * Re-hashing.cpp
// *
// *  Created on: Nov 5, 2020
// *      Author: ahmed
// */
#include "readfile.h"

int hashCodeMain(int key){
   return key % MBUCKETS;
}

int hashCode2(int key){ //function for re-hashing if collision takes place
   return 7-(key%7);
}

int insertItem3(int fd,DataItem item){
   //TODO: implement this function
	int count = 0;
	int rewind = 0;
	int hashIndex = hashCodeMain(item.key);
	//----------------------------------//
	int hashIndex2 = hashCode2(item.key);  //prepare the result of the second hash if there's a collision
	int startingOffset = hashIndex2*sizeof(Bucket); // to indicate the starting offset that open addressing will start from in case of second collision
	int Offset = hashIndex*sizeof(Bucket);
	//----------------------------------//
	int Offset2= hashIndex2*sizeof(Bucket); //the second offset of the second hash function
	struct DataItem data;
	bool secondHashDone=0;
	RESEEK:
		ssize_t result = pread(fd, &data, sizeof(DataItem), Offset);
		count++;
		if(result <= 0)
			return -1;
		else if(data.valid != 1)
		{
			int res = pwrite(fd, &item, sizeof(DataItem), Offset);
			return count;
		}
		else if(!secondHashDone){
			// if the there's collision with the first hash function, use the second one
			// if it still collides, start an open addressing technique from th second offset
			//----------------------------------//
			secondHashDone=1;
			ssize_t result = pread(fd, &data, sizeof(DataItem), Offset2);
			count++;
			if(result <= 0)
				return -1;
			else if(data.valid != 1){
				int res = pwrite(fd, &item, sizeof(DataItem), Offset2);
				return count;
			}
			else{
				//-- open addressing from offset 2 ---------//
				Offset2 +=sizeof(DataItem);
				if(Offset2 >= FILESIZE && rewind ==0 )  //reach the end of file
				{
					rewind = 1;
					Offset2 = 0;
					Offset=Offset2;
					goto RESEEK;
				}
				/*else if(rewind == 1 && Offset2 >= startingOffset) // no Empty record to insert
						return count;*/
				Offset=Offset2;
				goto RESEEK;
			}
		}
		else{
			Offset2 +=sizeof(DataItem);
			if(Offset2 >= FILESIZE && rewind ==0 )  //reach the end of file
    		{
				rewind = 1;
				Offset2 = 0;
				Offset=Offset2;
				goto RESEEK;
    	    }
    	    else if(rewind == 1 && Offset2 >= startingOffset) // no Empty record to insert
    				return count;
			Offset=Offset2;
    		goto RESEEK;
		}

	//return 0;
}


int searchItem3(int fd,struct DataItem* item,int *count)
{
	//TODO: implement this function

	//Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	*count = 0;				//No of accessed records
	int rewind = 0;			//A flag to start searching from the first bucket
	int hashIndex = hashCodeMain(item->key);  				//calculate the Bucket index
	int hashIndex2 = hashCode2(item->key);  //prepare the result of the second hash if there's a collision
	int startingOffset = hashIndex2*sizeof(Bucket); // to indicate the starting offset that open addressing will start from in case of second collision
	int Offset = hashIndex*sizeof(Bucket);
	int Offset2= hashIndex2*sizeof(Bucket); //the second offset of the second hash function
	bool secondHashDone=0;

	RESEEK:
		ssize_t result = pread(fd, &data, sizeof(DataItem), Offset);
		(*count)++;
		if(result <= 0)
			return -1;
		else if(data.valid == 1 && data.key == item->key)
		{
			//I found the needed record
			item->data = data.data ;
			return Offset;
		}
		else if(!secondHashDone){
			// if the there's collision with the first hash function, use the second one
			// if it still collides, start an open addressing technique from the second offset
			//----------------------------------//
			secondHashDone=1;
			ssize_t result = pread(fd, &data, sizeof(DataItem), Offset2);
			(*count)++;
			if(result <= 0)
				return -1;
			else if(data.valid == 1 && data.key == item->key ){
				//I found the needed record
				item->data = data.data ;
				return Offset;
			}
			else{
				//-- open addressing from offset 2 ---------//
				Offset2 +=sizeof(DataItem);
				if(Offset2 >= FILESIZE && rewind ==0 )  //reach the end of file
				{
					rewind = 1;
					Offset2 = 0;
					Offset=Offset2;
					goto RESEEK;
				}
				/*else if(rewind == 1 && Offset2 >= startingOffset) // no Empty record to insert
						return count;*/
				Offset=Offset2;
				Offset=Offset2;
				goto RESEEK;
			}
		}
		else{
			Offset2 +=sizeof(DataItem);
			if(Offset2 >= FILESIZE && rewind ==0 )  //reach the end of file
			{
				rewind = 1;
				Offset2 = 0;
				Offset=Offset2;
				goto RESEEK;
			}
			else if(rewind == 1 && Offset2 >= startingOffset) // no Empty record to insert
					return -1;
			Offset=Offset2;
			goto RESEEK;
		}
}


/* Functionality: Display all the file contents
 *
 * Input:  fd: filehandler which contains the db
 *
 * Output: no. of non-empty records
 */

/*
int DisplayFile(int fd){

	struct DataItem data;
	int count = 0;
	int Offset = 0;
	for(Offset =0; Offset< FILESIZE;Offset += sizeof(DataItem))
	{
		ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
		if(result < 0)
		{ 	  perror("some error occurred in pread");
			  return -1;
		} else if (result == 0 || data.valid == 0 ) { //empty space found or end of file
			printf("Bucket: %d, Offset %d:~\n",Offset/BUCKETSIZE,Offset);
		} else {
			pread(fd,&data,sizeof(DataItem), Offset);
			printf("Bucket: %d, Offset: %d, Data: %d, key: %d\n",Offset/BUCKETSIZE,Offset,data.data,data.key);
					 count++;
		}
	}
	return count;
}
*/

/* Functionality: Delete item at certain offset
 *
 * Input:  fd: filehandler which contains the db
 *         Offset: place where it should delete
 *
 * Hint: you could only set the valid key and write just and integer instead of the whole record
 */
/*
int deleteOffset(int fd, int Offset)
{
	struct DataItem dummyItem;
	dummyItem.valid = 0;
	dummyItem.key = -1;
	dummyItem.data = 0;
	int result = pwrite(fd,&dummyItem,sizeof(DataItem), Offset);
	return result;
}
*/
