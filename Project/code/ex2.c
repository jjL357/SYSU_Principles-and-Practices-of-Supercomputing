#define DEPTH 3//input的通道个数
#define KERNEL_SIZE 3//kernel的大小=3*3*3
#define KERNEL_NUM 3//kernel的个数

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>
#include <sys/time.h>
#include<omp.h>
// input每个通道的大小
int HEIGHT=5;//input的高度
int WEIGHT=5;//input的宽度

int stride=2;//步长

// output每个通道的大小
int output_height=0;//output的高度
int output_weight=0;//output的宽度

int output_depth=0;//output的通道数

int padding=0;//padding的大小
int padding_num=0;//padding填充的数字

int count=0;//记录完成计算的线程个数

void random_input(int ***matrix){//用随机数初始化输入(为方便观察计算结果输出可以直接赋值为1)
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

void show_kernel(int **kernel){//打印kernel
  printf("The kernel:\n");
    for(int i=0;i<KERNEL_NUM;i++){
        printf("NUM:%d  :\n",i+1);
        for(int j=0;j<KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE;j++){
            printf("%d ",kernel[i][j]);
        }
        printf("\n");
    }
  printf("----------------------\n");
}

void random_kernel(int **kernel){//用随机数初始化kernel(为方便观察计算结果输出可以直接赋值为1)
  
    for(int i=0;i<KERNEL_NUM;i++){
        for(int j=0;j<KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE;j++){
            int n=rand()%10;
            kernel[i][j]=1;
        }
    }
}

void show_output(int ***matrix){//打印输出
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

void input_and_kernel(int ***input,int **kernel){//通过输入数字 来初始化Input和kernel(需要手动输入时，在0进程调用此函数，通过进程之间的传递获取kernel和input)
     
     for(int i=0;i<DEPTH;i++){
        printf("Please input the input (%d depth)\n",i+1);
        for(int j=0;j<HEIGHT;j++){
            
            for(int k=0;k<WEIGHT;k++){
                scanf("%d",&input[i][j][k]);
            }
        }
    }

     printf("Please input the kernel\n");
     for(int i=0;i<KERNEL_NUM;i++){
         printf("NUM:%d :\n",i+1);
        for(int j=0;j<KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE;j++){
            scanf("%d",&kernel[i][j]);
        }
    }
    
}

void initialize_output_col2(int**output_col2){//初始化output_col2
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height*output_weight;j++){
            output_col2[i][j]=0;
        }
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

void conv_parallel_col2(int**input_col2,int**kernel,int**output_col2,int comm_sz,int my_rank,int d){//imcol2并行卷积计算
   
    int process_count=comm_sz;//获取进程数

     //将输出output[d][i][j](d取0,1,2)，即将输出的三个通道的同一位置，将所有这样output_height*output_weight个输出位置分给不同的进程计算
    int output_sum=output_height*output_weight;//总共要计算的位置个数
    int average_n=output_sum/process_count;//平均每个获取进程数负责的计算输出位置的个数
    int extra_n=output_sum%process_count;//前extra_n个获取进程数多负责一个位置
    int local_start,local_end;//每个获取进程数负责计算的输出位置的开头和结尾(不包括计算结果)

    if(my_rank<extra_n){//前extra_n个获取进程数多负责一个位置
        local_start=my_rank*average_n+my_rank;
        local_end=local_start+average_n+1;
    }
    else{//其他获取进程数计算average_n个位置
        local_start=my_rank*average_n+extra_n;
        local_end=local_start+average_n;
    }

    
    int*output_1d=(int*)malloc(sizeof(int)*(output_height*output_weight));
    for(int i=0;i<output_height*output_weight;i++)output_1d[i]=0;
  
    #pragma omp parallel for
    for(int i=local_start;i<local_end;i++){//选取计算输出的哪一个通道
        //imcol2计算卷积
        for(int j=0;j<KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE;j++){
            #pragma omp critical
            output_1d[i]+=input_col2[i][j]*kernel[d][j];
        }
    }
    MPI_Reduce(output_1d, output_col2[d],  output_height*output_weight, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );//将结果归约到0进程
    

}

int calculate_padding(){//计算使得输入和输出高度尺寸相同的padding 
    int p=0;
    while((HEIGHT-KERNEL_SIZE+2*p)/stride+1!=HEIGHT){
        p++;
    }
    return p;
}

void transform_input_col2(int***input,int**input_col){//将三维input转为二维input_col2
    for(int i=0;i<output_height;i++){
          for(int j=0;j<output_weight;j++){
            int n=0;
            for(int d=0;d<DEPTH;d++){
            for(int k1=0;k1<KERNEL_SIZE;k1++){
                for(int k2=0;k2<KERNEL_SIZE;k2++,n++){
                    int row=-padding+stride*i+k1;
                    int col=-padding+stride*j+k2;
                    if(row<0||col<0||col>=WEIGHT||row>=HEIGHT)input_col[i*output_weight+j][n]=padding_num;
                    else input_col[i*output_weight+j][n]=input[d][row][col];
                }
            }
            }
          }
        }
}

void transform_output(int**output_col2,int***output){//将二维output_col2转为三维output
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height*output_weight;j++){
            output[i][j/output_weight][j%output_weight]=output_col2[i][j];
        }
    }
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
    int **kernel=construct_two(KERNEL_NUM,KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE);

    //printf("Please input the height and weight of input\n");
    //scanf("%d%d",&HEIGHT,&WEIGHT);

    //输出input和kernel尺寸
    if(my_rank==0)printf("The size of input is %d*%d*%d\n",DEPTH,HEIGHT,WEIGHT);
    if(my_rank==0)printf("The size of kernel is %d*%d*%d*%d\n",KERNEL_NUM,KERNEL_SIZE,KERNEL_SIZE,KERNEL_SIZE);
    
    //初始化input和kernel
    //input_and_kernel(input,kernel);
    random_input(input);
    random_kernel(kernel);
    
    //打印input和kernel
    //if(my_rank==0)show_input(input);
    //if(my_rank==0)show_kernel(kernel);

    //用步长分别为1,2,3计算卷积
    for(stride=1;stride<=3;stride++){
    //计算适应的padding
    padding=calculate_padding();
    if(my_rank==0)printf("The stride is %d,the padding is %d\n",stride,padding);

    //计算output的尺寸
    output_height=(HEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_weight=(WEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_depth=KERNEL_NUM;

    //将三维input转为二维input_col
    int **input_col2=construct_two(output_height*output_weight,KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE);
    transform_input_col2(input,input_col2);

    //创建二维output_col2和三维output
    int **output_col2=construct_two(output_depth,output_height*output_weight);
    int ***output=construct_three(output_depth,output_height,output_weight);
    initialize_output_col2(output_col2);
    
    //输出output尺寸
    if(my_rank==0)printf("The size of output is %d*%d*%d\n", output_depth,output_height,output_weight);
    
    //记录卷积计算开始时间
    struct timeval start,end;
    if(my_rank==0)gettimeofday(&start, NULL );
    
    //并行计算卷积

	   
    for(int i=0;i<3;i++)conv_parallel_col2(input_col2,kernel,output_col2,comm_sz,my_rank,i);

    

    //记录卷积结束时间
    gettimeofday(&end, NULL );
    if(my_rank==0){
      transform_output(output_col2,output);
   long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    //输出计算结果
    show_output(output);

    //计算输出卷积时间
   printf("Costing time=%fs\n",timeuse /1000000.0);
   printf("--------------------------------------------------------------------\n");
    }
    
    
    }

    MPI_Finalize();
    return 0;
    
}
