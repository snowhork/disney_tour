//03-160603
//松尾洵
//j-matsuo.aicj06@hotmail.co.jp
//jun-matsuo076@g.ecc.u-tokyo.ac.jp

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
 char *date_path;

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
 d2=-2;
 for (a=0;a<N;a++){
  if (data[a][(tnext/60)-7]==999||data[a][(tnext/60)-7]+te+tnext>tc) {//並べない時やアトラクションの終了が閉園時間を過ぎる場合はやめる(このtはアトラクション選択時のt)
  } else if (Q[a]>Qmax){
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
 char input_path[101];
 strncpy(input_path,abs_path,100);
 char str[] = "input/w.txt";
 strcat(input_path,str);
 if ((fp = fopen(input_path, "r")) == NULL) {
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
 char input_path[101];
 char input_path2[101];
 strncpy(input_path,abs_path,100);
 strncpy(input_path2,date_path,100);
 char str[] = "input/pred_wait_time_";
 char str2[] = ".csv";
 strcat(input_path,str);
 strcat(input_path,input_path2);
 strcat(input_path,str2);
 if ((fp = fopen(input_path, "r")) == NULL) {
  printf("file open error!!\n%s\n",input_path);
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
 abs_path = argv[1];
 date_path = argv[2];
 
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
 wait_ave[0]=3.808875;
 wait_ave[1]=12.395412;
 wait_ave[2]=0.021626;
 wait_ave[3]=7.618011;
 wait_ave[4]=8.765184;
 wait_ave[5]=11.049432;
 wait_ave[6]=12.450847;
 wait_ave[7]=64.797797;
 wait_ave[8]=5.195437;
 wait_ave[9]=82.436684;
 wait_ave[10]=15.419333;
 wait_ave[11]=37.629191;
 wait_ave[12]=19.655855;
 wait_ave[13]=17.463673;
 wait_ave[14]=14.605172;
 wait_ave[15]=30.386488;
 wait_ave[16]=9.391763;
 wait_ave[17]=41.425452;
 wait_ave[18]=10.796528;
 wait_ave[19]=11.081821;
 wait_ave[20]=64.878277;
 wait_ave[21]=23.732076;
 wait_ave[22]=8.128489;
 wait_ave[23]=56.882091;
 wait_ave[24]=0.080872;
 wait_ave[26]=0.029197;
 wait_ave[27]=31.307689;
 wait_ave[28]=69.819020;
 wait_ave[29]=61.138322;
 wait_ave[30]=30.469456;
 wait_ave[31]=18.675692;
 wait_ave[32]=74.430707;
 wait_ave[33]=17.366376;
 wait_ave[34]=17.300527;
 wait_ave[35]=30.278877;
 wait_ave[36]=36.821347;
 wait_sig=(double *)malloc((N+1)*sizeof(double));
 check(wait_sig);
 wait_sig[0]=6.256951;
 wait_sig[1]=13.099838;
 wait_sig[2]=0.608648;
 wait_sig[3]=2.938845;
 wait_sig[4]=9.960943;
 wait_sig[5]=6.880620;
 wait_sig[6]=7.707485;
 wait_sig[7]=39.796702;
 wait_sig[8]=2.218259;
 wait_sig[9]=47.948419;
 wait_sig[10]=14.308194;
 wait_sig[11]=22.963866;
 wait_sig[12]=14.197422;
 wait_sig[13]=8.656622;
 wait_sig[14]=9.619236;
 wait_sig[15]=20.206377;
 wait_sig[16]=7.812653;
 wait_sig[17]=36.917932;
 wait_sig[18]=8.128198;
 wait_sig[19]=9.139773;
 wait_sig[20]=35.044455;
 wait_sig[21]=15.699021;
 wait_sig[22]=8.884726;
 wait_sig[23]=23.474580;
 wait_sig[24]=1.806423;
 wait_sig[25]=50;
 wait_sig[26]=0.712346;
 wait_sig[27]=30.808110;
 wait_sig[28]=40.044122;
 wait_sig[29]=41.856940;
 wait_sig[30]=28.348884;
 wait_sig[31]=13.280729;
 wait_sig[32]=44.571368;
 wait_sig[33]=12.477420;
 wait_sig[34]=13.472652;
 wait_sig[35]=23.253076;
 wait_sig[36]=34.543369;
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
 nar=-1;
 
 FILE *outputfile;
 char output_path[101];
 strncpy(output_path,abs_path,100);
 char str[] = "output/route_output2.json";
 strcat(output_path,str);
 outputfile=fopen(output_path,"w");
 if (outputfile==NULL){
  printf("cannot open");
  exit(1);
 }
 fprintf(outputfile, "{\"candidates\":[");
 fprintf(outputfile, "{\"attraction\":[");
 
 t+=10;
 
 while(t<tc){
  
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
  if (d2==-2) break;
  if (nar!=-1) fprintf(outputfile, ",");
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
   fprintf(outputfile, "0%d\",\"flag\":0,\"move\":10,\"start\":\"",min);
  } else {
   fprintf(outputfile, "%d\",\"flag\":0,\"move\":10,\"start\":\"",min);
  }
  
  hour=(t-10)/60;
  min=t-10-(hour*60);
  if (hour < 10) {
   fprintf(outputfile, "0%d:",hour);
  } else {
   fprintf(outputfile, "%d:",hour);
  }
  if (min < 10) {
   fprintf(outputfile, "0%d\",\"ride\":\"",min);
  } else {
   fprintf(outputfile, "%d\",\"ride\":\"",min);
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
 fprintf(outputfile, "\"discription\":\"AIのおすすめ\",");
 fprintf(outputfile, "\"start\":{ \"place\":0, \"time\": \"09:00\"}}");
 fprintf(outputfile,"]}");
 fclose(outputfile);
 
 datafree();
 
 return 0;
}



