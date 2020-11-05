#include "readfile.h"

/* Hash function to choose bucket
 * Input: key used to calculate the hash
 * Output: HashValue;
 */
int hashCode(int key){
   return key % MBUCKETS;
}


/* Functionality insert the data item into the correct position
 *          1. use the hash function to determine which bucket to insert into
 *          2. search for the first empty space within the bucket
 *          	2.1. if it has empty space
 *          	           insert the item
 *          	     else
 *          	          use chaining to insert the record
 *          3. return the number of records accessed (searched)
 *
 * Input:  fd: filehandler which contains the db
 *         item: the dataitem which should be inserted into the database
 *
 * Output: No. of record searched
 *
 * Hint: You can check the search function to give you some insights
 * Hint2: Don't forget to set the valid bit = 1 in the data item for the rest of functionalities to work
 * Hint3: you will want to check how to use the pwrite function using man pwrite on the terminal
 * 			 ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
 *
 * 	pwrite() writes up to count bytes from the buffer starting  at  buf  to
       the  file  descriptor  fd  at  offset  offset.
 */
int insertItem(int fd,DataItem item){
   //TODO: implement this function
   // use the hash function to determine which bucket to insert into 
   int hashIndex = hashCode(item.key);
   struct DataItem data;   //a variable to read in it the records from the db
   //check if collision happen 
    int saveOffset;
	int count = 0;
    int startingOffset = hashIndex*BUCKETSIZE;		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db
    int records_counter = 0; //  count records within one bucket
	//Main Loop
	RESEEK:
	//on the linux terminal use man pread to check the function manual
	ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
	saveOffset=Offset;
	//one record accessed
	 count++;
	//check whether it is a valid record or not
    if(data.valid==0 || result==0) //empty space
	{ 	 // perror("some error occurred in pread");
	// empty record 
	// insert into it 
	//printf("normal insert at %d\n",hashIndex);
	item.overflowPointer=-1; // no overflow ocurred 
    pwrite(fd,&item,sizeof(item), Offset);	
	
	return count;
    }
   else { // this is not an empty record 
   
        Offset +=sizeof(DataItem);  //move the offset to next record
		records_counter++; //increment counter of records within one bucket
    	if(records_counter>=RECORDSPERBUCKET){
			// apply chaining 		
			//printf("apply chaining this bucket is full \n");
			// 
			int startOffsetOverflow = MBUCKETS*BUCKETSIZE;
			for(int offset=startOffsetOverflow;offset<FILESIZE;offset += sizeof(DataItem)){
				struct DataItem tmp;
				ssize_t result = pread(fd,&tmp,sizeof(DataItem), offset);
				count++;
				if(result==0||tmp.valid==0){// unused overflow area
					pwrite(fd,&item,sizeof(item), offset);
					data.overflowPointer=offset;
					pwrite(fd,&data,sizeof(DataItem), saveOffset);
					return count;
				}
			}
			return count;
		}
    	goto RESEEK;
    }
	
}


/* Functionality: using a key, it searches for the data item
 *          1. use the hash function to determine which bucket to search into
 *          2. search for the element starting from this bucket and till it find it.
 *          3. return the number of records accessed (searched)
 *
 * Input:  fd: filehandler which contains the db
 *         item: the dataitem which contains the key you will search for
 *               the dataitem is modified with the data when found
 *         count: No. of record searched
 *
 * Output: the in the file where we found the item
 */

