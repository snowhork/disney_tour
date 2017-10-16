#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MT.h"

 double *Q;
 double **W;
 double *s;
 int **data;
 
 int i,j,k,l;
 int N=37,a;					//NはAgentに許される行動数（アトラクション数に一致）、a=0, ..., N-1
 int kaisu=10;					//kaisuは一日のデータからの学習回数
 int to,tc,te,t,tnext;
 int dayid,dayidl;
 int d,d2;
 int nar=0;
 double r;
 double Qmax;
 double *wait_ave;
 double *wait_sig;
 int rc=0;
 double rsum=0;
 //double neut=200.0;
 char st[1024];
 char s1[]=",";
 char s2[256];
 char s3[256];
 char *abs_path;

void check(void *y)
{
 if (y==NULL) {
  printf("Not enough memory.\n");
  exit(1);
 }
 return;
}

void Qnow()					//Q[d]の更新
{
 Q[d]=0.0;//+b[d]は未実装
 for (j=0;j<2*N;j++){
  if (j<N&&data[j][(t/60)-7]==999) continue;
  Q[d]+=W[d][j]*s[j];
 }
 return;
}

void Qmaxnew()					//Qmaxを計算（Qmaxが最大となるaであるd2を探すことが目的）
{
 Qmax=-9999999999;
 d2=0;
 for (a=0;a<N;a++){
  if (data[a][(tnext/60)-7]==999||data[a][(tnext/60)-7]+te+tnext>tc) {//並べない時やアトラクションの終了が閉園時間を過ぎる場合はやめる(このtはアトラクション選択時のt)
   continue;
  }
  if (Q[a]>Qmax){
   Qmax=Q[a];
   d2=0+a;
  }
 }
 if (Qmax==-9999999999) Qmax=0;
 //printf("%d %d\n",t ,d2);
}

void reward()
{
 if (s[N+d]==0) {
  s[N+d]=0.5;//
  r=(wait_ave[d] - (data[d][(t/60)-7]/2))/10;
 }
 else {
  s[N+d]+=0.5;//
  r=-1;
 }
 //if (dayid<1470) return;
 rc+=1;
 rsum+=r;
 if (dayid==1&&rc>100) {
  rsum/=rc;
  rc=0;
  //printf("%f\n",rsum);
  rsum=0;
 }
 return;
}

void wread()
{
 FILE *fp;
 if ((fp = fopen("w.txt", "r")) == NULL) {
  printf("file open error!!\n");
  exit(EXIT_FAILURE);
 }
 l=0;						//行が何番目か
 while (fgets(st, 1024, fp) != NULL) {
  i=0;						//st[i]を読む
  j=0;						//前に読んだi+1
  k=0;						//行の中で何番目の数値か
  while(1){
   if (st[i]==0||st[i]==' ') {
    strncpy(s2,st+j,i-j);
    char *s2s;
    char *s3s;
    s2s=s2;
    W[l][k]=0.0+strtod(s2s,&s3s);
    //printf("%f ",W[l][k]);
    j=1+i;
    k++;
   }
   i++;
   if (st[i]==0) break;
  }
  //printf(":%d\n",l);
  l++;
  if (l>N-1) break;
 }
 fclose(fp);
 return;
}

void readdata()
{
 data=(int **)malloc((N+1)*sizeof(int *));		//アトラクション番号
 check(data);
 for (i=0;i<(N+1);i++){
  data[i]=(int *)malloc(16*sizeof(int));		//時刻
  check(data[i]);
  for (j=1;j<16;j++){
   data[i][j]=999;
  }
 }
 
 FILE *fp;
 if ((fp = fopen("pred_wait_time.csv", "r")) == NULL) {
  printf("file open error!!\n");
  exit(EXIT_FAILURE);
 }
 l=-1;						//行が何番目か
 int tt=0;
 while (fgets(st, 256, fp) != NULL) {
  i=0;						//st[i]を読む
  j=0;						//前に読んだi+1
  k=0;					//行の中で何番目の数値か
  int id=-1;
  while(1){
   if (st[i]==0||st[i]==',') {
    strncpy(s2,st+j,i-j);
    s2[i-j]='\0';
    if (l==-1) {
     if (k==1) {
      tt=atoi(s2)-8;
      break;
     }
     j=1+i;
     k++;
     i++;
     continue;
    }
    if (k==0){
     id=atoi(s2);
    } else {
     if (j!=i) {
      data[id][k+tt]=0+atoi(s2);
     }
     //printf("%d(%d,%d,%d) ",data[id][k+tt],id,k,tt);
    }
    j=1+i;
    k++;
   }
   if (st[i]==0) break;
   i++;
  }
  //printf(":%d\n",l);
  if (l>N-1) break;
  l++;
 }
 fclose(fp);
 
 //printf("\nデータ読み込み完了（このメッセージはあとで消す）\n");
 return;
}

