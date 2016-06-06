#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BOOT_CODE_SKIP 		446
#define SECOND_PARTITION	462
#define THIRD_PARTITION		478
#define FOURTH_PARTITION	494
#define SKIP_TO_SIGNATURE   510

#define CHS_OFFSET		1
#define CHS_LENGTH		3
#define TYPE_OFFSET		4
#define TYPE_LENGTH		1
#define END_CHS_OFFSET		5
#define END_CHS_LENGTH		3
#define START_LBA_OFFSET	8
#define START_LBA_LENGTH	4
#define SIZE_SECTOR_OFFSET	12
#define SIZE_SECTOR_LENGTH	4

char parti_info[30];

long hexPower(unsigned char num,int pow);
long hexConverter(unsigned char* buf,int size);
void partitionInfo(unsigned char type);
void interprete(unsigned char* buf);

int main(int argc, char* argv[])
{
    int fd;
    unsigned char buf[16];
    if (argc != 2) {
        printf("ERROR!USAGE:MBR filename\n"); 
        exit(1);//error occurred.
    }
    if ((fd = open(argv[1],O_RDONLY)) == -1) {
        printf("Failed to open file:%s\n",argv[1]);
        exit(1);
    }
    if ((lseek(fd,SKIP_TO_SIGNATURE,SEEK_SET)) == -1) {
        printf("Error occurred in processing file: %s\n",argv[1]);
        exit(1);
    }

    memset(buf,0,sizeof(buf));

    if(read(fd,buf,2) != 2) {
        printf("Error occurred in processing file: %s\n",argv[1]);
        exit(1);
    }
    if (buf[0] != 0x55 || buf[1] != 0xAA) {
        printf("This file seems have been damaged!\n");
        exit(1);
    }
    if ((lseek(fd,BOOT_CODE_SKIP,SEEK_SET)) == -1) {
        printf("Error occurred in processing file: %s\n",argv[1]);
        exit(1);
    }
    if ((read(fd,buf,16)) != 16) {
        printf("Error occurred in processing file: %s\n",argv[1]);
        exit(1);
    }

    printf("The first partition entry is:\n");

    interprete(buf);
    
    close(fd);
    exit(0);
}

void interprete(unsigned char* buf)
{
    printf("Start CHS Addr:\t%07lx\n",hexConverter(buf+CHS_OFFSET,CHS_LENGTH));
    partitionInfo(buf[TYPE_OFFSET]);
    printf("Partition Type:\t%s",parti_info);
    printf("\n");
    printf("End CHS Addr:\t%07lx\n",hexConverter(buf+END_CHS_OFFSET,END_CHS_LENGTH));
    printf("Start LBA Addr:\t%07lx\n",hexConverter(buf+START_LBA_OFFSET,START_LBA_LENGTH));
    printf("Size in Sector:\t%ld\n",hexConverter(buf+SIZE_SECTOR_OFFSET,SIZE_SECTOR_LENGTH));
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

void partitionInfo(unsigned char type){
    memset(parti_info,0,sizeof(parti_info));
    switch(type) {
    case 0x01:  strcpy(parti_info,"FAT12,CHS");                 break;
    case 0x04:  strcpy(parti_info,"FAT16,16-32,CHS");           break;
    case 0x05:  strcpy(parti_info,"Microsoft Extended, CHS");   break;
    case 0x06:  strcpy(parti_info,"FAT16,32MB-2GB,CHS");        break;
    case 0x07:  strcpy(parti_info,"NTFS");                      break;
    case 0x0b:  strcpy(parti_info,"FAT32,CHS");                 break;
    case 0x0c:  strcpy(parti_info,"FAT32,LBA");                 break;
    case 0x0e:  strcpy(parti_info,"FAT16,32MB-2GB,LBA");        break;
    case 0x0f:  strcpy(parti_info,"Microsoft Extended, LBA");   break;
    case 0x11:  strcpy(parti_info,"Hidden FAT 12,CHS");         break;
    case 0x14:  strcpy(parti_info,"Hidden FAT 16,16-32MB,CHS"); break;
    case 0x16:  strcpy(parti_info,"Hidden FAT 16,32MB-2GB,CHS");break;
    case 0x1b:  strcpy(parti_info,"Hidden FAT32,CHS");          break;
    case 0x1c:  strcpy(parti_info,"Hidden FAT32,LBA");          break;
    case 0x1e:  strcpy(parti_info,"Hidden FAT16,32MB-2GB,LBA"); break;
    case 0x42:  strcpy(parti_info,"Microsoft MBR.Dynamic Disk");break;
    case 0x82:  strcpy(parti_info,"Solaris x86/Linux Swap");    break;
    case 0x83:  strcpy(parti_info,"Linux");                     break;
    case 0x84:  strcpy(parti_info,"Hibernation");               break;
    case 0x85:  strcpy(parti_info,"Linux Extended");            break;
    case 0x86:  
    case 0x87:  strcpy(parti_info,"NTFS Volume Set");           break;
    case 0xa0:
    case 0xa1:  strcpy(parti_info,"Hibernation");               break;
    case 0xa5:  strcpy(parti_info,"FreeBSD");                   break;
    case 0xa6:  strcpy(parti_info,"OpenBSD");                   break;
    case 0xa8:  strcpy(parti_info,"Mac OSX");                   break;
    case 0xa9:  strcpy(parti_info,"NetBSD");                    break;
    case 0xab:  strcpy(parti_info,"Mac OSX Boot");              break;
    case 0xb7:  strcpy(parti_info,"BSDI");                      break;
    case 0xb8:  strcpy(parti_info,"BSDI Swap");                 break;
    case 0xee:  strcpy(parti_info,"EFI GPT Disk");              break;
    case 0xef:  strcpy(parti_info,"EFI System Partition");      break;
    case 0xfb:  strcpy(parti_info,"VMware File System");        break;
    case 0xfc:  strcpy(parti_info,"VMware swap");               break;
    default:    strcpy(parti_info,"Unknown type");              break;
    }
}
