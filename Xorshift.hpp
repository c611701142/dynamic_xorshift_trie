#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include<algorithm>

namespace kuroda {

class XorshiftInterface {
public:
   virtual int get_parity(uint64_t x)const = 0;
   virtual int get_collision(uint64_t x)const = 0;
   virtual void set(uint64_t x) = 0;
};

class Xorshift : public XorshiftInterface{
public:
static constexpr int null = -1;//データが入っていないことを示す値
static constexpr int invalid = -1;
static constexpr int invalid_key = -1;
static constexpr uint64_t default_size = std::pow(2,4);
	
Xorshift(){
	pc_.resize(default_size);
	exists.resize(default_size,false);
    exists[0] = true;//0番目は使わない
    place.clear();
}

private:
std::vector<int> B_ = {13,-7,5};//ビットシフトパターン
std::vector<int> B_1 = {5,-7,13};//ビットシフトパターン(逆関数用)
int hash_use = 0;//配列P,C の使用数
int k = (std::log(default_size)/std::log(2)) + 8;//mask値の決定のため、P, C 拡張時にインクリメント

public:
struct DataItem {
    int p,c;
    DataItem(): p(invalid),c(invalid){}
};
std::vector<DataItem> pc_ ;//P,C配列
std::vector<bool> exists;//空判定配列
std::vector<int> place;
//setとexpandで共有する配列 "place[旧index] = 新index"

private:
void expand(){
    std::cout << "before table" << pc_.size() << "  " << k << std::endl;
    display();
	//出力値を遷移先候補とパリティ値から復元する
    std::vector<DataItem> pc_2(2*pc_.size());//P,C配列
    std::vector<bool> exists2(2*pc_.size());//空判定配列
    exists2[0] = true;//0番目は使わない
    int s = 0;uint8_t c = 0;
    int new_t = 0;//成長後の配列の要素の番号
    for(int i = 1;i < pc_.size();i++){
        if(exists[i]){//使用要素
            if(replace(i) == -1){//親が再配置されていない
                //std::cout << " 再配置開始 " << place.size() <<  std::endl;
                //std::cout << i << "      " << get_charcode(i) << std::endl;
                //std::cout << i << "格納します" << std::endl;
                for(int j = 0;j < place.size() ;j++){//-1になった経路を再配置
                    //std::cout << " jjjjj " << place[j] << std::endl;
                    s = place[j];c = get_charcode(s);
                    k++;//mask更新
                    uint64_t x1 = xos((s << 8) + c);//新しい出力値
                    new_t = x1 >> 8;//新しい遷移先
                    int collision = 0;
                    while(exists2[new_t]){//使用済みならば再Xos
                        x1 = xos(x1);//出力値をxorに再代入s
                        new_t = x1 >> 8;
                        collision++;   
                    }
                    int parity = x1 % 256;
                    pc_2[new_t].p = parity;
                    pc_2[new_t].c = collision;
                    exists2[new_t] = true;
                    exists[s] = false;
                    k--;//get_parentsでのxos,ixosのため、マスクを拡張前にもどす
                    std::cout << "   change_char   " << c << std::endl;
                    std::cout << "   change_number   " << s << std::endl;
                    std::cout << " new " << new_t << std::endl;
                }
                place.clear();
                std::cout << place.size() << std::endl;
            }//親から再配置(まずは0からの遷移)
        }//使用要素
    }
    k++;//最終的に、マスク＋１に更新する
    pc_ = std::move(pc_2);
    exists = std::move(exists2);
    std::cout << "after_expand" <<  pc_.size() << "  " << k << std::endl;
    display();
} 

//再配置で使う再帰関数
int replace(int node){
    std::cout << "調べ中" << node << std::endl;
    if(node == 0 || !exists[node]) return -1;//nodeは配置済み
    else if(node != 0 && exists[node]){
        place.insert(place.begin(),node);
        std::cout << " parent =  " << node << std::endl;
        node = get_parent(node);
        return replace(node);
    }
}

int get_seed(int t)const{//配列番号、パリティ値、衝突回数からシード値を得る
    uint64_t x = 0;
    if(pc_[t].c == 0){
        x = (t << 8) + pc_[t].p;
    }
    else{
        x = (t << 8) + pc_[t].p;
        for(int i = 0;i < pc_[t].c;i++){//衝突回数回ixosに代入することで、元のシード値をえる
            x = ixos(x);//仮の出力値を得る(再xosする前の)
        }
    }
    uint64_t seed = ixos(x);
    return seed;
}//ここから、親と遷移文字が分かる

int get_parent(int t){//子の状態番号→出力値→シード値→親番号
    if(t == 0){
        return -1;
    }
    if(exists[t] == false){
        return -1;
    }
    else{
    uint64_t seed = get_seed(t);
        return seed >> 8;
    }
}
uint8_t get_charcode(int t){//子の状態番号→出力値→シード値→文字コード
    uint64_t seed = get_seed(t);
    return seed % 256;
}


public:
void display(){
    int collision_max = 0;
    for(int i = 1; i < pc_.size();i++){
        //使用要素のみ表示
        if(exists[i]){
            if(collision_max < pc_[i].c){
                collision_max = pc_[i].c;
            }
        std::cout << i << "    " << exists[i] << "       ";
        std::cout << pc_[i].p << "  |  " << pc_[i].c << std::endl;
        //配列番号 
        }
    }
    std::cout << "collision_max" << collision_max << std::endl;
    std::cout << "mask :" << k << std::endl;
}

int get_parity(uint64_t x)const{//引数シード値
    uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先
    int parity = x1 % 256;
    while(exists[t]){//使用済みならば再Xos
        if(pc_[t].p == parity){
            return pc_[t].p;
        }
        x1 = xos(x1);//
        t = x1 >> 8;//遷移先
        parity = x1 % 256;
    }
	return -1;
}

int get_collision(uint64_t x)const{//引数シード値
    uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先
    int parity = x1 % 256;
    while(exists[t]){//使用済みならば再Xos
        if(pc_[t].p == parity){
            return pc_[t].c;
        }
        x1 = xos(x1);//
        t = x1 >> 8;//遷移先
        parity = x1 % 256;
    }
	return -1;
}
int get_nextnode(uint64_t x)const{//引数シード値
	uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先
    int parity = x1 % 256;
    while(exists[t]){//使用済みならば再Xos
        if(pc_[t].p == parity){
            return t;
        }
        x1 = xos(x1);//
        t = x1 >> 8;//遷移先
        parity = x1 % 256;
    }
	return -1;
}

private:
uint64_t xos(uint64_t x)const{//前&x
    uint64_t maskXos_ = 1ull << k;
	for(int b: B_){
		if(b >= 0){
			x = (x ^ (x << b)) % maskXos_;
		}
		else if(b < 0){
			x = (x ^ (x >> -b)) % maskXos_;
        }
	}
	return x;
}

int bit_len(uint32_t x)const{
    assert(x > 0);
    int n = 0;
    while ((1ull<<n) <= x-1)
        ++n;
    return n;
}

int loop_time(int k,int b)const{
	assert(k > 0 and b > 0);
    return 1 << bit_len((k-1) / b + 1);
}

uint64_t ixos(uint64_t x)const{//前シード値から出力
    uint64_t maskXos_ = 1ull << k;
	//xorの回数は2^ceil(log2(k/b))
	for(int b: B_1){
		int n = loop_time(k, abs(b)) - 1;
		for(int i = 0;i < n;i++){
			if(b >= 0){
				x = (x ^ (x << b)) % maskXos_;
			}
			else if(b < 0){
				x = (x ^ (x >> -b)) % maskXos_;
			}
		}
	}
	return x;
}

public:
//配列P,Cに要素を格納、衝突が起これば再配置
void set(uint64_t seed){//引数 : シード値
	int load_factor = hash_use*100/pc_.size();
    //keyによる探索の期待計算量が、負荷率をqとしてO(1/(1-q))になる
    if(load_factor >= 50){
        expand();
        int load_factor2 = hash_use*100/pc_.size();
    }
    uint64_t x1 = xos(seed);
    int t = x1 >> 8;//遷移先候補 8桁目以降 
    int collision = 0;
    while(exists[t]){//使用済みならば再Xos
        x1 = xos(x1);//出力値をxorに再代入
		t = x1 >> 8;
        collision++;        
    }
    int parity = x1 % 256;
    pc_[t].p = parity;
    pc_[t].c = collision;
    exists[t] = true;
	hash_use++;
    //place.emplace_back(t);
    std::cout << t << "配置" << get_charcode(t) << std::endl;
}

};

}
