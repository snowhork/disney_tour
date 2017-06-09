//debug用のコメントが残ってたり、候補のサジェストが雑なんでまだ仮の段階です
//ただひとまずバグっぽいことは起きなくなってるはず

#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <cstdio>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <set>
#include <map>
#include <stdlib.h>
#include "picojson.h"

#define INF 1000000005
#define MOD 1000000007
#define EPS 1e-10

using namespace std;
using namespace std::chrono;

typedef pair<int,int>P;

const int MAX_N = 40;	//アトラクションの数
const int MAX_S = 200;	//時間ステップの数

set<int> most_atr,med_atr;	    //行きたいアトラクションの種類別リスト(絶対行きたい:most_atr,行けたら行きたい:med_atr)
int prepos;		                //現在位置
int wtime[MAX_N][MAX_S];	    //待ち時間
int fptime_start[MAX_N][MAX_S]; //fpの使用開始時間(fpが存在しないもしくは終了している場合は0)
int fptime_end[MAX_N][MAX_S];   //fpの使用終了時間(fpが存在しないもしくは終了している場合は0)
int atime[MAX_N];	            //アトラクションに乗っている時間
int dtime[MAX_N][MAX_N];	    //移動時間
double mtime[MAX_N];	        //平均待ち時間
bool fp_flag[MAX_N];            //アトラクションがファストパスありならtrue
int sttime,endtime;             //入園時間,出園時間
int fp_wtime;	                //fpを使ったときの待ち時間(定数)
int fp_waitlimit;               //fpの制限が解除されるまで待つ時間もしくはfp使用開始時間になるまで待つ時間の限界値(定数)
int fp_itv;                     //新たにfpを取れない時間
int optdir;                     //最短移動時間(INFで初期化)
int optatr;	                    //最多アトラクション数(0で初期化)
int optAatr;                    //最多絶対乗りたいアトラクション数ルート
int opttime;    	            //最短時間(INFで初期化)
double opttimedir;              //最短時間と最短移動時間をブレンド
int num_atr;	                //アトラクションの数
int step_minute;                //1ステップの時間(分)
int step_in_hour;               //1時間あたりのステップ数
int park_open_time;             //ディズニーの開園時間
vector<P> optdir_route;	        //最短移動時間ルート
vector<P> optatr_route;	        //最多アトラクション数ルート
vector<P> optAatr_route;        //最大絶対乗りたいアトラクション数ルート
vector<P> opttime_route;	    //最短時間ルート
vector<P> opttimedir_route;     //最短時間と最短移動距離をブレンドしたルート
bool all_ride;                  //全てのアトラクションを回りきれるかどうか
bool small_check;               //trueならデータがsmall,falseならデータがlarge
bool diverge_check1;            //trueなら分岐を増やす
bool diverge_check2;            //trueなら分岐を増やす
bool diverge_check3;            //trueなら分岐を増やす
double time_limit;              //制限時間(ms)
system_clock::time_point measure_start,measure_check,measure_end;   //実行時間の測定
string current_directory;       //実行ファイルの存在するディレクトリ
string date;                    //日付


vector<string> split(string& input, char delimiter)
//delimiterを無視してinputを取得
{
    istringstream stream(input);
    string field;
    vector<string> result;
    while(getline(stream, field, delimiter)){
        result.push_back(field);
    }
    return result;
}

bool atr_same(const vector<int>& p1,const vector<int>& p2)
//回ったアトラクションの一致を調べる
{
    if(p1.size() != p2.size()){
        return false;
    }
    for(int i = 0; i < p1.size(); i++){
        if(p1[i] != p2[i]){
            return false;
        }
    }
    return true;
}

bool route_same(const vector<P>& p1,const vector<P>& p2)
//回ったルートの一致を調べる
{
    if(p1.size() != p2.size()){
        return false;
    }
    for(int i = 0; i < p1.size(); i++){
        if(p1[i] != p2[i]){
            return false;
        }
    }
    return true;
}

P real_time(int step)
//時間ステップを実際の時刻(pair)に直す
{
	return P(park_open_time + (step / step_in_hour), step % step_in_hour * step_minute);
}

string string_time(int step)
//時間ステップを実際の時刻(string)に直す
{
	stringstream ss;
	P p = real_time(step);
	if(p.first < 10){
		if(p.second < 10){
			ss << '0' << p.first << ":" << '0' << p.second;
		}else{
			ss << '0' << p.first << ":" << p.second;
		}
	}else{
		if(p.second < 10){
			ss << p.first << ":" << '0' << p.second;
		}else{
			ss << p.first << ":" << p.second;
		}
	}
	return ss.str();
}

int step_time_start(P time)
//実際の時刻をそれ以上かつ最小の時間ステップに直す
{
	return (time.first - park_open_time) * step_in_hour + (time.second + step_minute - 1) / step_minute;
}

int step_time_end(P time)
//実際の時刻をそれ以下かつ最大の時間ステップに直す
{
	return (time.first - park_open_time) * step_in_hour + time.second / step_minute;
}


int string_to_step(string& time, int id)
//idが0ならstep_time_start,idが1ならstep_time_end
{
    time.erase(time.begin() + 2);       //":"を削除
    string hour, minute;
    hour += time[0], hour += time[1];
    minute += time[2], minute += time[3];
    if(id){
        return step_time_end(P(stoi(hour), stoi(minute)));
    }else{
        return step_time_start(P(stoi(hour), stoi(minute)));
    }
}

void init()
{
    most_atr.clear(),med_atr.clear();
    fp_wtime = 2,fp_waitlimit = 2,fp_itv = 24;
    optdir = INF,optatr = 0,optAatr = 0,opttime = INF,opttimedir = INF;
    step_minute = 5,step_in_hour = 12,park_open_time = 8;
    optdir_route.clear(),optatr_route.clear(),optAatr_route.clear(),opttime_route.clear();
    all_ride = false;
    time_limit = 5000.0;
}

void fp_time_read(int atr_id, string& file)
//ファストパス時間の読み込み
{
    string str = current_directory + file;
    ifstream ifs(str);
    if(!ifs){
        cout << "入力エラー";
        return;
    }
    string line;
    int ptime;
    string buff;
    int loop = 0;
    while(getline(ifs, line)){
        vector<string> strvec = split(line, ',');
        if(!loop){
            buff = strvec[0];
            loop++;
        }else{
            if(buff != strvec[0]){
                break;
            }
        }
        ptime = string_to_step(strvec[1], 0);
        fptime_start[atr_id][ptime] = string_to_step(strvec[2], 0);
        fptime_end[atr_id][ptime] = string_to_step(strvec[3], 0);
    }
}

