#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
//vbr offsets
#define SKIP_TO_SIGNATURE   510
#define OEM_IN_ASCII        3
#define OEM_IN_ASCII_LEN    8
#define BYTE_PER_SECTOR    11
#define BYTE_PER_SECTOR_LEN    2
#define SECTOR_PER_CLUSTOR  13
#define RESERVED_SECTOR     14
#define RESERVED_SECTOR_LEN 2
#define FATS_NUMBER         16
#define FATS_SECTOR         36
#define FATS_SECTOR_LEN     4
#define ROOT_CLUSTOR        44
#define ROOT_CLUSTOR_LEN    4
//directory offsets
#define STATUS              0
#define FILE_NAME           1
#define FILE_NAME_LEN       10
#define FILE_ATTRIBUTES     11
#define CREATE_TIME         14//hours,minutes,seconds
#define CREATE_TIME_LEN     3
#define CREATE_DAY          16
#define CREATE_DAY_LEN      2
#define ACCESS_DAY          18
#define ACCESS_DAY_LEN      2
#define HIGH_2_BYTE         20
#define WRITTEN_TIME        22
#define WRITTEN_TIME_LEN    2
#define WRITTEN_DAY         24
#define WRITTEN_DAY_LEN     2
#define LOWER_2_BYTE        26
#define SIZE_OF_FILE        28
#define SIZE_OF_FILE_LEN    4

static long byte_p_sector,sector_p_cluster,reserved,fats_num,fats_sector,root_cluster;

long hexPower(unsigned char num,int pow);
long hexConverter(unsigned char* buf,int size);
void interprete(unsigned char* buf);
void printText(unsigned char* buf, int len);
void directory(unsigned char* buf);
void timeConverter(unsigned char* buf);
void dateConverter(unsigned char* buf);


int main(int argc, char* argv[])
{
    int fd;
    int i, total_sectors = 0;
    unsigned char buf[512];
    if (argc != 2) {
        printf("ERROR!USAGE:VBR filename\n"); 
        exit(1);//error occurred.
    }
    if ((fd = open(argv[1],O_RDONLY)) == -1) {
        printf("Failed to open file:%s\n",argv[1]);
        exit(1);
    }
    memset(buf,0,sizeof(buf));

    if(read(fd,buf,512) != 512) {
        printf("Error occurred in processing file: %s\n",argv[1]);
        exit(1);
    }
    if (buf[510] != 0x55 || buf[511] != 0xAA) {
        printf("This file seems have been damaged!\n");
        exit(1);
    }

    interprete(buf);
    total_sectors = reserved + fats_num*fats_sector;

    //skip the reserved area and FATS to the root cluster
    for (i = 0; i < total_sectors; i++) {
        if (lseek(fd,byte_p_sector,SEEK_CUR) == -1) {
            printf("Error occurred in processing file: %s\n",argv[1]);
            exit(1);
         }
    }

    if (read(fd,buf,32) != 32) {
        printf("Error occurred in processing file: %s\n",argv[1]);
        exit(1);
    }

//    directory(buf);

    close(fd);
    exit(0);
}

void interprete(unsigned char* buf)
{
    printf("OEM NAME:\t\t");
    printText(buf+OEM_IN_ASCII,OEM_IN_ASCII_LEN);
    printf("\n");
    byte_p_sector = hexConverter(buf+BYTE_PER_SECTOR,BYTE_PER_SECTOR_LEN);
    printf("Bytes per sector:\t%ld\n",byte_p_sector);
    sector_p_cluster = hexConverter(buf+SECTOR_PER_CLUSTOR,1);
    printf("Sector per cluster:\t%ld\n",sector_p_cluster);
    reserved = hexConverter(buf+RESERVED_SECTOR,RESERVED_SECTOR_LEN);
    printf("Reserved area sectors:\t%ld\n",reserved);
    fats_num = hexConverter(buf+FATS_NUMBER,1);
    printf("Number of FATS:\t\t%ld\n",fats_num);
    fats_sector = hexConverter(buf+FATS_SECTOR,FATS_SECTOR_LEN);
    printf("Sectors of FAT:\t\t%ld\n",fats_sector);
    root_cluster = hexConverter(buf+ROOT_CLUSTOR,ROOT_CLUSTOR_LEN);
    printf("Root cluster:\t\t%ld\n",root_cluster);
}

void directory(unsigned char* buf) 
{
    if (*buf == 0xe5 || *buf == 0) {
        printf("Not allocated!\n");
        return;
    }

    printf("NAME:\t");
    printText(buf+FILE_NAME,FILE_NAME_LEN);
    printf("\t");

    switch(buf[FILE_ATTRIBUTES]) {
        case 0x01: printf("Read only\n");          break;
        case 0x02: printf("Hidden File\n");        break;
        case 0x04: printf("System File\n");        break;
        case 0x08: printf("Volume label\n");       break;
        case 0x0f: printf("long file name\n");     break;
        case 0x10: printf("Directory\n");          break;
        case 0x20: printf("Archive\n");            break;
        default: printf("Unknown type!\n");        break;
    }
    
    printf("\tCreate time:\t");
    timeConverter(buf+CREATE_TIME);
    printf("\tCreate day:\t");
    dateConverter(buf+CREATE_DAY);
    printf("\tFile size:\t");
    printf("%ld",hexConverter(buf+SIZE_OF_FILE,SIZE_OF_FILE_LEN));
    
}
long hexConverter(unsigned char* buf,int size)
{
    long result = 0;
    int i;
    for (i = 0; i < size; i++) 
        result += hexPower(buf[i],i);
    return result;
}

long hexPower(unsigned char num, int pow)
{
    long result = num;
    int i;
    long we = 16*16;
    for (i = 0; i < pow; i++)
        result *= we;
    return result;
}

void printText(unsigned char* buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
        printf("%c",buf[i]);
}

void dateConverter(unsigned char*buf)
{
    long time;
    int year,month,day;
    time = hexConverter(buf,2);
    year = time >> 9;
    month = (time - (year<<9)) >> 5;
    day = time - (year<<9) - (month<<5);
    printf("%2d-%2d-%4d",month,day,year);
}

void timeConverter(unsigned char*buf)
{
    long time;
    int hour,minute,second;
    time = hexConverter(buf,2);
    hour = time >> 11;
    minute = (time-(hour<<11))>>5;
    second = time - (hour<<11) - (minute<<5);
    printf("%2d:%2d:%2d",hour,minute,second);
}

