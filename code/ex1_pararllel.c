#define DEPTH 3//输入的通道个数
#define KERNEL_SIZE 3//kernel的大小=3*3*3
#define KERNEL_NUM 3//kernel的个数

#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>
#include <sys/time.h>
#include<mpi.h>
#include<omp.h>

// input每个通道的大小
int HEIGHT=5;//输入的高度
int WEIGHT=5;//输入的宽度

int stride=2;//步长

// output每个通道的大小
int output_height=0;//输出的高度
int output_weight=0;//输出的宽度
int output_depth=0;//输出的通道数

int padding=0;//padding的大小
int padding_num=0;//padding填充的数字

int count=0;//记录完成计算的线程个数

void ini_input(int ***matrix){//用随机数初始化输入(为方便计算输出可以直接赋值为1)
    
    for(int i=0;i<DEPTH;i++){
        for(int j=0;j<HEIGHT;j++){
            for(int k=0;k<WEIGHT;k++){
                int n=rand()%10;
                matrix[i][j][k]=1;
            }
        }
    }
   
}
void show_input(int ***matrix){//打印input
    printf("The input:\n");
    for(int i=0;i<DEPTH;i++){
        printf("The %d depth:\n",i+1);
        for(int j=0;j<HEIGHT;j++){
            for(int k=0;k<WEIGHT;k++){
                printf("%d ",matrix[i][j][k]);
            }
            printf("\n");
        }
    }
    printf("----------------------\n");
}

void show_kernel(int ****kernel){//打印kernel
    printf("The kernel:\n");
    for(int i=0;i<KERNEL_NUM;i++){
        printf("The Num = %d\n",i+1);
        for(int d=0;d<KERNEL_SIZE;d++){
             printf("The Depth = %d\n",d+1);
            for(int h=0;h<KERNEL_SIZE;h++){
                for(int w=0;w<KERNEL_SIZE;w++)printf("%d ",kernel[i][d][h][w]);
                printf("\n");
            }
        }
        printf("-------------\n");
    }
}

void ini_kernel(int ****kernel){//用随机数初始化kernel(为方便观察计算结果输出可以直接赋值为1)
    for(int i=0;i<KERNEL_NUM;i++){
        for(int d=0;d<KERNEL_SIZE;d++){
            for(int h=0;h<KERNEL_SIZE;h++){
                for(int w=0;w<KERNEL_SIZE;w++){
                     int n=rand()%10;
                     kernel[i][d][h][w]=1;
                }
            }
        }
    }
}

void show_output(int ***matrix){//打印output
    printf("The output:\n");
    for(int i=0;i<output_depth;i++){
        printf("The %d depth:\n",i+1);
        for(int j=0;j<output_height;j++){
            for(int k=0;k<output_weight;k++){
                printf("%d ",matrix[i][j][k]);
            }
            printf("\n");
        }
    }
    printf("----------------------\n");
}

void trans_output(int***output,int**output_2d){//将MPI_Reduce的结果转为三维
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height;j++){
            for(int k=0;k<output_weight;k++){
                output[i][j][k]=output_2d[i][j*output_weight+k];
            }
        }
    }
}

void conv_parallel(int ***input,int ****kernel,int ***output,int comm_sz,int my_rank,int**output_2d,int n){//并行进行卷积运算

    int process_count=comm_sz;//获取进程数
    
    //将输出output[d][i][j](d取0,1,2)，即将输出的三个通道的同一位置，将所有这样output_height*output_weight个输出位置分给不同的进程计算
    int output_sum=output_height*output_weight;//总共要计算的位置个数
    int average_n=output_sum/process_count;//平均每个取进负责的计算输出位置的个数
    int extra_n=output_sum%process_count;//前extra_n个取进多负责一个位置
    int local_start,local_end;//每个取进负责计算的输出位置的开头和结尾(不包括计算结果)

    if(my_rank<extra_n){//前extra_n个取进多负责一个位置
        local_start=my_rank*average_n+my_rank;
        local_end=local_start+average_n+1;
    }
    else{//其他取进计算average_n个位置
        local_start=my_rank*average_n+extra_n;
        local_end=local_start+average_n;
    }
    
    //存储计算结果
    int*output_1d=(int*)malloc(sizeof(int)*(output_height*output_weight));
    for(int i=0;i<output_height*output_weight;i++)output_1d[i]=0;
  
		#pragma omp parallel for
        for(int d=0;d<output_depth;d++){//选取计算输出的哪一个通道
            for(int i=local_start;i<local_end;i++){//选取该进程负责计算的位置
                int row=i/output_weight;//计算位置的行下标
                int col=i%output_weight;//计算位置的列坐标
                for(int k1=0;k1<KERNEL_SIZE;k1++){
                    for(int k2=0;k2<KERNEL_SIZE;k2++){
                        int input_row=-padding+stride*row+k1;//对应输入位置行下标
                        int input_col=-padding+stride*col+k2;//对应输入位置列下标
                       if(input_row>=HEIGHT||input_col>=WEIGHT||input_row<0||input_col<0)output_1d[i]+=padding_num*kernel[n][d][k1][k2];//对应输入位置位于padding填充的位置

                        #pragma omp critical
                        else  output_1d[i] += input[d][input_row][input_col]*kernel[n][d][k1][k2];
                        
                    }
                }
            }
        }
        //将计算结果归约到0进程中
        MPI_Reduce(output_1d, output_2d[n],  output_height*output_weight, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    
}

void initialize_output_col2(int**output_col2){//初始化output_col2
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height*output_weight;j++){
            output_col2[i][j]=0;
        }
    }
}