int searchItem(int fd,struct DataItem* item,int *count)
{

	//Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	*count = 0;				//No of accessed records
	int rewind = 0;			//A flag to start searching from the first bucket
	int hashIndex = hashCode(item->key);  				//calculate the Bucket index
	int startingOffset = hashIndex*BUCKETSIZE;		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db
    int records_counter = 0; //  count records within one bucket
	//Main Loop
	RESEEK:
	//on the linux terminal use man pread to check the function manual
	ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
	//one record accessed
	(*count)++;
	//check whether it is a valid record or not
    if(result <= 0) //either an error happened in the pread or it hit an unallocated space
	{ 	 // perror("some error occurred in pread");
		  return -1;
    }
    else if (data.valid == 1 && data.key == item->key) {
    	//I found the needed record
    			item->data = data.data ;
    			return Offset;

    } else { //not the record I am looking for
    		Offset +=sizeof(DataItem);  //move the offset to next record
			records_counter++;
			if(records_counter>=RECORDSPERBUCKET){ // bucket is full 
			printf("hena el debug %d\n", *count);
			// search at overflow area 
			int startOffsetOverflow = MBUCKETS*BUCKETSIZE;
			for(int offset=startOffsetOverflow;offset<FILESIZE;offset += sizeof(DataItem)){
				(*count)++;
				ssize_t result = pread(fd,&data,sizeof(DataItem), offset);
				if(data.valid==1 && data.key == item->key){ // found
					ssize_t result = pread(fd,&data,sizeof(DataItem), offset);
					return offset;
				}
			}
				
			}
    		if(Offset >= FILESIZE && rewind ==0 )
    		 { //if reached end of the file start again
    				rewind = 1;
    				Offset = 0;
    				goto RESEEK;
    	     } else
    	    	  if(rewind == 1 && Offset >= startingOffset) {
    				return -1; //no empty spaces
    	     }
    		goto RESEEK;
    }
}


/* Functionality: Display all the file contents
 *
 * Input:  fd: filehandler which contains the db
 *
 * Output: no. of non-empty records
 */
int DisplayFile(int fd){

	struct DataItem data;
	struct Bucket bkt;
	int count = 0;
	int Offset = 0;
	int BucketOffset=0; // offset for bucket
	int bktSize = sizeof(Bucket);
	for(Offset =0; Offset< FILESIZE/2;Offset += sizeof(DataItem))
	{
		ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
	//	printf("hena el bucket awl elem feh %d\n", bkt.dataItem[0].data);
		BucketOffset+=bktSize;//increament offset of bucket
		if(result < 0)
		{ 	  perror("some error occurred in pread");
			  return -1;
		} else if (result == 0 || data.valid == 0 ) { //empty space found or end of file
			printf("Bucket: %d, Offset %d:~\n",Offset/bktSize,Offset);
		} else {
			pread(fd,&data,sizeof(DataItem), Offset);
			if(data.overflowPointer==-1)
				printf("Bucket: %d, Offset: %d, Data: %d, key: %d\n",Offset/bktSize,Offset,data.data,data.key);
			else
				printf("Bucket: %d, Offset: %d, Data: %d, key: %d, Points to Overflow offset : %d\n",Offset/bktSize,Offset,data.data,data.key,data.overflowPointer);
			count++;
		}
	}
	
	// overflow area
	printf("starting overflow area \n");
	for(Offset =FILESIZE/2; Offset< FILESIZE;Offset += sizeof(DataItem))
	{
		ssize_t result = pread(fd,&data,sizeof(DataItem), Offset);
		if(result < 0)
		{ 	  perror("some error occurred in pread");
			  return -1;
		} else if (result == 0 || data.valid == 0 ) { //empty space found or end of file
			printf("Offset %d:~\n",Offset);
		} else {
			pread(fd,&data,sizeof(DataItem), Offset);
			printf("Offset: %d, Data: %d, key: %d\n",Offset,data.data,data.key);
					 count++;
		}
	}
	return count;
}


/* Functionality: Delete item at certain offset
 *
 * Input:  fd: filehandler which contains the db
 *         Offset: place where it should delete
 *
 * Hint: you could only set the valid key and write just and integer instead of the whole record
 */
int deleteOffset(int fd, int Offset)
{
	struct DataItem dummyItem;
	dummyItem.valid = 0;
	dummyItem.key = -1;
	dummyItem.data = 0;
	dummyItem.overflowPointer=-1;
	int result = pwrite(fd,&dummyItem,sizeof(DataItem), Offset);
	return result;
}

