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
static constexpr uint64_t default_size = std::pow(2,5);
Xorshift(){
	pc_.resize(default_size);
	exists.resize(default_size,false);
}

private:
std::vector<int> B_ = {13,-7,5};//ビットシフトパターン
int hash_use = 0;//配列P,C の使用数
int k = (std::log(default_size)/std::log(2))+8;//mask値の決定のため、P, C 拡張時にインクリメント
//対数でkを決める()

struct DataItem {
    int p,c;
    DataItem(): p(invalid),c(invalid){}
};
std::vector<DataItem> pc_ ;//P,C配列
public:
std::vector<bool> exists;//空判定配列

private:
void expand_resize(){
	//出力値を遷移先候補とパリティ値から復元する
    std::vector<DataItem> pc_2(2*pc_.size());//P,C配列
    std::vector<bool> exists2(2*pc_.size());//空判定配列
    k++;//マスク更新
    for(int i = 0;i < pc_.size();i++){
        if(exists[i]){//使用要素
            uint64_t t = i << 8;
            uint64_t t1 = t +  pc_[i].p;//出力値を復元

            pc_2[i].p = pc_[i].p;
            pc_2[i].c = pc_[i].c;
            exists2[i] = true;
            //std::cout << i << "    " << pc_[i].p << "\n" ;
            //std::cout << "x[i] = " << t1 << "\n" ;
        }
        else{}
    }
    pc_ = std::move(pc_2);
    exists = std::move(exists2);
} 
int node_count = 0;
public:
void display(){
    for(int i = 0; i < pc_.size();i++){
        //使用要素のみ表示
        if(exists[i]){
        node_count++;
        std::cout << i << "    " << exists[i] << "       ";
        std::cout << pc_[i].p << "  |  " << pc_[i].c << std::endl;
        //配列番号 
        }
    }
    //std::cout << "node_sum" << node_count << std::endl;
}

int get_parity(uint64_t x)const{
    uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先候補 マスクで上限値設定
	return pc_[t].p;
}

int get_collision(uint64_t x)const{
    uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先候補 マスクで上限値設定
	return  pc_[t].c;
}
int get_nextnode(uint64_t x)const{
	uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先候補 マスクで上限値設定
	return  t;
}

uint64_t xos(uint64_t x)const{//前&x
    uint64_t maskXos_ = (1 << k);
	for(int b: B_){//ビットシフトパターン{a,b}なら先にa次にbの２回ループ
        //std::cout << "shift patern , seed" << b << "," << x << "\n";
		if(b >= 0){
			x = (x ^ (x << b) % maskXos_);
		}
		else if(b < 0){
			x = (x ^ (x >> -b) % maskXos_);
        }
		//std::cout << "b =" << b << "\n";
	}
	//std::cout << " size(2^):" << k << "\n";
	//std::cout << " size:" << pc_.size() << "\n";
	return x;
}

//配列P,Cに要素を格納、衝突が起これば再配置
void set(uint64_t x){//引数 : シード値
	int load_factor = hash_use*100/pc_.size();
    std::cout << "before_expand" << load_factor << "%" << std::endl;

    //keyによる探索の期待計算量が、負荷率をqとしてO(1/(1-q))になる
    if(load_factor >= 50){
        expand_resize();
        int load_factor2 = hash_use*100/pc_.size();
        std::cout << "after_expand" << load_factor2 << "%" << std::endl;
    }
	uint64_t maskXos_ = (1 << k) -1;
    uint64_t x1 = xos(x);
    int t = x1 >> 8;//遷移先候補 8桁目以降
    int collision = 0;
	//std::cout << "2の" << k << "\n";
	//std::cout << "SIZE" << pc_.size() << "\n";
    while(exists[t]){//使用済みならば再Xos
        x1 = xos(x1);//出力値をxorに再代入
		t = x1 >> 8;
        collision++;        
        //std::cout << "re_xos" << "\n";
		//std::cout << t << "\n";
		//std::cout << exists[t] << "\n";
        //std::cout << "c : " << collision << std::endl;
    }
    int parity = x1 % 256;
    pc_[t].p = parity;
    pc_[t].c = collision;
    exists[t] = true;
	hash_use++;
    //std::cout << "output : " << x1 << std::endl;
    //std::cout << "t : " << t << std::endl;
    //std::cout << "p : " << parity << std::endl;
    //std::cout << "c : " << collision << std::endl;
    //std::cout << "insert- data"<< default_size << std::endl;                  
	//std::cout << "SIZE = "<< pc_.size() << "  2^" << k <<  std::endl;
    //std::cout << "SIZE : " << pc_.size() << std::endl;
    //std::cout << "k : " << k << std::endl;
    //std::cout << "defalt : " << default_size << std::endl;
	}



};

}
#endif //OPEN_ADRESS__HASHING_HP
