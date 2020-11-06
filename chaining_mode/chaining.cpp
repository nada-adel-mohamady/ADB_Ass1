#include "readfile.h"

/* Hash function to choose bucket
 * Input: key used to calculate the hash
 * Output: HashValue;
 */
int hashCode(int key){
   return key % MBUCKETS;
}

int insertItem(int fd,DataItem item){
   //TODO: implement this function
   // use the hash function to determine which bucket to insert into 
   int hashIndex = hashCode(item.key);
   struct Bucket bkt;   //a variable to read in it the records from the db
   //check if collision happen 
    int saveOffset;
	int count = 0;
    int startingOffset = hashIndex*BUCKETSIZE;		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db
    int records_counter = 0; //  count records within one bucket
	//Main Loop
	RESEEK:
	//on the linux terminal use man pread to check the function manual
	ssize_t result = pread(fd,&bkt,sizeof(Bucket), Offset);
	saveOffset=Offset;
	//one record accessed
	 count++;
	//check whether it is a valid record or not
	for(int i=0;i<RECORDSPERBUCKET;i++){
		if(bkt.dataItem[i].valid==0 || result==0) //empty space
			{ 	
			// empty record 
			// insert into it
				bkt.dataItem[i] = item;// no overflow ocurred 
				bkt.overflowPointer=-1;
				pwrite(fd,&bkt,sizeof(Bucket), Offset);	
			
			return count;
			}
			count++;
	}
   
	// bucket is full
	// apply chaining 	 	
			int startOffsetOverflow = MBUCKETS*BUCKETSIZE;
			for(int offset=startOffsetOverflow;offset<FILESIZE;offset += sizeof(Bucket)){
				struct Bucket tmp;
				ssize_t result = pread(fd,&tmp,sizeof(Bucket), offset);
				for(int i=0;i<RECORDSPERBUCKET;i++){
					count++;
					if(result==0||tmp.dataItem[i].valid==0){// unused overflow area
						pwrite(fd,&item,sizeof(item), offset+i*sizeof(DataItem));
						bkt.overflowPointer=offset;
						pwrite(fd,&bkt,sizeof(Bucket), saveOffset);
						return count;
					}
				}
			
			}
			return count;	
    	goto RESEEK;
    }
	


int searchItem(int fd,struct DataItem* item,int *count)
{

	//Definitions
	struct DataItem data;   //a variable to read in it the records from the db
	struct Bucket bkt; // a variable to read in it the bucket from the database 
	*count = 0;				//No of accessed records
	int rewind = 0;			//A flag to start searching from the first bucket
	int hashIndex = hashCode(item->key);  				//calculate the Bucket index
	int startingOffset = hashIndex*BUCKETSIZE;		//calculate the starting address of the bucket
	int Offset = startingOffset;						//Offset variable which we will use to iterate on the db
	//Main Loop
	RESEEK:
	//on the linux terminal use man pread to check the function manual
	ssize_t result = pread(fd,&bkt,sizeof(Bucket), Offset);
	//one record accessed
	(*count)++;
	//check whether it is a valid record or not
    if(result <= 0) //either an error happened in the pread or it hit an unallocated space
	{ 	 // perror("some error occurred in pread");
		  return -1;
    }
	for(int i=0;i<RECORDSPERBUCKET;i++){
		if (bkt.dataItem[i].valid == 1 && bkt.dataItem[i].key == item->key) {
			// found needed record
			item->data = bkt.dataItem[i].data;
			return Offset+i*sizeof(DataItem);
		}
		(*count)++;
	}
     //not the record I am looking for
	Offset +=sizeof(DataItem);  //move the offset to next record
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


/* Functionality: Display all the file contents
 *
 * Input:  fd: filehandler which contains the db
 *
 * Output: no. of non-empty records
 */
int DisplayFile(int fd){

	struct Bucket bkt;
	int count = 0;
	int Offset = 0;
	int BucketOffset=0; // offset for bucket
	int bktSize = sizeof(Bucket);
	int dataSize = sizeof(DataItem);
	for(Offset =0; Offset< FILESIZE/2;Offset += sizeof(Bucket))
	{
		ssize_t result = pread(fd,&bkt,sizeof(Bucket), Offset);
	//	printf("hena el bucket awl elem feh %d\n", bkt.dataItem[0].data);
		BucketOffset+=bktSize;//increament offset of bucket
		for(int i=0;i<RECORDSPERBUCKET;i++){
				if(result < 0)
				{ 	  perror("some error occurred in pread");
					  return -1;
				} else if (result == 0 || bkt.dataItem[i].valid == 0 ) { //empty space found or end of file
					printf("Bucket: %d, Offset %d:~\n",Offset/bktSize,Offset+i*dataSize);
				} else {
					pread(fd,&bkt,sizeof(Bucket), Offset);					
					printf("Bucket: %d, Offset: %d, Data: %d, key: %d \n",Offset/bktSize,Offset+i*dataSize, bkt.dataItem[i].data, bkt.dataItem[i].key);
					count++;
				}
		}
		
		if(bkt.dataItem[1].valid==1 && bkt.overflowPointer!=-1){
			printf("Bucket: %d points to overflow location at %d\n",Offset/bktSize, bkt.overflowPointer);
		}
		
	}
	
	// overflow area
	printf("starting overflow area \n");
	for(Offset =FILESIZE/2; Offset< FILESIZE;Offset += sizeof(Bucket))
	{
		ssize_t result = pread(fd,&bkt,sizeof(Bucket), Offset);
		for(int i=0;i<RECORDSPERBUCKET;i++){
			if(result < 0)
			{ 	  perror("some error occurred in pread");
				  return -1;
			} else if (result == 0 || bkt.dataItem[i].valid == 0 ) { //empty space found or end of file
				printf("Offset %d:~\n",Offset+i*dataSize);
			} else {
				
				printf("Offset: %d, Data: %d, key: %d\n",Offset+i*dataSize,bkt.dataItem[i].data,bkt.dataItem[i].key);
						 count++;
			}
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
	//dummyItem.overflowPointer=-1;
	int result = pwrite(fd,&dummyItem,sizeof(DataItem), Offset);
	return result;
}

