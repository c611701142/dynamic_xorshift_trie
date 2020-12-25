#ifndef OPEN_ADRESS__HASHING_HPP_
#define OPEN_ADRESS__HASHING_HPP_
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
static constexpr uint8_t MaxUint8_t = 0xFF;//255　文字コード最大
Xorshift(){
	pc_.resize(default_size);
	exists.resize(default_size,false);
    exists[0] = true;//0番目は使わない
}

private:
std::vector<int> B_ = {13,-7,5};//ビットシフトパターン
std::vector<int> B_1 = {5,-7,13};//ビットシフトパターン(32ビット)
int hash_use = 0;//配列P,C の使用数
int k = (std::log(default_size)/std::log(2)) + 8;//mask値の決定のため、P, C 拡張時にインクリメント
//対数でkを決める()

public:
struct DataItem {
    int p,c;
    DataItem(): p(invalid),c(invalid){}
};
std::vector<DataItem> pc_ ;//P,C配列
std::vector<bool> exists;//空判定配列

private:
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

void expand_resize(){
    std::cout << "before table" << pc_.size() << "  " << k << std::endl;
    //display();
	//出力値を遷移先候補とパリティ値から復元する
    std::vector<DataItem> pc_2(2*pc_.size());//P,C配列
    std::vector<bool> exists2(2*pc_.size());//空判定配列
    exists2[0] = true;//0番目は使わない
    int parent = 0;uint8_t c = 0;
    int replace = hash_use;
    for(int i = 1;i < pc_.size();i++){
        std::vector<uint8_t> codes;
        if(exists[i]){//使用要素
            int s = get_parent(i);
            c = get_charcode(i);
            k++;
            int new_t = 0;
            if(s == parent){//set
                uint64_t x1 = xos((s << 8) + c);//新しい出力値
                new_t = x1 >> 8;//新しい遷移先
                int collision = 0;
                while(exists2[new_t]){//使用済みならば再Xos
                    x1 = xos(x1);//出力値をxorに再代入
		            new_t = x1 >> 8;
                    collision++;   
                }
            int parity = x1 % 256;
            pc_2[new_t].p = parity;
            pc_2[new_t].c = collision;
            exists2[new_t] = true;
            parent = i;//10,13を親として、14,3,10番目の配列にアクセスしたい
            i = 1;//もう一度右から探索
            replace--;
            }//親から再配置(まずは0からの遷移)
            k--;//get_parentsでのxos,ixosのため、マスクをもとにもどす
        }
        if(i == pc_.size()-1 && replace != 0){//左端まで行ってまだ残ってたら
                exists[parent] = false; 
                i = 1;
                parent = 0;
        }
        //std::cout << replace << "\n";
        if(replace == 0){
            break;
        }
    }
    k++;//最終的に、マスク＋１に更新する
    pc_ = std::move(pc_2);
    exists = std::move(exists2);
    //display();
} 
int get_parent(int t){//子の状態番号→出力値→シード値→親番号
    uint64_t seed = get_seed(t);
    return seed >> 8;
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
}

//get関数ですが、今はそのまま返しているだけです
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
	return pc_[t].p;
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
	return pc_[t].c;
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
	return  t;
}

private:
uint64_t xos(uint64_t x)const{//前&x
    uint64_t maskXos_ = 1ull << k;
	for(int b: B_){//ビットシフトパターン{a,b}なら先にa次にbの２回ループ
        //std::cout << "shift patern , seed" << b << "," << x << "\n";
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
        expand_resize();
        int load_factor2 = hash_use*100/pc_.size();
        std::cout << "after_expand" << load_factor2 << "%" << std::endl;
    }
    uint64_t x1 = xos(seed);
    int t = x1 >> 8;//遷移先候補 8桁目以降  koko12
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
}

};

}
#endif //OPEN_ADRESS__HASHING_HP