void user_data_input()
//データの読み込み(ユーザーから)
{
    string str = current_directory + "input/user_input.json";
	ifstream ifs(str, ios::binary);
    if(!ifs){
        cout << "入力エラー";
        return;
    }
    picojson::value val;
    ifs >> val;
    picojson::object& o = val.get<picojson::object>()["user"].get<picojson::object>();
    picojson::array& ary = o["list"].get<picojson::array>();
    for(int i = 0; i < ary.size(); i++){
        if((int)ary[i].get<picojson::object>()["hope"].get<double>()){
			med_atr.insert((int)ary[i].get<picojson::object>()["ID"].get<double>());
		}else{
			most_atr.insert(((int)ary[i].get<picojson::object>()["ID"].get<double>()));
		}
	}
	sttime = string_to_step(o["start"].get<string>(), 0);
	endtime = string_to_step(o["end"].get<string>(), 1);
	prepos = (int)o["position"].get<double>();
}

void known_data_input()
//データの読み込み(既知のデータから標準入力)
{
    string str = current_directory + "input/known_data.csv";
    ifstream ifs(str, ios::binary);
    if(!ifs){
        cout << "入力エラー";
        return;
    }
    int loop = 0;
    string line;
    while(getline(ifs,line)){
        vector<string> strvec = split(line, ',');
        if(loop == 0){
            num_atr = stoi(strvec[0]);
            loop++;
        }else if(loop == 1){
            for(int i = 0; i < num_atr; i++){
                int flag;
                flag = stoi(strvec[i]);
                if(flag == 1){
                    fp_flag[i] = true;
                }else{
                    fp_flag[i] = false;
                }
            }
            loop++;
        }else if(loop == 2){
            for(int i = 0; i < num_atr; i++){
                atime[i] = stoi(strvec[i]);
            }
            loop++;
        }else{
        	for(int j = 0; j < num_atr; j++){
        		dtime[loop - 3][j] = stoi(strvec[j]);
        	}
            loop++;
        }
    }
}

void predict_data_input()
//データの読み込み(データ班から)
{
    //待ち時間の読み込み
    string str = current_directory + "input/pred_wait_time.csv";
	ifstream wait(str);
    if(!wait){
        cout << "入力エラー";
        return;
    }
    for(int i = 0; i < MAX_N; i++){
        for(int j = 0; j < MAX_S; j++){
            wtime[i][j] = INF;
        }
    }
    string line;
    int row = 0;
	int wait_start = 0;
    while(getline(wait,line)){
        vector<string> strvec = split(line, ',');
		if(row == 0){
            row++;
        }else{
            wait_start = (sttime / step_in_hour) * step_in_hour;
			vector<int> intvec;
			for(int i = 0; i < strvec.size(); i++){
                if(strvec[i] == ""){
                    continue;
                }
				intvec.push_back(stoi(strvec[i]));
			}
			for(int i = 1; i < intvec.size(); i++){
				mtime[intvec[0]] = accumulate(intvec.begin()+1, intvec.end(), 0) / (double)(intvec.size() - 1);
			}
        	for(int i = 0; i < intvec.size() - 2; i++){
				double step = abs(intvec[i + 2] - intvec[i + 1]) / (double)step_in_hour;
				if(intvec[i + 2] > intvec[i + 1]){
					for(int j = 0; j < step_in_hour; j++){
						wtime[intvec[0]][wait_start + j] = (int)(intvec[i + 1] + j * step + step_minute / 2.0) / step_minute;	//量子化
					}
                    wait_start += step_in_hour;
				}else{
					for(int j = 0; j < step_in_hour; j++){
						wtime[intvec[0]][wait_start + j] = (int)(intvec[i + 1] - j * step + step_minute / 2.0) / step_minute;	//量子化
					}
                    wait_start += step_in_hour;
				}
        	}
		}
    }

    //ファストパス時間の初期化
    for(int i = 0; i < MAX_N; i++){
        for(int j = 0; j < MAX_S; j++){
            fptime_start[i][j] = 0;
            fptime_end[i][j] = 0;
        }
    }

    //ファストパス時間の読み込み
    str = "input/110_fastpass_sample.csv";
    fp_time_read(7, str);
    str = "input/112_fastpass_sample.csv";
    fp_time_read(9, str);
    str = "input/120_fastpass_sample.csv";
    fp_time_read(17, str);
    str = "input/123_fastpass_sample.csv";
    fp_time_read(20, str);
    str = "input/132_fastpass_sample.csv";
    fp_time_read(27, str);
    str = "input/133_fastpass_sample.csv";
    fp_time_read(28, str);
    str = "input/134_fastpass_sample.csv";
    fp_time_read(29, str);
    str = "input/163_fastpass_sample.csv";
    fp_time_read(32, str);
}

