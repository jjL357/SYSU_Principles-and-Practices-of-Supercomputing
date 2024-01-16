#define DEPTH 3//�����ͨ������
#define KERNEL_SIZE 3//kernel�Ĵ�С=3*3*3
#define KERNEL_NUM 3//kernel�ĸ���

#include<string.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<time.h>
#include <sys/time.h>
#include<mpi.h>
#include<omp.h>

// inputÿ��ͨ���Ĵ�С
int HEIGHT=5;//����ĸ߶�
int WEIGHT=5;//����Ŀ��

int stride=2;//����

// outputÿ��ͨ���Ĵ�С
int output_height=0;//����ĸ߶�
int output_weight=0;//����Ŀ��
int output_depth=0;//�����ͨ����

int padding=0;//padding�Ĵ�С
int padding_num=0;//padding��������

int count=0;//��¼��ɼ�����̸߳���

void ini_input(int ***matrix){//���������ʼ������(Ϊ��������������ֱ�Ӹ�ֵΪ1)
    
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

void show_kernel(int ****kernel){//��ӡkernel
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

void ini_kernel(int ****kernel){//���������ʼ��kernel(Ϊ����۲�������������ֱ�Ӹ�ֵΪ1)
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

void show_output(int ***matrix){//��ӡoutput
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

void trans_output(int***output,int**output_2d){//��MPI_Reduce�Ľ��תΪ��ά
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height;j++){
            for(int k=0;k<output_weight;k++){
                output[i][j][k]=output_2d[i][j*output_weight+k];
            }
        }
    }
}

void conv_parallel(int ***input,int ****kernel,int ***output,int comm_sz,int my_rank,int**output_2d,int n){//���н��о������

    int process_count=comm_sz;//��ȡ������
    
    //�����output[d][i][j](dȡ0,1,2)���������������ͨ����ͬһλ�ã�����������output_height*output_weight�����λ�÷ָ���ͬ�Ľ��̼���
    int output_sum=output_height*output_weight;//�ܹ�Ҫ�����λ�ø���
    int average_n=output_sum/process_count;//ƽ��ÿ��ȡ������ļ������λ�õĸ���
    int extra_n=output_sum%process_count;//ǰextra_n��ȡ���ฺ��һ��λ��
    int local_start,local_end;//ÿ��ȡ�������������λ�õĿ�ͷ�ͽ�β(������������)

    if(my_rank<extra_n){//ǰextra_n��ȡ���ฺ��һ��λ��
        local_start=my_rank*average_n+my_rank;
        local_end=local_start+average_n+1;
    }
    else{//����ȡ������average_n��λ��
        local_start=my_rank*average_n+extra_n;
        local_end=local_start+average_n;
    }
    
    //�洢������
    int*output_1d=(int*)malloc(sizeof(int)*(output_height*output_weight));
    for(int i=0;i<output_height*output_weight;i++)output_1d[i]=0;
  
		#pragma omp parallel for
        for(int d=0;d<output_depth;d++){//ѡȡ�����������һ��ͨ��
            for(int i=local_start;i<local_end;i++){//ѡȡ�ý��̸�������λ��
                int row=i/output_weight;//����λ�õ����±�
                int col=i%output_weight;//����λ�õ�������
                for(int k1=0;k1<KERNEL_SIZE;k1++){
                    for(int k2=0;k2<KERNEL_SIZE;k2++){
                        int input_row=-padding+stride*row+k1;//��Ӧ����λ�����±�
                        int input_col=-padding+stride*col+k2;//��Ӧ����λ�����±�
                       if(input_row>=HEIGHT||input_col>=WEIGHT||input_row<0||input_col<0)output_1d[i]+=padding_num*kernel[n][d][k1][k2];//��Ӧ����λ��λ��padding����λ��

                        #pragma omp critical
                        else  output_1d[i] += input[d][input_row][input_col]*kernel[n][d][k1][k2];
                        
                    }
                }
            }
        }
        //����������Լ��0������
        MPI_Reduce(output_1d, output_2d[n],  output_height*output_weight, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    
}

void initialize_output_col2(int**output_col2){//��ʼ��output_col2
    for(int i=0;i<output_depth;i++){
        for(int j=0;j<output_height*output_weight;j++){
            output_col2[i][j]=0;
        }
    }
}

void input_and_kernel(int ***input,int ****kernel){//ͨ���������� ����ʼ��Input��kernel(��Ҫ�ֶ�����ʱ����0���̵��ô˺�����ͨ������֮��Ĵ��ݻ�ȡkernel��input)
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

int calculate_padding(){//����ʹ�����������߶ȳߴ���ͬ��padding 
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
  	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   //��ȡ��ǰ���̵ı��
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);   //��ȡ��������

    srand( (unsigned)time( NULL ) );

    //����input��kernel����
    int ***input=construct_three(DEPTH,HEIGHT,WEIGHT);
    int ****kernel=construct_four(KERNEL_NUM,KERNEL_SIZE,KERNEL_SIZE,KERNEL_SIZE);
    
    //printf("Please input the height and weight of input\n");
    //scanf("%d%d",&HEIGHT,&WEIGHT);

    //��ʼ��input��kernel
    //input_and_kernel(input,kernel);
    ini_input(input);
    ini_kernel(kernel);

    
    if(my_rank==0){
    //0�������input��kernel�ߴ�
    printf("The size of input is %d*%d*%d\n",DEPTH,HEIGHT,WEIGHT);
    printf("The size of kernel is %d*%d*%d*%d\n",KERNEL_NUM,KERNEL_SIZE,KERNEL_SIZE,KERNEL_SIZE);
    //0���̴�ӡinput��kernel
    //show_input(input);
    //show_kernel(kernel);
    }
    

    //�ò����ֱ�Ϊ1,2,3������
    for(stride=1;stride<=3;stride++){
    //������Ӧ��padding
    padding=calculate_padding();
    output_height=(HEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_weight=(WEIGHT-KERNEL_SIZE+2*padding)/stride+1;
    output_depth=KERNEL_NUM;
    
   
    //����output�ĳߴ� stride padding
    if(my_rank==0)printf("The stride is %d,the padding is %d\n",stride,padding);
    if(my_rank==0)printf("The size of output is %d*%d*%d\n", output_depth,output_height,output_weight);

    //������ʼ��output output_2d�������չ�Լ���
    int**output_2d=construct_two(output_depth,output_height*output_weight);
    int ***output=construct_three(output_depth,output_height,output_weight);
    initialize_output(output);
    initialize_output_col2(output_2d);
    
    //��¼������㿪ʼʱ��
    struct timeval start,end;
    if(my_rank==0)gettimeofday(&start, NULL );
    
    //���м�����
 	for(int i=1;i<=3;i++)conv_parallel(input,kernel,output,comm_sz,my_rank,output_2d,i-1);
	
    //��¼�������ʱ��
    if(my_rank==0){
    gettimeofday(&end, NULL );
    //����������ʱ��
    long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("Costing time=%fs\n",timeuse /1000000.0);
    printf("------------------------\n");
    trans_output(output,output_2d);
    show_output(output);
    }

    }
	    
   MPI_Finalize();//�ͷŽ�����Դ
    return 0;    
}
