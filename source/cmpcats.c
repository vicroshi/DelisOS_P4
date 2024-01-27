#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <fcntl.h>
#include <unistd.h>
typedef struct file_info{
    char *name;
    char *path;
    char *path_inside_dir;
    struct stat file_stats;
} file_info;

int filter(const struct dirent *entry) {
    // Return 1 to include the entry, 0 to exclude it

    // Example: Include regular files and directories, excluding hidden files
    if ( (strcmp(entry->d_name,".")==0 )|| (strcmp(entry->d_name,"..")==0)  )
        return 0;  // Exclude hidden files
    
    return 1;  // Exclude other types of entries
}


char * get_path_inside_hierarchy(char *p){
    int index,i;
    for(index=0;index<strlen(p);index++) if(p[index]=='/') break; //ftasame sto prwto '/',kratame mono to path meta apo auto gia na ftiaksoume to monopati entos ierarxias
    int size=(strlen(p)-index);
    char *copy=malloc((size+1)*sizeof(char));
    if(copy==NULL){
        perror("malloc()");
        exit(1);
    }
    copy[size-1]=0; //kleinoume to string mas
    int j=0;
    for(i=index+1;i<strlen(p);i++) copy[j++]=p[i];
    return copy;

}

file_info *create_file_info_array(int n,struct dirent** namelist ,char *path){
    file_info *a1;
    a1 = malloc(n * sizeof(file_info));
    if (a1 == NULL){
        perror("malloc()");
        exit(1);
    }
    for (int i = 0; i < n; i++){
        a1[i].name = malloc((strlen(namelist[i]->d_name) + 1) * sizeof(char));
        if (a1[i].name == NULL){
            perror("malloc()");
            exit(1);
        }
        strcpy(a1[i].name, namelist[i]->d_name);
        a1[i].path = malloc((strlen(path) + strlen(a1[i].name) + 2) * sizeof(char)); //+2 gia na baloume to '/' kai to terminating character
        if (a1[i].path == NULL){
            perror("malloc()");
            exit(1);
        }
        strcpy(a1[i].path, path); // ftiaxnoume to path gia to file
        strcat(a1[i].path, "/");
        strcat(a1[i].path, a1[i].name);
        if (stat(a1[i].path, &a1[i].file_stats) == -1){
            perror("stat()");
            exit(1);
        }
        a1[i].path_inside_dir=get_path_inside_hierarchy(a1[i].path);
        free(namelist[i]); //to kanoume free afou pleon oses plirofories thelame tis antigrapsame
    }
    free(namelist);
    return a1;
}


int files_have_same_contents(char *pathA,char*pathB,size_t bytes){
    char a,b;
    int fd1,fd2;
    //printf("pathA:[%s],pathB:[%s]\n",pathA,pathB);
    fd1=open(pathA,O_RDONLY);
    if(fd1==-1){
        perror("open()");
        exit(1);
    }
    fd2=open(pathB,O_RDONLY);
    if(fd2==-1){
        perror("open()");
        exit(1);
    }
    for(size_t i=0; i<bytes;i++){
        if(read(fd1,&a,1)==-1){
            perror("read()");
            exit(1);
        }
        if(read(fd2,&b,1)==-1){
            perror("read()");
            exit(1);
        }
        if(a!=b){//brikame byte to opoio den einai idio, ara ta arxeia den exoun idia contents 
            close(fd1);
            close(fd2);
            return -1;
        }
    }
    close(fd1);
    close(fd2);
    return 1;
}