void dfs(int ptime, int patr, set<P> fp, int fp_free, set<int> Aatr, set<int> Batr, vector<P> route)
//保持している状態変数
//現在時間,現在位置,今持っているfp(アトラクション名,取得した時刻),ファストパス解禁時間,まだ乗っていないアトラクションの種類別リスト(fp取ったものは除く),今までのルート(アトラクション名,0:乗った,1:fp取った,2:fpを取るために待つ)
//dfsが呼び出されるのは入場時,乗車後,ファストパス取得後のいずれかのタイミング
{
    //終了条件
    //実行時間がtime_limitを超えたらそれ以上の探索を行わない
    measure_check = system_clock::now();
    if(duration_cast<milliseconds>(measure_check - measure_start).count() > time_limit){
        return;
    }

    //全てのアトラクションを回りつくした
	if(Aatr.empty() && Batr.empty() && fp.empty()){
        all_ride = true;
        //最短移動時間のルート
		int dsum = 0;
		for(int i = 1; i < route.size(); i++){
			dsum += dtime[route[i-1].first][route[i].first];
		}
		if(dsum < optdir){
			optdir_route.clear();
			for(int i = 0; i < route.size(); i++){
				optdir_route.push_back(route[i]);
			}
			optdir = dsum;
		}
        //最短時間のルート
		if(ptime < opttime){
			opttime_route.clear();
			for(int i = 0; i < route.size(); i++){
				opttime_route.push_back(route[i]);
			}
			opttime = ptime;
		}
        double rate = (double)opttime / optdir;
        if((ptime-sttime) + rate * dsum < opttimedir){
            opttimedir_route.clear();
            for(int i = 0; i < route.size(); i++){
                opttimedir_route.push_back(route[i]);
            }
            opttimedir = (ptime-sttime) + rate * dsum;
        }
		return;
	}

    //fpを使う行動
    int nptime,npatr,nfp_free;
	vector<P> nfp;		       //バッファ
	bool fp_use = false;	   //fp使う行動を行ったか
	for(auto it = fp.begin(); it != fp.end(); it++){
		nfp.push_back(*it);
	}

    //fpが使える場合は即使う
	for(int i = 0; i < nfp.size(); i++){
        //fp取ったけど使えない場合
		if(ptime + dtime[patr][nfp[i].first] > fptime_end[nfp[i].first][nfp[i].second] || ptime + dtime[patr][nfp[i].first] + fp_wtime + atime[nfp[i].first] > endtime){
            return;
		}
        //ファストパスをすぐに使える時
		if(ptime + dtime[patr][nfp[i].first] >= fptime_start[nfp[i].first][nfp[i].second]){
			nptime = ptime + dtime[patr][nfp[i].first] + fp_wtime + atime[nfp[i].first];
			npatr = nfp[i].first;
			fp.erase(nfp[i]);
			route.push_back(P(nfp[i].first, 0));
			dfs(nptime, npatr, fp, fp_free, Aatr, Batr, route);
			fp_use = true;
			fp.insert(nfp[i]);		//backtrack
			route.pop_back();		//backtrack
        //少し待てばファストパスを使える時
		}else if(ptime + dtime[patr][nfp[i].first] + fp_waitlimit >= fptime_start[nfp[i].first][nfp[i].second]){
            nptime = fptime_start[nfp[i].first][nfp[i].second] + fp_wtime + atime[nfp[i].first];
            npatr = nfp[i].first;
            fp.erase(nfp[i]);
            route.push_back(P(nfp[i].first, 0));
            dfs(nptime, npatr, fp, fp_free, Aatr, Batr, route);
            fp_use = true;
            fp.insert(nfp[i]);		//backtrack
            route.pop_back();		//backtrack
        }
	}
    //ファストパスを使ったら終了
	if(fp_use){
		return;
	}

    //fp取ろう行動
	vector<int> fpatr;		//most_atrに属していてかつファストパスが終了していないアトラクション
	for(auto it = Aatr.begin(); it != Aatr.end(); it++){
		if(fptime_start[*it][ptime] > 0){		//fpが存在しまだ終了していないアトラクションはその値が0である
			fpatr.push_back(*it);
		}
	}
	bool fp_get = false;	//fp取る行動を行ったか

	for(int i = 0; i < fpatr.size(); i++){
		nptime = ptime + dtime[patr][fpatr[i]];
        //ファストパス制限が解除されそうにないorファストパスが終了しているなら
		if(nptime + fp_waitlimit < fp_free || fptime_start[fpatr[i]][nptime] == 0){
			continue;
		}
        //今すぐファストパスが使える時
		if(nptime >= fp_free){
            //ファストパスの開始時間が終了時刻-アトラクション時間を超えていたらダメ
            if(endtime-atime[fpatr[i]] >= fptime_start[fpatr[i]][nptime]){
                npatr = fpatr[i];
    			fp.insert(P(npatr, nptime));
    			nfp_free = min(nptime + fp_itv, fptime_start[npatr][nptime]);
    			Aatr.erase(npatr);
    			route.push_back(P(npatr, 1));
    			fp_get = true;
    			dfs(nptime, npatr, fp, nfp_free, Aatr, Batr, route);
                fp.erase(P(npatr, nptime)); //backtrack
    			Aatr.insert(npatr);	        //backtrack
    			route.pop_back();	        //backtrack
            }
        //まだファストパスの制限が解除されていないなら
		}else{
            //ファストパスが終了していない かつ ファストパスの開始時間が終了時刻-アトラクション時間を超えていない
            if(fptime_start[fpatr[i]][fp_free] > 0 && endtime - atime[fpatr[i]] >= fptime_start[fpatr[i]][fp_free]){
                npatr = fpatr[i];
			    fp.insert(P(npatr, nptime));
			    nfp_free = min(fp_free + fp_itv, fptime_start[npatr][fp_free]);
			    Aatr.erase(npatr);
			    route.push_back(P(npatr, 2));
                fp_get = true;
			    dfs(fp_free, npatr, fp, nfp_free, Aatr, Batr, route);
                fp.erase(P(npatr, nptime)); //backtrack
		        Aatr.insert(npatr);		    //backtrack
	            route.pop_back();		    //backtrack
            }
		}
	}
    vector<int> fp_Batr;		//most_atrに属していてかつファストパスが終了していないアトラクション
	for(auto it = Batr.begin(); it != Batr.end(); it++){
		if(fptime_start[*it][ptime] > 0){		//fpが存在しまだ終了していないアトラクションはその値が0である
			fp_Batr.push_back(*it);
		}
	}

    for(int i = 0; i < fp_Batr.size(); i++){
        nptime = ptime + dtime[patr][fp_Batr[i]];
        //ファストパス制限が解除されそうにないorファストパスが終了しているなら
        if(nptime + fp_waitlimit < fp_free || fptime_start[fp_Batr[i]][nptime] == 0){
            continue;
        }
        //今すぐファストパスが使える時
        if(nptime >= fp_free){
            //ファストパスの開始時間が終了時刻-アトラクション時間を超えていたらダメ
            if(endtime-atime[fp_Batr[i]] >= fptime_start[fp_Batr[i]][nptime]){
                npatr = fp_Batr[i];
                fp.insert(P(npatr, nptime));
                nfp_free = min(nptime + fp_itv, fptime_start[npatr][nptime]);
                Batr.erase(npatr);
                route.push_back(P(npatr, 1));
                fp_get = true;
                dfs(nptime, npatr, fp, nfp_free, Aatr, Batr, route);
                fp.erase(P(npatr, nptime)); //backtrack
                Batr.insert(npatr);	        //backtrack
                route.pop_back();	        //backtrack
            }
        //まだファストパスの制限が解除されていないなら
        }else{
            //ファストパスが終了していない かつ ファストパスの開始時間が終了時刻-アトラクション時間を超えていない
            if(fptime_start[fp_Batr[i]][fp_free] > 0 && endtime - atime[fp_Batr[i]] >= fptime_start[fp_Batr[i]][fp_free]){
                npatr = fp_Batr[i];
                fp.insert(P(npatr, nptime));
                nfp_free = min(fp_free + fp_itv, fptime_start[npatr][fp_free]);
                Batr.erase(npatr);
                route.push_back(P(npatr, 2));
                fp_get = true;
                dfs(fp_free, npatr, fp, nfp_free, Aatr, Batr, route);
                fp.erase(P(npatr, nptime)); //backtrack
                Batr.insert(npatr);		    //backtrack
                route.pop_back();		    //backtrack
            }
        }
    }
    //ファストパスを取ったら終了
	if(fp_get){
		return;
	}

    // //fpの存在するBatrの中から一番早くfpが使えるようになるものを１つ選ぶ
    // vector<P> fp_Batr;		                       //med_atrに属していてかつファストパスが終了していないアトラクション
	// for(auto it = Batr.begin(); it != Batr.end(); it++){
	// 	if(fptime_start[*it][ptime] > 0){		   //fpが存在しまだ終了していないアトラクションはその値が0である
	// 		fp_Batr.push_back(P(fptime_start[*it][ptime],*it));
	// 	}
	// }
    // sort(fp_Batr.begin(),fp_Batr.end());
	// fp_get = false;	//fp取る行動を行ったか
    //
	// for(int i = 0; i < fp_Batr.size(); i++){
	// 	nptime = ptime + dtime[patr][fp_Batr[i].second];
    //     //ファストパス制限が解除されそうにないorファストパスが終了しているなら
	// 	if(nptime + fp_waitlimit < fp_free || fptime_start[fp_Batr[i].second][nptime] == 0){
	// 		continue;
	// 	}
    //     //今すぐファストパスが使える時
	// 	if(nptime >= fp_free){
    //         //ファストパスの開始時間が終了時刻-アトラクション時間を超えていたらダメ
    //         if(endtime-atime[fp_Batr[i].second] >= fptime_start[fp_Batr[i].second][nptime]){
    //             npatr = fp_Batr[i].second;
    // 			fp.insert(P(npatr, nptime));
    // 			nfp_free = min(nptime + fp_itv, fptime_start[npatr][nptime]);
    // 			Batr.erase(npatr);
    // 			route.push_back(P(npatr, 1));
    // 			fp_get = true;
    // 			dfs(nptime, npatr, fp, nfp_free, Aatr, Batr, route);
    //             fp.erase(P(npatr, nptime)); //backtrack
    // 			Batr.insert(npatr);	         //backtrack
    // 			route.pop_back();	         //backtrack
    //         }
    //     //まだファストパスの制限が解除されていない
	// 	}else{
    //         //ファストパスが終了していない かつ ファストパスの開始時間が終了時刻-アトラクション時間を超えていない
    //         if(fptime_start[fp_Batr[i].second][fp_free] > 0 && endtime - atime[fp_Batr[i].second] >= fptime_start[fp_Batr[i].second][fp_free]){
    //             npatr = fp_Batr[i].second;
	// 		    fp.insert(P(npatr, nptime));
	// 		    nfp_free = min(fp_free + fp_itv, fptime_start[npatr][fp_free]);
	// 		    Batr.erase(npatr);
	// 		    route.push_back(P(npatr, 2));
    //             fp_get = true;
	// 		    dfs(fp_free, npatr, fp, nfp_free, Aatr, Batr, route);
    //             fp.erase(P(npatr, nptime)); //backtrack
	// 	        Batr.insert(npatr);		    //backtrack
	//             route.pop_back();		    //backtrack
    //         }
	// 	}
    //     //ファストパスを一回でも取ったら終了
    //     if(fp_get){
    // 		return;
    // 	}
	// }


	//アトラクション乗ろう行動
	bool ride = false;	    //アトラクションに乗る行動を行ったか
	vector<P> ride_cand;    //(アトラクション名,Aatr:0,Batr:1)
	int res = INF;
	int resit = -1;
	double ress = -INF;
    int kind;               //Aatr:0,Batr:1

    if(!diverge_check3){
        //候補を3つ選ぶ
        //「平均待ち時間-今の待ち時間-距離」が大きいものを１つ選ぶ
        for(auto it = Aatr.begin(); it != Aatr.end(); it++){
            if(mtime[*it] - dtime[patr][*it] - wtime[*it][ptime + dtime[patr][*it]] > ress){
                ress = mtime[*it] - dtime[patr][*it] - wtime[*it][ptime + dtime[patr][*it]];
                resit = *it;
                kind = 0;
            }
        }
        for(auto it = Batr.begin(); it != Batr.end(); it++){
            if(mtime[*it] - dtime[patr][*it] - wtime[*it][ptime + dtime[patr][*it]] > ress){
                ress = mtime[*it] - dtime[patr][*it] - wtime[*it][ptime + dtime[patr][*it]];
                resit = *it;
                kind = 1;
            }
        }
        if(resit >= 0){
            ride_cand.push_back(P(resit, kind));
        }
        ress = -INF, resit = -1;	//backtrack

        //Aatrのうち「移動時間+待ち時間」が小さいものを1つ選ぶ
        for(auto it = Aatr.begin(); it != Aatr.end(); it++){
            if(dtime[patr][*it] + wtime[*it][ptime + dtime[patr][*it]] < res){
                //既にある候補と被っている場合はムシ
                bool flag = false;
                for(int i = 0; i < ride_cand.size(); i++){
                    if(ride_cand[i].first == *it){
                        flag = true;
                    }
                }
                if(!flag){
                    res = dtime[patr][*it] + wtime[*it][ptime + dtime[patr][*it]];
                    resit = *it;
                    kind = 0;
                }
            }
        }
        if(resit >= 0){
            ride_cand.push_back(P(resit, kind));
        }
        res = INF, resit = -1;	//backtrack

        //Aatr+Batrのうちで「移動時間+待ち時間」が一番小さいものを選ぶ
        for(auto it = Aatr.begin(); it != Aatr.end(); it++){
            if(dtime[patr][*it] + wtime[*it][ptime + dtime[patr][*it]] < res){
                bool flag = false;
                for(int i = 0; i < ride_cand.size(); i++){
                    if(ride_cand[i].first == *it){
                        flag = true;
                    }
                }
                if(!flag){
                    res = dtime[patr][*it] + wtime[*it][ptime + dtime[patr][*it]];
                    resit = *it;
                    kind = 0;
                }
            }
        }
        for(auto it = Batr.begin(); it != Batr.end(); it++){
            if(dtime[patr][*it] + wtime[*it][ptime + dtime[patr][*it]] < res){
                bool flag = false;
                for(int i = 0; i < ride_cand.size(); i++){
                    if(ride_cand[i].first == *it){
                        flag = true;
                    }
                }
                if(!flag){
                    res = dtime[patr][*it] + wtime[*it][ptime + dtime[patr][*it]];
                    resit = *it;
                    kind = 1;
                }
            }
        }
        if(resit >= 0){
            ride_cand.push_back(P(resit, kind));
        }

        res = INF, resit = -1;	//backtrack

        if(diverge_check1){
            for(auto it = Aatr.begin(); it != Aatr.end(); it++){
                if(mtime[*it] - dtime[patr][*it] - wtime[*it][ptime + dtime[patr][*it]] > ress){
                    bool flag = false;
                    for(int i = 0; i < ride_cand.size(); i++){
                        if(ride_cand[i].first == *it){
                            flag = true;
                        }
                    }
                    if(!flag){
                        ress = mtime[*it] - dtime[patr][*it] - wtime[*it][ptime + dtime[patr][*it]];
                        resit = *it;
                        kind = 0;
                    }
                }
            }
            if(resit >= 0){
                ride_cand.push_back(P(resit, kind));
            }

            ress = -INF,resit = -1;	//backtrack
        }

        if(diverge_check2){
            //Aatr+Batrのうちで「移動時間」が一番小さいものを選ぶ
            for(auto it = Aatr.begin(); it != Aatr.end(); it++){
                if(dtime[patr][*it] < res){
                    bool flag = false;
                    for(int i = 0; i < ride_cand.size(); i++){
                        if(ride_cand[i].first == *it){
                            flag = true;
                        }
                    }
                    if(!flag){
                        res = dtime[patr][*it];
                        resit = *it;
                        kind = 0;
                    }
                }
            }
            for(auto it = Batr.begin(); it != Batr.end(); it++){
                if(dtime[patr][*it] < res){
                    bool flag = false;
                    for(int i = 0; i < ride_cand.size(); i++){
                        if(ride_cand[i].first == *it){
                            flag = true;
                        }
                    }
                    if(!flag){
                        res = dtime[patr][*it];
                        resit = *it;
                        kind = 1;
                    }
                }
            }
            if(resit >= 0){
                ride_cand.push_back(P(resit, kind));
            }

            res = INF,resit = -1;	//backtrack
        }
    }else{
        for(auto it = Aatr.begin(); it != Aatr.end(); it++){
            ride_cand.push_back(P(*it,0));
        }
        for(auto it = Batr.begin(); it != Batr.end(); it++){
            ride_cand.push_back(P(*it,1));
        }
    }

    //乗るアトラクションの候補のどれに乗ってもあるfpの使用時間を超えてしまうときはfpを使う
    //fpを使った後に終了時間を超えてしまうものはダメ
    //残りのアトラクションがfpを持っているもののみでfp使おう行動に引っかからなかったものを使う場合も含む
    fp_use = false;
    int comp;
    int min_time = INF;     //乗るアトラクションの候補のうち最も移動+待ち＋乗車の時間の最小値(INFで初期化)
    int min_i;

    if(!fp.empty()){
        for(int i = 0; i < ride_cand.size(); i++){
            comp = dtime[patr][ride_cand[i].first] + wtime[ride_cand[i].first][ptime + dtime[patr][ride_cand[i].first]] + atime[ride_cand[i].first];
            if(min_time > comp){
                min_time = comp;
                min_i = i;
            }
        }
        if(ride_cand.size() == 0){
            for(int i = 0; i < nfp.size(); i++){
                fp_use = true;
                if(fptime_start[nfp[i].first][nfp[i].second] + fp_wtime + atime[nfp[i].second] > endtime){
                    continue;
                }
                nptime = fptime_start[nfp[i].first][nfp[i].second] + fp_wtime + atime[nfp[i].second];
                fp.erase(P(nfp[i].first, nfp[i].second));
                route.push_back(P(nfp[i].first, 0));
                dfs(nptime, nfp[i].first, fp, fp_free, Aatr, Batr,route);
                fp.insert(P(nfp[i].first, nfp[i].second));
                route.pop_back();
            }
        }else{
            for(int i = 0; i < nfp.size(); i++){
                if(fptime_end[nfp[i].first][nfp[i].second] < ptime + min_time + dtime[ride_cand[min_i].first][nfp[i].first]){
                    fp_use = true;
                    if(fptime_start[nfp[i].first][nfp[i].second] + fp_wtime + atime[nfp[i].second] > endtime){
                        continue;
                    }
                    nptime = fptime_start[nfp[i].first][nfp[i].second] + fp_wtime + atime[nfp[i].second];
                    fp.erase(P(nfp[i].first, nfp[i].second));
                    route.push_back(P(nfp[i].first, 0));
                    dfs(nptime, nfp[i].first, fp, fp_free, Aatr, Batr,route);
                    fp.insert(P(nfp[i].first, nfp[i].second));
                    route.pop_back();
                }
            }
        }
    }
    //ファストパスを使ったら終了
    if(fp_use){
        return;
    }

	//候補となった行動を実行
	for(int i = 0; i < ride_cand.size(); i++){
		int arv = ptime + dtime[patr][ride_cand[i].first];
        //到着した時点で時間オーバーなら終了
		if(arv >= endtime){
			continue;
		}
		nptime = arv + wtime[ride_cand[i].first][arv] + atime[ride_cand[i].first];
        //アトラクションに乗車した後で時間オーバーなら終了
		if(nptime > endtime){
			continue;
		}
		npatr = ride_cand[i].first;
        if(ride_cand[i].second == 0){
            Aatr.erase(npatr);
        }else{
            Batr.erase(npatr);
        }
		route.push_back(P(npatr, 0));
        ride = true;
		dfs(nptime, npatr, fp, fp_free, Aatr, Batr, route);
        if(ride_cand[i].second == 0){
            Aatr.insert(npatr);   //backtrack
        }else{
            Batr.insert(npatr);   //backtrack
        }
		route.pop_back();         //backtrack
	}
	if(ride){
		return;
	}
    if(!all_ride){
        //ファストパス取れない,ファストパス使えない,乗り物乗れない→これ以上行動を取れない場合(終了)
    	int atrcnt = most_atr.size() + med_atr.size() - Aatr.size() - Batr.size();
        //最多アトラクション数のルート
    	if(atrcnt > optatr){
    		optatr_route.clear();
    		for(int i = 0; i < route.size(); i++){
    			optatr_route.push_back(route[i]);
    		}
    		optatr = atrcnt;
    	}
        //最多絶対行きたいアトラクション数のルート
        if(most_atr.size() - Aatr.size() > optAatr){
            optAatr_route.clear();
            for(int i = 0; i < route.size(); i++){
                optAatr_route.push_back(route[i]);
            }
            optAatr = most_atr.size() - Aatr.size();
        }
    }
	return;
}