void writedata()
{//Wを出力する
 for (a=0;a<N;a++){
  for (j=0;j<2*N;j++){
   printf( "%f ", W[a][j]);
  }
  printf("\n");
 }
 return;
}

void datafree()
{
 for (i=0; i<N+1; i++) {
  free(data[i]);
 }
 free(data);
 free(Q);
 for (i=0; i<N+1; i++) {
  free(W[i]);
 }
 free(W);
 free(s);
 free(wait_ave);
 free(wait_sig);
 return;
}

int main(int argc,char **argv)
{
 for(i = 1;i < argc; i++){
  abs_path = argv[i];
 }
 
 init_genrand(10);
 //各変数malloc
 Q=(double *)malloc((N+1)*sizeof(double));
 check(Q);
 W=(double **)malloc((N+1)*sizeof(double *));
 check(W);
 for (a=0;a<(N+1);a++){
  W[a]=(double *)malloc((2*N+1)*sizeof(double));
  check(W[a]);
  Q[a]=0.0;
  for (j=0;j<(2*N+1);j++){
   W[a][j]=0.0;
  }
 }
 s=(double *)malloc((2*N+1)*sizeof(double));
 check(s);
 for (j=0;j<(2*N+1);j++){
  s[j]=0.0;
 }
 wait_ave=(double *)malloc((N+1)*sizeof(double));
 check(wait_ave);
 wait_ave[0]=5.158720;				//空欄は無視してカウント
 wait_ave[1]=13.027216;
 wait_ave[2]=0.021626;
 wait_ave[3]=7.755273;
 wait_ave[4]=9.683291;
 wait_ave[5]=11.056463;
 wait_ave[6]=12.461017;
 wait_ave[7]=66.589083;
 wait_ave[8]=5.627198;
 wait_ave[9]=82.655975;
 wait_ave[10]=15.589394;
 wait_ave[11]=37.931158;
 wait_ave[12]=20.149541;
 wait_ave[13]=18.364049;
 wait_ave[14]=15.250428;
 wait_ave[15]=30.903343;
 wait_ave[16]=10.109886;
 wait_ave[17]=42.004858;
 wait_ave[18]=11.448977;
 wait_ave[19]=11.800058;
 wait_ave[20]=66.162924;
 wait_ave[21]=24.607150;
 wait_ave[22]=9.262131;
 wait_ave[23]=57.081361;
 wait_ave[24]=0.126084;
 wait_ave[26]=0.040935;
 wait_ave[27]=31.607543;
 wait_ave[28]=69.868253;
 wait_ave[29]=61.217088;
 wait_ave[30]=30.976933;
 wait_ave[31]=19.275038;
 wait_ave[32]=74.509935;
 wait_ave[33]=18.723272;
 wait_ave[34]=17.806272;
 wait_ave[35]=30.886838;
 wait_ave[36]=37.096507;
 wait_sig=(double *)malloc((N+1)*sizeof(double));
 check(wait_sig);
 wait_sig[0]=11.625662;
 wait_sig[1]=14.533640;
 wait_sig[2]=0.608648;
 wait_sig[3]=4.184205;
 wait_sig[4]=12.495799;
 wait_sig[5]=6.908179;
 wait_sig[6]=7.743742;
 wait_sig[7]=38.971300;
 wait_sig[8]=5.902355;
 wait_sig[9]=47.786938;
 wait_sig[10]=14.601525;
 wait_sig[11]=23.036609;
 wait_sig[12]=14.984813;
 wait_sig[13]=10.914702;
 wait_sig[14]=11.379645;
 wait_sig[15]=20.540868;
 wait_sig[16]=10.541671;
 wait_sig[17]=36.935925;
 wait_sig[18]=10.442171;
 wait_sig[19]=11.394254;
 wait_sig[20]=34.314763;
 wait_sig[21]=16.800111;
 wait_sig[22]=12.349206;
 wait_sig[23]=23.309927;
 wait_sig[24]=2.664260;
 wait_sig[25]=50;
 wait_sig[26]=1.214484;
 wait_sig[27]=30.927665;
 wait_sig[28]=40.011197;
 wait_sig[29]=41.853929;
 wait_sig[30]=28.589430;
 wait_sig[31]=14.346334;
 wait_sig[32]=44.544456;
 wait_sig[33]=15.095897;
 wait_sig[34]=14.431509;
 wait_sig[35]=23.599565;
 wait_sig[36]=34.653579;
 for(i=0; i<N;i++){
  wait_sig[i]*=10;
 }
 //mallocここまで
 
 double alpha=0.0008;
 double gamma=0.8;
 
 readdata();
 wread();
 //writedata();
 //return;
 
 to=540;					//開園時刻(hour*60+min)
 tc=1320;					//閉園時刻(hour*60+min)1320
 t=0+to;
 te=20;
 d2=-1;
 
 FILE *outputfile;
 char str[] = "output/route_output.json";
 strcat(abs_path,str);
 outputfile=fopen(abs_path,"w");
 if (outputfile==NULL){
  printf("cannot open");
  exit(1);
 }
 fprintf(outputfile, "{\"candidates\":[");
 fprintf(outputfile, "{\"attraction\":[");
 
 t+=10;
 
 while(t<tc){
  
  if (d2!=-1) fprintf(outputfile, ",");
  for (a=0;a<N;a++){
   s[a]=(data[a][(t/60)-7]-wait_ave[a])/wait_sig[a];	//dataは3つ目のパラメータが1で8時台として、+7時台に対応
  }
   
  for(d=0;d<N;d++) {				//全aに対してQ[s_t,a]を計算
   Qnow();
   //printf("%f\n",Q[d]);
  }
    
  tnext=t;
  Qmaxnew();					//選べるやつの中から選んでくることにする(d2=a_t)
      
  d=d2;
  if (data[d][(t/60)-7]==999) {
   printf("error");
   return;
  }
  tnext=t+data[d][(t/60)-7]+te;
    
  //Qnow();					//Q[d]を新しい値に更新
    
  for (a=0;a<N;a++){				//状態ベクトルとs_t+1に更新
   s[a]=(data[a][(tnext/60)-7]-wait_ave[a])/wait_sig[a];			//dataは3つ目のパラメータが1で8時台として、+7時台に対応
  }

  reward();					//rewardを更新
  
  fprintf(outputfile, "{\"ID\":%d,\"arrive\":\"",d);
  int hour=t/60;
  int min=t-(hour*60);
  if (hour < 10) {
   fprintf(outputfile, "0%d:",hour);
  } else {
   fprintf(outputfile, "%d:",hour);
  }
  if (min < 10) {
   fprintf(outputfile, "0%d\",\"duration\":10,\"end\":\"",min);
  } else {
   fprintf(outputfile, "%d\",\"duration\":10,\"end\":\"",min);
  }
  
  hour=(t+10+data[d][(t/60)-7])/60;
  min=t+10+data[d][(t/60)-7]-(hour*60);
  if (hour < 10) {
   fprintf(outputfile, "0%d:",hour);
  } else {
   fprintf(outputfile, "%d:",hour);
  }
  if (min < 10) {
   fprintf(outputfile, "0%d\",\"flag\":0,\"move\":10,\"ride\":\"",min);
  } else {
   fprintf(outputfile, "%d\",\"flag\":0,\"move\":10,\"ride\":\"",min);
  }
  
  hour=(t+data[d][(t/60)-7])/60;
  min=t+data[d][(t/60)-7]-(hour*60);
  if (hour < 10) {
   fprintf(outputfile, "0%d:",hour);
  } else {
   fprintf(outputfile, "%d:",hour);
  }
  if (min < 10) {
   fprintf(outputfile, "0%d\",\"wait\":%d}",min,data[d][(t/60)-7]);
  } else {
   fprintf(outputfile, "%d\",\"wait\":%d}",min,data[d][(t/60)-7]);
  }
  
  nar=0+d;					//これ以降はnar=a_t
  for(d=0;d<N;d++) {				//全aに対してQ[s_t+1,a]を計算
   Qnow();
   //printf("%f\n",Q[d]);
  }
  Qmaxnew();					//Qmaxを更新（ここで得られる）
  for (a=0;a<N;a++){
   s[a]=(data[a][(t/60)-7]-wait_ave[a])/wait_sig[a];	//dataは3つ目のパラメータが1で8時台として、+7時台に対応
  }
  for (j=0;j<2*N;j++){			//Wを更新（状態ベクトルsは与えられている）
   if (j>N-1||data[j][(t/60)-7]<999) {
    if (s[j] >10) {
     W[nar][j] = W[nar][j] + alpha * (r + gamma * Qmax - Q[nar]) * 10;
     //printf("%f %f %f %d %d %d %d\n",s[j],wait_ave[j],wait_sig[j],j,dayid,d,(t/60)-7);
    } else if (s[j] <-10) {
     W[nar][j] = W[nar][j] + alpha * (r + (gamma * Qmax) - Q[nar]) * (-10);
    } else {
     W[nar][j] = W[nar][j] + alpha * (r + (gamma * Qmax) - Q[nar]) * s[j];
    }
    //printf(" ");
   }
   if (Qmax<-9999998||Qmax>9999998) {
    printf("Qmax発散");
    return;
   }
     
   //printf("k%d t%d r%f M%f nar%d Q%f s%f j%d W%f\n", k, t, r, Qmax, nar, Q[nar], s[j], j, W[nar][j]);
  }
  //printf("%f\n",Qmax);
  //printf("%f %d %d\n", r,dayid, t);
 
  t+=data[nar][(t/60)-7]+te;
 }
 
 fprintf(outputfile, "],");
 fprintf(outputfile, "\"discription\":\"おすすめ\",");
 fprintf(outputfile, "\"start\":{ \"place\":0, \"time\": \"09:00\"}}");
 fprintf(outputfile,"]}");
 fclose(outputfile);
 
 datafree();
 
 return 0;
}

