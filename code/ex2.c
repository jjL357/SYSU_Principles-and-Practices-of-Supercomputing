#define DEPTH 3//input��ͨ������
#define KERNEL_SIZE 3//kernel�Ĵ�С=3*3*3
#define KERNEL_NUM 3//kernel�ĸ���

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>
#include <sys/time.h>
#include<omp.h>
// inputÿ��ͨ���Ĵ�С
int HEIGHT=5;//input�ĸ߶�
int WEIGHT=5;//input�Ŀ��

int stride=2;//����

// outputÿ��ͨ���Ĵ�С
int output_height=0;//output�ĸ߶�
int output_weight=0;//output�Ŀ��

int output_depth=0;//output��ͨ����

int padding=0;//padding�Ĵ�С
int padding_num=0;//padding��������

int count=0;//��¼��ɼ�����̸߳���

void random_input(int ***matrix){//���������ʼ������(Ϊ����۲�������������ֱ�Ӹ�ֵΪ1)
    for(int i=0;i<DEPTH;i++){
        for(int j=0;j<HEIGHT;j++){
            for(int k=0;k<WEIGHT;k++){
                int n=rand()%10;
                matrix[i][j][k]=1;
            }
        }
    }
   
}

void show_input(int ***matrix){//��ӡinput
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

void show_kernel(int **kernel){//��ӡkernel
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

void random_kernel(int **kernel){//���������ʼ��kernel(Ϊ����۲�������������ֱ�Ӹ�ֵΪ1)
  
    for(int i=0;i<KERNEL_NUM;i++){
        for(int j=0;j<KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE;j++){
            int n=rand()%10;
            kernel[i][j]=1;
        }
    }
}

void show_output(int ***matrix){//��ӡ���
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

void input_and_kernel(int ***input,int **kernel){//ͨ���������� ����ʼ��Input��kernel(��Ҫ�ֶ�����ʱ����0���̵��ô˺�����ͨ������֮��Ĵ��ݻ�ȡkernel��input)
     
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

void initialize_output_col2(int**output_col2){//��ʼ��output_col2
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height*output_weight;j++){
            output_col2[i][j]=0;
        }
    }
}

void initialize_output(int ***output){//��ʼ��output
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height;j++){
            for(int k=0;k<output_weight;k++)output[i][j][k]=0;
        }
    }
}

int **construct_two(int height,int weight){//������ά����
    int**matrix=(int**)malloc(sizeof(int*)*height);
    for(int i=0;i<height;i++){
        matrix[i]=(int*)malloc(sizeof(int)*weight);
    }
    return matrix;
}

int ***construct_three(int depth,int height,int weight){//������ά����
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

int ****construct_four(int num,int depth,int height,int weight){//������ά����
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

void conv_parallel_col2(int**input_col2,int**kernel,int**output_col2,int comm_sz,int my_rank,int d){//imcol2���о������
   
    int process_count=comm_sz;//��ȡ������

     //�����output[d][i][j](dȡ0,1,2)���������������ͨ����ͬһλ�ã�����������output_height*output_weight�����λ�÷ָ���ͬ�Ľ��̼���
    int output_sum=output_height*output_weight;//�ܹ�Ҫ�����λ�ø���
    int average_n=output_sum/process_count;//ƽ��ÿ����ȡ����������ļ������λ�õĸ���
    int extra_n=output_sum%process_count;//ǰextra_n����ȡ�������ฺ��һ��λ��
    int local_start,local_end;//ÿ����ȡ�����������������λ�õĿ�ͷ�ͽ�β(������������)

    if(my_rank<extra_n){//ǰextra_n����ȡ�������ฺ��һ��λ��
        local_start=my_rank*average_n+my_rank;
        local_end=local_start+average_n+1;
    }
    else{//������ȡ����������average_n��λ��
        local_start=my_rank*average_n+extra_n;
        local_end=local_start+average_n;
    }

    
    int*output_1d=(int*)malloc(sizeof(int)*(output_height*output_weight));
    for(int i=0;i<output_height*output_weight;i++)output_1d[i]=0;
  
    #pragma omp parallel for
    for(int i=local_start;i<local_end;i++){//ѡȡ�����������һ��ͨ��
        //imcol2������
        for(int j=0;j<KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE;j++){
            #pragma omp critical
            output_1d[i]+=input_col2[i][j]*kernel[d][j];
        }
    }
    MPI_Reduce(output_1d, output_col2[d],  output_height*output_weight, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );//�������Լ��0����
    

}

int calculate_padding(){//����ʹ�����������߶ȳߴ���ͬ��padding 
    int p=0;
    while((HEIGHT-KERNEL_SIZE+2*p)/stride+1!=HEIGHT){
        p++;
    }
    return p;
}

void transform_input_col2(int***input,int**input_col){//����άinputתΪ��άinput_col2
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

void transform_output(int**output_col2,int***output){//����άoutput_col2תΪ��άoutput
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
  	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   //��ȡ��ǰ���̵ı��
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);   //��ȡ��������

    srand( (unsigned)time( NULL ) );

    //����input��kernel����
    int ***input=construct_three(DEPTH,HEIGHT,WEIGHT);
    int **kernel=construct_two(KERNEL_NUM,KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE);

    //printf("Please input the height and weight of input\n");
    //scanf("%d%d",&HEIGHT,&WEIGHT);

    //���input��kernel�ߴ�
    if(my_rank==0)printf("The size of input is %d*%d*%d\n",DEPTH,HEIGHT,WEIGHT);
    if(my_rank==0)printf("The size of kernel is %d*%d*%d*%d\n",KERNEL_NUM,KERNEL_SIZE,KERNEL_SIZE,KERNEL_SIZE);
    
    //��ʼ��input��kernel
    //input_and_kernel(input,kernel);
    random_input(input);
    random_kernel(kernel);
    
    //��ӡinput��kernel
    //if(my_rank==0)show_input(input);
    //if(my_rank==0)show_kernel(kernel);

    //�ò����ֱ�Ϊ1,2,3������
    for(stride=1;stride<=3;stride++){
    //������Ӧ��padding
    padding=calculate_padding();
    if(my_rank==0)printf("The stride is %d,the padding is %d\n",stride,padding);

    //����output�ĳߴ�
    output_height=(HEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_weight=(WEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_depth=KERNEL_NUM;

    //����άinputתΪ��άinput_col
    int **input_col2=construct_two(output_height*output_weight,KERNEL_SIZE*KERNEL_SIZE*KERNEL_SIZE);
    transform_input_col2(input,input_col2);

    //������άoutput_col2����άoutput
    int **output_col2=construct_two(output_depth,output_height*output_weight);
    int ***output=construct_three(output_depth,output_height,output_weight);
    initialize_output_col2(output_col2);
    
    //���output�ߴ�
    if(my_rank==0)printf("The size of output is %d*%d*%d\n", output_depth,output_height,output_weight);
    
    //��¼������㿪ʼʱ��
    struct timeval start,end;
    if(my_rank==0)gettimeofday(&start, NULL );
    
    //���м�����

	   
    for(int i=0;i<3;i++)conv_parallel_col2(input_col2,kernel,output_col2,comm_sz,my_rank,i);

    

    //��¼�������ʱ��
    gettimeofday(&end, NULL );
    if(my_rank==0){
      transform_output(output_col2,output);
   long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    //���������
    show_output(output);

    //����������ʱ��
   printf("Costing time=%fs\n",timeuse /1000000.0);
   printf("--------------------------------------------------------------------\n");
    }
    
    
    }

    MPI_Finalize();
    return 0;
    
}