void solve_large()
//nが大きい場合の実行
{
	set<P> sfp;
	int sfp_free = sttime;
	vector<P> sroute;
	sroute.push_back(P(prepos, 0));
	dfs(sttime, prepos, sfp, sfp_free, most_atr, med_atr, sroute);
}

void solve_small()
//nが小さい場合の実行
{
    vector<P> action;     //行動のリスト(アトラクション名,0:乗る,1:fp取る)
    map<int,int> atr_kind;  //アトラクションが絶対行きたいに属するか行ければ行きたいに属するか(0:絶対行きたい,1:行ければ行きたい)

    for(auto it = most_atr.begin(); it != most_atr.end(); it++){
        atr_kind[*it] = 0;
        if(fp_flag[*it]){
            action.push_back(P(*it, 1));   //ファストパス取る
        }
        action.push_back(P(*it, 0));
    }
    for(auto it = med_atr.begin(); it != med_atr.end(); it++){
        atr_kind[*it] = 1;
        if(fp_flag[*it]){
            action.push_back(P(*it, 1));   //ファストパス取る
        }
        action.push_back(P(*it, 0));
    }

    do{
        measure_check = system_clock::now();
        if(duration_cast<milliseconds>(measure_check - measure_start).count() > time_limit){
            break;
        }
        vector<P> route;        //今までのルート(アトラクション名,0:乗った,1:fp取った,2:fpを取るために待つ)
        int fp_free = 0;        //fpの制限が解除される時間
        int ptime = sttime;     //現在の時間
        int patr = prepos;      //現在いる場所
        int dsum = 0;           //今まで歩いた時間の総和
        set<int> ride_atr;      //すでに乗ったアトラクションの集合
        set<int> fp_atr;        //すでにfpを取ったアトラクションの集合
        map<int,int> fp_get_time;  //アトラクションのfpを取った時刻
        bool for_flag = false;          //for文を途中で抜けたかどうかのフラグ
        route.push_back(P(patr,0));

        for(int i = 0; i < action.size(); i++){
            //アトラクションに乗る行動
            if(action[i].second == 0){
                int buff1 = ptime;
                int buff2 = dsum;
                ptime += dtime[patr][action[i].first];
                dsum += dtime[patr][action[i].first];
                if(ptime >= endtime){
                    ptime = buff1;
                    dsum = buff2;
                    break;
                }
                patr = action[i].first;
                //fpを持ってるアトラクションについて
                if(fp_atr.find(patr) != fp_atr.end()){
                    //まだ乗車開始時刻になっていないとき
                    if(fptime_start[patr][fp_get_time[patr]] > ptime){
                        ptime = fptime_start[patr][fp_get_time[patr]] + fp_wtime + atime[patr];
                        if(ptime > endtime){
                            ptime = buff1;
                            dsum = buff2;
                            break;
                        }
                    //fp乗車期間が終わってしまっているとき
                    }else if(fptime_end[patr][fp_get_time[patr]] < ptime){
                        for_flag = true;
                        break;
                    //今がfpの乗車期間であるとき
                    }else{
                        ptime += fp_wtime + atime[patr];
                        if(ptime > endtime){
                            ptime = buff1;
                            dsum = buff2;
                            break;
                        }
                    }
                    ride_atr.insert(patr);
                    route.push_back(P(patr, 0));
                //fpを持っていないアトラクションについて
                }else{
                    ptime += wtime[patr][ptime] + atime[patr];
                    if(ptime > endtime){
                        ptime = buff1;
                        dsum = buff2;
                        break;
                    }
                    ride_atr.insert(patr);
                    route.push_back(P(patr, 0));
                }
            //fp取る行動
            }else{
                //すでに乗ったアトラクションについてファストパスを取る必要はない(fpが存在するアトラクションをfpなしで乗る場合に相当)
                if(ride_atr.find(action[i].first) != ride_atr.end()){
                    continue;
                }
                int buff1 = ptime;
                int buff2 = dsum;
                ptime += dtime[patr][action[i].first];
                dsum += dtime[patr][action[i].first];
                if(ptime >= endtime){
                    ptime = buff1;
                    dsum = buff2;
                    break;
                }
                patr = action[i].first;
                //fpを取る制限がかかっていないなら
                if(ptime >= fp_free){
                    //ファストパスが終了していたら
                    if(!fptime_start[patr][ptime]){
                        for_flag = true;
                        break;
                    }else{
                        //終了時刻を超えてしまうような時間のファストパスはとっても意味がない
                        if(endtime - atime[patr] < fptime_start[patr][ptime]){
                            for_flag = true;
                            break;
                        }else{
                            fp_atr.insert(patr);
                            fp_get_time[patr] = ptime;
                            fp_free = min(ptime + fp_itv, fptime_start[patr][ptime]);
                        }
                    }
                    route.push_back(P(patr, 1));
                //fpを取る制限がかかっていたら解除されるまで待つ
                }else{
                    if(fp_free >= endtime){
                        for_flag = true;
                        break;
                    }
                    //ファストパスが終了していたら
                    if(fptime_start[patr][fp_free]){
                        for_flag = true;
                        break;
                    }else{
                        //終了時刻を超えてしまうような時間のファストパスはとっても意味がない
                        if(endtime - atime[patr] < fptime_start[patr][fp_free]){
                            for_flag = true;
                            break;
                        }else{
                            ptime = fp_free;
                            fp_atr.insert(patr);
                            fp_get_time[patr] = ptime;
                            fp_free = min(ptime + fp_itv, fptime_start[patr][ptime]);
                        }
                    }
                }
            }
        }
        if(for_flag){
            continue;
        }

        //全てのアトラクションに乗ったなら
        if(ride_atr.size() == most_atr.size() + med_atr.size()){
            all_ride = true;
            //最短時間のルート
            if(ptime < opttime){
                opttime_route.clear();
                for(int i = 0;i < route.size(); i++){
                    opttime_route.push_back(route[i]);
                }
                opttime = ptime;
            }
            //最短移動時間のルート
            if(dsum < optdir){
                optdir_route.clear();
                for(int i = 0; i < route.size(); i++){
                    optdir_route.push_back(route[i]);
                }
                optdir = dsum;
            }
            double rate = (double)opttime / optdir;
            if((ptime-sttime) + rate * dsum < opttimedir){
                opttimedir_route.clear();
                for(int i = 0; i < route.size(); i++){
                    opttimedir_route.push_back(route[i]);
                }
                opttimedir = (ptime-sttime) + rate * dsum;
            }
        //全てのアトラクションに乗っていないなら
        }else{
            if(!all_ride){
                int Aatr_count = 0;
                int Batr_count = 0;
                for(auto it = ride_atr.begin(); it != ride_atr.end(); it++){
                    if(atr_kind[*it]){
                        Batr_count++;
                    }else{
                        Aatr_count++;
                    }
                }
                //最多アトラクション数のルート
                if(ride_atr.size() > optatr){
                    optatr_route.clear();
                    for(int i = 0; i < route.size(); i++){
                        optatr_route.push_back(route[i]);
                    }
                    optatr = ride_atr.size();
                }
                //最多絶対いきたいアトラクション数のルート
                if(Aatr_count > optAatr){
                    optAatr_route.clear();
                    for(int i = 0; i < route.size(); i++){
                        optAatr_route.push_back(route[i]);
                    }
                    optAatr = Aatr_count;
                }
            }
        }
    }while(next_permutation(action.begin(), action.end()));
}