void compare_files(file_info *a1,int size1,file_info *a2,int size2,char ***array,int *size){//dwsmenwn duo arrays me ta files kathe dir, elegxoume poia apo to ena array den uparxoun sto allo
    (*array)=NULL; //arxika den uparxei kapoio koino dir.
    *size=0;
    //sygkrisi tou dirA me to dirB gia mi koina stoixeia
    for(int i=0;i<size1;i++){
        int found=0;
        for(int j=0;j<size2;j++){
            //sygkrisi dirs
            if( ((a1[i].file_stats.st_mode & S_IFMT)==S_IFDIR) && ((a2[j].file_stats.st_mode & S_IFMT)==S_IFDIR) && (strcmp(a1[i].name,a2[j].name)==0) && (strcmp(a1[i].path_inside_dir,a2[j].path_inside_dir)==0) )  {
                found=1;
                if(*(size)==0){ //twra dimiourgoume ton pinaka
                    (*array)=malloc(1*sizeof(char *));
                    if(*array==NULL){
                        perror("malloc()");
                        exit(1);
                    }
                }
                else{ //exei idi kapoio stixeio mesa, kanoume realloc
                    *(array)=realloc(*array,(*size+1)*sizeof(char*));
                    if(*array==NULL){
                        perror("malloc()");
                        exit(1);
                    }
                }
                (*size)++;
                (*array)[*size-1]=malloc((strlen(a1[i].name)+1)*sizeof(char));
                if((*array)[*size-1]==NULL){
                   perror("malloc()");
                    exit(1);
                }
                strcpy((*array)[*size-1],a1[i].name); //kratame to name tou koinou dir
                break; //brikame oti uparxei, de synexizoume tin anazitisi
            }
            //sygkrisi gia regular files
            else if (((a1[i].file_stats.st_mode & S_IFMT)==S_IFREG) && ((a2[j].file_stats.st_mode & S_IFMT)==S_IFREG) && (strcmp(a1[i].name,a2[j].name)==0) && (strcmp(a1[i].path_inside_dir,a2[j].path_inside_dir)==0) && (a1[i].file_stats.st_size==a2[j].file_stats.st_size) && (files_have_same_contents(a1[i].path,a2[j].path,a1[i].file_stats.st_size)==1) ){
                found=1;
                break; //brikame oti uparxei, de synexizoume tin anazitisi
            }
            //gia links
            //else if()
        }
        if(found==0) printf("%s\n",a1[i].path_inside_dir);
    }
    //sygkrisi tou dirB me to dirA gia mi koina stoixeia
    for(int j=0;j<size2;j++){
        int found=0;
        for(int i=0;i<size1;i++){
            //sygkrisi dirs
            if( ((a1[i].file_stats.st_mode & S_IFMT)==S_IFDIR) && ((a2[j].file_stats.st_mode & S_IFMT)==S_IFDIR) && (strcmp(a1[i].name,a2[j].name)==0) && (strcmp(a1[i].path_inside_dir,a2[j].path_inside_dir)==0) )  {
                found=1;
                break; //brikame oti uparxei, de synexizoume tin anazitisi
            }
            //sygkrisi gia regular files
            else if (((a1[i].file_stats.st_mode & S_IFMT)==S_IFREG) && ((a2[j].file_stats.st_mode & S_IFMT)==S_IFREG) && (strcmp(a1[i].name,a2[j].name)==0) && (strcmp(a1[i].path_inside_dir,a2[j].path_inside_dir)==0) && (a1[i].file_stats.st_size==a2[j].file_stats.st_size) && (files_have_same_contents(a1[i].path,a2[j].path,a1[i].file_stats.st_size)==1) ){
                found=1;
                break; //brikame oti uparxei, de synexizoume tin anazitisi
            }
            //gia links
            //else if()
        }
        if(found==0) printf("%s\n",a2[j].path_inside_dir);
    }
}

//edw sygkrinoume ta stoixeia tou kathe path kai ta dirs pou briskontai mono se ena apo ta duo dirs epistrefontai gia na mpoume anadromika ekei na psaksoume gia stoixeia
char **find_unique_dirs(file_info *a1,int size1, file_info *a2,int size2,int *size){
    char ** unique_dirs;
    for(int i=0;i<size1;i++){
        int found=0;
        for(int j=0;j<size2;j++){
            if( ((a1[i].file_stats.st_mode & S_IFMT)==S_IFDIR) && ((a2[j].file_stats.st_mode & S_IFMT)==S_IFDIR) && (strcmp(a1[i].name,a2[j].name)==0) && (strcmp(a1[i].path_inside_dir,a2[j].path_inside_dir)==0) )  {
                found=1;
                break; //brikame oti uparxei kai sta duo arrays, de synexizoume tin anazitisi
            }
        }
        if(!found && ((a1[i].file_stats.st_mode & S_IFMT)==S_IFDIR)){//de brethike, einai monadiko sto dirA
            if((*size==0)){ //den exei alla stoixeia o pinakas, ton dimiourgoume twra
                unique_dirs=malloc(1*sizeof(char *));
                if(unique_dirs==NULL){
                    perror("malloc()");
                    exit(1);
                }
                unique_dirs[0]=malloc((strlen(a1[i].name)+1)*sizeof(char));
                if(unique_dirs[0]==NULL){
                    perror("malloc()");
                    exit(1);
                }
                strcpy(unique_dirs[0],a1[i].name);
            }
            else{
                unique_dirs=realloc(unique_dirs,((*size)+1)*sizeof(char*));
                if(unique_dirs==NULL){
                    perror("malloc()");
                    exit(1);
                }
                unique_dirs[(*size)]=malloc((strlen(a1[i].name)+1)*sizeof(char));
                if(unique_dirs[(*size)]==NULL){
                    perror("malloc()");
                    exit(1);
                }
                strcpy(unique_dirs[(*size)],a1[i].name);
            }
            (*size)++;
        }
    }
    return unique_dirs;
}



