#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
double f(double x){//����f(x)=1/(1+x^2)
 return 1/(double)(1+x*x);
}
double Trap(double left_endpt,double right_endpt,int trap_count,double base_len){
    double estimate,x;
    int i;
    estimate=(f(left_endpt)+f(right_endpt))/2.0;
    for (i=0;i<=trap_count-1;i++){
        x=left_endpt+i*base_len;
        estimate+=f(x);        
    }
    return estimate*base_len;
}
int main(int argc, char** argv) {
int my_rank,comm_sz,n=1024,local_n;//�����������Ϊn�����μ���
double a=0.0,b=1.0,h,local_a,local_b;//��������[a,b]
double local_int,total_int;
int source;
MPI_Init(NULL,NULL);
MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
h=(b-a)/n;
local_n=n/comm_sz;
local_a=a+my_rank*local_n*h;//��ǰ���̵�������
local_b=local_a+local_n*h;//��ǰ���̵�������
local_int=Trap(local_a,local_b,local_n,h);//�������ǰ������Ҫ����ľ������֮��
if(my_rank!=0){
    MPI_Send(&local_int,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);//��0�Ž��̷��͵�ǰ������Ҫ����ľ������֮��
}
else{
    total_int=local_int;
    for(source=1;source<comm_sz;source++){//�����������̵Ĵ��ݽ�������������
        MPI_Recv(&local_int,1,MPI_DOUBLE,source,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        total_int+=local_int;
    }
}
if(my_rank==0){//0�Ž��̴�ӡ������ƽ��
    printf("With n= %d trapezoids,our estimate\n",n);
    printf("of the intergral from %f to %f =%.15e \n",a,b,total_int*4);
}
MPI_Finalize();
return 0;
}