picojson::value route_to_object_large(vector<P> route)
//largeの場合のルートをjson形式(picojson::value)に変換
{
    string t[9];
	t[0] = "flag",t[1] = "ID",t[2] = "move",t[3] = "arrive",t[4] = "wait";
	t[5] = "ride",t[6] = "duration",t[7] = "end",t[8] = "start";
	picojson::array ary;
	int temp = sttime;
	int free = 0;
	set<int> fpset;
    string buff = string_time(sttime);
	for(int i = 1; i < route.size(); i++){
        picojson::object obj;
        //アトラクションに乗る行動
        if(route[i].second == 0){
            obj.insert(make_pair(t[0], picojson::value(0.0)));
            obj.insert(make_pair(t[1], picojson::value((double)route[i].first)));
            obj.insert(make_pair(t[2], picojson::value(dtime[route[i-1].first][route[i].first] * (double)step_minute)));
            temp += dtime[route[i-1].first][route[i].first];
            obj.insert(make_pair(t[3], picojson::value(string_time(temp))));
            //ファストパスを使わず乗った
            if(fpset.find(route[i].first) == fpset.end()){
                obj.insert(make_pair(t[4], picojson::value(wtime[route[i].first][temp] * (double)step_minute)));
                temp += wtime[route[i].first][temp];
                obj.insert(make_pair(t[5], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[6], picojson::value(atime[route[i].first] * (double)step_minute)));
                temp += atime[route[i].first];
                obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[8], picojson::value(buff)));
                buff = string_time(temp);
            //ファストパスを使って乗った
            }else{
                fpset.erase(route[i].first);
                obj.insert(make_pair(t[4], picojson::value(fp_wtime * (double)step_minute)));
                temp += fp_wtime;
                obj.insert(make_pair(t[5], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[6], picojson::value(atime[route[i].first] * (double)step_minute)));
                temp += atime[route[i].first];
                obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[8], picojson::value(buff)));
                buff = string_time(temp);
            }
        //ファストパスを取る行動
        }else if(route[i].second == 1){
                obj.insert(make_pair(t[0], picojson::value(1.0)));
                obj.insert(make_pair(t[1], picojson::value((double)route[i].first)));
                obj.insert(make_pair(t[2], picojson::value(dtime[route[i-1].first][route[i].first] * (double)step_minute)));
                temp += dtime[route[i-1].first][route[i].first];
                obj.insert(make_pair(t[3], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[4], picojson::value(0.0)));
                obj.insert(make_pair(t[5], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[6], picojson::value(0.0)));
                obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[8], picojson::value(buff)));
                buff = string_time(temp);
                fpset.insert(route[i].first);
                free = min(temp+fp_itv,fptime_start[route[i].first][temp]);
        //ファストパス解禁まで待って取る行動
        }else{
			obj.insert(make_pair(t[0], picojson::value(1.0)));
            obj.insert(make_pair(t[1], picojson::value((double)route[i].first)));
			obj.insert(make_pair(t[2], picojson::value((free - temp) * (double)step_minute)));
			temp = free;
			obj.insert(make_pair(t[3], picojson::value(string_time(temp))));
			obj.insert(make_pair(t[4], picojson::value(0.0)));
			obj.insert(make_pair(t[5], picojson::value(string_time(temp))));
			obj.insert(make_pair(t[6], picojson::value(0.0)));
			obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
            obj.insert(make_pair(t[8], picojson::value(buff)));
            buff = string_time(temp);
			fpset.insert(route[i].first);
			free = min(temp + fp_itv, fptime_start[route[i].first][temp]);
	    }
		ary.push_back(picojson::value(obj));
	}
	return picojson::value(ary);
}