void recursive_traverse(char *pA,char *pB){
    struct dirent **namelistA,**namelistB;
    int n1=0,n2=0;
    char **common_dir_array;
    int size=0;
    char *new_pathA;
    file_info *a1,*a2; //pinakes pou kratame ta stoixeia kathe file pou briskoume stous fakelous
    if(pA!=NULL){
        n1=scandir(pA,&namelistA,filter,alphasort);
        if(n1==-1){
            perror("scnadir()");
            exit(1);
        }
        if(n1!=0){ //antigrafi olwn twn stoixeiwn kathe file ston pinaka a1
            a1=create_file_info_array(n1,namelistA,pA);
            for(int i=0;i<n1;i++){
                //printf("found file with name:%s, path:%s\n", a1[i].name, a1[i].path);
            }
        }
    }
    //printf("---\n");
    if(pB!=NULL){
        n2=scandir(pB,&namelistB,filter,alphasort);
        if(n2==-1){
            perror("scandir()");
            exit(1);
        }
        if(n2!=0){ //antigrafi olwn twn stoixeiwn kathe file ston pinaka a1
            a2=create_file_info_array(n2,namelistB,pB);
            for(int i=0;i<n2;i++){
                //printf("found file with name:%s, path:%s\n", a2[i].name, a2[i].path);

            }
        }
    }
    if(pA==NULL && pB!=NULL){//eimaste se dir tou dirB pou den uparxei sto dirA, ara ektypwnoume ola ta entries
        for(int i=0;i<n2;i++) printf("%s is NOT in %s.\n",a2[i].path_inside_dir,pA);
    }
    else if(pA!=NULL && pB==NULL){//eimaste se dir tou dirA pou den uparxei sto dirB,etkypwnoume ola ta entries
        for(int i=0;i<n1;i++) printf("%s is NOT in %s.\n",a1[i].path_inside_dir,pB);
    }
    else if(pA!=NULL && pB!=NULL){
        
        compare_files(a1,n1,a2,n2,&common_dir_array,&size);
        //for(int i=0;i<size;i++) printf("common dirs:%s\n",common_dir_array[i]);
        
        
        
        //ftiaxnoume ta paths gia ta koina monopatia gia na mpoume anadromika se auta
        for(int i=0;i<size;i++){
            //make common dirs
            new_pathA=malloc((strlen(pA)+strlen(common_dir_array[i])+2)*sizeof(char));
            if(new_pathA==NULL){
                perror("malloc()");
                exit(1);
            }
            strcpy(new_pathA,pA);
            strcat(new_pathA,"/");
            strcat(new_pathA,common_dir_array[i]);
            char *new_pathB=malloc((strlen(pB)+strlen(common_dir_array[i])+2)*sizeof(char));
            if(new_pathB==NULL){
                perror("malloc()");
                exit(1);
            }
            strcpy(new_pathB,pB);
            strcat(new_pathB,"/");
            strcat(new_pathB,common_dir_array[i]);
           // printf("mpainw sto koino dir:%s\n",common_dir_array[i]);
            recursive_traverse(new_pathA,new_pathB);

            //gyrisame apo tin anadromi, kanoume free tous porous
            free(new_pathA);
            free(new_pathB);
        }
        
    }
    if(pA!=NULL){
        int unique_sizeA=0;
        char** u_dA=find_unique_dirs(a1,n1,a2,n2,&unique_sizeA);
        for(int i=0;i<unique_sizeA;i++){
           // printf("unique dirs in %s:%s\n",pA,u_dA[i]);
            //ftiaxnw to path gia to neo dir
            new_pathA=malloc((strlen(pA)+strlen(u_dA[i])+2)*sizeof(char));
            if(new_pathA==NULL){
                perror("malloc()");
                exit(1);
            }
            strcpy(new_pathA,pA);
            strcat(new_pathA,"/");
            strcat(new_pathA,u_dA[i]);
            recursive_traverse(new_pathA,NULL); 
            free(new_pathA);
            free(u_dA[i]);
        }
       
        if(unique_sizeA!=0) free(u_dA);
    }
    if(pB!=NULL){
        int unique_sizeB=0;
        char** u_dB=find_unique_dirs(a2,n2,a1,n1,&unique_sizeB);
        for(int i=0;i<unique_sizeB;i++){
            //printf("unique dirs in %s:%s\n",pB,u_dB[i]);
            //ftiaxnw to path gia to neo dir
            char *new_pathB=malloc((strlen(pB)+strlen(u_dB[i])+2)*sizeof(char));
            if(new_pathB==NULL){
                perror("malloc()");
                exit(1);
            }
            strcpy(new_pathB,pB);
            strcat(new_pathB,"/");
            strcat(new_pathB,u_dB[i]);
            recursive_traverse(new_pathB,NULL); 
            free(new_pathB);
            free(u_dB[i]);
        }
       
        if(unique_sizeB!=0) free(u_dB);
    }











    //teleiwsame, kanoume free osous porous desmeysame
    for(int i=0;i<size;i++) free(common_dir_array[i]);
    if(size!=0)  free(common_dir_array);
    for(int i=0;i<n1;i++){
        free(a1[i].name);
        free(a1[i].path);
        free(a1[i].path_inside_dir);
    }
    if(n1!=0) free(a1);
    for(int i=0;i<n2;i++){
        free(a2[i].name);
        free(a2[i].path);
        free(a2[i].path_inside_dir);
    }
    if(n2!=0) free(a2);
}

int main() {
    recursive_traverse("dirB","dirA");
    return 0;
}