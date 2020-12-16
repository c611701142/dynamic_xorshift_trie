#include <iostream>
#include<vector>
#include<algorithm>
#include <math.h>
#include <sstream>
#include <cmath>
#include <limits>
#include <cassert>

static constexpr uint64_t default_size = std::pow(2,24);//(ここは24)
std::vector<int> B_ = {13,-7,5};//ビットシフトパターン(32ビット)
std::vector<int> B_1 = {5,-7,13};//ビットシフトパターン(32ビット)
int k = (std::log(default_size)/std::log(2)) + 7;//mask値の決定のため、P, C 拡張時にインクリメント

uint64_t xos(uint64_t x){//前&x
    uint64_t maskXos_ = (1 << k);
	for(int b: B_){//ビットシフトパターン{a,b}なら先にa次にbの２回ループ
        //std::cout << "shift patern , seed" << b << "," << x << "\n";
		if(b >= 0){
			x = (x ^ (x << b)) % maskXos_;
		}
		else if(b < 0){
			x = (x ^ (x >> -b)) % maskXos_;
        }
		std::cout << "xos b" << b << "\n";
		std::cout << "output :" << x << "\n";
	}
	return x;
}

int bit_len(uint32_t x) {
    assert(x > 0);
    int n = 0;
    while ((1<<n) <= x-1)
        ++n;
    return 1 << n;
}

int loop_time(int k,int b){
	double n1 = (std::log(k) - std::log(b))/std::log(2);
	double n = std::ceil(n1);
	int a = std::pow(2,n); 
	return a;
}

uint64_t ixos(uint64_t x){//前シード値から出力
    uint64_t maskXos_ = (1 << k);
	uint64_t z = 0;
	int n = 0;
	//xorの回数は2^ceil(log2(k/b))
	for(int b: B_1){
		/*	パターン１　そのまま
		if(b >= 0){
			n = loop_time(k,b);
		}
		else if(b < 0){
			n = loop_time(k,-b);
		}*/
		if(b >= 0){
			n = bit_len(k/b);
		}
		else if(b < 0){
			n = bit_len(k/-b);
		}

		std::cout << "roop time" << n << "\n";
		std::cout << "ixos b" << b << "\n";
		for(int i = 0;i < n;i++){
			if(b >= 0){
				x = (x ^ (x << b)) % maskXos_;
			}
			else if(b < 0){
				x = (x ^ (x >> -b)) % maskXos_;
			}
		}
		std::cout << "seed" << x << "\n";
	}
	return x;
} 

int main(){
    uint64_t seed = 98;
    uint64_t x = xos(seed);//シード値→出力値
	std::cout << "-----------------------"<< "\n";
    uint64_t s1 = ixos(x);//出力値→シード値
	if(s1 == seed){
		std::cout << "OK"<< "\n";
	}
	else{
		std::cout << "Failed"<< "\n";
	}
	int a = bit_len(4);
    return 0;
}


 