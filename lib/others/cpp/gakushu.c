#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MT.h"

 double *Q;
 double **W;
 double *s;
 int ***data;
 
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
  if (j<N&&data[dayid][j][(t/60)-7]==999) continue;
  Q[d]+=W[d][j]*s[j];
 }
 return;
}

void Qmaxnew()					//Qmaxを計算（Qmaxが最大となるaであるd2を探すことが目的）
{
 Qmax=-9999999999;
 d2=0;
 for (a=0;a<N;a++){
  if (data[dayid][a][(tnext/60)-7]==999||data[dayid][a][(tnext/60)-7]+te+tnext>tc) {//並べない時やアトラクションの終了が閉園時間を過ぎる場合はやめる(このtはアトラクション選択時のt)
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
  r=(wait_ave[d] - (data[dayid][d][(t/60)-7]/2))/10;
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

void readdata()
{
 data=(int ***)malloc(1800*sizeof(int **));		//日にち
 check(data);
 for (i=0;i<1800;i++){
  data[i]=(int **)malloc((N+1)*sizeof(int *));		//アトラクション番号
  check(data[i]);
  for (j=0;j<(N+1);j++){
   data[i][j]=(int *)malloc(16*sizeof(int));		//時刻
   check(data[i][j]);
  }
 }
 char st[200];
 char s1[]=",";
 char s2[30];
 scanf("%s", st);
 
 int id=0,lid=0;
 i=0;						//iを日にちの番号にすることにする
 while(scanf("%s", st) != EOF)			//最後まで読む設定（あとで、日付が変わったら変える設定に変更）
 {
  j=0;
  k=0;
  l=0;
  //id=-1;
  while(1)					//このループである日の1つのアトラクションのデータを読み込む
  {
   if (st[j]==','||st[j]==0) {
    strncpy(s2,st+l,j-l);
    s2[j-l]='\0';
    if (k==7) {
     id=atoi(s2);
     if (id<lid) {
      i++;					//ここで日にちを更新
      dayidl=0+i;
     }
     lid=0+id;
    }
    if (k>8) {
     if (j!=l) {
      data[i][id][k-8]=0.0+atoi(s2);
     } else {
      data[i][id][k-8]=999;
     }
     //if (i>1400) printf("%d(%d,%d) ",data[i][id][k-8],i,id);
    }
    k+=1;
    l=1+j;
   }
   if (st[j]==0) break;
   j++;
  }
  //printf("\n");
 }
 
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
 for (i=0; i<1800; i++) {
  for (j=0; j<N+1; j++) {
   free(data[i][j]);
  }
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

int main()
{
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
 
 for (k=0;k<kaisu;k++){
  
  for(dayid=0;dayid<(dayidl+1);dayid++){
  to=540;					//開園時刻(hour*60+min)
  tc=1320;					//閉園時刻(hour*60+min)1320
  t=0+to;
  te=20;
  
   for (j=0;j<N;j++){				//行ったことがあるカウントを0に戻す
    s[j+N]=0.0;
   }
   nar=0;					//前のアトラクション
   d2=-1;					//次のアトラクション
  
   while(t<tc){
   
    for (a=0;a<N;a++){
     s[a]=(data[dayid][a][(t/60)-7]-wait_ave[a])/wait_sig[a];	//dataは3つ目のパラメータが1で8時台として、+7時台に対応
    }
   
    for(d=0;d<N;d++) {				//全aに対してQ[s_t,a]を計算
     Qnow();
     //printf("%f\n",Q[d]);
    }
    
    tnext=t;
    Qmaxnew();					//選べるやつの中から選んでくることにする(d2=a_t)
    
    int bs=0;
    int b[N];					//並べるアトラクションを返す関数
    for (a=0;a<N;a++){
     if (data[dayid][a][(t/60)-7]<999&&data[dayid][a][(t/60)-7]+te+t<=tc) {//ここはイコールをいれていいのか？
      b[bs]=a;
      bs++;
     }
    }
    if (bs==0) break;
    
    int d3=(genrand_int32()%(10*bs));			//アトラクションをランダムに選択(ただしbs以上ならQmaxとなるアトラクションを選択)
    if (d3>bs-1) {
     d=d2;
    } else {
     d=b[d3];
    }
    //printf("\n%d",d);
    if (data[dayid][d][(t/60)-7]==999) {
     printf("error");
     return;
    }
    tnext=t+data[dayid][d][(t/60)-7]+te;
    
    //Qnow();					//Q[d]を新しい値に更新
    
    for (a=0;a<N;a++){				//状態ベクトルとs_t+1に更新
     s[a]=(data[dayid][a][(tnext/60)-7]-wait_ave[a])/wait_sig[a];			//dataは3つ目のパラメータが1で8時台として、+7時台に対応
    }

    reward();					//rewardを更新
    nar=0+d;					//これ以降はnar=a_t
    for(d=0;d<N;d++) {				//全aに対してQ[s_t+1,a]を計算
     Qnow();
     //printf("%f\n",Q[d]);
    }
    Qmaxnew();					//Qmaxを更新（ここで得られる）
    for (a=0;a<N;a++){
     s[a]=(data[dayid][a][(t/60)-7]-wait_ave[a])/wait_sig[a];	//dataは3つ目のパラメータが1で8時台として、+7時台に対応
    }
    for (j=0;j<2*N;j++){			//Wを更新（状態ベクトルsは与えられている）
     if (j>N-1||data[dayid][j][(t/60)-7]<999) {
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
     
     //printf("k%d t%d r%f M%f d%d Q%f s%f j%d W%f\n", k, t, r, Qmax, d, Q[d], s[j], j, W[d][j]);
    }
    //printf("%f\n",Qmax);
    //printf("%f %d %d\n", r,dayid, t);
 
    t+=data[dayid][nar][(t/60)-7]+te;
   }
  }
 }
 
 writedata();
 datafree();
 
 return 0;
}



