
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <thread>
using namespace std;
const size_t size=50000000;

void pivot(double* start,double * end, double middle,size_t& koniec){
    double * beg=start;
    end--;
    while (start!=end){
        if (*start>middle) swap (*start,*end--);
        else start++;
    }
    if (*end<middle) start+=1;
    koniec= start-beg;
}
void s(double * a, double* b){
    sort(a,b);
}
int main(){
    double *data=new double[size];
    FILE *f = fopen("gaussian.dat", "rb");
    fread(data,8,size,f);
    size_t end1,end2,end3,temp;
    pivot(data, data+size,0,end2);
    pivot(data, data+end2,-0.6745,end1);
    pivot(data+end2,data+size,0.6745,end3);
    end3+=end2;
    thread ts1(s,data,data+end1);
    thread ts2(s,data+end1,data+end2);
    thread ts3(s,data+end2,data+end3);
    thread ts4(s,data+end3,data+size);
    ts1.join(),ts2.join(),ts3.join(),ts4.join();
    //for (int i=0; i<size-1; i++){
    //  if (data[i]>data[i+1]) cerr<<"BLAD\n";
    //}
    fclose(f);
    //fwrite(data,8,size,stdout);
}
