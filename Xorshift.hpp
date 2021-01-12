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
   virtual int set(int node,uint8_t c) = 0;
};

class Xorshift : public XorshiftInterface{
public:
static constexpr int null = -1;//データが入っていないことを示す値
static constexpr int invalid = -1;
static constexpr int invalid_key = -1;
static constexpr uint64_t default_size = 1<<4;
	
Xorshift(){
	pc_.resize(default_size);
	exists.resize(default_size,false);
    //exists[0] = true;//0番目は使わない
}

private:
std::vector<int> B_ = {13,-7,5};//ビットシフトパターン
std::vector<int> B_1 = {5,-7,13};//ビットシフトパターン(逆関数用)
//先行研究のビットシフトパターン
//{26,-5,6}{7,-30,1}{7,-6}


uint64_t hash_use = 0;//配列P,C の使用数
int k = (std::log(default_size)/std::log(2)) + 8;//mask値の決定のため、P, C 拡張時にインクリメント

public:
struct DataItem {
    int p,c;
    DataItem(): p(invalid),c(invalid){}
};
std::vector<DataItem> pc_ ;//P,C配列
std::vector<bool> exists;//空判定配列
//再配置が起こるタイミングの番号を保存、シード値を更新する
//setとexpandで共有する配列 "place[旧index] = 新index"

private:
int expand(int node){
    //std::cout << "before table" << pc_.size() << "  " << k << std::endl;
    //display();
	//出力値を遷移先候補とパリティ値から復元する
    std::vector<DataItem> pc_2(2*pc_.size());//P,C配列
    std::vector<bool> exists2(2*pc_.size());//空判定配列
    std::vector<int> place(pc_.size(),-1);
    place[0] = 0;
    exists2[0] = true;//0番目は使わない
    int s = 0;uint8_t c = 0;
    int new_t = 0;//成長後の配列の要素の番号
   // std::cout << " re_place start " <<  std::endl;
    for(int i = 1;i < pc_.size();i++){
        if (!exists[i] or replace(i,place,pc_2,exists2) != -1){
            //std::cout << i << "access point" << std::endl;
            continue;
        }
    }
    k++;//最終的に、マスク＋１に更新する
    pc_ = std::move(pc_2);
    exists = std::move(exists2);
    //display();
    //std::cout <<  "--------------------------------------------------------"  << std::endl;
    //for(int i = 0;i < place.size();i++){
    //    std::cout << place[i] <<  "    "  << i << std::endl;
    // }
    //std::cout <<  "--------------------------------------------------------"  << std::endl;
    return place[node];
} 

int replace(int node,std::vector<int>& place,std::vector<DataItem>& pc_2,std::vector<bool>& exists2){
    if (place[node] != -1){//再配置済みの時、無視
        return -1;
    }
    uint64_t seed = get_seed(node);
    uint8_t c = seed % 256;
    int parent = seed >> 8;
    if (place[parent] == -1){//前のトライ上で、再配置が終わっていないとき
        replace(parent,place,pc_2,exists2);
    }    
    seed = (place[parent] << 8 ) + c;

    k++;//mask更新
    uint64_t x1 = xos(seed);
    int new_node = x1 >> 8;
    int collision = 0;
    while(exists2[new_node]){//使用済みならば再Xos
        x1 = xos(x1);//出力値をxorに再代入s
        new_node = x1 >> 8;
        collision++;
    }
    int parity = x1 % 256;
    pc_2[new_node].p = parity;
    pc_2[new_node].c = collision;
    exists2[new_node] = true;
    place[node] = new_node;
    k--;
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
    //std::cout << t << " 　　 " << seed << std::endl;
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
    for(uint64_t i = 1; i < pc_.size();i++){
        //使用要素のみ表示
        if(exists[i]){
            if(collision_max < pc_[i].c){
                collision_max = pc_[i].c;
            }
            //std::cout << i << "    " << exists[i] << "       ";
            //std::cout << pc_[i].p << "  |  " << pc_[i].c << "  " << get_charcode(i) << std::endl;
            //配列番号
        }
    }
    //std::cout << "collision_max" << collision_max << std::endl;
    //std::cout << "mask :" << k << std::endl;
}

int get_parity(uint64_t x)const override{//引数シード値
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

int get_collision(uint64_t x)const override{//引数シード値
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

int create_seed(int node, uint8_t c)const{
    return (node << 8) + c;
}


public:
//配列P,Cに要素を格納、衝突が起これば再配置
int set(int node,uint8_t c){//引数 : シード値
	uint64_t load_factor = hash_use*100/pc_.size();
    //std::cout << load_factor <<  " % " << std::endl;
    //keyによる探索の期待計算量が、負荷率をqとしてO(1/(1-q))になる
    if(load_factor >= 50){
        node = expand(node);
        //std::cout << (seed >> 8) <<  "    " << std::endl;
        uint64_t load_factor2 = hash_use*100/pc_.size();
    }
    uint64_t seed = create_seed(node,c);
    std::cout << node <<  "    " << c << std::endl;
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
    return node;
}

};

}