picojson::value route_to_object_small(vector<P> route)
//smallの場合のルートをjson形式(picojson::value)に変換
{
    string t[9];
	t[0] = "flag",t[1] = "ID",t[2] = "move",t[3] = "arrive",t[4] = "wait";
	t[5] = "ride",t[6] = "duration",t[7] = "end",t[8] = "start";
	picojson::array ary;
	int temp = sttime;
	int free = 0;
	set<int> fpset;
    map<int,int> fp_get_time;
    string buff = string_time(sttime);
    for(int i = 1; i < route.size(); i++){
        picojson::object obj;
        //アトラクションに乗る行動
        if(route[i].second == 0){
            obj.insert(make_pair(t[0], picojson::value(0.0)));
            obj.insert(make_pair(t[1], picojson::value((double)route[i].first)));
            obj.insert(make_pair(t[2], picojson::value(dtime[route[i-1].first][route[i].first] * (double)step_minute)));
            temp += dtime[route[i-1].first][route[i].first];
            //ファストパスを使わず乗った
            if(fpset.find(route[i].first) == fpset.end()){
                obj.insert(make_pair(t[3], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[4], picojson::value(wtime[route[i].first][temp] * (double)step_minute)));
                temp += wtime[route[i].first][temp];
                obj.insert(make_pair(t[5], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[6], picojson::value(atime[route[i].first] * (double)step_minute)));
                temp += atime[route[i].first];
                obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[8], picojson::value(buff)));
                buff = string_time(temp);
            //ファストパスを使って乗った
            }else{
                //到着した時にはまだファストパスの乗車開始時間に達していないとき
                if(fptime_start[route[i].first][fp_get_time[route[i].first]] > temp){
                    temp = fptime_start[route[i].first][fp_get_time[route[i].first]];
                }
                obj.insert(make_pair(t[3], picojson::value(string_time(temp))));
                fpset.erase(route[i].first);
                obj.insert(make_pair(t[4], picojson::value(fp_wtime * (double)step_minute)));
                temp += fp_wtime;
                obj.insert(make_pair(t[5], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[6], picojson::value(atime[route[i].first] * (double)step_minute)));
                temp += atime[route[i].first];
                obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
                obj.insert(make_pair(t[8], picojson::value(buff)));
                buff = string_time(temp);
            }
        //ファストパスを取る行動
        }else{
            obj.insert(make_pair(t[0], picojson::value(1.0)));
            obj.insert(make_pair(t[1], picojson::value((double)route[i].first)));
            if(free > temp + dtime[route[i-1].first][route[i].first]){
                obj.insert(make_pair(t[2], picojson::value((free - temp) * (double)step_minute)));
                temp = free;
            }else{
                obj.insert(make_pair(t[2], picojson::value(dtime[route[i-1].first][route[i].first] * (double)step_minute)));
                temp += dtime[route[i-1].first][route[i].first];
            }
            obj.insert(make_pair(t[3], picojson::value(string_time(temp))));
            obj.insert(make_pair(t[4], picojson::value(0.0)));
            obj.insert(make_pair(t[5], picojson::value("0")));
            obj.insert(make_pair(t[6], picojson::value(0.0)));
            obj.insert(make_pair(t[7], picojson::value(string_time(temp))));
            obj.insert(make_pair(t[8], picojson::value(buff)));
            buff = string_time(temp);
            fpset.insert(route[i].first);
            fp_get_time[route[i].first] = temp;
            free = min(temp + fp_itv, fptime_start[route[i].first][temp]);
        }
        ary.push_back(picojson::value(obj));
    }
    return picojson::value(ary);
}

