#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <thread>
using namespace std;
const size_t size=50000000;
void s(double ** bucketsstart, double** bucketsend, int start,int delta, int numbuckets){
  for (;start<=numbuckets; start+=delta) 
    sort(bucketsstart[start],bucketsend[start]);
}
int main(){
  double *data=new double[size];
  FILE *f = fopen("gaussian.dat", "rb");
  fread(data,8,size,f);
  FILE* h=fopen("helper.txt","rb");
  int numbuckets;
  fread(&numbuckets,sizeof(int),1,h);
  int dscale;
  fread(&dscale,sizeof(int),1,h);
  double dmin; //minimum value of tabularised distribution
  double dmax; //,aximum ditto
  fread(&dmin,sizeof(double),1,h);
  fread(&dmax,sizeof(double),1,h);
  //int minn=dmin*dscale;
  int ile;
  fread(&ile,sizeof(int),1,h);
  int *tab=new int[ile];
  int test=fread(tab,sizeof(int),ile,h);
  if (test!=ile) cout<<"blad\n";
  double * databuckets=new double [size*4+size/numbuckets*4]; //just to be sure
  double ** bucketsstart= new double* [numbuckets+1];// pointer to beggining of every bucket
  double ** bucketsend= new double* [numbuckets+1]; //pointer to end
  int offset=4*size/numbuckets;
  for (int i=0; i<numbuckets+1; i++){
    bucketsend[i]=bucketsstart[i]=databuckets+i*offset;
  }
  for (int i=0; i<size; i++){
    if (data[i]>dmax) {
      *bucketsend[numbuckets]++=data[i];
    }
    else if (data[i]<dmin) *bucketsend[0]++=data[i];
    else {
      *bucketsend[tab[(int)((data[i]-dmin)*dscale)]]++=data[i];
    }
  }
  const int numthreads=4;
  thread** thr=new thread*[numthreads];
  for (int i=0; i<numthreads; i++){
    thr[i] =new thread (s,bucketsstart,bucketsend,i,numthreads,numbuckets);
  }
  for (int i=0; i<numthreads; i++){
    thr[i]->join();
    delete thr[i];
  }
  delete [] thr;
  double* currData=data;
  for (int i=0; i<=numbuckets; i++){
    memcpy(currData,bucketsstart[i],(bucketsend[i]-bucketsstart[i])*8);
    currData+=bucketsend[i]-bucketsstart[i];
  }
  //for (int i=0; i<size-1; i++){
  //  if (data[i]>data[i+1]) cout<<i<<' '<<data[i]<<' '<<data[i+1]<<endl;
  //}

  //fwrite(data,8,size,stdout);
  fclose(h);
  fclose(f);
}