void input_and_kernel(int ***input,int ****kernel){//通过输入数字 来初始化Input和kernel(需要手动输入时，在0进程调用此函数，通过进程之间的传递获取kernel和input)
     for(int i=0;i<DEPTH;i++){
        printf("Please input the input (%d depth)\n",i+1);
        for(int j=0;j<HEIGHT;j++){
            for(int k=0;k<WEIGHT;k++){
                scanf("%d",&input[i][j][k]);
            }
        }
    }
    
     for(int i=0;i<KERNEL_NUM;i++){
         printf("The Num = %d\n",i+1);
        for(int d=0;d<KERNEL_SIZE;d++){
            printf("The Depth = %d\n",d+1);
            for(int h=0;h<KERNEL_SIZE;h++){
                for(int w=0;w<KERNEL_SIZE;w++){
                    scanf("%d",& kernel[i][d][h][w]);
                }
            }
        }
        printf("----------------------\n");
    }
    
}

void initialize_output(int ***output){//初始化output
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height;j++){
            for(int k=0;k<output_weight;k++)output[i][j][k]=0;
        }
    }
}

int **construct_two(int height,int weight){//创建二维数组
    int**matrix=(int**)malloc(sizeof(int*)*height);
    for(int i=0;i<height;i++){
        matrix[i]=(int*)malloc(sizeof(int)*weight);
    }
    return matrix;
}

int ***construct_three(int depth,int height,int weight){//创建三维数组
    int***matrix=(int***)malloc(sizeof(int**)*depth);
    for(int i=0;i<depth;i++){
        matrix[i]=(int**)malloc(sizeof(int*)*height);
    }
    for(int i=0;i<depth;i++){
        for(int j=0;j<height;j++){
            matrix[i][j]=(int*)malloc(sizeof(int)*weight);
        }   
    }
    return matrix;
}

int ****construct_four(int num,int depth,int height,int weight){//创建四维数组
    int****matrix=(int****)malloc(sizeof(int***)*num);
    for(int i=0;i<num;i++){
        matrix[i]=(int***)malloc(sizeof(int**)*depth);
    }
    for(int i=0;i<num;i++){
        for(int j=0;j<depth;j++){
            matrix[i][j]=(int**)malloc(sizeof(int*)*height);
        }   
    }
    for(int i=0;i<num;i++){
        for(int j=0;j<depth;j++){
            for(int k=0;k<height;k++){
                 matrix[i][j][k]=(int*)malloc(sizeof(int)*height);
            }
        }   
    }
    return matrix;
}

int calculate_padding(){//计算使得输入和输出高度尺寸相同的padding 
    int p=0;
    while((HEIGHT-KERNEL_SIZE+2*p)/stride+1!=HEIGHT){
        p++;
    }
    return p;
}

int main(){
    int comm_sz;
    int my_rank;
	MPI_Init(NULL, NULL);
  	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   //获取当前进程的标号
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);   //获取进程数量

    srand( (unsigned)time( NULL ) );

    //创建input和kernel数组
    int ***input=construct_three(DEPTH,HEIGHT,WEIGHT);
    int ****kernel=construct_four(KERNEL_NUM,KERNEL_SIZE,KERNEL_SIZE,KERNEL_SIZE);
    
    //printf("Please input the height and weight of input\n");
    //scanf("%d%d",&HEIGHT,&WEIGHT);

    //初始化input和kernel
    //input_and_kernel(input,kernel);
    ini_input(input);
    ini_kernel(kernel);

    
    if(my_rank==0){
    //0进程输出input和kernel尺寸
    printf("The size of input is %d*%d*%d\n",DEPTH,HEIGHT,WEIGHT);
    printf("The size of kernel is %d*%d*%d*%d\n",KERNEL_NUM,KERNEL_SIZE,KERNEL_SIZE,KERNEL_SIZE);
    //0进程打印input和kernel
    //show_input(input);
    //show_kernel(kernel);
    }
    

    //用步长分别为1,2,3计算卷积
    for(stride=1;stride<=3;stride++){
    //计算适应的padding
    padding=calculate_padding();
    output_height=(HEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_weight=(WEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_depth=KERNEL_NUM;
    
   
    //计算output的尺寸 stride padding
    if(my_rank==0)printf("The stride is %d,the padding is %d\n",stride,padding);
    if(my_rank==0)printf("The size of output is %d*%d*%d\n", output_depth,output_height,output_weight);

    //创建初始化output output_2d用来接收归约结果
    int**output_2d=construct_two(output_depth,output_height*output_weight);
    int ***output=construct_three(output_depth,output_height,output_weight);
    initialize_output(output);
    initialize_output_col2(output_2d);
    
    //记录卷积计算开始时间
    struct timeval start,end;
    if(my_rank==0)gettimeofday(&start, NULL );
    
    //并行计算卷积
 	for(int i=1;i<=3;i++)conv_parallel(input,kernel,output,comm_sz,my_rank,output_2d,i-1);
	
    //记录卷积结束时间
    if(my_rank==0){
    gettimeofday(&end, NULL );
    //计算输出卷积时间
    long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("Costing time=%fs\n",timeuse /1000000.0);
    printf("------------------------\n");
    trans_output(output,output_2d);
    show_output(output);
    }

    }
	    
   MPI_Finalize();//释放进程资源
    return 0;    
}