void data_output()
//結果の出力
{
	string s[2];
	s[0] = "place",s[1] = "time";
    picojson::object obj_st;
    picojson::object obj_res;
    picojson::array cand_ary;
	obj_st.insert(make_pair(s[0], picojson::value((double)prepos)));	   //startのplace
	obj_st.insert(make_pair(s[1], picojson::value(string_time(sttime))));  //startのtime
    vector<picojson::value> cand_vec;
    vector<string> kind;
    //smallの場合
    if(small_check){
        if(all_ride){
            if(route_same(opttime_route,optdir_route)){
                cand_vec.push_back(route_to_object_small(opttime_route));
                kind.push_back("時間・移動距離重視");
            }else{
                cand_vec.push_back(route_to_object_small(opttime_route));
                cand_vec.push_back(route_to_object_small(optdir_route));
                if(!route_same(opttime_route,opttimedir_route) && !route_same(optdir_route,opttimedir_route)){
                    cand_vec.push_back(route_to_object_small(opttimedir_route));
                    kind.push_back("時間重視");
                    kind.push_back("移動距離重視");
                    kind.push_back("おすすめ");
                }else{
                    if(route_same(opttime_route,opttimedir_route)){
                        kind.push_back("時間重視(おすすめ)");
                        kind.push_back("移動距離重視");
                    }else{
                        kind.push_back("時間重視");
                        kind.push_back("移動距離重視(おすすめ)");
                    }
                }
            }
    	}else{
            if(route_same(optatr_route,optAatr_route)){
                cand_vec.push_back(route_to_object_small(optatr_route));
                kind.push_back("乗車回数・希望度重視");
            }else{
                cand_vec.push_back(route_to_object_small(optatr_route));
                cand_vec.push_back(route_to_object_small(optAatr_route));
                kind.push_back("乗車回数重視");
                kind.push_back("希望度重視");
            }
    	}
    //largeの場合
	}else{
        if(all_ride){
            if(route_same(opttime_route,optdir_route)){
                cand_vec.push_back(route_to_object_large(opttime_route));
                kind.push_back("時間・移動距離重視");
            }else{
                cand_vec.push_back(route_to_object_large(opttime_route));
                cand_vec.push_back(route_to_object_large(optdir_route));
                if(!route_same(opttime_route,opttimedir_route) && !route_same(optdir_route,opttimedir_route)){
                    cand_vec.push_back(route_to_object_large(opttimedir_route));
                    kind.push_back("時間重視");
                    kind.push_back("移動距離重視");
                    kind.push_back("おすすめ");
                }else{
                    if(route_same(opttime_route,opttimedir_route)){
                        kind.push_back("時間重視(おすすめ)");
                        kind.push_back("移動距離重視");
                    }else{
                        kind.push_back("時間重視");
                        kind.push_back("移動距離重視(おすすめ)");
                    }
                }
            }
        }else{
            if(route_same(optatr_route,optAatr_route)){
                cand_vec.push_back(route_to_object_large(optatr_route));
                kind.push_back("乗車回数重視");
            }else{
                cand_vec.push_back(route_to_object_large(optatr_route));
                cand_vec.push_back(route_to_object_large(optAatr_route));
                kind.push_back("乗車回数重視");
                kind.push_back("希望度重視");
            }
            cout << optatr << " " << optAatr << "\n";
    	}
    }

    for(int i = 0; i < cand_vec.size(); i++){
        picojson::object obj;
        obj.insert(make_pair("start", picojson::value(obj_st)));
        obj.insert(make_pair("attraction", cand_vec[i]));
        obj.insert(make_pair("discription", picojson::value(kind[i])));
        cand_ary.push_back(picojson::value(obj));
    }
    //cand_aryには候補の数だけ要素がある
    obj_res.insert(make_pair("candidates",picojson::value(cand_ary)));  //候補１つ分全部のary
    current_directory += "output/route_output.json";
    ofstream ofs(current_directory,ios::out);
	ofs << picojson::value(obj_res).serialize(true) << endl; // trueだと整形あり
	//printf("'開始'1:場所(ID)　2,3:時刻(?時?分)\n");
	//for(int i=0;i<opt_time_route;i++){

	//printf("'アトラクション'1.ID　2:移動時間(分)　3,4:到着時刻(?時?分)　5:待ち時間(分)　6,7:乗車時刻(?時?分)　8:所要時間(分)　9:終了時刻(?時?分)\n");
}

int main(int argc,char **argv)
{
    //スタート時間の測定
    measure_start = system_clock::now();
    current_directory = argv[1];
    date = argv[2];

    init();

    user_data_input();

    /*
    for(auto it = most_atr.begin();it != most_atr.end();it++){
        cout << *it << "\n";
    }
    cout << "\n";
    for(auto it = med_atr.begin(); it != med_atr.end();it++){
        cout << *it << "\n";
    }
    most_atr.clear();
    med_atr.clear();
    //userのinput(仮)
    int Aatr_count, Batr_count;
    scanf("%d%d", &Aatr_count, &Batr_count);
    for(int i = 0; i < Aatr_count; i++){
        int element;
        scanf("%d", &element);
        most_atr.insert(element);
    }
    for(int i = 0; i < Batr_count; i++){
        int element;
        scanf("%d", &element);
        med_atr.insert(element);
    }

    sttime = 6;
    endtime = 160;
    prepos = 0;
    */

    known_data_input();

    //アトラクションリストのうちのfpが存在するアトラクションの数
    int fp_atr_count = 0, not_fp_atr_count = 0;
    for(auto it = most_atr.begin(); it != most_atr.end(); it++){
        if(fp_flag[*it]){
            fp_atr_count++;
        }else{
            not_fp_atr_count++;
        }
    }
    for(auto it = med_atr.begin(); it != med_atr.end(); it++){
        if(fp_flag[*it]){
            fp_atr_count++;
        }else{
            not_fp_atr_count++;
        }
    }
    //2n + m <= 9 が全探索の限界
    //(n:ファストパスのアトラクション数,m:ファストパスのないアトラクション数)
    small_check = (2 * fp_atr_count + not_fp_atr_count <= 11);
    cout << fp_atr_count << " " << not_fp_atr_count << "\n";
    diverge_check1 = (fp_atr_count + not_fp_atr_count <= 15);
    diverge_check2 = (fp_atr_count + not_fp_atr_count <= 14);
    diverge_check3 = (fp_atr_count + not_fp_atr_count <= 12);
    /* //for debug
    for(int i = 0; i < num_atr; i++){
        cout << fp_flag[i] << " ";
    }
    cout << "\n";
    for(int i = 0; i < num_atr; i++){
        cout << atime[i] << " ";
    }
    cout << "\n";
    for(int i = 0; i < num_atr; i++){
        for(int j = 0; j < num_atr; j++){
            cout << dtime[i][j] << " ";
        }
        cout << "\n";
    }
    */

	predict_data_input();

    /*
    for(int i = 0; i < 37; i++){
        cout << i << " ";
        for(int j = 0; j < 140; j++){
            cout << wtime[i][j] << " ";
        }
        cout << "\n";
    }
    */

    /*
    for(int i = 0; i < num_atr; i++){
        cout << mtime[i] << " ";
    }

    for(int i = 0; i < num_atr; i++){
        for(int j = 0; j < 150; j++){
            cout << fptime_start[i][j] << " ";
        }
        cout << "\n";
    }
    cout << "\n";
    for(int i = 0; i < num_atr; i++){
        for(int j = 0; j < 150; j++){
            cout << fptime_end[i][j] << " ";
        }
        cout << "\n";
    }
    */
    if(small_check){
        solve_small();
    }else{
        solve_large();
    }
    /*
    cout << "1\n";
    for(int i = 0; i < opttime_route.size(); i++){
        cout << opttime_route[i].first << " " << opttime_route[i].second << "\n";
    }
    cout << "2\n";
    for(int i = 0; i < optdir_route.size(); i++){
        cout << optdir_route[i].first << " " << optdir_route[i].second << "\n";
    }
    cout << "3\n";
    for(int i = 0; i < optatr_route.size(); i++){
        cout << optatr_route[i].first << " " << optatr_route[i].second << "\n";
    }
    cout << "4\n";
    for(int i = 0; i < optAatr_route.size(); i++){
        cout << optAatr_route[i].first << " " << optAatr_route[i].second << "\n";
    }
    */
	data_output();
    //終了時間の測定
    measure_end = system_clock::now();
    cout << duration_cast<milliseconds>(measure_end-measure_start).count() << " milli sec \n";
	return 0;
}
